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
#include <turbo/unicode/engine/isa_select.h>

namespace turbo {

     [[nodiscard]] bool validate_utf32(const char32_t* buf,
        size_t len) noexcept {
        return UnicodeRegistry::get_best_isa()->validate_utf32(buf, len);
    }
     [[nodiscard]] UnicodeResult validate_utf32_with_errors(const char32_t* buf,
        size_t len) noexcept {
        return UnicodeRegistry::get_best_isa()->validate_utf32_with_errors(buf, len);
    }

     [[nodiscard]] size_t convert_utf32_to_utf8(const char32_t* buf,
        size_t len,
        char* utf8_buffer) noexcept {
        return UnicodeRegistry::get_best_isa()->convert_utf32_to_utf8(buf, len,
            utf8_buffer);
    }
     [[nodiscard]] UnicodeResult convert_utf32_to_utf8_with_errors(
        const char32_t* buf, size_t len, char* utf8_buffer) noexcept {
        return UnicodeRegistry::get_best_isa()->convert_utf32_to_utf8_with_errors(
            buf, len, utf8_buffer);
    }
     [[nodiscard]] size_t convert_valid_utf32_to_utf8(
        const char32_t* buf, size_t len, char* utf8_buffer) noexcept {
        return UnicodeRegistry::get_best_isa()->convert_valid_utf32_to_utf8(buf, len,
            utf8_buffer);
    }

     [[nodiscard]] size_t convert_utf32_to_utf16(
        const char32_t* buf, size_t len, char16_t* utf16_buffer) noexcept {
#if KUMO_ENDIAN_BIG
        return convert_utf32_to_utf16be(buf, len, utf16_buffer);
#else
        return convert_utf32_to_utf16le(buf, len, utf16_buffer);
#endif
    }

     [[nodiscard]] size_t convert_utf32_to_latin1(
        const char32_t* input, size_t length, char* latin1_output) noexcept {
        return UnicodeRegistry::get_best_isa()->convert_utf32_to_latin1(input, length,
            latin1_output);
    }
     [[nodiscard]] UnicodeResult convert_utf32_to_latin1_with_errors(
        const char32_t* input, size_t length, char* latin1_buffer) noexcept {
        return UnicodeRegistry::get_best_isa()->convert_utf32_to_latin1_with_errors(
            input, length, latin1_buffer);
    }
     [[nodiscard]] size_t convert_valid_utf32_to_latin1(
        const char32_t* input, size_t length, char* latin1_buffer) noexcept {
        return UnicodeRegistry::get_best_isa()->convert_valid_utf32_to_latin1(
            input, length, latin1_buffer);
    }

     [[nodiscard]] size_t utf8_length_from_utf32(const char32_t* input,
        size_t length) noexcept {
        return UnicodeRegistry::get_best_isa()->utf8_length_from_utf32(input, length);
    }

     [[nodiscard]] size_t utf16_length_from_utf32(const char32_t* input,
        size_t length) noexcept {
        return UnicodeRegistry::get_best_isa()->utf16_length_from_utf32(input, length);
    }

     [[nodiscard]] size_t convert_utf32_to_utf16be(
       const char32_t* buf, size_t len, char16_t* utf16_buffer) noexcept {
        return UnicodeRegistry::get_best_isa()->convert_utf32_to_utf16be(buf, len,
            utf16_buffer);
    }
     [[nodiscard]] UnicodeResult convert_utf32_to_utf16_with_errors(
        const char32_t* buf, size_t len, char16_t* utf16_buffer) noexcept {
#if KUMO_ENDIAN_BIG
        return convert_utf32_to_utf16be_with_errors(buf, len, utf16_buffer);
#else
        return convert_utf32_to_utf16le_with_errors(buf, len, utf16_buffer);
#endif
    }
     [[nodiscard]] UnicodeResult convert_utf32_to_utf16le_with_errors(
        const char32_t* buf, size_t len, char16_t* utf16_buffer) noexcept {
        return UnicodeRegistry::get_best_isa()->convert_utf32_to_utf16le_with_errors(
            buf, len, utf16_buffer);
    }
     [[nodiscard]] UnicodeResult convert_utf32_to_utf16be_with_errors(
        const char32_t* buf, size_t len, char16_t* utf16_buffer) noexcept {
        return UnicodeRegistry::get_best_isa()->convert_utf32_to_utf16be_with_errors(
            buf, len, utf16_buffer);
    }


     [[nodiscard]] size_t convert_valid_utf32_to_utf16(
        const char32_t* buf, size_t len, char16_t* utf16_buffer) noexcept {
#if KUMO_ENDIAN_BIG
        return convert_valid_utf32_to_utf16be(buf, len, utf16_buffer);
#else
        return convert_valid_utf32_to_utf16le(buf, len, utf16_buffer);
#endif
    }
     [[nodiscard]] size_t convert_valid_utf32_to_utf16le(
        const char32_t* buf, size_t len, char16_t* utf16_buffer) noexcept {
        return UnicodeRegistry::get_best_isa()->convert_valid_utf32_to_utf16le(
            buf, len, utf16_buffer);
    }
     [[nodiscard]] size_t convert_valid_utf32_to_utf16be(
        const char32_t* buf, size_t len, char16_t* utf16_buffer) noexcept {
        return UnicodeRegistry::get_best_isa()->convert_valid_utf32_to_utf16be(
            buf, len, utf16_buffer);
    }

}  // namespace turbo
