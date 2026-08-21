#include <turbo/unicode/engine/lasx.h>
#include <turbo/unicode/engine/implementation.h>
#if UNICODE_IMPLEMENTATION_LASX

#include <turbo/unicode/tables/utf8_to_utf16_tables.h>
#include <turbo/unicode/tables/utf16_to_utf8_tables.h>
#include <turbo/unicode/tables/utf32_to_utf16_tables.h>


#include <turbo/unicode/engine/lasx/begin.h>
namespace turbo {
    namespace UNICODE_IMPLEMENTATION {
        namespace {
#ifndef UNICODE_LASX_H
#error "lasx.h must be included"
#endif
            using namespace simd;

            // convert vmskltz/vmskgez/vmsknz to
            // turbo::tables::utf16_to_utf8::pack_1_2_utf8_bytes index
            const uint8_t lasx_1_2_utf8_bytes_mask[] = {
                0, 1, 4, 5, 16, 17, 20, 21, 64, 65, 68, 69, 80, 81, 84,
                85, 2, 3, 6, 7, 18, 19, 22, 23, 66, 67, 70, 71, 82, 83,
                86, 87, 8, 9, 12, 13, 24, 25, 28, 29, 72, 73, 76, 77, 88,
                89, 92, 93, 10, 11, 14, 15, 26, 27, 30, 31, 74, 75, 78, 79,
                90, 91, 94, 95, 32, 33, 36, 37, 48, 49, 52, 53, 96, 97, 100,
                101, 112, 113, 116, 117, 34, 35, 38, 39, 50, 51, 54, 55, 98, 99,
                102, 103, 114, 115, 118, 119, 40, 41, 44, 45, 56, 57, 60, 61, 104,
                105, 108, 109, 120, 121, 124, 125, 42, 43, 46, 47, 58, 59, 62, 63,
                106, 107, 110, 111, 122, 123, 126, 127, 128, 129, 132, 133, 144, 145, 148,
                149, 192, 193, 196, 197, 208, 209, 212, 213, 130, 131, 134, 135, 146, 147,
                150, 151, 194, 195, 198, 199, 210, 211, 214, 215, 136, 137, 140, 141, 152,
                153, 156, 157, 200, 201, 204, 205, 216, 217, 220, 221, 138, 139, 142, 143,
                154, 155, 158, 159, 202, 203, 206, 207, 218, 219, 222, 223, 160, 161, 164,
                165, 176, 177, 180, 181, 224, 225, 228, 229, 240, 241, 244, 245, 162, 163,
                166, 167, 178, 179, 182, 183, 226, 227, 230, 231, 242, 243, 246, 247, 168,
                169, 172, 173, 184, 185, 188, 189, 232, 233, 236, 237, 248, 249, 252, 253,
                170, 171, 174, 175, 186, 187, 190, 191, 234, 235, 238, 239, 250, 251, 254,
                255
            };

            KUMO_FORCE_INLINE __m128i lsx_swap_bytes(__m128i vec) {
                return __lsx_vshuf4i_b(vec, 0b10110001);
            }
            KUMO_FORCE_INLINE __m256i lasx_swap_bytes(__m256i vec) {
                return __lasx_xvshuf4i_b(vec, 0b10110001);
            }

            KUMO_FORCE_INLINE bool is_ascii(const simd8x64<uint8_t>& input) {
                return input.is_ascii();
            }


            KUMO_FORCE_INLINE simd8<bool>
            must_be_2_3_continuation(const simd8<uint8_t> prev2,
                const simd8<uint8_t> prev3) {
                simd8<bool> is_third_byte = prev2 >= uint8_t(0b11100000u);
                simd8<bool> is_fourth_byte = prev3 >= uint8_t(0b11110000u);
                return is_third_byte ^ is_fourth_byte;
            }

            // common functions for utf8 conversions
            KUMO_FORCE_INLINE __m128i convert_utf8_3_byte_to_utf16(__m128i in) {
                // Low half contains  10bbbbbb|10cccccc
                // High half contains 1110aaaa|1110aaaa
                const v16u8 sh = { 2, 1, 5, 4, 8, 7, 11, 10, 0, 0, 3, 3, 6, 6, 9, 9 };
                const v8u16 v0fff = { 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff };

                __m128i perm = __lsx_vshuf_b(__lsx_vldi(0), in, (__m128i)sh);
                // 1110aaaa => aaaa0000
                __m128i perm_high = __lsx_vslli_b(__lsx_vbsrl_v(perm, 8), 4);
                // 10bbbbbb 10cccccc => 0010bbbb bbcccccc
                __m128i composed = __lsx_vbitsel_v(__lsx_vsrli_h(perm, 2), /* perm >> 2*/
                    perm, __lsx_vrepli_h(0x3f) /* 0x003f */);
                // 0010bbbb bbcccccc => aaaabbbb bbcccccc
                composed = __lsx_vbitsel_v(perm_high, composed, (__m128i)v0fff);

                return composed;
            }

            KUMO_FORCE_INLINE __m128i convert_utf8_2_byte_to_utf16(__m128i in) {
                // 10bbbbb 110aaaaa => 00bbbbb 000aaaaa
                __m128i composed = __lsx_vand_v(in, __lsx_vldi(0x3f));
                // 00bbbbbb 000aaaaa => 00000aaa aabbbbbb
                composed = __lsx_vbitsel_v(
                    __lsx_vsrli_h(__lsx_vslli_h(composed, 8), 2), /* (aaaaa << 8) >> 2 */
                    __lsx_vsrli_h(composed, 8), /* bbbbbb >> 8 */
                    __lsx_vrepli_h(0x3f)); /* 0x003f */
                return composed;
            }

            KUMO_FORCE_INLINE __m128i
            convert_utf8_1_to_2_byte_to_utf16(__m128i in, size_t shufutf8_idx) {
                // Converts 6 1-2 byte UTF-8 characters to 6 UTF-16 characters.
                // This is a relatively easy scenario
                // we process SIX (6) input code-code units. The max length in bytes of six
                // code code units spanning between 1 and 2 bytes each is 12 bytes.
                __m128i sh = __lsx_vld(reinterpret_cast<const uint8_t*>(
                                           turbo::tables::utf8_to_utf16::shufutf8[shufutf8_idx]),
                    0);
                // Shuffle
                // 1 byte: 00000000 0bbbbbbb
                // 2 byte: 110aaaaa 10bbbbbb
                __m128i perm = __lsx_vshuf_b(__lsx_vldi(0), in, sh);
                // 1 byte: 00000000 0bbbbbbb
                // 2 byte: 00000000 00bbbbbb
                __m128i ascii = __lsx_vand_v(perm, __lsx_vrepli_h(0x7f)); // 6 or 7 bits
                // 1 byte: 00000000 00000000
                // 2 byte: 00000aaa aa000000
                __m128i v1f00 = lsx_splat_u16(0x1f00);
                __m128i composed = __lsx_vsrli_h(__lsx_vand_v(perm, v1f00), 2); // 5 bits
                // Combine with a shift right accumulate
                // 1 byte: 00000000 0bbbbbbb
                // 2 byte: 00000aaa aabbbbbb
                composed = __lsx_vadd_h(ascii, composed);
                return composed;
            }

#include <turbo/unicode/engine/lasx/lasx_validate_utf16.cpp>
#include <turbo/unicode/engine/lasx/lasx_validate_utf32le.cpp>

#include <turbo/unicode/engine/lasx/lasx_convert_latin1_to_utf8.cpp>
#include <turbo/unicode/engine/lasx/lasx_convert_latin1_to_utf16.cpp>
#include <turbo/unicode/engine/lasx/lasx_convert_latin1_to_utf32.cpp>

#include <turbo/unicode/engine/lasx/lasx_convert_utf8_to_utf16.cpp>
#include <turbo/unicode/engine/lasx/lasx_convert_utf8_to_utf32.cpp>
#include <turbo/unicode/engine/lasx/lasx_convert_utf8_to_latin1.cpp>

#include <turbo/unicode/engine/lasx/lasx_convert_utf16_to_latin1.cpp>
#include <turbo/unicode/engine/lasx/lasx_convert_utf16_to_utf8.cpp>
#include <turbo/unicode/engine/lasx/lasx_convert_utf16_to_utf32.cpp>

#include <turbo/unicode/engine/lasx/lasx_convert_utf32_to_latin1.cpp>
#include <turbo/unicode/engine/lasx/lasx_convert_utf32_to_utf8.cpp>
#include <turbo/unicode/engine/lasx/lasx_convert_utf32_to_utf16.cpp>
#include <turbo/unicode/engine/lasx/lasx_base64.cpp>
#include <turbo/unicode/engine/lasx/lasx_find.cpp>

        } // namespace
    } // namespace UNICODE_IMPLEMENTATION
} // namespace turbo

#include <turbo/unicode/generic/buf_block_reader.h>
#include <turbo/unicode/generic/utf8_validation/utf8_lookup4_algorithm.h>
#include <turbo/unicode/generic/utf8_validation/utf8_validator.h>
#include <turbo/unicode/generic/ascii_validation.h>

  // transcoding from UTF-8 to Latin 1
#include <turbo/unicode/generic/utf8_to_latin1/utf8_to_latin1.h>
#include <turbo/unicode/generic/utf8_to_latin1/valid_utf8_to_latin1.h>
  // transcoding from UTF-8 to UTF-16
#include <turbo/unicode/generic/utf8_to_utf16/valid_utf8_to_utf16.h>
#include <turbo/unicode/generic/utf8_to_utf16/utf8_to_utf16.h>
#include <turbo/unicode/generic/utf8/utf16_length_from_utf8_bytemask.h>
  // transcoding from UTF-16 to UTF-8
#include <turbo/unicode/generic/utf16_to_utf8/utf16_to_utf8_with_replacement.h>
  // transcoding from UTF-8 to UTF-32
#include <turbo/unicode/generic/utf8_to_utf32/valid_utf8_to_utf32.h>
#include <turbo/unicode/generic/utf8_to_utf32/utf8_to_utf32.h>

#include <turbo/unicode/generic/utf8.h>

#include <turbo/unicode/generic/utf16/count_code_points_bytemask.h>
#include <turbo/unicode/generic/utf16/change_endianness.h>
#include <turbo/unicode/generic/utf16/utf8_length_from_utf16_bytemask.h>
#include <turbo/unicode/generic/utf16/utf32_length_from_utf16.h>
#include <turbo/unicode/generic/utf16/to_well_formed.h>

#include <turbo/unicode/generic/validate_utf16.h>

#include <turbo/unicode/generic/utf32.h>
#include <turbo/unicode/generic/base64lengths.h>

//
// Implementation-specific overrides
//
namespace turbo {
    namespace UNICODE_IMPLEMENTATION {

         [[nodiscard]] int
        UnicodeImplementLasx::detect_encodings(const char* input,
            size_t length) const noexcept {
            // If there is a BOM, then we trust it.
            auto bom_encoding = turbo::BOM::check_bom(input, length);
            // todo: reimplement as a one-pass algorithm.
            if (bom_encoding != TextEncoding::unspecified) {
                return bom_encoding;
            }
            int out = 0;
            if (validate_utf8(input, length)) {
                out |= TextEncoding::UTF8;
            }
            if ((length % 2) == 0) {
                if (validate_utf16le(reinterpret_cast<const char16_t*>(input),
                        length / 2)) {
                    out |= TextEncoding::UTF16_LE;
                }
            }
            if ((length % 4) == 0) {
                if (validate_utf32(reinterpret_cast<const char32_t*>(input), length / 4)) {
                    out |= TextEncoding::UTF32_LE;
                }
            }
            return out;
        }

         [[nodiscard]] bool
        UnicodeImplementLasx::validate_utf8(const char* buf, size_t len) const noexcept {
            return lasx::utf8_validation::generic_validate_utf8(buf, len);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementLasx::validate_utf8_with_errors(
            const char* buf, size_t len) const noexcept {
            return lasx::utf8_validation::generic_validate_utf8_with_errors(buf, len);
        }

         [[nodiscard]] bool
        UnicodeImplementLasx::validate_ascii(const char* buf, size_t len) const noexcept {
            return lasx::ascii_validation::generic_validate_ascii(buf, len);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementLasx::validate_ascii_with_errors(
            const char* buf, size_t len) const noexcept {
            return lasx::ascii_validation::generic_validate_ascii_with_errors(buf, len);
        }
         [[nodiscard]] bool
        UnicodeImplementLasx::validate_utf16le_as_ascii(const char16_t* buf,
            size_t len) const noexcept {
            return lasx::utf16::validate_utf16_as_ascii_with_errors<Endian::little>(
                       buf, len)
                       .error
                == SUCCESS;
        }

         [[nodiscard]] bool
        UnicodeImplementLasx::validate_utf16be_as_ascii(const char16_t* buf,
            size_t len) const noexcept {
            return lasx::utf16::validate_utf16_as_ascii_with_errors<Endian::big>(buf,
                       len)
                       .error
                == SUCCESS;
        }
         [[nodiscard]] bool
        UnicodeImplementLasx::validate_utf16le(const char16_t* buf,
            size_t len) const noexcept {
            if (KUMO_UNLIKELY(len == 0)) {
                // empty input is valid. protected the implementation from nullptr.
                return true;
            }
            const auto res = lasx::utf16::validate_utf16_with_errors<Endian::little>(buf, len);
            if (res.is_err()) {
                return false;
            }

            if (res.count != len) {
                return scalar::utf16::validate<Endian::little>(buf + res.count,
                    len - res.count);
            }

            return true;
        }

         [[nodiscard]] bool
        UnicodeImplementLasx::validate_utf16be(const char16_t* buf,
            size_t len) const noexcept {
            if (KUMO_UNLIKELY(len == 0)) {
                // empty input is valid. protected the implementation from nullptr.
                return true;
            }

            const auto res = lasx::utf16::validate_utf16_with_errors<Endian::big>(buf, len);
            if (res.is_err()) {
                return false;
            }

            if (res.count != len) {
                return scalar::utf16::validate<Endian::big>(buf + res.count,
                    len - res.count);
            }

            return true;
        }

         [[nodiscard]] UnicodeResult UnicodeImplementLasx::validate_utf16le_with_errors(
            const char16_t* buf, size_t len) const noexcept {
            if (KUMO_UNLIKELY(len == 0)) {
                return UnicodeResult(UnicodeError::SUCCESS, 0);
            }
            const UnicodeResult res = lasx::utf16::validate_utf16_with_errors<Endian::little>(buf, len);
            if (res.count != len) {
                const UnicodeResult scalar_res = scalar::utf16::validate_with_errors<Endian::little>(
                    buf + res.count, len - res.count);
                return UnicodeResult(scalar_res.error, res.count + scalar_res.count);
            } else {
                return res;
            }
        }

         [[nodiscard]] UnicodeResult UnicodeImplementLasx::validate_utf16be_with_errors(
            const char16_t* buf, size_t len) const noexcept {
            if (KUMO_UNLIKELY(len == 0)) {
                return UnicodeResult(UnicodeError::SUCCESS, 0);
            }
            const UnicodeResult res = lasx::utf16::validate_utf16_with_errors<Endian::big>(buf, len);
            if (res.count != len) {
                const UnicodeResult scalar_res = scalar::utf16::validate_with_errors<Endian::big>(buf + res.count,
                    len - res.count);
                return UnicodeResult(scalar_res.error, res.count + scalar_res.count);
            } else {
                return res;
            }
        }

        void UnicodeImplementLasx::to_well_formed_utf16le(const char16_t* input, size_t len,
            char16_t* output) const noexcept {
            return utf16::to_well_formed<Endian::little>(input, len, output);
        }

        void UnicodeImplementLasx::to_well_formed_utf16be(const char16_t* input, size_t len,
            char16_t* output) const noexcept {
            return utf16::to_well_formed<Endian::big>(input, len, output);
        }

         [[nodiscard]] bool
        UnicodeImplementLasx::validate_utf32(const char32_t* buf, size_t len) const noexcept {
            if (KUMO_UNLIKELY(len == 0)) {
                // empty input is valid. protected the implementation from nullptr.
                return true;
            }
            const char32_t* tail = lasx_validate_utf32le(buf, len);
            if (tail) {
                return scalar::utf32::validate(tail, len - (tail - buf));
            } else {
                return false;
            }
        }

         [[nodiscard]] UnicodeResult UnicodeImplementLasx::validate_utf32_with_errors(
            const char32_t* buf, size_t len) const noexcept {
            if (KUMO_UNLIKELY(len == 0)) {
                return UnicodeResult(UnicodeError::SUCCESS, 0);
            }
            UnicodeResult res = lasx_validate_utf32le_with_errors(buf, len);
            if (res.count != len) {
                UnicodeResult scalar_res = scalar::utf32::validate_with_errors(buf + res.count, len - res.count);
                return UnicodeResult(scalar_res.error, res.count + scalar_res.count);
            } else {
                return res;
            }
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_latin1_to_utf8(
            const char* buf, size_t len, char* utf8_output) const noexcept {
            std::pair<const char*, char*> ret = lasx_convert_latin1_to_utf8(buf, len, utf8_output);
            size_t converted_chars = ret.second - utf8_output;

            if (ret.first != buf + len) {
                const size_t scalar_converted_chars = scalar::latin1_to_utf8::convert(
                    ret.first, len - (ret.first - buf), ret.second);
                converted_chars += scalar_converted_chars;
            }
            return converted_chars;
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_latin1_to_utf16le(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            std::pair<const char*, char16_t*> ret = lasx_convert_latin1_to_utf16le(buf, len, utf16_output);
            size_t converted_chars = ret.second - utf16_output;
            if (ret.first != buf + len) {
                const size_t scalar_converted_chars = scalar::latin1_to_utf16::convert<Endian::little>(
                    ret.first, len - (ret.first - buf), ret.second);
                converted_chars += scalar_converted_chars;
            }
            return converted_chars;
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_latin1_to_utf16be(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            std::pair<const char*, char16_t*> ret = lasx_convert_latin1_to_utf16be(buf, len, utf16_output);
            size_t converted_chars = ret.second - utf16_output;
            if (ret.first != buf + len) {
                const size_t scalar_converted_chars = scalar::latin1_to_utf16::convert<Endian::big>(
                    ret.first, len - (ret.first - buf), ret.second);
                converted_chars += scalar_converted_chars;
            }
            return converted_chars;
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_latin1_to_utf32(
            const char* buf, size_t len, char32_t* utf32_output) const noexcept {
            std::pair<const char*, char32_t*> ret = lasx_convert_latin1_to_utf32(buf, len, utf32_output);
            size_t converted_chars = ret.second - utf32_output;
            if (ret.first != buf + len) {
                const size_t scalar_converted_chars = scalar::latin1_to_utf32::convert(
                    ret.first, len - (ret.first - buf), ret.second);
                converted_chars += scalar_converted_chars;
            }
            return converted_chars;
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_utf8_to_latin1(
            const char* buf, size_t len, char* latin1_output) const noexcept {
            size_t pos = 0;
            char* output_start { latin1_output };
            // Performance degradation when memory address is not 32-byte aligned
            while (((uint64_t)latin1_output & 0x1F) && pos < len) {
                if (buf[pos] & 0x80) {
                    if (pos + 1 >= len)
                        return 0;
                    if ((buf[pos] & 0b11100000) == 0b11000000) {
                        if ((buf[pos + 1] & 0b11000000) != 0b10000000)
                            return 0;
                        uint32_t code_point = (buf[pos] & 0b00011111) << 6 | (buf[pos + 1] & 0b00111111);
                        if (code_point < 0x80 || 0xFF < code_point) {
                            return 0;
                        }
                        *latin1_output++ = char(code_point);
                        pos += 2;
                    } else {
                        return 0;
                    }
                } else {
                    *latin1_output++ = char(buf[pos]);
                    pos++;
                }
            }
            size_t convert_size = latin1_output - output_start;
            if (pos == len)
                return convert_size;
            utf8_to_latin1::validating_transcoder converter;
            size_t convert_result = converter.convert(buf + pos, len - pos, latin1_output);
            return convert_result ? convert_size + convert_result : 0;
        }

         [[nodiscard]] UnicodeResult UnicodeImplementLasx::convert_utf8_to_latin1_with_errors(
            const char* buf, size_t len, char* latin1_output) const noexcept {
            size_t pos = 0;
            char* output_start { latin1_output };
            // Performance degradation when memory address is not 32-byte aligned
            while (((uint64_t)latin1_output & 0x1F) && pos < len) {
                if (buf[pos] & 0x80) {
                    if ((buf[pos] & 0b11100000) == 0b11000000) {
                        if (pos + 1 >= len)
                            return UnicodeResult(UnicodeError::TOO_SHORT, pos);
                        if ((buf[pos + 1] & 0b11000000) != 0b10000000)
                            return UnicodeResult(UnicodeError::TOO_SHORT, pos);
                        uint32_t code_point = (buf[pos] & 0b00011111) << 6 | (buf[pos + 1] & 0b00111111);
                        if (code_point < 0x80)
                            return UnicodeResult(UnicodeError::OVERLONG, pos);
                        if (0xFF < code_point)
                            return UnicodeResult(UnicodeError::TOO_LARGE, pos);
                        *latin1_output++ = char(code_point);
                        pos += 2;
                    } else if ((buf[pos] & 0b11110000) == 0b11100000) {
                        return UnicodeResult(UnicodeError::TOO_LARGE, pos);
                    } else if ((buf[pos] & 0b11111000) == 0b11110000) {
                        return UnicodeResult(UnicodeError::TOO_LARGE, pos);
                    } else {
                        if ((buf[pos] & 0b11000000) == 0b10000000) {
                            return UnicodeResult(UnicodeError::TOO_LONG, pos);
                        }
                        return UnicodeResult(UnicodeError::HEADER_BITS, pos);
                    }
                } else {
                    *latin1_output++ = char(buf[pos]);
                    pos++;
                }
            }
            size_t convert_size = latin1_output - output_start;
            if (pos == len)
                return UnicodeResult(UnicodeError::SUCCESS, convert_size);

            utf8_to_latin1::validating_transcoder converter;
            UnicodeResult res = converter.convert_with_errors(buf + pos, len - pos, latin1_output);
            return res.error ? UnicodeResult(res.error, res.count + pos)
                             : UnicodeResult(res.error, res.count + convert_size);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_valid_utf8_to_latin1(
            const char* buf, size_t len, char* latin1_output) const noexcept {
            size_t pos = 0;
            char* output_start { latin1_output };
            // Performance degradation when memory address is not 32-byte aligned
            while (((uint64_t)latin1_output & 0x1F) && pos < len) {
                if (buf[pos] & 0x80) {
                    if (pos + 1 >= len)
                        break;
                    if ((buf[pos] & 0b11100000) == 0b11000000) {
                        if ((buf[pos + 1] & 0b11000000) != 0b10000000)
                            return 0;
                        uint32_t code_point = (buf[pos] & 0b00011111) << 6 | (buf[pos + 1] & 0b00111111);
                        *latin1_output++ = char(code_point);
                        pos += 2;
                    } else {
                        return 0;
                    }
                } else {
                    *latin1_output++ = char(buf[pos]);
                    pos++;
                }
            }
            size_t convert_size = latin1_output - output_start;
            if (pos == len)
                return convert_size;

            size_t convert_result = lasx::utf8_to_latin1::convert_valid(buf + pos, len - pos, latin1_output);
            return convert_result ? convert_size + convert_result : 0;
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_utf8_to_utf16le(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            utf8_to_utf16::validating_transcoder converter;
            return converter.convert<Endian::little>(buf, len, utf16_output);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_utf8_to_utf16be(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            utf8_to_utf16::validating_transcoder converter;
            return converter.convert<Endian::big>(buf, len, utf16_output);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementLasx::convert_utf8_to_utf16le_with_errors(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            utf8_to_utf16::validating_transcoder converter;
            return converter.convert_with_errors<Endian::little>(buf, len,
                utf16_output);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementLasx::convert_utf8_to_utf16be_with_errors(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            utf8_to_utf16::validating_transcoder converter;
            return converter.convert_with_errors<Endian::big>(buf, len, utf16_output);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_valid_utf8_to_utf16le(
            const char* input, size_t size, char16_t* utf16_output) const noexcept {
            return utf8_to_utf16::convert_valid<Endian::little>(input, size,
                utf16_output);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_valid_utf8_to_utf16be(
            const char* input, size_t size, char16_t* utf16_output) const noexcept {
            return utf8_to_utf16::convert_valid<Endian::big>(input, size,
                utf16_output);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_utf8_to_utf32(
            const char* buf, size_t len, char32_t* utf32_output) const noexcept {
            utf8_to_utf32::validating_transcoder converter;
            return converter.convert(buf, len, utf32_output);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementLasx::convert_utf8_to_utf32_with_errors(
            const char* buf, size_t len, char32_t* utf32_output) const noexcept {
            utf8_to_utf32::validating_transcoder converter;
            return converter.convert_with_errors(buf, len, utf32_output);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_valid_utf8_to_utf32(
            const char* input, size_t size, char32_t* utf32_output) const noexcept {
            return utf8_to_utf32::convert_valid(input, size, utf32_output);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_utf16le_to_latin1(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            std::pair<const char16_t*, char*> ret = lasx_convert_utf16_to_latin1<Endian::little>(buf, len, latin1_output);
            if (ret.first == nullptr) {
                return 0;
            }
            size_t saved_bytes = ret.second - latin1_output;

            if (ret.first != buf + len) {
                const size_t scalar_saved_bytes = scalar::utf16_to_latin1::convert<Endian::little>(
                    ret.first, len - (ret.first - buf), ret.second);
                if (scalar_saved_bytes == 0) {
                    return 0;
                }
                saved_bytes += scalar_saved_bytes;
            }
            return saved_bytes;
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_utf16be_to_latin1(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            std::pair<const char16_t*, char*> ret = lasx_convert_utf16_to_latin1<Endian::big>(buf, len, latin1_output);
            if (ret.first == nullptr) {
                return 0;
            }
            size_t saved_bytes = ret.second - latin1_output;

            if (ret.first != buf + len) {
                const size_t scalar_saved_bytes = scalar::utf16_to_latin1::convert<Endian::big>(
                    ret.first, len - (ret.first - buf), ret.second);
                if (scalar_saved_bytes == 0) {
                    return 0;
                }
                saved_bytes += scalar_saved_bytes;
            }
            return saved_bytes;
        }

         [[nodiscard]] UnicodeResult
        UnicodeImplementLasx::convert_utf16le_to_latin1_with_errors(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            std::pair<UnicodeResult, char*> ret = lasx_convert_utf16_to_latin1_with_errors<Endian::little>(
                buf, len, latin1_output);
            if (ret.first.error) {
                return ret.first;
            } // Can return directly since scalar fallback already found correct
              // ret.first.count
            if (ret.first.count != len) { // All good so far, but not finished
                UnicodeResult scalar_res = scalar::utf16_to_latin1::convert_with_errors<Endian::little>(
                    buf + ret.first.count, len - ret.first.count, ret.second);
                if (scalar_res.error) {
                    scalar_res.count += ret.first.count;
                    return scalar_res;
                } else {
                    ret.second += scalar_res.count;
                }
            }
            ret.first.count = ret.second - latin1_output; // Set count to the number of 8-bit code units written
            return ret.first;
        }

         [[nodiscard]] UnicodeResult
        UnicodeImplementLasx::convert_utf16be_to_latin1_with_errors(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            std::pair<UnicodeResult, char*> ret = lasx_convert_utf16_to_latin1_with_errors<Endian::big>(buf, len,
                latin1_output);
            if (ret.first.error) {
                return ret.first;
            } // Can return directly since scalar fallback already found correct
              // ret.first.count
            if (ret.first.count != len) { // All good so far, but not finished
                UnicodeResult scalar_res = scalar::utf16_to_latin1::convert_with_errors<Endian::big>(
                    buf + ret.first.count, len - ret.first.count, ret.second);
                if (scalar_res.error) {
                    scalar_res.count += ret.first.count;
                    return scalar_res;
                } else {
                    ret.second += scalar_res.count;
                }
            }
            ret.first.count = ret.second - latin1_output; // Set count to the number of 8-bit code units written
            return ret.first;
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_valid_utf16be_to_latin1(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            // optimization opportunity: implement a custom function.
            return convert_utf16be_to_latin1(buf, len, latin1_output);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_valid_utf16le_to_latin1(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            // optimization opportunity: implement a custom function.
            return convert_utf16le_to_latin1(buf, len, latin1_output);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_utf16le_to_utf8(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            std::pair<const char16_t*, char*> ret = lasx_convert_utf16_to_utf8<Endian::little>(buf, len, utf8_output);
            if (ret.first == nullptr) {
                return 0;
            }
            size_t saved_bytes = ret.second - utf8_output;
            if (ret.first != buf + len) {
                const size_t scalar_saved_bytes = scalar::utf16_to_utf8::convert<Endian::little>(
                    ret.first, len - (ret.first - buf), ret.second);
                if (scalar_saved_bytes == 0) {
                    return 0;
                }
                saved_bytes += scalar_saved_bytes;
            }
            return saved_bytes;
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_utf16be_to_utf8(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            std::pair<const char16_t*, char*> ret = lasx_convert_utf16_to_utf8<Endian::big>(buf, len, utf8_output);
            if (ret.first == nullptr) {
                return 0;
            }
            size_t saved_bytes = ret.second - utf8_output;
            if (ret.first != buf + len) {
                const size_t scalar_saved_bytes = scalar::utf16_to_utf8::convert<Endian::big>(
                    ret.first, len - (ret.first - buf), ret.second);
                if (scalar_saved_bytes == 0) {
                    return 0;
                }
                saved_bytes += scalar_saved_bytes;
            }
            return saved_bytes;
        }

         [[nodiscard]] UnicodeResult UnicodeImplementLasx::convert_utf16le_to_utf8_with_errors(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            // ret.first.count is always the position in the buffer, not the number of
            // code units written even if finished
            std::pair<UnicodeResult, char*> ret = lasx_convert_utf16_to_utf8_with_errors<Endian::little>(buf, len,
                utf8_output);
            if (ret.first.error) {
                return ret.first;
            } // Can return directly since scalar fallback already found correct
              // ret.first.count
            if (ret.first.count != len) { // All good so far, but not finished
                UnicodeResult scalar_res = scalar::utf16_to_utf8::convert_with_errors<Endian::little>(
                    buf + ret.first.count, len - ret.first.count, ret.second);
                if (scalar_res.error) {
                    scalar_res.count += ret.first.count;
                    return scalar_res;
                } else {
                    ret.second += scalar_res.count;
                }
            }
            ret.first.count = ret.second - utf8_output; // Set count to the number of 8-bit code units written
            return ret.first;
        }

         [[nodiscard]] UnicodeResult UnicodeImplementLasx::convert_utf16be_to_utf8_with_errors(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            // ret.first.count is always the position in the buffer, not the number of
            // code units written even if finished
            std::pair<UnicodeResult, char*> ret = lasx_convert_utf16_to_utf8_with_errors<Endian::big>(buf, len,
                utf8_output);
            if (ret.first.error) {
                return ret.first;
            } // Can return directly since scalar fallback already found correct
              // ret.first.count
            if (ret.first.count != len) { // All good so far, but not finished
                UnicodeResult scalar_res = scalar::utf16_to_utf8::convert_with_errors<Endian::big>(
                    buf + ret.first.count, len - ret.first.count, ret.second);
                if (scalar_res.error) {
                    scalar_res.count += ret.first.count;
                    return scalar_res;
                } else {
                    ret.second += scalar_res.count;
                }
            }
            ret.first.count = ret.second - utf8_output; // Set count to the number of 8-bit code units written
            return ret.first;
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_valid_utf16le_to_utf8(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            return convert_utf16le_to_utf8(buf, len, utf8_output);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_valid_utf16be_to_utf8(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            return convert_utf16be_to_utf8(buf, len, utf8_output);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_utf32_to_utf8(
            const char32_t* buf, size_t len, char* utf8_output) const noexcept {
            if (KUMO_UNLIKELY(len == 0)) {
                return 0;
            }
            std::pair<const char32_t*, char*> ret = lasx_convert_utf32_to_utf8(buf, len, utf8_output);
            if (ret.first == nullptr) {
                return 0;
            }
            size_t saved_bytes = ret.second - utf8_output;
            if (ret.first != buf + len) {
                const size_t scalar_saved_bytes = scalar::utf32_to_utf8::convert(
                    ret.first, len - (ret.first - buf), ret.second);
                if (scalar_saved_bytes == 0) {
                    return 0;
                }
                saved_bytes += scalar_saved_bytes;
            }
            return saved_bytes;
        }

         [[nodiscard]] UnicodeResult UnicodeImplementLasx::convert_utf32_to_utf8_with_errors(
            const char32_t* buf, size_t len, char* utf8_output) const noexcept {
            if (KUMO_UNLIKELY(len == 0)) {
                return UnicodeResult(UnicodeError::SUCCESS, 0);
            }
            // ret.first.count is always the position in the buffer, not the number of
            // code units written even if finished
            std::pair<UnicodeResult, char*> ret = lasx_convert_utf32_to_utf8_with_errors(buf, len, utf8_output);
            if (ret.first.count != len) {
                UnicodeResult scalar_res = scalar::utf32_to_utf8::convert_with_errors(
                    buf + ret.first.count, len - ret.first.count, ret.second);
                if (scalar_res.error) {
                    scalar_res.count += ret.first.count;
                    return scalar_res;
                } else {
                    ret.second += scalar_res.count;
                }
            }
            ret.first.count = ret.second - utf8_output; // Set count to the number of 8-bit code units written
            return ret.first;
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_utf16le_to_utf32(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            std::pair<const char16_t*, char32_t*> ret = lasx_convert_utf16_to_utf32<Endian::little>(buf, len, utf32_output);
            if (ret.first == nullptr) {
                return 0;
            }
            size_t saved_bytes = ret.second - utf32_output;
            if (ret.first != buf + len) {
                const size_t scalar_saved_bytes = scalar::utf16_to_utf32::convert<Endian::little>(
                    ret.first, len - (ret.first - buf), ret.second);
                if (scalar_saved_bytes == 0) {
                    return 0;
                }
                saved_bytes += scalar_saved_bytes;
            }
            return saved_bytes;
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_utf16be_to_utf32(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            std::pair<const char16_t*, char32_t*> ret = lasx_convert_utf16_to_utf32<Endian::big>(buf, len, utf32_output);
            if (ret.first == nullptr) {
                return 0;
            }
            size_t saved_bytes = ret.second - utf32_output;
            if (ret.first != buf + len) {
                const size_t scalar_saved_bytes = scalar::utf16_to_utf32::convert<Endian::big>(
                    ret.first, len - (ret.first - buf), ret.second);
                if (scalar_saved_bytes == 0) {
                    return 0;
                }
                saved_bytes += scalar_saved_bytes;
            }
            return saved_bytes;
        }

         [[nodiscard]] UnicodeResult UnicodeImplementLasx::convert_utf16le_to_utf32_with_errors(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            // ret.first.count is always the position in the buffer, not the number of
            // code units written even if finished
            std::pair<UnicodeResult, char32_t*> ret = lasx_convert_utf16_to_utf32_with_errors<Endian::little>(buf, len,
                utf32_output);
            if (ret.first.error) {
                return ret.first;
            } // Can return directly since scalar fallback already found correct
              // ret.first.count
            if (ret.first.count != len) { // All good so far, but not finished
                UnicodeResult scalar_res = scalar::utf16_to_utf32::convert_with_errors<Endian::little>(
                    buf + ret.first.count, len - ret.first.count, ret.second);
                if (scalar_res.error) {
                    scalar_res.count += ret.first.count;
                    return scalar_res;
                } else {
                    ret.second += scalar_res.count;
                }
            }
            ret.first.count = ret.second - utf32_output; // Set count to the number of 8-bit code units written
            return ret.first;
        }

         [[nodiscard]] UnicodeResult UnicodeImplementLasx::convert_utf16be_to_utf32_with_errors(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            // ret.first.count is always the position in the buffer, not the number of
            // code units written even if finished
            std::pair<UnicodeResult, char32_t*> ret = lasx_convert_utf16_to_utf32_with_errors<Endian::big>(buf, len,
                utf32_output);
            if (ret.first.error) {
                return ret.first;
            } // Can return directly since scalar fallback already found correct
              // ret.first.count
            if (ret.first.count != len) { // All good so far, but not finished
                UnicodeResult scalar_res = scalar::utf16_to_utf32::convert_with_errors<Endian::big>(
                    buf + ret.first.count, len - ret.first.count, ret.second);
                if (scalar_res.error) {
                    scalar_res.count += ret.first.count;
                    return scalar_res;
                } else {
                    ret.second += scalar_res.count;
                }
            }
            ret.first.count = ret.second - utf32_output; // Set count to the number of 8-bit code units written
            return ret.first;
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_utf32_to_latin1(
            const char32_t* buf, size_t len, char* latin1_output) const noexcept {
            std::pair<const char32_t*, char*> ret = lasx_convert_utf32_to_latin1(buf, len, latin1_output);
            if (ret.first == nullptr) {
                return 0;
            }
            size_t saved_bytes = ret.second - latin1_output;

            if (ret.first != buf + len) {
                const size_t scalar_saved_bytes = scalar::utf32_to_latin1::convert(
                    ret.first, len - (ret.first - buf), ret.second);
                if (scalar_saved_bytes == 0) {
                    return 0;
                }
                saved_bytes += scalar_saved_bytes;
            }
            return saved_bytes;
        }

         [[nodiscard]] UnicodeResult UnicodeImplementLasx::convert_utf32_to_latin1_with_errors(
            const char32_t* buf, size_t len, char* latin1_output) const noexcept {
            std::pair<UnicodeResult, char*> ret = lasx_convert_utf32_to_latin1_with_errors(buf, len, latin1_output);
            if (ret.first.error) {
                return ret.first;
            } // Can return directly since scalar fallback already found correct
              // ret.first.count
            if (ret.first.count != len) { // All good so far, but not finished
                UnicodeResult scalar_res = scalar::utf32_to_latin1::convert_with_errors(
                    buf + ret.first.count, len - ret.first.count, ret.second);
                if (scalar_res.error) {
                    scalar_res.count += ret.first.count;
                    return scalar_res;
                } else {
                    ret.second += scalar_res.count;
                }
            }
            ret.first.count = ret.second - latin1_output; // Set count to the number of 8-bit code units written
            return ret.first;
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_valid_utf32_to_latin1(
            const char32_t* buf, size_t len, char* latin1_output) const noexcept {
            std::pair<const char32_t*, char*> ret = lasx_convert_utf32_to_latin1(buf, len, latin1_output);
            if (ret.first == nullptr) {
                return 0;
            }
            size_t saved_bytes = ret.second - latin1_output;

            if (ret.first != buf + len) {
                const size_t scalar_saved_bytes = scalar::utf32_to_latin1::convert_valid(
                    ret.first, len - (ret.first - buf), ret.second);
                saved_bytes += scalar_saved_bytes;
            }
            return saved_bytes;
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_valid_utf32_to_utf8(
            const char32_t* buf, size_t len, char* utf8_output) const noexcept {
            // optimization opportunity: implement a custom function.
            return convert_utf32_to_utf8(buf, len, utf8_output);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_utf32_to_utf16le(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {
            std::pair<const char32_t*, char16_t*> ret = lasx_convert_utf32_to_utf16<Endian::little>(buf, len, utf16_output);
            if (ret.first == nullptr) {
                return 0;
            }
            size_t saved_bytes = ret.second - utf16_output;
            if (ret.first != buf + len) {
                const size_t scalar_saved_bytes = scalar::utf32_to_utf16::convert<Endian::little>(
                    ret.first, len - (ret.first - buf), ret.second);
                if (scalar_saved_bytes == 0) {
                    return 0;
                }
                saved_bytes += scalar_saved_bytes;
            }

            return saved_bytes;
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_utf32_to_utf16be(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {
            std::pair<const char32_t*, char16_t*> ret = lasx_convert_utf32_to_utf16<Endian::big>(buf, len, utf16_output);
            if (ret.first == nullptr) {
                return 0;
            }
            size_t saved_bytes = ret.second - utf16_output;
            if (ret.first != buf + len) {
                const size_t scalar_saved_bytes = scalar::utf32_to_utf16::convert<Endian::big>(
                    ret.first, len - (ret.first - buf), ret.second);
                if (scalar_saved_bytes == 0) {
                    return 0;
                }
                saved_bytes += scalar_saved_bytes;
            }
            return saved_bytes;
        }

         [[nodiscard]] UnicodeResult UnicodeImplementLasx::convert_utf32_to_utf16le_with_errors(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {
            // ret.first.count is always the position in the buffer, not the number of
            // code units written even if finished
            std::pair<UnicodeResult, char16_t*> ret = lasx_convert_utf32_to_utf16_with_errors<Endian::little>(buf, len,
                utf16_output);
            if (ret.first.count != len) {
                UnicodeResult scalar_res = scalar::utf32_to_utf16::convert_with_errors<Endian::little>(
                    buf + ret.first.count, len - ret.first.count, ret.second);
                if (scalar_res.error) {
                    scalar_res.count += ret.first.count;
                    return scalar_res;
                } else {
                    ret.second += scalar_res.count;
                }
            }
            ret.first.count = ret.second - utf16_output; // Set count to the number of 8-bit code units written
            return ret.first;
        }

         [[nodiscard]] UnicodeResult UnicodeImplementLasx::convert_utf32_to_utf16be_with_errors(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {
            // ret.first.count is always the position in the buffer, not the number of
            // code units written even if finished
            std::pair<UnicodeResult, char16_t*> ret = lasx_convert_utf32_to_utf16_with_errors<Endian::big>(buf, len,
                utf16_output);
            if (ret.first.count != len) {
                UnicodeResult scalar_res = scalar::utf32_to_utf16::convert_with_errors<Endian::big>(
                    buf + ret.first.count, len - ret.first.count, ret.second);
                if (scalar_res.error) {
                    scalar_res.count += ret.first.count;
                    return scalar_res;
                } else {
                    ret.second += scalar_res.count;
                }
            }
            ret.first.count = ret.second - utf16_output; // Set count to the number of 8-bit code units written
            return ret.first;
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_valid_utf32_to_utf16le(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {
            return convert_utf32_to_utf16le(buf, len, utf16_output);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_valid_utf32_to_utf16be(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {
            return convert_utf32_to_utf16be(buf, len, utf16_output);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_valid_utf16le_to_utf32(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return convert_utf16le_to_utf32(buf, len, utf32_output);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::convert_valid_utf16be_to_utf32(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return convert_utf16be_to_utf32(buf, len, utf32_output);
        }

        void UnicodeImplementLasx::change_endianness_utf16(const char16_t* input,
            size_t length,
            char16_t* output) const noexcept {
            utf16::change_endianness_utf16(input, length, output);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::count_utf16le(
            const char16_t* input, size_t length) const noexcept {
            return utf16::count_code_points<Endian::little>(input, length);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::count_utf16be(
            const char16_t* input, size_t length) const noexcept {
            return utf16::count_code_points<Endian::big>(input, length);
        }

         [[nodiscard]] size_t
        UnicodeImplementLasx::count_utf8(const char* input, size_t length) const noexcept {
            size_t pos = 0;
            size_t count = 0;
            // Performance degradation when memory address is not 32-byte aligned
            while ((((uint64_t)input + pos) & 0x1F && pos < length)) {
                if (input[pos++] > -65) {
                    count++;
                }
            }
            __m256i v_bf = __lasx_xvldi(0xBF); // 0b10111111
            for (; pos + 32 <= length; pos += 32) {
                __m256i in = __lasx_xvld(reinterpret_cast<const int8_t*>(input + pos), 0);
                __m256i utf8_count = __lasx_xvpcnt_h(__lasx_xvmskltz_b(__lasx_xvslt_b(v_bf, in)));
                count = count + __lasx_xvpickve2gr_wu(utf8_count, 0) + __lasx_xvpickve2gr_wu(utf8_count, 4);
            }
            return count + scalar::utf8::count_code_points(input + pos, length - pos);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::latin1_length_from_utf8(
            const char* buf, size_t len) const noexcept {
            return count_utf8(buf, len);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::utf8_length_from_latin1(
            const char* input, size_t length) const noexcept {
            const uint8_t* data = reinterpret_cast<const uint8_t*>(input);
            const uint8_t* data_end = data + length;
            uint64_t UnicodeResult = 0;
            while (data_end - data > 16) {
                uint64_t two_bytes = 0;
                __m128i input_vec = __lsx_vld(data, 0);
                two_bytes = __lsx_vpickve2gr_hu(__lsx_vpcnt_h(__lsx_vmskltz_b(input_vec)), 0);
                UnicodeResult += 16 + two_bytes;
                data += 16;
            }
            return UnicodeResult + scalar::latin1::utf8_length_from_latin1((const char*)data, data_end - data);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::utf8_length_from_utf16le(
            const char16_t* input, size_t length) const noexcept {
            return utf16::utf8_length_from_utf16_bytemask<Endian::little>(input,
                length);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::utf8_length_from_utf16be(
            const char16_t* input, size_t length) const noexcept {
            return utf16::utf8_length_from_utf16_bytemask<Endian::big>(input, length);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::utf32_length_from_utf16le(
            const char16_t* input, size_t length) const noexcept {
            return utf16::utf32_length_from_utf16<Endian::little>(input, length);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::utf32_length_from_utf16be(
            const char16_t* input, size_t length) const noexcept {
            return utf16::utf32_length_from_utf16<Endian::big>(input, length);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::utf16_length_from_utf8(
            const char* input, size_t length) const noexcept {
            return utf8::utf16_length_from_utf8_bytemask(input, length);
        }
         [[nodiscard]] UnicodeResult
        UnicodeImplementLasx::utf8_length_from_utf16le_with_replacement(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::utf8_length_from_utf16_with_replacement<
                Endian::little>(input, length);
        }

         [[nodiscard]] UnicodeResult
        UnicodeImplementLasx::utf8_length_from_utf16be_with_replacement(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::utf8_length_from_utf16_with_replacement<
                Endian::big>(input, length);
        }

         [[nodiscard]] size_t
        UnicodeImplementLasx::convert_utf16le_to_utf8_with_replacement(
            const char16_t* input, size_t length, char* utf8_buffer) const noexcept {
            return utf16_to_utf8::convert_with_replacement_via(
                [this](const char16_t* b, size_t l, char* o) {
                    return this->convert_utf16le_to_utf8_with_errors(b, l, o);
                },
                [this](const char16_t* b, size_t l) {
                    return this->utf8_length_from_utf16le(b, l);
                },
                input, length, utf8_buffer);
        }

         [[nodiscard]] size_t
        UnicodeImplementLasx::convert_utf16be_to_utf8_with_replacement(
            const char16_t* input, size_t length, char* utf8_buffer) const noexcept {
            return utf16_to_utf8::convert_with_replacement_via(
                [this](const char16_t* b, size_t l, char* o) {
                    return this->convert_utf16be_to_utf8_with_errors(b, l, o);
                },
                [this](const char16_t* b, size_t l) {
                    return this->utf8_length_from_utf16be(b, l);
                },
                input, length, utf8_buffer);
        }


         [[nodiscard]] size_t UnicodeImplementLasx::utf8_length_from_utf32(
            const char32_t* input, size_t length) const noexcept {
            return utf32::utf8_length_from_utf32(input, length);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::utf16_length_from_utf32(
            const char32_t* input, size_t length) const noexcept {
            __m128i v_ffff = lsx_splat_u32(0x0000ffff);
            size_t pos = 0;
            size_t count = 0;
            for (; pos + 4 <= length; pos += 4) {
                __m128i in = __lsx_vld(reinterpret_cast<const uint32_t*>(input + pos), 0);
                __m128i surrogate_bytemask = __lsx_vslt_wu(v_ffff, in);
                size_t surrogate_count = __lsx_vpickve2gr_bu(
                    __lsx_vpcnt_b(__lsx_vmskltz_w(surrogate_bytemask)), 0);
                count += 4 + surrogate_count;
            }
            return count + scalar::utf32::utf16_length_from_utf32(input + pos, length - pos);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::utf32_length_from_utf8(
            const char* input, size_t length) const noexcept {
            return utf8::count_code_points(input, length);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementLasx::base64_to_binary(
            const char* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            if (options & base64_default_or_url) {
                if (options == Base64Options::base64_default_or_url_accept_garbage) {
                    return compress_decode_base64<false, true, true>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return compress_decode_base64<false, false, true>(
                        output, input, length, options, last_chunk_options);
                }
            } else if (options & base64_url) {
                if (options == Base64Options::base64_url_accept_garbage) {
                    return compress_decode_base64<true, true, false>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return compress_decode_base64<true, false, false>(
                        output, input, length, options, last_chunk_options);
                }
            } else {
                if (options == Base64Options::base64_default_accept_garbage) {
                    return compress_decode_base64<false, true, false>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return compress_decode_base64<false, false, false>(
                        output, input, length, options, last_chunk_options);
                }
            }
        }

         [[nodiscard]] full_result UnicodeImplementLasx::base64_to_binary_details(
            const char* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            if (options & base64_default_or_url) {
                if (options == Base64Options::base64_default_or_url_accept_garbage) {
                    return compress_decode_base64<false, true, true>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return compress_decode_base64<false, false, true>(
                        output, input, length, options, last_chunk_options);
                }
            } else if (options & base64_url) {
                if (options == Base64Options::base64_url_accept_garbage) {
                    return compress_decode_base64<true, true, false>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return compress_decode_base64<true, false, false>(
                        output, input, length, options, last_chunk_options);
                }
            } else {
                if (options == Base64Options::base64_default_accept_garbage) {
                    return compress_decode_base64<false, true, false>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return compress_decode_base64<false, false, false>(
                        output, input, length, options, last_chunk_options);
                }
            }
        }

         [[nodiscard]] UnicodeResult UnicodeImplementLasx::base64_to_binary(
            const char16_t* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            if (options & base64_default_or_url) {
                if (options == Base64Options::base64_default_or_url_accept_garbage) {
                    return compress_decode_base64<false, true, true>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return compress_decode_base64<false, false, true>(
                        output, input, length, options, last_chunk_options);
                }
            } else if (options & base64_url) {
                if (options == Base64Options::base64_url_accept_garbage) {
                    return compress_decode_base64<true, true, false>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return compress_decode_base64<true, false, false>(
                        output, input, length, options, last_chunk_options);
                }
            } else {
                if (options == Base64Options::base64_default_accept_garbage) {
                    return compress_decode_base64<false, true, false>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return compress_decode_base64<false, false, false>(
                        output, input, length, options, last_chunk_options);
                }
            }
        }

         [[nodiscard]] full_result UnicodeImplementLasx::base64_to_binary_details(
            const char16_t* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            if (options & base64_default_or_url) {
                if (options == Base64Options::base64_default_or_url_accept_garbage) {
                    return compress_decode_base64<false, true, true>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return compress_decode_base64<false, false, true>(
                        output, input, length, options, last_chunk_options);
                }
            } else if (options & base64_url) {
                if (options == Base64Options::base64_url_accept_garbage) {
                    return compress_decode_base64<true, true, false>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return compress_decode_base64<true, false, false>(
                        output, input, length, options, last_chunk_options);
                }
            } else {
                if (options == Base64Options::base64_default_accept_garbage) {
                    return compress_decode_base64<false, true, false>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return compress_decode_base64<false, false, false>(
                        output, input, length, options, last_chunk_options);
                }
            }
        }

        size_t UnicodeImplementLasx::binary_to_base64(const char* input, size_t length,
            char* output,
            Base64Options options) const noexcept {
            if (options & base64_url) {
                return encode_base64<true>(output, input, length, options);
            } else {
                return encode_base64<false>(output, input, length, options);
            }
        }

        size_t UnicodeImplementLasx::binary_to_base64_with_lines(
            const char* input, size_t length, char* output, size_t line_length,
            Base64Options options) const noexcept {
            return scalar::base64::tail_encode_base64_impl<true>(output, input, length,
                options, line_length);
        }

        const char* UnicodeImplementLasx::find(const char* start, const char* end,
            char character) const noexcept {
            return util_find(start, end, character);
        }

        const char16_t* UnicodeImplementLasx::find(const char16_t* start, const char16_t* end,
            char16_t character) const noexcept {
            return util_find(start, end, character);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::binary_length_from_base64(
            const char* input, size_t length) const noexcept {
            return base64_lengths::binary_length_from_base64(input, length);
        }

         [[nodiscard]] size_t UnicodeImplementLasx::binary_length_from_base64(
            const char16_t* input, size_t length) const noexcept {
            return base64_lengths::binary_length_from_base64(input, length);
        }

    } // namespace UNICODE_IMPLEMENTATION

    static turbo::UnicodeImplement *get_lasx_instance() {
        static lasx::UnicodeImplementLasx ins;
        return &ins;
    }
} // namespace turbo

#include <turbo/unicode/engine/lasx/end.h>
#else
namespace turbo {
    static turbo::UnicodeImplement *get_lasx_instance() {
        return nullptr;
    }
}
#endif

namespace turbo {
    IsaInfo get_lasx_info() {
        static IsaInfo ins = {
            .compiled = UNICODE_IMPLEMENTATION_LASX,
            .failback = false,
            .required_isa = static_cast<uint32_t>(InstructionSet::LSX | InstructionSet::LASX),
            .isa_name = "lasx",
            .engine = get_lasx_instance(),
        };
        return ins;
    }
} // namespace turbo
