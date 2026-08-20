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

#include <turbo/unicode/api/utf8.h>
#include <turbo/unicode/scalar/utf8.h>
#include <turbo/unicode/engine/backend_select.h>

namespace turbo {

     [[nodiscard]] size_t trim_partial_utf8(const char* input, size_t length) {
        return scalar::utf8::trim_partial_utf8(input, length);
    }

     [[nodiscard]] bool validate_utf8(const char* buf, size_t len) noexcept {
        return get_default_implementation()->validate_utf8(buf, len);
    }

     [[nodiscard]] UnicodeResult validate_utf8_with_errors(const char* buf,
    size_t len) noexcept {
        return get_default_implementation()->validate_utf8_with_errors(buf, len);
    }

     [[nodiscard]] size_t convert_utf8_to_latin1(
        const char* buf, size_t len, char* latin1_output) noexcept {
        return get_default_implementation()->convert_utf8_to_latin1(buf, len,
            latin1_output);
    }
     [[nodiscard]] UnicodeResult convert_utf8_to_latin1_with_errors(
        const char* buf, size_t len, char* latin1_output) noexcept {
        return get_default_implementation()->convert_utf8_to_latin1_with_errors(
            buf, len, latin1_output);
    }
     [[nodiscard]] size_t convert_valid_utf8_to_latin1(
        const char* buf, size_t len, char* latin1_output) noexcept {
        return get_default_implementation()->convert_valid_utf8_to_latin1(
            buf, len, latin1_output);
    }

     [[nodiscard]] size_t convert_utf8_to_utf16le(
       const char* input, size_t length, char16_t* utf16_output) noexcept {
        return get_default_implementation()->convert_utf8_to_utf16le(input, length,
            utf16_output);
    }
     [[nodiscard]] size_t convert_utf8_to_utf16be(
        const char* input, size_t length, char16_t* utf16_output) noexcept {
        return get_default_implementation()->convert_utf8_to_utf16be(input, length,
            utf16_output);
    }
     [[nodiscard]] UnicodeResult convert_utf8_to_utf16_with_errors(
        const char* input, size_t length, char16_t* utf16_output) noexcept {
#if KUMO_ENDIAN_BIG
        return convert_utf8_to_utf16be_with_errors(input, length, utf16_output);
#else
        return convert_utf8_to_utf16le_with_errors(input, length, utf16_output);
#endif
    }
     [[nodiscard]] UnicodeResult convert_utf8_to_utf16le_with_errors(
        const char* input, size_t length, char16_t* utf16_output) noexcept {
        return get_default_implementation()->convert_utf8_to_utf16le_with_errors(
            input, length, utf16_output);
    }
     [[nodiscard]] UnicodeResult convert_utf8_to_utf16be_with_errors(
        const char* input, size_t length, char16_t* utf16_output) noexcept {
        return get_default_implementation()->convert_utf8_to_utf16be_with_errors(
            input, length, utf16_output);
    }

     [[nodiscard]] size_t convert_utf8_to_utf32(
        const char* input, size_t length, char32_t* utf32_output) noexcept {
        return get_default_implementation()->convert_utf8_to_utf32(input, length,
            utf32_output);
    }
     [[nodiscard]] UnicodeResult convert_utf8_to_utf32_with_errors(
        const char* input, size_t length, char32_t* utf32_output) noexcept {
        return get_default_implementation()->convert_utf8_to_utf32_with_errors(
            input, length, utf32_output);
    }

     [[nodiscard]] size_t convert_valid_utf8_to_utf16(
        const char* input, size_t length, char16_t* utf16_buffer) noexcept {
#if KUMO_ENDIAN_BIG
        return convert_valid_utf8_to_utf16be(input, length, utf16_buffer);
#else
        return convert_valid_utf8_to_utf16le(input, length, utf16_buffer);
#endif
    }
     [[nodiscard]] size_t convert_valid_utf8_to_utf16le(
        const char* input, size_t length, char16_t* utf16_buffer) noexcept {
        return get_default_implementation()->convert_valid_utf8_to_utf16le(
            input, length, utf16_buffer);
    }
     [[nodiscard]] size_t convert_valid_utf8_to_utf16be(
        const char* input, size_t length, char16_t* utf16_buffer) noexcept {
        return get_default_implementation()->convert_valid_utf8_to_utf16be(
            input, length, utf16_buffer);
    }

     [[nodiscard]] size_t convert_valid_utf8_to_utf32(
    const char* input, size_t length, char32_t* utf32_buffer) noexcept {
        return get_default_implementation()->convert_valid_utf8_to_utf32(
            input, length, utf32_buffer);
    }

     [[nodiscard]] size_t count_utf8(const char* input,
       size_t length) noexcept {
        return get_default_implementation()->count_utf8(input, length);
    }


     [[nodiscard]] size_t latin1_length_from_utf8(const char* buf,
        size_t len) noexcept {
        return get_default_implementation()->latin1_length_from_utf8(buf, len);
    }


     [[nodiscard]] size_t utf32_length_from_utf8(const char* input,
        size_t length) noexcept {
        return get_default_implementation()->utf32_length_from_utf8(input, length);
    }


     [[nodiscard]] size_t convert_utf8_to_utf16(
        const char* input, size_t length, char16_t* utf16_output) noexcept {
#if KUMO_ENDIAN_BIG
        return convert_utf8_to_utf16be(input, length, utf16_output);
#else
        return convert_utf8_to_utf16le(input, length, utf16_output);
#endif
    }

}  // namespace turbo
