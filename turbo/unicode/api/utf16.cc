// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <turbo/unicode/api/utf16.h>
#include <turbo/unicode/scalar/utf16.h>
#include <turbo/unicode/scalar/utf16_to_utf8/utf16_to_utf8.h>
#include <turbo/unicode/engine/backend_select.h>

namespace turbo {

    simdutf_warn_unused size_t trim_partial_utf16be(const char16_t* input,
        size_t length) {
        return scalar::utf16::trim_partial_utf16<BIG>(input, length);
    }

    simdutf_warn_unused size_t trim_partial_utf16le(const char16_t* input,
        size_t length) {
        return scalar::utf16::trim_partial_utf16<LITTLE>(input, length);
    }

    simdutf_warn_unused size_t trim_partial_utf16(const char16_t* input,
        size_t length) {
#if SIMDUTF_IS_BIG_ENDIAN
        return trim_partial_utf16be(input, length);
#else
        return trim_partial_utf16le(input, length);
#endif
    }

    simdutf_warn_unused bool validate_utf16le_as_ascii(const char16_t* buf,
        size_t len) noexcept {
        return get_default_implementation()->validate_utf16le_as_ascii(buf, len);
    }
    simdutf_warn_unused bool validate_utf16be_as_ascii(const char16_t* buf,
        size_t len) noexcept {
        return get_default_implementation()->validate_utf16be_as_ascii(buf, len);
    }
    simdutf_warn_unused bool validate_utf16_as_ascii(const char16_t* input,
        size_t length) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return validate_utf16be_as_ascii(input, length);
#else
        return validate_utf16le_as_ascii(input, length);
#endif
    }

    simdutf_warn_unused bool validate_utf16(const char16_t* buf,
        size_t len) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return validate_utf16be(buf, len);
#else
        return validate_utf16le(buf, len);
#endif
    }
    void to_well_formed_utf16be(const char16_t* input, size_t len,
        char16_t* output) noexcept {
        return get_default_implementation()->to_well_formed_utf16be(input, len,
            output);
    }
    void to_well_formed_utf16le(const char16_t* input, size_t len,
        char16_t* output) noexcept {
        return get_default_implementation()->to_well_formed_utf16le(input, len,
            output);
    }
    void to_well_formed_utf16(const char16_t* input, size_t len,
        char16_t* output) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        to_well_formed_utf16be(input, len, output);
#else
        to_well_formed_utf16le(input, len, output);
#endif
    }

    simdutf_warn_unused bool validate_utf16le(const char16_t* buf,
    size_t len) noexcept {
        return get_default_implementation()->validate_utf16le(buf, len);
    }

    simdutf_warn_unused bool validate_utf16be(const char16_t* buf,
        size_t len) noexcept {
        return get_default_implementation()->validate_utf16be(buf, len);
    }
    simdutf_warn_unused result validate_utf16_with_errors(const char16_t* buf,
        size_t len) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return validate_utf16be_with_errors(buf, len);
#else
        return validate_utf16le_with_errors(buf, len);
#endif
    }
    simdutf_warn_unused result validate_utf16le_with_errors(const char16_t* buf,
        size_t len) noexcept {
        return get_default_implementation()->validate_utf16le_with_errors(buf, len);
    }
    simdutf_warn_unused result validate_utf16be_with_errors(const char16_t* buf,
        size_t len) noexcept {
        return get_default_implementation()->validate_utf16be_with_errors(buf, len);
    }

    simdutf_warn_unused size_t convert_utf16_to_utf8(const char16_t* buf,
        size_t len,
        char* utf8_buffer) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return convert_utf16be_to_utf8(buf, len, utf8_buffer);
#else
        return convert_utf16le_to_utf8(buf, len, utf8_buffer);
#endif
    }

    simdutf_warn_unused size_t
    convert_utf16_to_utf8_safe(const char16_t* buf, size_t len, char* utf8_output,
        size_t utf8_len) noexcept {
        const auto start { utf8_output };
        // We might be able to go faster by first scanning the input buffer to
        // determine how many char16_t characters we can read without exceeding the
        // utf8_len. This is a one-pass algorithm that has the benefit of not
        // requiring a first pass to determine the length.
        while (true) {
            // The worst case for convert_utf16_to_utf8 is when you go from 1 char16_t
            // to 3 characters of UTF-8. So we can read at most utf8_len / 3 char16_t
            // characters.
            auto read_len = std::min(len, utf8_len / 3);
            if (read_len <= 16) {
                break;
            }
            if (read_len < len) {
                //  If we have a high surrogate at the end of the buffer, we need to
                //  either read one more char16_t or backtrack.
                if (scalar::utf16::high_surrogate(buf[read_len - 1])) {
                    read_len--;
                }
            }
            if (read_len == 0) {
                // If we cannot read anything, we are done.
                break;
            }
            const auto write_len = turbo::convert_utf16_to_utf8(buf, read_len, utf8_output);
            if (write_len == 0) {
                // There was an error in the conversion, we cannot continue.
                return 0; // indicating failure
            }

            utf8_output += write_len;
            utf8_len -= write_len;
            buf += read_len;
            len -= read_len;
        }
#if SIMDUTF_IS_BIG_ENDIAN
        full_result r = scalar::utf16_to_utf8::convert_with_errors<endianness::BIG, true>(
            buf, len, utf8_output, utf8_len);
#else
        full_result r = scalar::utf16_to_utf8::convert_with_errors<endianness::LITTLE, true>(
            buf, len, utf8_output, utf8_len);
#endif
        if (r.error != error_code::SUCCESS && r.error != error_code::OUTPUT_BUFFER_TOO_SMALL) {
            // If there was an error, we return 0 to indicate failure.
            return 0; // indicating failure
        }
        return r.output_count + (utf8_output - start);
    }

    simdutf_warn_unused size_t convert_utf16_to_latin1(
        const char16_t* buf, size_t len, char* latin1_buffer) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return convert_utf16be_to_latin1(buf, len, latin1_buffer);
#else
        return convert_utf16le_to_latin1(buf, len, latin1_buffer);
#endif
    }

    simdutf_warn_unused size_t convert_utf16be_to_latin1(
        const char16_t* buf, size_t len, char* latin1_buffer) noexcept {
        return get_default_implementation()->convert_utf16be_to_latin1(buf, len,
            latin1_buffer);
    }
    simdutf_warn_unused size_t convert_utf16le_to_latin1(
        const char16_t* buf, size_t len, char* latin1_buffer) noexcept {
        return get_default_implementation()->convert_utf16le_to_latin1(buf, len,
            latin1_buffer);
    }
    simdutf_warn_unused size_t convert_valid_utf16be_to_latin1(
        const char16_t* buf, size_t len, char* latin1_buffer) noexcept {
        return get_default_implementation()->convert_valid_utf16be_to_latin1(
            buf, len, latin1_buffer);
    }
    simdutf_warn_unused size_t convert_valid_utf16le_to_latin1(
        const char16_t* buf, size_t len, char* latin1_buffer) noexcept {
        return get_default_implementation()->convert_valid_utf16le_to_latin1(
            buf, len, latin1_buffer);
    }
    simdutf_warn_unused result convert_utf16le_to_latin1_with_errors(
        const char16_t* buf, size_t len, char* latin1_buffer) noexcept {
        return get_default_implementation()->convert_utf16le_to_latin1_with_errors(
            buf, len, latin1_buffer);
    }
    simdutf_warn_unused result convert_utf16be_to_latin1_with_errors(
        const char16_t* buf, size_t len, char* latin1_buffer) noexcept {
        return get_default_implementation()->convert_utf16be_to_latin1_with_errors(
            buf, len, latin1_buffer);
    }

    simdutf_warn_unused size_t convert_utf16le_to_utf8(const char16_t* buf,
        size_t len,
        char* utf8_buffer) noexcept {
        return get_default_implementation()->convert_utf16le_to_utf8(buf, len,
            utf8_buffer);
    }
    simdutf_warn_unused size_t convert_utf16be_to_utf8(const char16_t* buf,
        size_t len,
        char* utf8_buffer) noexcept {
        return get_default_implementation()->convert_utf16be_to_utf8(buf, len,
            utf8_buffer);
    }
    simdutf_warn_unused result convert_utf16_to_utf8_with_errors(
        const char16_t* buf, size_t len, char* utf8_buffer) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return convert_utf16be_to_utf8_with_errors(buf, len, utf8_buffer);
#else
        return convert_utf16le_to_utf8_with_errors(buf, len, utf8_buffer);
#endif
    }

    simdutf_warn_unused result convert_utf16_to_latin1_with_errors(
       const char16_t* buf, size_t len, char* latin1_buffer) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return convert_utf16be_to_latin1_with_errors(buf, len, latin1_buffer);
#else
        return convert_utf16le_to_latin1_with_errors(buf, len, latin1_buffer);
#endif
    }


    simdutf_warn_unused result convert_utf16le_to_utf8_with_errors(
        const char16_t* buf, size_t len, char* utf8_buffer) noexcept {
        return get_default_implementation()->convert_utf16le_to_utf8_with_errors(
            buf, len, utf8_buffer);
    }
    simdutf_warn_unused result convert_utf16be_to_utf8_with_errors(
        const char16_t* buf, size_t len, char* utf8_buffer) noexcept {
        return get_default_implementation()->convert_utf16be_to_utf8_with_errors(
            buf, len, utf8_buffer);
    }
    simdutf_warn_unused size_t convert_valid_utf16_to_utf8(
        const char16_t* buf, size_t len, char* utf8_buffer) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return convert_valid_utf16be_to_utf8(buf, len, utf8_buffer);
#else
        return convert_valid_utf16le_to_utf8(buf, len, utf8_buffer);
#endif
    }

    simdutf_warn_unused size_t convert_valid_utf16_to_latin1(
        const char16_t* buf, size_t len, char* latin1_buffer) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return convert_valid_utf16be_to_latin1(buf, len, latin1_buffer);
#else
        return convert_valid_utf16le_to_latin1(buf, len, latin1_buffer);
#endif
    }

    simdutf_warn_unused size_t convert_valid_utf16le_to_utf8(
        const char16_t* buf, size_t len, char* utf8_buffer) noexcept {
        return get_default_implementation()->convert_valid_utf16le_to_utf8(
            buf, len, utf8_buffer);
    }
    simdutf_warn_unused size_t convert_valid_utf16be_to_utf8(
        const char16_t* buf, size_t len, char* utf8_buffer) noexcept {
        return get_default_implementation()->convert_valid_utf16be_to_utf8(
            buf, len, utf8_buffer);
    }

    simdutf_warn_unused size_t convert_utf32_to_utf16le(
        const char32_t* buf, size_t len, char16_t* utf16_buffer) noexcept {
        return get_default_implementation()->convert_utf32_to_utf16le(buf, len,
            utf16_buffer);
    }

    simdutf_warn_unused size_t convert_utf16_to_utf32(
        const char16_t* buf, size_t len, char32_t* utf32_buffer) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return convert_utf16be_to_utf32(buf, len, utf32_buffer);
#else
        return convert_utf16le_to_utf32(buf, len, utf32_buffer);
#endif
    }
    simdutf_warn_unused size_t convert_utf16le_to_utf32(
        const char16_t* buf, size_t len, char32_t* utf32_buffer) noexcept {
        return get_default_implementation()->convert_utf16le_to_utf32(buf, len,
            utf32_buffer);
    }
    simdutf_warn_unused size_t convert_utf16be_to_utf32(
        const char16_t* buf, size_t len, char32_t* utf32_buffer) noexcept {
        return get_default_implementation()->convert_utf16be_to_utf32(buf, len,
            utf32_buffer);
    }
    simdutf_warn_unused result convert_utf16_to_utf32_with_errors(
        const char16_t* buf, size_t len, char32_t* utf32_buffer) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return convert_utf16be_to_utf32_with_errors(buf, len, utf32_buffer);
#else
        return convert_utf16le_to_utf32_with_errors(buf, len, utf32_buffer);
#endif
    }
    simdutf_warn_unused result convert_utf16le_to_utf32_with_errors(
        const char16_t* buf, size_t len, char32_t* utf32_buffer) noexcept {
        return get_default_implementation()->convert_utf16le_to_utf32_with_errors(
            buf, len, utf32_buffer);
    }
    simdutf_warn_unused result convert_utf16be_to_utf32_with_errors(
        const char16_t* buf, size_t len, char32_t* utf32_buffer) noexcept {
        return get_default_implementation()->convert_utf16be_to_utf32_with_errors(
            buf, len, utf32_buffer);
    }
    simdutf_warn_unused size_t convert_valid_utf16_to_utf32(
        const char16_t* buf, size_t len, char32_t* utf32_buffer) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return convert_valid_utf16be_to_utf32(buf, len, utf32_buffer);
#else
        return convert_valid_utf16le_to_utf32(buf, len, utf32_buffer);
#endif
    }
    simdutf_warn_unused size_t convert_valid_utf16le_to_utf32(
        const char16_t* buf, size_t len, char32_t* utf32_buffer) noexcept {
        return get_default_implementation()->convert_valid_utf16le_to_utf32(
            buf, len, utf32_buffer);
    }
    simdutf_warn_unused size_t convert_valid_utf16be_to_utf32(
        const char16_t* buf, size_t len, char32_t* utf32_buffer) noexcept {
        return get_default_implementation()->convert_valid_utf16be_to_utf32(
            buf, len, utf32_buffer);
    }

    void change_endianness_utf16(const char16_t* input, size_t length,
        char16_t* output) noexcept {
        get_default_implementation()->change_endianness_utf16(input, length, output);
    }
    simdutf_warn_unused size_t count_utf16(const char16_t* input,
        size_t length) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return count_utf16be(input, length);
#else
        return count_utf16le(input, length);
#endif
    }
    simdutf_warn_unused size_t count_utf16le(const char16_t* input,
        size_t length) noexcept {
        return get_default_implementation()->count_utf16le(input, length);
    }
    simdutf_warn_unused size_t count_utf16be(const char16_t* input,
        size_t length) noexcept {
        return get_default_implementation()->count_utf16be(input, length);
    }

    simdutf_warn_unused size_t utf8_length_from_utf16(const char16_t* input,
        size_t length) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return utf8_length_from_utf16be(input, length);
#else
        return utf8_length_from_utf16le(input, length);
#endif
    }
    simdutf_warn_unused result utf8_length_from_utf16_with_replacement(
        const char16_t* input, size_t length) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return utf8_length_from_utf16be_with_replacement(input, length);
#else
        return utf8_length_from_utf16le_with_replacement(input, length);
#endif
    }
    simdutf_warn_unused size_t utf8_length_from_utf16le(const char16_t* input,
        size_t length) noexcept {
        return get_default_implementation()->utf8_length_from_utf16le(input, length);
    }
    simdutf_warn_unused size_t utf8_length_from_utf16be(const char16_t* input,
        size_t length) noexcept {
        return get_default_implementation()->utf8_length_from_utf16be(input, length);
    }

    simdutf_warn_unused size_t utf32_length_from_utf16(const char16_t* input,
        size_t length) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return utf32_length_from_utf16be(input, length);
#else
        return utf32_length_from_utf16le(input, length);
#endif
    }
    simdutf_warn_unused size_t utf32_length_from_utf16le(const char16_t* input,
        size_t length) noexcept {
        return get_default_implementation()->utf32_length_from_utf16le(input, length);
    }
    simdutf_warn_unused size_t utf32_length_from_utf16be(const char16_t* input,
        size_t length) noexcept {
        return get_default_implementation()->utf32_length_from_utf16be(input, length);
    }

    simdutf_warn_unused size_t utf16_length_from_utf8(const char* input,
        size_t length) noexcept {
        return get_default_implementation()->utf16_length_from_utf8(input, length);
    }
    simdutf_warn_unused result utf8_length_from_utf16le_with_replacement(
        const char16_t* input, size_t length) noexcept {
        return get_default_implementation()
            ->utf8_length_from_utf16le_with_replacement(input, length);
    }

    simdutf_warn_unused result utf8_length_from_utf16be_with_replacement(
        const char16_t* input, size_t length) noexcept {
        return get_default_implementation()
            ->utf8_length_from_utf16be_with_replacement(input, length);
    }

    simdutf_warn_unused size_t convert_utf16_to_utf8_with_replacement(
        const char16_t* input, size_t length, char* utf8_buffer) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return convert_utf16be_to_utf8_with_replacement(input, length, utf8_buffer);
#else
        return convert_utf16le_to_utf8_with_replacement(input, length, utf8_buffer);
#endif
    }

    simdutf_warn_unused size_t convert_utf16le_to_utf8_with_replacement(
        const char16_t* input, size_t length, char* utf8_buffer) noexcept {
        return get_default_implementation()->convert_utf16le_to_utf8_with_replacement(
            input, length, utf8_buffer);
    }

    simdutf_warn_unused size_t convert_utf16be_to_utf8_with_replacement(
        const char16_t* input, size_t length, char* utf8_buffer) noexcept {
        return get_default_implementation()->convert_utf16be_to_utf8_with_replacement(
            input, length, utf8_buffer);
    }


}  // namespace turbo
