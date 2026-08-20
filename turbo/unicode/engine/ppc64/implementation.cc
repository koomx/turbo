
#include <turbo/unicode/engine/ppc64.h>
#include <turbo/unicode/engine/implementation.h>
#if UNICODE_IMPLEMENTATION_PPC64

#include <turbo/unicode/tables/utf8_to_utf16_tables.h>
#include <turbo/unicode/tables/utf16_to_utf8_tables.h>
#include <turbo/unicode/tables/utf32_to_utf16_tables.h>



#include <turbo/unicode/engine/ppc64/begin.h>

#include <turbo/unicode/engine/ppc64/ppc64_utf16_to_utf8_tables.h>

namespace turbo {
    namespace UNICODE_IMPLEMENTATION {
        namespace {
#ifndef UNICODE_PPC64_H
#error "ppc64.h must be included"
#endif
            using namespace simd;

            KUMO_FORCE_INLINE bool is_ascii(const simd8x64<uint8_t>& input) {
                // careful: 0x80 is not ascii.
                return input.reduce_or().saturating_sub(0b01111111u).bits_not_set_anywhere();
            }

            KUMO_FORCE_INLINE simd8<bool>
            must_be_2_3_continuation(const simd8<uint8_t> prev2,
                const simd8<uint8_t> prev3) {
                simd8<uint8_t> is_third_byte = prev2.saturating_sub(0xe0u - 0x80); // Only 111_____ will be >= 0x80
                simd8<uint8_t> is_fourth_byte = prev3.saturating_sub(0xf0u - 0x80); // Only 1111____ will be >= 0x80
                // Caller requires a bool (all 1's). All values resulting from the subtraction
                // will be <= 64, so signed comparison is fine.
                return simd8<bool>(is_third_byte | is_fourth_byte);
            }

            /// ErrorReporting describes behaviour of a vectorized procedure regarding error
            /// checking
            enum class ErrorReporting {
                precise, // the procedure will report *approximate* or *precise* error
                         // position
                at_the_end, // the procedure will only inform about an error after scanning
                            // the whole input (or its significant portion)
                none, // no error checking is done, we assume valid inputs
            };

#include <turbo/unicode/engine/ppc64/ppc64_validate_utf16.cpp>

#include <turbo/unicode/engine/ppc64/ppc64_write_to_utf8.cpp>

#include <turbo/unicode/engine/ppc64/ppc64_convert_latin1_to_utf8.cpp>

#include <turbo/unicode/engine/ppc64/ppc64_convert_latin1_to_utf16.cpp>

#include <turbo/unicode/engine/ppc64/ppc64_convert_latin1_to_utf32.cpp>

#include <turbo/unicode/engine/ppc64/ppc64_convert_utf8_to_latin1.cpp>

#include <turbo/unicode/engine/ppc64/ppc64_convert_utf8_to_utf16.cpp>

#include <turbo/unicode/engine/ppc64/ppc64_convert_utf8_to_utf32.cpp>

#include <turbo/unicode/engine/ppc64/ppc64_convert_utf16_to_latin1.cpp>

#include <turbo/unicode/engine/ppc64/ppc64_convert_utf16_to_utf8.cpp>


#include <turbo/unicode/engine/ppc64/ppc64_convert_utf16_to_utf32.cpp>

#include <turbo/unicode/engine/ppc64/ppc64_convert_utf32_to_latin1.cpp>

#include <turbo/unicode/engine/ppc64/ppc64_convert_utf32_to_utf16.cpp>

#include <turbo/unicode/engine/ppc64/ppc64_convert_utf32_to_utf8.cpp>

#include <turbo/unicode/engine/ppc64/ppc64_utf8_length_from_latin1.cpp>

#include <turbo/unicode/engine/ppc64/ppc64_base64.cpp>

        } // unnamed namespace
    } // namespace UNICODE_IMPLEMENTATION
} // namespace turbo

#include <turbo/unicode/generic/buf_block_reader.h>
#include <turbo/unicode/generic/utf8_validation/utf8_lookup4_algorithm.h>
#include <turbo/unicode/generic/utf8_validation/utf8_validator.h>

#include <turbo/unicode/generic/utf8_to_utf16/utf8_to_utf16.h>
#include <turbo/unicode/generic/utf8_to_utf16/valid_utf8_to_utf16.h>
#include <turbo/unicode/generic/utf16_to_utf8/utf16_to_utf8_with_replacement.h>

#include <turbo/unicode/generic/utf8_to_utf32/utf8_to_utf32.h>
#include <turbo/unicode/generic/utf8_to_utf32/valid_utf8_to_utf32.h>

#include <turbo/unicode/generic/utf8.h>

#include <turbo/unicode/generic/utf16.h>
#include <turbo/unicode/generic/validate_utf16.h>

#include <turbo/unicode/generic/utf32.h>
#include <turbo/unicode/generic/validate_utf32.h>

#include <turbo/unicode/generic/ascii_validation.h>

#include <turbo/unicode/generic/utf8_to_latin1/utf8_to_latin1.h>
#include <turbo/unicode/generic/utf8_to_latin1/valid_utf8_to_latin1.h>

#include <turbo/unicode/generic/base64.h>
#include <turbo/unicode/generic/find.h>

#include <turbo/unicode/engine/ppc64/templates.cpp>

#ifdef UNICODE_INTERNAL_TESTS
#include "ppc64_base64_internal_tests.cpp"
#endif // UNICODE_INTERNAL_TESTS
//
// Implementation-specific overrides
//
namespace turbo {
    namespace UNICODE_IMPLEMENTATION {

         [[nodiscard]] int
        implementation::detect_encodings(const char* input,
            size_t length) const noexcept {
            // If there is a BOM, then we trust it.
            auto bom_encoding = turbo::BOM::check_bom(input, length);
            if (bom_encoding != TextEncoding::unspecified) {
                return bom_encoding;
            }
            int out = 0;
            // todo: reimplement as a one-pass algorithm.
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
        implementation::validate_utf8(const char* buf, size_t len) const noexcept {
            return ppc64::utf8_validation::generic_validate_utf8(buf, len);
        }

         [[nodiscard]] UnicodeResult implementation::validate_utf8_with_errors(
            const char* buf, size_t len) const noexcept {
            return ppc64::utf8_validation::generic_validate_utf8_with_errors(buf, len);
        }

         [[nodiscard]] bool
        implementation::validate_ascii(const char* buf, size_t len) const noexcept {
            return ppc64::ascii_validation::generic_validate_ascii(buf, len);
        }

         [[nodiscard]] UnicodeResult implementation::validate_ascii_with_errors(
            const char* buf, size_t len) const noexcept {
            return ppc64::ascii_validation::generic_validate_ascii_with_errors(buf, len);
        }
         [[nodiscard]] bool
        implementation::validate_utf16le_as_ascii(const char16_t* buf,
            size_t len) const noexcept {
            return ppc64::utf16::validate_utf16_as_ascii_with_errors<Endian::little>(
                       buf, len)
                       .error
                == SUCCESS;
        }

         [[nodiscard]] bool
        implementation::validate_utf16be_as_ascii(const char16_t* buf,
            size_t len) const noexcept {
            return ppc64::utf16::validate_utf16_as_ascii_with_errors<Endian::big>(buf,
                       len)
                       .error
                == SUCCESS;
        }
         [[nodiscard]] bool
        implementation::validate_utf16le(const char16_t* buf,
            size_t len) const noexcept {
            const auto res = ppc64::utf16::validate_utf16_with_errors<Endian::little>(buf, len);
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
        implementation::validate_utf16be(const char16_t* buf,
            size_t len) const noexcept {
            return validate_utf16be_with_errors(buf, len).is_ok();
        }

        void implementation::to_well_formed_utf16le(const char16_t* input, size_t len,
            char16_t* output) const noexcept {
            return scalar::utf16::to_well_formed_utf16<Endian::little>(input, len,
                output);
        }

        void implementation::to_well_formed_utf16be(const char16_t* input, size_t len,
            char16_t* output) const noexcept {
            return scalar::utf16::to_well_formed_utf16<Endian::big>(input, len,
                output);
        }

         [[nodiscard]] UnicodeResult implementation::validate_utf16le_with_errors(
            const char16_t* buf, size_t len) const noexcept {
            const auto res = ppc64::utf16::validate_utf16_with_errors<Endian::little>(buf, len);
            if (res.count != len) {
                auto scalar = scalar::utf16::validate_with_errors<Endian::little>(
                    buf + res.count, len - res.count);
                scalar.count += res.count;
                return scalar;
            }

            return res;
        }

         [[nodiscard]] UnicodeResult implementation::validate_utf16be_with_errors(
            const char16_t* buf, size_t len) const noexcept {
            const auto res = ppc64::utf16::validate_utf16_with_errors<Endian::big>(buf, len);
            if (res.count != len) {
                auto scalar = scalar::utf16::validate_with_errors<Endian::big>(
                    buf + res.count, len - res.count);
                scalar.count += res.count;
                return scalar;
            }

            return res;
        }

         [[nodiscard]] bool
        implementation::validate_utf32(const char32_t* buf, size_t len) const noexcept {
            return utf32::validate(buf, len);
        }

         [[nodiscard]] UnicodeResult implementation::validate_utf32_with_errors(
            const char32_t* buf, size_t len) const noexcept {
            return utf32::validate_with_errors(buf, len);
        }

         [[nodiscard]] size_t implementation::convert_latin1_to_utf8(
            const char* buf, size_t len, char* utf8_output) const noexcept {
            const auto ret = ppc64_convert_latin1_to_utf8(buf, len, utf8_output);
            size_t converted_chars = ret.second - utf8_output;

            if (ret.first != buf + len) {
                const size_t scalar_converted_chars = scalar::latin1_to_utf8::convert(
                    ret.first, len - (ret.first - buf), ret.second);
                converted_chars += scalar_converted_chars;
            }

            return converted_chars;
        }

         [[nodiscard]] size_t implementation::convert_latin1_to_utf16le(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            size_t n = ppc64_convert_latin1_to_utf16<Endian::little>(buf, len, utf16_output);
            if (n < len) {
                n += scalar::latin1_to_utf16::convert<Endian::little>(buf + n, len - n,
                    utf16_output + n);
            }

            return n;
        }

         [[nodiscard]] size_t implementation::convert_latin1_to_utf16be(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            size_t n = ppc64_convert_latin1_to_utf16<Endian::big>(buf, len, utf16_output);
            if (n < len) {
                n += scalar::latin1_to_utf16::convert<Endian::big>(buf + n, len - n,
                    utf16_output + n);
            }

            return n;
        }

         [[nodiscard]] size_t implementation::convert_latin1_to_utf32(
            const char* buf, size_t len, char32_t* utf32_output) const noexcept {
            const auto ret = ppc64_convert_latin1_to_utf32(buf, len, utf32_output);
            if (ret.first != buf + len) {
                const size_t processed = ret.first - buf;
                scalar::latin1_to_utf32::convert(ret.first, len - processed, ret.second);
            }

            return len;
        }

         [[nodiscard]] size_t implementation::convert_utf8_to_latin1(
            const char* buf, size_t len, char* latin1_output) const noexcept {
            utf8_to_latin1::validating_transcoder converter;
            return converter.convert(buf, len, latin1_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf8_to_latin1_with_errors(
            const char* buf, size_t len, char* latin1_output) const noexcept {
            utf8_to_latin1::validating_transcoder converter;
            return converter.convert_with_errors(buf, len, latin1_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf8_to_latin1(
            const char* buf, size_t len, char* latin1_output) const noexcept {
            return ppc64::utf8_to_latin1::convert_valid(buf, len, latin1_output);
        }

         [[nodiscard]] size_t implementation::convert_utf8_to_utf16le(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            utf8_to_utf16::validating_transcoder converter;
            return converter.convert<Endian::little>(buf, len, utf16_output);
        }

         [[nodiscard]] size_t implementation::convert_utf8_to_utf16be(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            utf8_to_utf16::validating_transcoder converter;
            return converter.convert<Endian::big>(buf, len, utf16_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf8_to_utf16le_with_errors(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            utf8_to_utf16::validating_transcoder converter;
            return converter.convert_with_errors<Endian::little>(buf, len,
                utf16_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf8_to_utf16be_with_errors(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            utf8_to_utf16::validating_transcoder converter;
            return converter.convert_with_errors<Endian::big>(buf, len, utf16_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf8_to_utf16le(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            return utf8_to_utf16::convert_valid<Endian::little>(buf, len,
                utf16_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf8_to_utf16be(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            return utf8_to_utf16::convert_valid<Endian::big>(buf, len, utf16_output);
        }

         [[nodiscard]] size_t implementation::convert_utf8_to_utf32(
            const char* buf, size_t len, char32_t* utf32_output) const noexcept {
            utf8_to_utf32::validating_transcoder converter;
            return converter.convert(buf, len, utf32_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf8_to_utf32_with_errors(
            const char* buf, size_t len, char32_t* utf32_output) const noexcept {
            utf8_to_utf32::validating_transcoder converter;
            return converter.convert_with_errors(buf, len, utf32_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf8_to_utf32(
            const char* input, size_t size, char32_t* utf32_output) const noexcept {
            return utf8_to_utf32::convert_valid(input, size, utf32_output);
        }

         [[nodiscard]] size_t implementation::convert_utf16le_to_latin1(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {

            return convert_impl(
                ppc64_convert_utf16_to_latin1<Endian::little>,
                scalar::utf16_to_latin1::convert<Endian::little, const char16_t*,
                    char*>,
                buf, len, latin1_output);
        }

         [[nodiscard]] size_t implementation::convert_utf16be_to_latin1(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {

            return convert_impl(
                ppc64_convert_utf16_to_latin1<Endian::big>,
                scalar::utf16_to_latin1::convert<Endian::big, const char16_t*,
                    char*>,
                buf, len, latin1_output);
        }

         [[nodiscard]] UnicodeResult
        implementation::convert_utf16le_to_latin1_with_errors(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {

            return convert_with_errors_impl(
                ppc64_convert_utf16_to_latin1<Endian::little>,
                scalar::utf16_to_latin1::convert_with_errors<Endian::little,
                    const char16_t*, char*>,
                buf, len, latin1_output);
        }

         [[nodiscard]] UnicodeResult
        implementation::convert_utf16be_to_latin1_with_errors(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {

            return convert_with_errors_impl(
                ppc64_convert_utf16_to_latin1<Endian::big>,
                scalar::utf16_to_latin1::convert_with_errors<Endian::big,
                    const char16_t*, char*>,
                buf, len, latin1_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf16be_to_latin1(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            // optimization opportunity: we could provide an optimized function.
            return convert_utf16be_to_latin1(buf, len, latin1_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf16le_to_latin1(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            // optimization opportunity: we could provide an optimized function.
            return convert_utf16le_to_latin1(buf, len, latin1_output);
        }

         [[nodiscard]] size_t implementation::convert_utf16le_to_utf8(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {

            return convert_impl(ppc64_convert_utf16_to_utf8<Endian::little>,
                scalar::utf16_to_utf8::convert<Endian::little,
                    const char16_t*, char*>,
                buf, len, utf8_output);
        }

         [[nodiscard]] size_t implementation::convert_utf16be_to_utf8(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {

            return convert_impl(
                ppc64_convert_utf16_to_utf8<Endian::big>,
                scalar::utf16_to_utf8::convert<Endian::big, const char16_t*, char*>,
                buf, len, utf8_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf16le_to_utf8_with_errors(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {

            return convert_with_errors_impl(
                ppc64_convert_utf16_to_utf8<Endian::little>,
                scalar::utf16_to_utf8::simple_convert_with_errors<Endian::little>,
                buf, len, utf8_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf16be_to_utf8_with_errors(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {

            return convert_with_errors_impl(
                ppc64_convert_utf16_to_utf8<Endian::big>,
                scalar::utf16_to_utf8::simple_convert_with_errors<Endian::big>, buf,
                len, utf8_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf16le_to_utf8(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            return convert_utf16le_to_utf8(buf, len, utf8_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf16be_to_utf8(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            return convert_utf16be_to_utf8(buf, len, utf8_output);
        }

         [[nodiscard]] size_t implementation::convert_utf32_to_latin1(
            const char32_t* buf, size_t len, char* latin1_output) const noexcept {
            return convert_impl(ppc64_convert_utf32_to_latin1<ErrorChecking::enabled>,
                scalar::utf32_to_latin1::convert, buf, len,
                latin1_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf32_to_latin1_with_errors(
            const char32_t* buf, size_t len, char* latin1_output) const noexcept {
            return convert_with_errors_impl(
                ppc64_convert_utf32_to_latin1<ErrorChecking::enabled>,
                scalar::utf32_to_latin1::convert_with_errors, buf, len, latin1_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf32_to_latin1(
            const char32_t* buf, size_t len, char* latin1_output) const noexcept {
            return convert_impl(ppc64_convert_utf32_to_latin1<ErrorChecking::disabled>,
                scalar::utf32_to_latin1::convert, buf, len,
                latin1_output);
        }

         [[nodiscard]] size_t implementation::convert_utf32_to_utf8(
            const char32_t* buf, size_t len, char* utf8_output) const noexcept {
            return convert_impl(ppc64_convert_utf32_to_utf8<ErrorReporting::at_the_end>,
                scalar::utf32_to_utf8::convert<const char32_t*, char*>,
                buf, len, utf8_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf32_to_utf8_with_errors(
            const char32_t* buf, size_t len, char* utf8_output) const noexcept {
            return convert_with_errors_impl(
                ppc64_convert_utf32_to_utf8<ErrorReporting::precise>,
                scalar::utf32_to_utf8::convert_with_errors<const char32_t*, char*>, buf,
                len, utf8_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf32_to_utf8(
            const char32_t* buf, size_t len, char* utf8_output) const noexcept {
            return convert_impl(ppc64_convert_utf32_to_utf8<ErrorReporting::none>,
                scalar::utf32_to_utf8::convert<const char32_t*, char*>,
                buf, len, utf8_output);
        }

         [[nodiscard]] size_t implementation::convert_utf32_to_utf16le(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {

            return convert_impl(ppc64_convert_utf32_to_utf16<Endian::little,
                                    ErrorReporting::at_the_end>,
                scalar::utf32_to_utf16::convert<Endian::little>, buf,
                len, utf16_output);
        }

         [[nodiscard]] size_t implementation::convert_utf32_to_utf16be(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {

            return convert_impl(
                ppc64_convert_utf32_to_utf16<Endian::big, ErrorReporting::at_the_end>,
                scalar::utf32_to_utf16::convert<Endian::big>, buf, len, utf16_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf32_to_utf16le_with_errors(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {

            return convert_with_errors_impl(
                ppc64_convert_utf32_to_utf16<Endian::little, ErrorReporting::precise>,
                scalar::utf32_to_utf16::convert_with_errors<Endian::little>, buf, len,
                utf16_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf32_to_utf16be_with_errors(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {

            return convert_with_errors_impl(
                ppc64_convert_utf32_to_utf16<Endian::big, ErrorReporting::precise>,
                scalar::utf32_to_utf16::convert_with_errors<Endian::big>, buf, len,
                utf16_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf32_to_utf16le(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {

            return convert_impl(
                ppc64_convert_utf32_to_utf16<Endian::little, ErrorReporting::none>,
                scalar::utf32_to_utf16::convert<Endian::little>, buf, len,
                utf16_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf32_to_utf16be(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {

            return convert_impl(
                ppc64_convert_utf32_to_utf16<Endian::big, ErrorReporting::none>,
                scalar::utf32_to_utf16::convert<Endian::big>, buf, len, utf16_output);
        }

         [[nodiscard]] size_t implementation::convert_utf16le_to_utf32(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return convert_impl(ppc64_convert_utf16_to_utf32<Endian::little>,
                scalar::utf16_to_utf32::convert<Endian::little>, buf,
                len, utf32_output);
        }

         [[nodiscard]] size_t implementation::convert_utf16be_to_utf32(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return convert_impl(ppc64_convert_utf16_to_utf32<Endian::big>,
                scalar::utf16_to_utf32::convert<Endian::big>, buf,
                len, utf32_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf16le_to_utf32_with_errors(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return convert_with_errors_impl(
                ppc64_convert_utf16_to_utf32<Endian::little>,
                scalar::utf16_to_utf32::convert_with_errors<Endian::little>, buf, len,
                utf32_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf16be_to_utf32_with_errors(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return convert_with_errors_impl(
                ppc64_convert_utf16_to_utf32<Endian::big>,
                scalar::utf16_to_utf32::convert_with_errors<Endian::big>, buf, len,
                utf32_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf16le_to_utf32(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return convert_utf16le_to_utf32(buf, len, utf32_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf16be_to_utf32(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return convert_utf16be_to_utf32(buf, len, utf32_output);
        }

        void implementation::change_endianness_utf16(const char16_t* input,
            size_t length,
            char16_t* output) const noexcept {
            utf16::change_endianness_utf16(input, length, output);
        }

         [[nodiscard]] size_t implementation::count_utf16le(
            const char16_t* input, size_t length) const noexcept {
            return utf16::count_code_points<Endian::little>(input, length);
        }

         [[nodiscard]] size_t implementation::count_utf16be(
            const char16_t* input, size_t length) const noexcept {
            return utf16::count_code_points<Endian::big>(input, length);
        }

         [[nodiscard]] size_t
        implementation::count_utf8(const char* input, size_t length) const noexcept {
            return utf8::count_code_points(input, length);
        }

         [[nodiscard]] size_t implementation::latin1_length_from_utf8(
            const char* buf, size_t len) const noexcept {
            return count_utf8(buf, len);
        }

         [[nodiscard]] size_t implementation::utf8_length_from_latin1(
            const char* input, size_t length) const noexcept {
            const auto ret = ppc64_utf8_length_from_latin1(input, length);
            const size_t consumed = ret.first - input;

            if (consumed == length) {
                return ret.second;
            }

            const auto scalar = scalar::latin1::utf8_length_from_latin1(ret.first, length - consumed);
            return scalar + ret.second;
        }

         [[nodiscard]] size_t implementation::utf8_length_from_utf16le(
            const char16_t* input, size_t length) const noexcept {
            return utf16::utf8_length_from_utf16<Endian::little>(input, length);
        }

         [[nodiscard]] size_t implementation::utf8_length_from_utf16be(
            const char16_t* input, size_t length) const noexcept {
            return utf16::utf8_length_from_utf16<Endian::big>(input, length);
        }

         [[nodiscard]] size_t implementation::utf32_length_from_utf16le(
            const char16_t* input, size_t length) const noexcept {
            return utf16::utf32_length_from_utf16<Endian::little>(input, length);
        }

         [[nodiscard]] size_t implementation::utf32_length_from_utf16be(
            const char16_t* input, size_t length) const noexcept {
            return utf16::utf32_length_from_utf16<Endian::big>(input, length);
        }

         [[nodiscard]] size_t implementation::utf16_length_from_utf8(
            const char* input, size_t length) const noexcept {
            return utf8::utf16_length_from_utf8(input, length);
        }
         [[nodiscard]] UnicodeResult
        implementation::utf8_length_from_utf16le_with_replacement(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::utf8_length_from_utf16_with_replacement<
                Endian::little>(input, length);
        }

         [[nodiscard]] UnicodeResult
        implementation::utf8_length_from_utf16be_with_replacement(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::utf8_length_from_utf16_with_replacement<
                Endian::big>(input, length);
        }

         [[nodiscard]] size_t
        implementation::convert_utf16le_to_utf8_with_replacement(
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
        implementation::convert_utf16be_to_utf8_with_replacement(
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


         [[nodiscard]] size_t implementation::utf8_length_from_utf32(
            const char32_t* input, size_t length) const noexcept {
            return utf32::utf8_length_from_utf32(input, length);
        }

         [[nodiscard]] size_t implementation::utf16_length_from_utf32(
            const char32_t* input, size_t length) const noexcept {
            return scalar::utf32::utf16_length_from_utf32(input, length);
        }

         [[nodiscard]] size_t implementation::utf32_length_from_utf8(
            const char* input, size_t length) const noexcept {
            return utf8::count_code_points(input, length);
        }

         [[nodiscard]] size_t implementation::maximal_binary_length_from_base64(
            const char* input, size_t length) const noexcept {
            return scalar::base64::maximal_binary_length_from_base64(input, length);
        }

         [[nodiscard]] UnicodeResult implementation::base64_to_binary(
            const char* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            if (options & base64_default_or_url) {
                if (options == Base64Options::base64_default_or_url_accept_garbage) {
                    return base64::compress_decode_base64<false, true, true>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return base64::compress_decode_base64<false, false, true>(
                        output, input, length, options, last_chunk_options);
                }
            } else if (options & base64_url) {
                if (options == Base64Options::base64_url_accept_garbage) {
                    return base64::compress_decode_base64<true, true, false>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return base64::compress_decode_base64<true, false, false>(
                        output, input, length, options, last_chunk_options);
                }
            } else {
                if (options == Base64Options::base64_default_accept_garbage) {
                    return base64::compress_decode_base64<false, true, false>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return base64::compress_decode_base64<false, false, false>(
                        output, input, length, options, last_chunk_options);
                }
            }
        }

         [[nodiscard]] full_result implementation::base64_to_binary_details(
            const char* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            if (options & base64_default_or_url) {
                if (options == Base64Options::base64_default_or_url_accept_garbage) {
                    return base64::compress_decode_base64<false, true, true>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return base64::compress_decode_base64<false, false, true>(
                        output, input, length, options, last_chunk_options);
                }
            } else if (options & base64_url) {
                if (options == Base64Options::base64_url_accept_garbage) {
                    return base64::compress_decode_base64<true, true, false>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return base64::compress_decode_base64<true, false, false>(
                        output, input, length, options, last_chunk_options);
                }
            } else {
                if (options == Base64Options::base64_default_accept_garbage) {
                    return base64::compress_decode_base64<false, true, false>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return base64::compress_decode_base64<false, false, false>(
                        output, input, length, options, last_chunk_options);
                }
            }
        }

         [[nodiscard]] UnicodeResult implementation::base64_to_binary(
            const char16_t* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            if (options & base64_default_or_url) {
                if (options == Base64Options::base64_default_or_url_accept_garbage) {
                    return base64::compress_decode_base64<false, true, true>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return base64::compress_decode_base64<false, false, true>(
                        output, input, length, options, last_chunk_options);
                }
            } else if (options & base64_url) {
                if (options == Base64Options::base64_url_accept_garbage) {
                    return base64::compress_decode_base64<true, true, false>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return base64::compress_decode_base64<true, false, false>(
                        output, input, length, options, last_chunk_options);
                }
            } else {
                if (options == Base64Options::base64_default_accept_garbage) {
                    return base64::compress_decode_base64<false, true, false>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return base64::compress_decode_base64<false, false, false>(
                        output, input, length, options, last_chunk_options);
                }
            }
        }

         [[nodiscard]] full_result implementation::base64_to_binary_details(
            const char16_t* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            if (options & base64_default_or_url) {
                if (options == Base64Options::base64_default_or_url_accept_garbage) {
                    return base64::compress_decode_base64<false, true, true>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return base64::compress_decode_base64<false, false, true>(
                        output, input, length, options, last_chunk_options);
                }
            } else if (options & base64_url) {
                if (options == Base64Options::base64_url_accept_garbage) {
                    return base64::compress_decode_base64<true, true, false>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return base64::compress_decode_base64<true, false, false>(
                        output, input, length, options, last_chunk_options);
                }
            } else {
                if (options == Base64Options::base64_default_accept_garbage) {
                    return base64::compress_decode_base64<false, true, false>(
                        output, input, length, options, last_chunk_options);
                } else {
                    return base64::compress_decode_base64<false, false, false>(
                        output, input, length, options, last_chunk_options);
                }
            }
        }

        size_t implementation::binary_to_base64(const char* input, size_t length,
            char* output,
            Base64Options options) const noexcept {
            if (options & base64_url) {
                return encode_base64<true>(output, input, length, options);
            } else {
                return encode_base64<false>(output, input, length, options);
            }
        }

        size_t implementation::binary_to_base64_with_lines(
            const char* input, size_t length, char* output, size_t line_length,
            Base64Options options) const noexcept {
            return scalar::base64::tail_encode_base64_impl<true>(output, input, length,
                options, line_length);
        }

        const char* implementation::find(const char* start, const char* end,
            char character) const noexcept {
            return util::find(start, end, character);
        }

        const char16_t* implementation::find(const char16_t* start, const char16_t* end,
            char16_t character) const noexcept {
            return util::find(start, end, character);
        }

#ifdef UNICODE_INTERNAL_TESTS
        std::vector<implementation::TestProcedure>
        implementation::internal_tests() const {
#define entry(proc) \
    TestProcedure { \
        #proc, proc \
    }
            return { entry(base64_encoding_translate_6bit_values),
                entry(base64_encoding_expand_6bit_fields),
                entry(base64_decoding_valid),
                entry(base64_decoding_invalid_ignore_errors),
                entry(base64url_decoding_invalid_ignore_errors),
                entry(base64_decoding_invalid_strict_errors),
                entry(base64url_decoding_invalid_strict_errors),
                entry(base64_decoding_pack),
                entry(base64_compress) };
#undef entry
        }
#endif

    } // namespace UNICODE_IMPLEMENTATION

    static turbo::implementation *get_ppc64_instance() {
        static ppc64::implementation ins;
        return &ins;
    }
} // namespace turbo

#include <turbo/unicode/engine/ppc64/end.h>
#else
namespace turbo {
    static turbo::implementation *get_ppc64_instance() {
        return nullptr;
    }
}
#endif

namespace turbo {
    IsaInfo get_ppc64_info() {
        static IsaInfo ins = {
            .compiled = UNICODE_IMPLEMENTATION_PPC64,
            .failback = false,
            .required_isa = static_cast<uint32_t>(InstructionSet::ALTIVEC),
            .isa_name = "ppc64",
            .engine = get_ppc64_instance(),
        };
        return ins;
    }
} // namespace turbo
