
#include <turbo/unicode/engine/fallback.h>
#if SIMDUTF_IMPLEMENTATION_FALLBACK

#include <turbo/unicode/tables/utf8_to_utf16_tables.h>
#include <turbo/unicode/tables/utf16_to_utf8_tables.h>
#include <turbo/unicode/tables/utf32_to_utf16_tables.h>


#include <turbo/unicode/engine/fallback/begin.h>

namespace turbo {
    namespace SIMDUTF_IMPLEMENTATION {

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
            return scalar::utf8::validate(buf, len);
        }

         [[nodiscard]] UnicodeResult implementation::validate_utf8_with_errors(
            const char* buf, size_t len) const noexcept {
            return scalar::utf8::validate_with_errors(buf, len);
        }

         [[nodiscard]] bool
        implementation::validate_ascii(const char* buf, size_t len) const noexcept {
            return scalar::ascii::validate(buf, len);
        }

         [[nodiscard]] UnicodeResult implementation::validate_ascii_with_errors(
            const char* buf, size_t len) const noexcept {
            return scalar::ascii::validate_with_errors(buf, len);
        }

         [[nodiscard]] bool
        implementation::validate_utf16le_as_ascii(const char16_t* buf,
            size_t len) const noexcept {
            return scalar::utf16::validate_as_ascii<endianness::LITTLE>(buf, len);
        }

         [[nodiscard]] bool
        implementation::validate_utf16be_as_ascii(const char16_t* buf,
            size_t len) const noexcept {
            return scalar::utf16::validate_as_ascii<endianness::BIG>(buf, len);
        }
         [[nodiscard]] bool
        implementation::validate_utf16le(const char16_t* buf,
            size_t len) const noexcept {
            return scalar::utf16::validate<endianness::LITTLE>(buf, len);
        }

         [[nodiscard]] bool
        implementation::validate_utf16be(const char16_t* buf,
            size_t len) const noexcept {
            return scalar::utf16::validate<endianness::BIG>(buf, len);
        }

         [[nodiscard]] UnicodeResult implementation::validate_utf16le_with_errors(
            const char16_t* buf, size_t len) const noexcept {
            return scalar::utf16::validate_with_errors<endianness::LITTLE>(buf, len);
        }

         [[nodiscard]] UnicodeResult implementation::validate_utf16be_with_errors(
            const char16_t* buf, size_t len) const noexcept {
            return scalar::utf16::validate_with_errors<endianness::BIG>(buf, len);
        }

        void implementation::to_well_formed_utf16le(const char16_t* input, size_t len,
            char16_t* output) const noexcept {
            return scalar::utf16::to_well_formed_utf16<endianness::LITTLE>(input, len,
                output);
        }

        void implementation::to_well_formed_utf16be(const char16_t* input, size_t len,
            char16_t* output) const noexcept {
            return scalar::utf16::to_well_formed_utf16<endianness::BIG>(input, len,
                output);
        }

         [[nodiscard]] bool
        implementation::validate_utf32(const char32_t* buf, size_t len) const noexcept {
            return scalar::utf32::validate(buf, len);
        }

         [[nodiscard]] UnicodeResult implementation::validate_utf32_with_errors(
            const char32_t* buf, size_t len) const noexcept {
            return scalar::utf32::validate_with_errors(buf, len);
        }

         [[nodiscard]] size_t implementation::convert_latin1_to_utf8(
            const char* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::latin1_to_utf8::convert(buf, len, utf8_output);
        }

         [[nodiscard]] size_t implementation::convert_latin1_to_utf16le(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::latin1_to_utf16::convert<endianness::LITTLE>(buf, len,
                utf16_output);
        }

         [[nodiscard]] size_t implementation::convert_latin1_to_utf16be(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::latin1_to_utf16::convert<endianness::BIG>(buf, len,
                utf16_output);
        }

         [[nodiscard]] size_t implementation::convert_latin1_to_utf32(
            const char* buf, size_t len, char32_t* utf32_output) const noexcept {
            return scalar::latin1_to_utf32::convert(buf, len, utf32_output);
        }

         [[nodiscard]] size_t implementation::convert_utf8_to_latin1(
            const char* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf8_to_latin1::convert(buf, len, latin1_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf8_to_latin1_with_errors(
            const char* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf8_to_latin1::convert_with_errors(buf, len, latin1_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf8_to_latin1(
            const char* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf8_to_latin1::convert_valid(buf, len, latin1_output);
        }

         [[nodiscard]] size_t implementation::convert_utf8_to_utf16le(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf8_to_utf16::convert<endianness::LITTLE>(buf, len,
                utf16_output);
        }

         [[nodiscard]] size_t implementation::convert_utf8_to_utf16be(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf8_to_utf16::convert<endianness::BIG>(buf, len,
                utf16_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf8_to_utf16le_with_errors(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf8_to_utf16::convert_with_errors<endianness::LITTLE>(
                buf, len, utf16_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf8_to_utf16be_with_errors(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf8_to_utf16::convert_with_errors<endianness::BIG>(
                buf, len, utf16_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf8_to_utf16le(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf8_to_utf16::convert_valid<endianness::LITTLE>(buf, len,
                utf16_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf8_to_utf16be(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf8_to_utf16::convert_valid<endianness::BIG>(buf, len,
                utf16_output);
        }

         [[nodiscard]] size_t implementation::convert_utf8_to_utf32(
            const char* buf, size_t len, char32_t* utf32_output) const noexcept {
            return scalar::utf8_to_utf32::convert(buf, len, utf32_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf8_to_utf32_with_errors(
            const char* buf, size_t len, char32_t* utf32_output) const noexcept {
            return scalar::utf8_to_utf32::convert_with_errors(buf, len, utf32_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf8_to_utf32(
            const char* input, size_t size, char32_t* utf32_output) const noexcept {
            return scalar::utf8_to_utf32::convert_valid(input, size, utf32_output);
        }

         [[nodiscard]] size_t implementation::convert_utf16le_to_latin1(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf16_to_latin1::convert<endianness::LITTLE>(buf, len,
                latin1_output);
        }

         [[nodiscard]] size_t implementation::convert_utf16be_to_latin1(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf16_to_latin1::convert<endianness::BIG>(buf, len,
                latin1_output);
        }

         [[nodiscard]] UnicodeResult
        implementation::convert_utf16le_to_latin1_with_errors(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf16_to_latin1::convert_with_errors<endianness::LITTLE>(
                buf, len, latin1_output);
        }

         [[nodiscard]] UnicodeResult
        implementation::convert_utf16be_to_latin1_with_errors(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf16_to_latin1::convert_with_errors<endianness::BIG>(
                buf, len, latin1_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf16le_to_latin1(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf16_to_latin1::convert_valid<endianness::LITTLE>(
                buf, len, latin1_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf16be_to_latin1(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf16_to_latin1::convert_valid<endianness::BIG>(buf, len,
                latin1_output);
        }

         [[nodiscard]] size_t implementation::convert_utf16le_to_utf8(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::utf16_to_utf8::convert<endianness::LITTLE>(buf, len,
                utf8_output);
        }

         [[nodiscard]] size_t implementation::convert_utf16be_to_utf8(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::utf16_to_utf8::convert<endianness::BIG>(buf, len, utf8_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf16le_to_utf8_with_errors(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::utf16_to_utf8::convert_with_errors<endianness::LITTLE>(
                buf, len, utf8_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf16be_to_utf8_with_errors(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::utf16_to_utf8::convert_with_errors<endianness::BIG>(
                buf, len, utf8_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf16le_to_utf8(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::utf16_to_utf8::convert_valid<endianness::LITTLE>(buf, len,
                utf8_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf16be_to_utf8(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::utf16_to_utf8::convert_valid<endianness::BIG>(buf, len,
                utf8_output);
        }

         [[nodiscard]] size_t implementation::convert_utf32_to_latin1(
            const char32_t* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf32_to_latin1::convert(buf, len, latin1_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf32_to_latin1_with_errors(
            const char32_t* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf32_to_latin1::convert_with_errors(buf, len, latin1_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf32_to_latin1(
            const char32_t* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf32_to_latin1::convert_valid(buf, len, latin1_output);
        }

         [[nodiscard]] size_t implementation::convert_utf32_to_utf8(
            const char32_t* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::utf32_to_utf8::convert(buf, len, utf8_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf32_to_utf8_with_errors(
            const char32_t* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::utf32_to_utf8::convert_with_errors(buf, len, utf8_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf32_to_utf8(
            const char32_t* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::utf32_to_utf8::convert_valid(buf, len, utf8_output);
        }

         [[nodiscard]] size_t implementation::convert_utf32_to_utf16le(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf32_to_utf16::convert<endianness::LITTLE>(buf, len,
                utf16_output);
        }

         [[nodiscard]] size_t implementation::convert_utf32_to_utf16be(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf32_to_utf16::convert<endianness::BIG>(buf, len,
                utf16_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf32_to_utf16le_with_errors(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf32_to_utf16::convert_with_errors<endianness::LITTLE>(
                buf, len, utf16_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf32_to_utf16be_with_errors(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf32_to_utf16::convert_with_errors<endianness::BIG>(
                buf, len, utf16_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf32_to_utf16le(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf32_to_utf16::convert_valid<endianness::LITTLE>(
                buf, len, utf16_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf32_to_utf16be(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf32_to_utf16::convert_valid<endianness::BIG>(buf, len,
                utf16_output);
        }

         [[nodiscard]] size_t implementation::convert_utf16le_to_utf32(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return scalar::utf16_to_utf32::convert<endianness::LITTLE>(buf, len,
                utf32_output);
        }

         [[nodiscard]] size_t implementation::convert_utf16be_to_utf32(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return scalar::utf16_to_utf32::convert<endianness::BIG>(buf, len,
                utf32_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf16le_to_utf32_with_errors(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return scalar::utf16_to_utf32::convert_with_errors<endianness::LITTLE>(
                buf, len, utf32_output);
        }

         [[nodiscard]] UnicodeResult implementation::convert_utf16be_to_utf32_with_errors(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return scalar::utf16_to_utf32::convert_with_errors<endianness::BIG>(
                buf, len, utf32_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf16le_to_utf32(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return scalar::utf16_to_utf32::convert_valid<endianness::LITTLE>(
                buf, len, utf32_output);
        }

         [[nodiscard]] size_t implementation::convert_valid_utf16be_to_utf32(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return scalar::utf16_to_utf32::convert_valid<endianness::BIG>(buf, len,
                utf32_output);
        }

        void implementation::change_endianness_utf16(const char16_t* input,
            size_t length,
            char16_t* output) const noexcept {
            scalar::utf16::change_endianness_utf16(input, length, output);
        }

         [[nodiscard]] size_t implementation::count_utf16le(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::count_code_points<endianness::LITTLE>(input, length);
        }

         [[nodiscard]] size_t implementation::count_utf16be(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::count_code_points<endianness::BIG>(input, length);
        }

         [[nodiscard]] size_t
        implementation::count_utf8(const char* input, size_t length) const noexcept {
            return scalar::utf8::count_code_points(input, length);
        }

         [[nodiscard]] size_t implementation::latin1_length_from_utf8(
            const char* buf, size_t len) const noexcept {
            return scalar::utf8::count_code_points(buf, len);
        }

         [[nodiscard]] size_t implementation::utf8_length_from_latin1(
            const char* input, size_t length) const noexcept {
            return scalar::latin1_to_utf8::utf8_length_from_latin1(input, length);
        }

         [[nodiscard]] size_t implementation::utf8_length_from_utf16le(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::utf8_length_from_utf16<endianness::LITTLE>(input,
                length);
        }

         [[nodiscard]] size_t implementation::utf8_length_from_utf16be(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::utf8_length_from_utf16<endianness::BIG>(input, length);
        }

         [[nodiscard]] size_t implementation::utf32_length_from_utf16le(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::utf32_length_from_utf16<endianness::LITTLE>(input,
                length);
        }

         [[nodiscard]] size_t implementation::utf32_length_from_utf16be(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::utf32_length_from_utf16<endianness::BIG>(input, length);
        }

         [[nodiscard]] size_t implementation::utf16_length_from_utf8(
            const char* input, size_t length) const noexcept {
            return scalar::utf8::utf16_length_from_utf8(input, length);
        }
         [[nodiscard]] UnicodeResult
        implementation::utf8_length_from_utf16le_with_replacement(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::utf8_length_from_utf16_with_replacement<
                endianness::LITTLE>(input, length);
        }

         [[nodiscard]] UnicodeResult
        implementation::utf8_length_from_utf16be_with_replacement(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::utf8_length_from_utf16_with_replacement<
                endianness::BIG>(input, length);
        }

         [[nodiscard]] size_t
        implementation::convert_utf16le_to_utf8_with_replacement(
            const char16_t* input, size_t length, char* utf8_buffer) const noexcept {
            return scalar::utf16_to_utf8::convert_with_replacement<endianness::LITTLE>(
                input, length, utf8_buffer);
        }

         [[nodiscard]] size_t
        implementation::convert_utf16be_to_utf8_with_replacement(
            const char16_t* input, size_t length, char* utf8_buffer) const noexcept {
            return scalar::utf16_to_utf8::convert_with_replacement<endianness::BIG>(
                input, length, utf8_buffer);
        }


         [[nodiscard]] size_t implementation::utf8_length_from_utf32(
            const char32_t* input, size_t length) const noexcept {
            return scalar::utf32::utf8_length_from_utf32(input, length);
        }

         [[nodiscard]] size_t implementation::utf16_length_from_utf32(
            const char32_t* input, size_t length) const noexcept {
            return scalar::utf32::utf16_length_from_utf32(input, length);
        }

         [[nodiscard]] size_t implementation::utf32_length_from_utf8(
            const char* input, size_t length) const noexcept {
            return scalar::utf8::count_code_points(input, length);
        }


         [[nodiscard]] UnicodeResult implementation::base64_to_binary(
            const char* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            return turbo::scalar::base64::base64_to_binary_details_impl(
                input, length, output, options, last_chunk_options);
        }

         [[nodiscard]] UnicodeResult implementation::base64_to_binary(
            const char16_t* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            return turbo::scalar::base64::base64_to_binary_details_impl(
                input, length, output, options, last_chunk_options);
        }

         [[nodiscard]] full_result implementation::base64_to_binary_details(
            const char* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            return turbo::scalar::base64::base64_to_binary_details_impl(
                input, length, output, options, last_chunk_options);
        }

         [[nodiscard]] full_result implementation::base64_to_binary_details(
            const char16_t* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            return turbo::scalar::base64::base64_to_binary_details_impl(
                input, length, output, options, last_chunk_options);
        }

        size_t implementation::binary_to_base64(const char* input, size_t length,
            char* output,
            Base64Options options) const noexcept {
            return scalar::base64::tail_encode_base64(output, input, length, options);
        }

        size_t implementation::binary_to_base64_with_lines(
            const char* input, size_t length, char* output, size_t line_length,
            Base64Options options) const noexcept {
            return scalar::base64::tail_encode_base64_impl<true>(output, input, length,
                options, line_length);
        }

        const char* implementation::find(const char* start, const char* end,
            char character) const noexcept {
            for (; start < end; ++start) {
                if (*start == character) {
                    return start;
                }
            }
            return end;
        }

        const char16_t* implementation::find(const char16_t* start, const char16_t* end,
            char16_t character) const noexcept {
            for (; start < end; ++start) {
                if (*start == character) {
                    return start;
                }
            }
            return end;
        }

    } // namespace SIMDUTF_IMPLEMENTATION
} // namespace turbo

#include <turbo/unicode/engine/fallback/end.h>
#endif
