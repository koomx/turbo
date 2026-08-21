
#include <turbo/unicode/engine/fallback.h>
#include <turbo/unicode/engine/implementation.h>

#include <turbo/unicode/tables/utf8_to_utf16_tables.h>
#include <turbo/unicode/tables/utf16_to_utf8_tables.h>
#include <turbo/unicode/tables/utf32_to_utf16_tables.h>


#include <turbo/unicode/engine/fallback/begin.h>

namespace turbo {
    namespace UNICODE_IMPLEMENTATION {

         [[nodiscard]] int
        UnicodeImplementFallback::detect_encodings(const char* input,
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
        UnicodeImplementFallback::validate_utf8(const char* buf, size_t len) const noexcept {
            return scalar::utf8::validate(buf, len);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementFallback::validate_utf8_with_errors(
            const char* buf, size_t len) const noexcept {
            return scalar::utf8::validate_with_errors(buf, len);
        }

         [[nodiscard]] bool
        UnicodeImplementFallback::validate_ascii(const char* buf, size_t len) const noexcept {
            return scalar::ascii::validate(buf, len);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementFallback::validate_ascii_with_errors(
            const char* buf, size_t len) const noexcept {
            return scalar::ascii::validate_with_errors(buf, len);
        }

         [[nodiscard]] bool
        UnicodeImplementFallback::validate_utf16le_as_ascii(const char16_t* buf,
            size_t len) const noexcept {
            return scalar::utf16::validate_as_ascii<Endian::little>(buf, len);
        }

         [[nodiscard]] bool
        UnicodeImplementFallback::validate_utf16be_as_ascii(const char16_t* buf,
            size_t len) const noexcept {
            return scalar::utf16::validate_as_ascii<Endian::big>(buf, len);
        }
         [[nodiscard]] bool
        UnicodeImplementFallback::validate_utf16le(const char16_t* buf,
            size_t len) const noexcept {
            return scalar::utf16::validate<Endian::little>(buf, len);
        }

         [[nodiscard]] bool
        UnicodeImplementFallback::validate_utf16be(const char16_t* buf,
            size_t len) const noexcept {
            return scalar::utf16::validate<Endian::big>(buf, len);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementFallback::validate_utf16le_with_errors(
            const char16_t* buf, size_t len) const noexcept {
            return scalar::utf16::validate_with_errors<Endian::little>(buf, len);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementFallback::validate_utf16be_with_errors(
            const char16_t* buf, size_t len) const noexcept {
            return scalar::utf16::validate_with_errors<Endian::big>(buf, len);
        }

        void UnicodeImplementFallback::to_well_formed_utf16le(const char16_t* input, size_t len,
            char16_t* output) const noexcept {
            return scalar::utf16::to_well_formed_utf16<Endian::little>(input, len,
                output);
        }

        void UnicodeImplementFallback::to_well_formed_utf16be(const char16_t* input, size_t len,
            char16_t* output) const noexcept {
            return scalar::utf16::to_well_formed_utf16<Endian::big>(input, len,
                output);
        }

         [[nodiscard]] bool
        UnicodeImplementFallback::validate_utf32(const char32_t* buf, size_t len) const noexcept {
            return scalar::utf32::validate(buf, len);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementFallback::validate_utf32_with_errors(
            const char32_t* buf, size_t len) const noexcept {
            return scalar::utf32::validate_with_errors(buf, len);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_latin1_to_utf8(
            const char* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::latin1_to_utf8::convert(buf, len, utf8_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_latin1_to_utf16le(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::latin1_to_utf16::convert<Endian::little>(buf, len,
                utf16_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_latin1_to_utf16be(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::latin1_to_utf16::convert<Endian::big>(buf, len,
                utf16_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_latin1_to_utf32(
            const char* buf, size_t len, char32_t* utf32_output) const noexcept {
            return scalar::latin1_to_utf32::convert(buf, len, utf32_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_utf8_to_latin1(
            const char* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf8_to_latin1::convert(buf, len, latin1_output);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementFallback::convert_utf8_to_latin1_with_errors(
            const char* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf8_to_latin1::convert_with_errors(buf, len, latin1_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_valid_utf8_to_latin1(
            const char* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf8_to_latin1::convert_valid(buf, len, latin1_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_utf8_to_utf16le(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf8_to_utf16::convert<Endian::little>(buf, len,
                utf16_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_utf8_to_utf16be(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf8_to_utf16::convert<Endian::big>(buf, len,
                utf16_output);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementFallback::convert_utf8_to_utf16le_with_errors(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf8_to_utf16::convert_with_errors<Endian::little>(
                buf, len, utf16_output);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementFallback::convert_utf8_to_utf16be_with_errors(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf8_to_utf16::convert_with_errors<Endian::big>(
                buf, len, utf16_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_valid_utf8_to_utf16le(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf8_to_utf16::convert_valid<Endian::little>(buf, len,
                utf16_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_valid_utf8_to_utf16be(
            const char* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf8_to_utf16::convert_valid<Endian::big>(buf, len,
                utf16_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_utf8_to_utf32(
            const char* buf, size_t len, char32_t* utf32_output) const noexcept {
            return scalar::utf8_to_utf32::convert(buf, len, utf32_output);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementFallback::convert_utf8_to_utf32_with_errors(
            const char* buf, size_t len, char32_t* utf32_output) const noexcept {
            return scalar::utf8_to_utf32::convert_with_errors(buf, len, utf32_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_valid_utf8_to_utf32(
            const char* input, size_t size, char32_t* utf32_output) const noexcept {
            return scalar::utf8_to_utf32::convert_valid(input, size, utf32_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_utf16le_to_latin1(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf16_to_latin1::convert<Endian::little>(buf, len,
                latin1_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_utf16be_to_latin1(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf16_to_latin1::convert<Endian::big>(buf, len,
                latin1_output);
        }

         [[nodiscard]] UnicodeResult
        UnicodeImplementFallback::convert_utf16le_to_latin1_with_errors(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf16_to_latin1::convert_with_errors<Endian::little>(
                buf, len, latin1_output);
        }

         [[nodiscard]] UnicodeResult
        UnicodeImplementFallback::convert_utf16be_to_latin1_with_errors(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf16_to_latin1::convert_with_errors<Endian::big>(
                buf, len, latin1_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_valid_utf16le_to_latin1(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf16_to_latin1::convert_valid<Endian::little>(
                buf, len, latin1_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_valid_utf16be_to_latin1(
            const char16_t* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf16_to_latin1::convert_valid<Endian::big>(buf, len,
                latin1_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_utf16le_to_utf8(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::utf16_to_utf8::convert<Endian::little>(buf, len,
                utf8_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_utf16be_to_utf8(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::utf16_to_utf8::convert<Endian::big>(buf, len, utf8_output);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementFallback::convert_utf16le_to_utf8_with_errors(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::utf16_to_utf8::convert_with_errors<Endian::little>(
                buf, len, utf8_output);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementFallback::convert_utf16be_to_utf8_with_errors(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::utf16_to_utf8::convert_with_errors<Endian::big>(
                buf, len, utf8_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_valid_utf16le_to_utf8(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::utf16_to_utf8::convert_valid<Endian::little>(buf, len,
                utf8_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_valid_utf16be_to_utf8(
            const char16_t* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::utf16_to_utf8::convert_valid<Endian::big>(buf, len,
                utf8_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_utf32_to_latin1(
            const char32_t* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf32_to_latin1::convert(buf, len, latin1_output);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementFallback::convert_utf32_to_latin1_with_errors(
            const char32_t* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf32_to_latin1::convert_with_errors(buf, len, latin1_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_valid_utf32_to_latin1(
            const char32_t* buf, size_t len, char* latin1_output) const noexcept {
            return scalar::utf32_to_latin1::convert_valid(buf, len, latin1_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_utf32_to_utf8(
            const char32_t* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::utf32_to_utf8::convert(buf, len, utf8_output);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementFallback::convert_utf32_to_utf8_with_errors(
            const char32_t* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::utf32_to_utf8::convert_with_errors(buf, len, utf8_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_valid_utf32_to_utf8(
            const char32_t* buf, size_t len, char* utf8_output) const noexcept {
            return scalar::utf32_to_utf8::convert_valid(buf, len, utf8_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_utf32_to_utf16le(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf32_to_utf16::convert<Endian::little>(buf, len,
                utf16_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_utf32_to_utf16be(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf32_to_utf16::convert<Endian::big>(buf, len,
                utf16_output);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementFallback::convert_utf32_to_utf16le_with_errors(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf32_to_utf16::convert_with_errors<Endian::little>(
                buf, len, utf16_output);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementFallback::convert_utf32_to_utf16be_with_errors(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf32_to_utf16::convert_with_errors<Endian::big>(
                buf, len, utf16_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_valid_utf32_to_utf16le(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf32_to_utf16::convert_valid<Endian::little>(
                buf, len, utf16_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_valid_utf32_to_utf16be(
            const char32_t* buf, size_t len, char16_t* utf16_output) const noexcept {
            return scalar::utf32_to_utf16::convert_valid<Endian::big>(buf, len,
                utf16_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_utf16le_to_utf32(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return scalar::utf16_to_utf32::convert<Endian::little>(buf, len,
                utf32_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_utf16be_to_utf32(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return scalar::utf16_to_utf32::convert<Endian::big>(buf, len,
                utf32_output);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementFallback::convert_utf16le_to_utf32_with_errors(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return scalar::utf16_to_utf32::convert_with_errors<Endian::little>(
                buf, len, utf32_output);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementFallback::convert_utf16be_to_utf32_with_errors(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return scalar::utf16_to_utf32::convert_with_errors<Endian::big>(
                buf, len, utf32_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_valid_utf16le_to_utf32(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return scalar::utf16_to_utf32::convert_valid<Endian::little>(
                buf, len, utf32_output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::convert_valid_utf16be_to_utf32(
            const char16_t* buf, size_t len, char32_t* utf32_output) const noexcept {
            return scalar::utf16_to_utf32::convert_valid<Endian::big>(buf, len,
                utf32_output);
        }

        void UnicodeImplementFallback::change_endianness_utf16(const char16_t* input,
            size_t length,
            char16_t* output) const noexcept {
            scalar::utf16::change_endianness_utf16(input, length, output);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::count_utf16le(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::count_code_points<Endian::little>(input, length);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::count_utf16be(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::count_code_points<Endian::big>(input, length);
        }

         [[nodiscard]] size_t
        UnicodeImplementFallback::count_utf8(const char* input, size_t length) const noexcept {
            return scalar::utf8::count_code_points(input, length);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::latin1_length_from_utf8(
            const char* buf, size_t len) const noexcept {
            return scalar::utf8::count_code_points(buf, len);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::utf8_length_from_latin1(
            const char* input, size_t length) const noexcept {
            return scalar::latin1_to_utf8::utf8_length_from_latin1(input, length);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::utf8_length_from_utf16le(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::utf8_length_from_utf16<Endian::little>(input,
                length);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::utf8_length_from_utf16be(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::utf8_length_from_utf16<Endian::big>(input, length);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::utf32_length_from_utf16le(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::utf32_length_from_utf16<Endian::little>(input,
                length);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::utf32_length_from_utf16be(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::utf32_length_from_utf16<Endian::big>(input, length);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::utf16_length_from_utf8(
            const char* input, size_t length) const noexcept {
            return scalar::utf8::utf16_length_from_utf8(input, length);
        }
         [[nodiscard]] UnicodeResult
        UnicodeImplementFallback::utf8_length_from_utf16le_with_replacement(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::utf8_length_from_utf16_with_replacement<
                Endian::little>(input, length);
        }

         [[nodiscard]] UnicodeResult
        UnicodeImplementFallback::utf8_length_from_utf16be_with_replacement(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::utf8_length_from_utf16_with_replacement<
                Endian::big>(input, length);
        }

         [[nodiscard]] size_t
        UnicodeImplementFallback::convert_utf16le_to_utf8_with_replacement(
            const char16_t* input, size_t length, char* utf8_buffer) const noexcept {
            return scalar::utf16_to_utf8::convert_with_replacement<Endian::little>(
                input, length, utf8_buffer);
        }

         [[nodiscard]] size_t
        UnicodeImplementFallback::convert_utf16be_to_utf8_with_replacement(
            const char16_t* input, size_t length, char* utf8_buffer) const noexcept {
            return scalar::utf16_to_utf8::convert_with_replacement<Endian::big>(
                input, length, utf8_buffer);
        }


         [[nodiscard]] size_t UnicodeImplementFallback::utf8_length_from_utf32(
            const char32_t* input, size_t length) const noexcept {
            return scalar::utf32::utf8_length_from_utf32(input, length);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::utf16_length_from_utf32(
            const char32_t* input, size_t length) const noexcept {
            return scalar::utf32::utf16_length_from_utf32(input, length);
        }

         [[nodiscard]] size_t UnicodeImplementFallback::utf32_length_from_utf8(
            const char* input, size_t length) const noexcept {
            return scalar::utf8::count_code_points(input, length);
        }


         [[nodiscard]] UnicodeResult UnicodeImplementFallback::base64_to_binary(
            const char* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            return turbo::scalar::base64::base64_to_binary_details_impl(
                input, length, output, options, last_chunk_options);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementFallback::base64_to_binary(
            const char16_t* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            return turbo::scalar::base64::base64_to_binary_details_impl(
                input, length, output, options, last_chunk_options);
        }

         [[nodiscard]] full_result UnicodeImplementFallback::base64_to_binary_details(
            const char* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            return turbo::scalar::base64::base64_to_binary_details_impl(
                input, length, output, options, last_chunk_options);
        }

         [[nodiscard]] full_result UnicodeImplementFallback::base64_to_binary_details(
            const char16_t* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            return turbo::scalar::base64::base64_to_binary_details_impl(
                input, length, output, options, last_chunk_options);
        }

        size_t UnicodeImplementFallback::binary_to_base64(const char* input, size_t length,
            char* output,
            Base64Options options) const noexcept {
            return scalar::base64::tail_encode_base64(output, input, length, options);
        }

        size_t UnicodeImplementFallback::binary_to_base64_with_lines(
            const char* input, size_t length, char* output, size_t line_length,
            Base64Options options) const noexcept {
            return scalar::base64::tail_encode_base64_impl<true>(output, input, length,
                options, line_length);
        }

        const char* UnicodeImplementFallback::find(const char* start, const char* end,
            char character) const noexcept {
            for (; start < end; ++start) {
                if (*start == character) {
                    return start;
                }
            }
            return end;
        }

        const char16_t* UnicodeImplementFallback::find(const char16_t* start, const char16_t* end,
            char16_t character) const noexcept {
            for (; start < end; ++start) {
                if (*start == character) {
                    return start;
                }
            }
            return end;
        }

    } // namespace UNICODE_IMPLEMENTATION
    static turbo::UnicodeImplement *get_fallback_instance() {
       static turbo::fallback::UnicodeImplementFallback ins;
        return &ins;
    }

} // namespace turbo

#include <turbo/unicode/engine/fallback/end.h>


namespace turbo {
    IsaInfo get_fallback_info() {
        static IsaInfo ins = {
            .compiled = UNICODE_IMPLEMENTATION_FALLBACK == 1,
            .failback = true,
            .required_isa = static_cast<uint32_t>(InstructionSet::NEON),
            .isa_name ="fallback",
            .engine = get_fallback_instance(),
       };
        return ins;
    }
} // namespace turbo
