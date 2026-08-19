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

#include <turbo/unicode/api/latin1.h>
#include <turbo/unicode/engine/backend_select.h>
#include <turbo/unicode/scalar/latin1_to_utf8/latin1_to_utf8.h>
#include <algorithm>

namespace turbo {

     [[nodiscard]] size_t convert_latin1_to_utf8(const char* buf, size_t len,
    char* utf8_output) noexcept {
        return get_default_implementation()->convert_latin1_to_utf8(buf, len,
            utf8_output);
    }

     [[nodiscard]] size_t convert_latin1_to_utf16le(
        const char* buf, size_t len, char16_t* utf16_output) noexcept {
        return get_default_implementation()->convert_latin1_to_utf16le(buf, len,
            utf16_output);
    }
     [[nodiscard]] size_t convert_latin1_to_utf16be(
        const char* buf, size_t len, char16_t* utf16_output) noexcept {
        return get_default_implementation()->convert_latin1_to_utf16be(buf, len,
            utf16_output);
    }

     [[nodiscard]] size_t convert_latin1_to_utf32(
     const char* buf, size_t len, char32_t* latin1_output) noexcept {
        return get_default_implementation()->convert_latin1_to_utf32(buf, len,
            latin1_output);
    }

     [[nodiscard]] size_t utf8_length_from_latin1(const char* buf,
        size_t len) noexcept {
        return get_default_implementation()->utf8_length_from_latin1(buf, len);
    }

     [[nodiscard]] size_t convert_latin1_to_utf8_safe(
        const char* buf, size_t len, char* utf8_output, size_t utf8_len) noexcept {
        const auto start { utf8_output };

        while (true) {
            // convert_latin1_to_utf8 will never write more than input length * 2
            auto read_len = std::min(len, utf8_len >> 1);
            if (read_len <= 16) {
                break;
            }

            const auto write_len = turbo::convert_latin1_to_utf8(buf, read_len, utf8_output);

            utf8_output += write_len;
            utf8_len -= write_len;
            buf += read_len;
            len -= read_len;
        }

        utf8_output += scalar::latin1_to_utf8::convert_safe(buf, len, utf8_output, utf8_len);

        return utf8_output - start;
    }

     [[nodiscard]] size_t convert_latin1_to_utf16(
       const char* buf, size_t len, char16_t* utf16_output) noexcept {
#if SIMDUTF_IS_BIG_ENDIAN
        return convert_latin1_to_utf16be(buf, len, utf16_output);
#else
        return convert_latin1_to_utf16le(buf, len, utf16_output);
#endif
    }

}  // namespace turbo
