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

#include <turbo/unicode/api/utf32.h>
#include <turbo/unicode/engine/backend_select.h>

namespace turbo {

    simdutf_warn_unused bool validate_utf32(const char32_t* buf,
        size_t len) noexcept {
        return get_default_implementation()->validate_utf32(buf, len);
    }
    simdutf_warn_unused result validate_utf32_with_errors(const char32_t* buf,
        size_t len) noexcept {
        return get_default_implementation()->validate_utf32_with_errors(buf, len);
    }

    simdutf_warn_unused size_t convert_utf32_to_utf8(const char32_t* buf,
        size_t len,
        char* utf8_buffer) noexcept {
        return get_default_implementation()->convert_utf32_to_utf8(buf, len,
            utf8_buffer);
    }
    simdutf_warn_unused result convert_utf32_to_utf8_with_errors(
        const char32_t* buf, size_t len, char* utf8_buffer) noexcept {
        return get_default_implementation()->convert_utf32_to_utf8_with_errors(
            buf, len, utf8_buffer);
    }
    simdutf_warn_unused size_t convert_valid_utf32_to_utf8(
        const char32_t* buf, size_t len, char* utf8_buffer) noexcept {
        return get_default_implementation()->convert_valid_utf32_to_utf8(buf, len,
            utf8_buffer);
    }

    simdutf_warn_unused size_t convert_utf32_to_utf16(
        const char32_t* buf, size_t len, char16_t* utf16_buffer) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return convert_utf32_to_utf16be(buf, len, utf16_buffer);
#else
        return convert_utf32_to_utf16le(buf, len, utf16_buffer);
#endif
    }

    simdutf_warn_unused size_t convert_utf32_to_latin1(
        const char32_t* input, size_t length, char* latin1_output) noexcept {
        return get_default_implementation()->convert_utf32_to_latin1(input, length,
            latin1_output);
    }
    simdutf_warn_unused result convert_utf32_to_latin1_with_errors(
        const char32_t* input, size_t length, char* latin1_buffer) noexcept {
        return get_default_implementation()->convert_utf32_to_latin1_with_errors(
            input, length, latin1_buffer);
    }
    simdutf_warn_unused size_t convert_valid_utf32_to_latin1(
        const char32_t* input, size_t length, char* latin1_buffer) noexcept {
        return get_default_implementation()->convert_valid_utf32_to_latin1(
            input, length, latin1_buffer);
    }

    simdutf_warn_unused size_t utf8_length_from_utf32(const char32_t* input,
        size_t length) noexcept {
        return get_default_implementation()->utf8_length_from_utf32(input, length);
    }

    simdutf_warn_unused size_t utf16_length_from_utf32(const char32_t* input,
        size_t length) noexcept {
        return get_default_implementation()->utf16_length_from_utf32(input, length);
    }

    simdutf_warn_unused size_t convert_utf32_to_utf16be(
       const char32_t* buf, size_t len, char16_t* utf16_buffer) noexcept {
        return get_default_implementation()->convert_utf32_to_utf16be(buf, len,
            utf16_buffer);
    }
    simdutf_warn_unused result convert_utf32_to_utf16_with_errors(
        const char32_t* buf, size_t len, char16_t* utf16_buffer) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return convert_utf32_to_utf16be_with_errors(buf, len, utf16_buffer);
#else
        return convert_utf32_to_utf16le_with_errors(buf, len, utf16_buffer);
#endif
    }
    simdutf_warn_unused result convert_utf32_to_utf16le_with_errors(
        const char32_t* buf, size_t len, char16_t* utf16_buffer) noexcept {
        return get_default_implementation()->convert_utf32_to_utf16le_with_errors(
            buf, len, utf16_buffer);
    }
    simdutf_warn_unused result convert_utf32_to_utf16be_with_errors(
        const char32_t* buf, size_t len, char16_t* utf16_buffer) noexcept {
        return get_default_implementation()->convert_utf32_to_utf16be_with_errors(
            buf, len, utf16_buffer);
    }


    simdutf_warn_unused size_t convert_valid_utf32_to_utf16(
        const char32_t* buf, size_t len, char16_t* utf16_buffer) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return convert_valid_utf32_to_utf16be(buf, len, utf16_buffer);
#else
        return convert_valid_utf32_to_utf16le(buf, len, utf16_buffer);
#endif
    }
    simdutf_warn_unused size_t convert_valid_utf32_to_utf16le(
        const char32_t* buf, size_t len, char16_t* utf16_buffer) noexcept {
        return get_default_implementation()->convert_valid_utf32_to_utf16le(
            buf, len, utf16_buffer);
    }
    simdutf_warn_unused size_t convert_valid_utf32_to_utf16be(
        const char32_t* buf, size_t len, char16_t* utf16_buffer) noexcept {
        return get_default_implementation()->convert_valid_utf32_to_utf16be(
            buf, len, utf16_buffer);
    }

}  // namespace turbo
