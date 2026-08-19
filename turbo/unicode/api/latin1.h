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

#pragma once

#include <turbo/unicode/engine/common_defs.h>

#include <turbo/unicode/text_encoding.h>
#include <turbo/unicode/error.h>

namespace turbo {

    /// Convert Latin1 string into UTF-8 string.
    ///
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// @param input         the Latin1 string to convert
    /// @param length        the length of the string in bytes
    /// @param utf8_output   the pointer to buffer that can hold conversion result
    /// @return the number of written char; 0 if conversion is not possible
     [[nodiscard]] size_t convert_latin1_to_utf8(const char* input,
        size_t length,
        char* utf8_output) noexcept;

    /// Convert Latin1 string into UTF-8 string with output limit.
    ///
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// We write as many characters as possible.
    ///
    /// Using convert_latin1_to_utf8_safe instead of convert_latin1_to_utf8 comes
    /// with a significant penalty in some cases, being up to four times slower,
    /// especially on short inputs. If you have allocated the output buffer so that
    /// it contains utf8_length_from_latin1(input, length) bytes, then prefer
    /// convert_latin1_to_utf8.
    ///
    /// @param input         the Latin1 string to convert
    /// @param length        the length of the string in bytes
    /// @param utf8_output  	the pointer to buffer that can hold conversion result
    /// @param utf8_len      the maximum output length
    /// @return the number of written char; 0 if conversion is not possible
     [[nodiscard]] size_t
    convert_latin1_to_utf8_safe(const char* input, size_t length, char* utf8_output,
        size_t utf8_len) noexcept;

    /// Convert possibly Latin1 string into UTF-16LE string.
    ///
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// @param input         the Latin1 string to convert
    /// @param length        the length of the string in bytes
    /// @param utf16_output  the pointer to buffer that can hold conversion result
    /// @return the number of written char16_t; 0 if conversion is not possible
     [[nodiscard]] size_t convert_latin1_to_utf16le(
        const char* input, size_t length, char16_t* utf16_output) noexcept;

    /// Convert Latin1 string into UTF-16BE string.
    ///
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// @param input         the Latin1 string to convert
    /// @param length        the length of the string in bytes
    /// @param utf16_output  the pointer to buffer that can hold conversion result
    /// @return the number of written char16_t; 0 if conversion is not possible
     [[nodiscard]] size_t convert_latin1_to_utf16be(
        const char* input, size_t length, char16_t* utf16_output) noexcept;

    /// Compute the number of code units that this Latin1 string would require in
    /// UTF-16 format.
    ///
    /// @param length        the length of the string in Latin1 code units (char)
    /// @return the length of the string in 2-byte code units (char16_t) required to
    /// encode the Latin1 string as UTF-16
    [[nodiscard]] KUMO_FORCE_INLINE  size_t
    utf16_length_from_latin1(size_t length) noexcept {
        return length;
    }

    /// Convert Latin1 string into UTF-32 string.
    ///
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// @param input         the Latin1 string to convert
    /// @param length        the length of the string in bytes
    /// @param utf32_buffer  the pointer to buffer that can hold conversion result
    /// @return the number of written char32_t; 0 if conversion is not possible
     [[nodiscard]] size_t convert_latin1_to_utf32(
        const char* input, size_t length, char32_t* utf32_buffer) noexcept;

    /// Return the number of bytes that this Latin1 string would require in UTF-8
    /// format.
    ///
    /// @param input         the Latin1 string to convert
    /// @param length        the length of the string bytes
    /// @return the number of bytes required to encode the Latin1 string as UTF-8
     [[nodiscard]] size_t utf8_length_from_latin1(const char* input,
        size_t length) noexcept;

    /// Using native endianness, convert a Latin1 string into a UTF-16 string.
    ///
    /// @param input         the Latin1 string to convert
    /// @param length        the length of the string in bytes
    /// @param utf16_output  the pointer to buffer that can hold conversion result
    /// @return the number of written char16_t.
     [[nodiscard]] size_t convert_latin1_to_utf16(
        const char* input, size_t length, char16_t* utf16_output) noexcept;


    /// Compute the number of bytes that this Latin1 string would require in UTF-32
    /// format.
    ///
    /// @param length        the length of the string in Latin1 code units (char)
    /// @return the length of the string in 4-byte code units (char32_t) required to
    /// encode the Latin1 string as UTF-32
     [[nodiscard]] KUMO_FORCE_INLINE  size_t
    utf32_length_from_latin1(size_t length) noexcept {
        return length;
    }

}  // namespace turbo
