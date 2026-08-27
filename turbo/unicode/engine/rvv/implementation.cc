#include <turbo/unicode/engine/rvv.h>
#include <turbo/unicode/engine/implementation.h>
#if UNICODE_IMPLEMENTATION_RVV

#include <turbo/unicode/tables/utf8_to_utf16_tables.h>
#include <turbo/unicode/tables/utf16_to_utf8_tables.h>
#include <turbo/unicode/tables/utf32_to_utf16_tables.h>


#include <turbo/unicode/engine/rvv/begin.h>
namespace turbo {
    namespace UNICODE_IMPLEMENTATION {
        namespace {
#ifndef UNICODE_RVV_H
#error "rvv.h must be included"
#endif

        } // unnamed namespace
    } // namespace UNICODE_IMPLEMENTATION
} // namespace turbo

  // transcoding from UTF-16 to UTF-8 (self-wrapping generic header, must be
// included at namespace scope zero)
#include <turbo/unicode/generic/utf16_to_utf8/utf16_to_utf8_with_replacement.h>

//
// Implementation-specific overrides
//
namespace turbo {
    namespace UNICODE_IMPLEMENTATION {
#include <turbo/unicode/engine/rvv/rvv_helpers.inl.cpp>

#include <turbo/unicode/engine/rvv/rvv_length_from.inl.cpp>
#include <turbo/unicode/engine/rvv/rvv_validate.inl.cpp>

#include <turbo/unicode/engine/rvv/rvv_latin1_to.inl.cpp>
#include <turbo/unicode/engine/rvv/rvv_utf16_to.inl.cpp>

#include <turbo/unicode/engine/rvv/rvv_utf32_to.inl.cpp>
#include <turbo/unicode/engine/rvv/rvv_utf8_to.inl.cpp>

#include <turbo/unicode/engine/rvv/rvv_base64.cpp>
#include <turbo/unicode/engine/rvv/rvv_find.cpp>

#include <turbo/unicode/engine/rvv/rvv_utf16fix.cpp>

         [[nodiscard]] int
        UnicodeImplementRvv::detect_encodings(const char* input,
            size_t length) const noexcept {
            // If there is a BOM, then we trust it.
            auto bom_encoding = turbo::BOM::check_bom(input, length);
            if (bom_encoding != TextEncoding::unspecified)
                return bom_encoding;
            // todo: reimplement as a one-pass algorithm.
            int out = 0;
            if (validate_utf8(input, length))
                out |= TextEncoding::UTF8;
            if (length % 2 == 0) {
                if (validate_utf16le(reinterpret_cast<const char16_t*>(input), length / 2))
                    out |= TextEncoding::UTF16_LE;
            }
            if (length % 4 == 0) {
                if (validate_utf32(reinterpret_cast<const char32_t*>(input), length / 4))
                    out |= TextEncoding::UTF32_LE;
            }

            return out;
        }

         [[nodiscard]] UnicodeResult UnicodeImplementRvv::base64_to_binary(
            const char* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            return turbo::scalar::base64::base64_to_binary_details_impl(
                input, length, output, options, last_chunk_options);
        }

         [[nodiscard]] UnicodeResult UnicodeImplementRvv::base64_to_binary(
            const char16_t* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            return turbo::scalar::base64::base64_to_binary_details_impl(
                input, length, output, options, last_chunk_options);
        }

         [[nodiscard]] full_result UnicodeImplementRvv::base64_to_binary_details(
            const char* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            return turbo::scalar::base64::base64_to_binary_details_impl(
                input, length, output, options, last_chunk_options);
        }

         [[nodiscard]] full_result UnicodeImplementRvv::base64_to_binary_details(
            const char16_t* input, size_t length, char* output, Base64Options options,
            last_chunk_handling_options last_chunk_options) const noexcept {
            return turbo::scalar::base64::base64_to_binary_details_impl(
                input, length, output, options, last_chunk_options);
        }

        size_t UnicodeImplementRvv::binary_to_base64(const char* input, size_t length,
            char* output,
            Base64Options options) const noexcept {
            return encode_base64(output, input, length, options);
        }

        size_t UnicodeImplementRvv::binary_to_base64_with_lines(
            const char* input, size_t length, char* output, size_t line_length,
            Base64Options options) const noexcept {
            return encode_base64_rvv<true>(output, input, length, options, line_length);
        }
         [[nodiscard]] UnicodeResult
        UnicodeImplementRvv::utf8_length_from_utf16le_with_replacement(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::utf8_length_from_utf16_with_replacement<
                Endian::little>(input, length);
        }

         [[nodiscard]] UnicodeResult
        UnicodeImplementRvv::utf8_length_from_utf16be_with_replacement(
            const char16_t* input, size_t length) const noexcept {
            return scalar::utf16::utf8_length_from_utf16_with_replacement<
                Endian::big>(input, length);
        }

         [[nodiscard]] size_t
        UnicodeImplementRvv::convert_utf16le_to_utf8_with_replacement(
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
        UnicodeImplementRvv::convert_utf16be_to_utf8_with_replacement(
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


    } // namespace UNICODE_IMPLEMENTATION

    static turbo::UnicodeImplement *get_rvv_instance() {
        static rvv::UnicodeImplementRvv ins;
        return &ins;
    }
} // namespace turbo

#include <turbo/unicode/engine/rvv/end.h>
#else
namespace turbo {
    static turbo::UnicodeImplement *get_rvv_instance() {
        return nullptr;
    }
}
#endif

namespace turbo {
    IsaInfo get_rvv_info() {
        static IsaInfo ins = {
            UNICODE_IMPLEMENTATION_RVV == 1,
            false,
            { kRiscvV },
             "rvv",
            get_rvv_instance(),
        };
        return ins;
    }
} // namespace turbo
