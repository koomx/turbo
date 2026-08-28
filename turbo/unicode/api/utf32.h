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

#include <turbo/unicode/engine/portability.h>
#include <turbo/unicode/text_encoding.h>
#include <turbo/unicode/error.h>

namespace turbo {


    /// Validate the UTF-32 string. This function may be best when you expect
    /// the input to be almost always valid. Otherwise, consider using
    /// validate_utf32_with_errors.
    ///
    /// Overridden by each implementation.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param buf the UTF-32 string to validate.
    /// @param len the length of the string in number of 4-byte code units
    /// (char32_t).
    /// @return true if and only if the string is valid UTF-32.
     [[nodiscard]] bool validate_utf32(const char32_t* buf,
        size_t len) noexcept;

    /// Validate the UTF-32 string and stop on error. It might be faster than
    /// validate_utf32 when an error is expected to occur early.
    ///
    /// Overridden by each implementation.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param buf the UTF-32 string to validate.
    /// @param len the length of the string in number of 4-byte code units
    /// (char32_t).
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of code units validated if
    /// successful.
     [[nodiscard]] UnicodeResult validate_utf32_with_errors(const char32_t* buf,
        size_t len) noexcept;

     /// Convert possibly broken UTF-32 string into UTF-8 string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-32 string to convert
    /// @param length        the length of the string in 4-byte code units (char32_t)
    /// @param utf8_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units; 0 if input is not a valid UTF-32 string
     [[nodiscard]] size_t convert_utf32_to_utf8(const char32_t* input,
        size_t length,
        char* utf8_buffer) noexcept;


    /// Convert possibly broken UTF-32 string into UTF-8 string and stop on error.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-32 string to convert
    /// @param length        the length of the string in 4-byte code units (char32_t)
    /// @param utf8_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of char written if
    /// successful.
     [[nodiscard]] UnicodeResult convert_utf32_to_utf8_with_errors(
        const char32_t* input, size_t length, char* utf8_buffer) noexcept;

    /// Convert valid UTF-32 string into UTF-8 string.
    ///
    /// This function assumes that the input string is valid UTF-32.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-32 string to convert
    /// @param length        the length of the string in 4-byte code units (char32_t)
    /// @param utf8_buffer   the pointer to a buffer that can hold the conversion
    /// UnicodeResult
    /// @return number of written code units; 0 if conversion is not possible
     [[nodiscard]] size_t convert_valid_utf32_to_utf8(
        const char32_t* input, size_t length, char* utf8_buffer) noexcept;

    /// Using native Endian, convert possibly broken UTF-32 string into a UTF-16
    /// string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-32 string to convert
    /// @param length        the length of the string in 4-byte code units (char32_t)
    /// @param utf16_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units; 0 if input is not a valid UTF-32 string
     [[nodiscard]] size_t convert_utf32_to_utf16(
        const char32_t* input, size_t length, char16_t* utf16_buffer) noexcept;


    /// Convert possibly broken UTF-32 string into UTF-16LE string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-32 string to convert
    /// @param length        the length of the string in 4-byte code units (char32_t)
    /// @param utf16_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units; 0 if input is not a valid UTF-32 string
     [[nodiscard]] size_t convert_utf32_to_utf16le(
        const char32_t* input, size_t length, char16_t* utf16_buffer) noexcept;


    /// Convert possibly broken UTF-32 string into Latin1 string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-32 string to convert
    /// @param length        the length of the string in 4-byte code units (char32_t)
    /// @param latin1_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units; 0 if input is not a valid UTF-32 string
    /// or if it cannot be represented as Latin1
     [[nodiscard]] size_t convert_utf32_to_latin1(
        const char32_t* input, size_t length, char* latin1_buffer) noexcept;

    /// Convert possibly broken UTF-32 string into Latin1 string and stop on error.
    /// If the string cannot be represented as Latin1, an error is returned.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-32 string to convert
    /// @param length        the length of the string in 4-byte code units (char32_t)
    /// @param latin1_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of char written if
    /// successful.
     [[nodiscard]] UnicodeResult convert_utf32_to_latin1_with_errors(
        const char32_t* input, size_t length, char* latin1_buffer) noexcept;

    /// Convert valid UTF-32 string into Latin1 string.
    ///
    /// This function assumes that the input string is valid UTF-32 and that it can
    /// be represented as Latin1. If you violate this assumption, the UnicodeResult is
    /// implementation defined and may include system-dependent behavior such as
    /// crashes.
    ///
    /// This function is for expert users only and not part of our public API. Use
    /// convert_utf32_to_latin1 instead. The function may be removed from the library
    /// in the future.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-32 string to convert
    /// @param length        the length of the string in 4-byte code units (char32_t)
    /// @param latin1_buffer   the pointer to a buffer that can hold the conversion
    /// UnicodeResult
    /// @return number of written code units; 0 if conversion is not possible
     [[nodiscard]] size_t convert_valid_utf32_to_latin1(
        const char32_t* input, size_t length, char* latin1_buffer) noexcept;


    /// Compute the number of bytes that this UTF-32 string would require in Latin1
    /// format.
    ///
    /// This function does not validate the input. It is acceptable to pass invalid
    /// UTF-32 strings but in such cases the UnicodeResult is implementation defined.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param length        the length of the string in 4-byte code units (char32_t)
    /// @return the number of bytes required to encode the UTF-32 string as Latin1
     [[nodiscard]] KUMO_FORCE_INLINE  size_t
    latin1_length_from_utf32(size_t length) noexcept {
        return length;
    }



    /// Convert possibly broken UTF-32 string into UTF-16BE string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-32 string to convert
    /// @param length        the length of the string in 4-byte code units (char32_t)
    /// @param utf16_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units; 0 if input is not a valid UTF-32 string
     [[nodiscard]] size_t convert_utf32_to_utf16be(
        const char32_t* input, size_t length, char16_t* utf16_buffer) noexcept;

    /// Using native Endian, convert possibly broken UTF-32 string into UTF-16
    /// string and stop on error.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-32 string to convert
    /// @param length        the length of the string in 4-byte code units (char32_t)
    /// @param utf16_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of char16_t written if
    /// successful.
     [[nodiscard]] UnicodeResult convert_utf32_to_utf16_with_errors(
        const char32_t* input, size_t length, char16_t* utf16_buffer) noexcept;


    /// Convert possibly broken UTF-32 string into UTF-16LE string and stop on error.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-32 string to convert
    /// @param length        the length of the string in 4-byte code units (char32_t)
    /// @param utf16_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of char16_t written if
    /// successful.
     [[nodiscard]] UnicodeResult convert_utf32_to_utf16le_with_errors(
        const char32_t* input, size_t length, char16_t* utf16_buffer) noexcept;


    /// Convert possibly broken UTF-32 string into UTF-16BE string and stop on error.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-32 string to convert
    /// @param length        the length of the string in 4-byte code units (char32_t)
    /// @param utf16_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of char16_t written if
    /// successful.
     [[nodiscard]] UnicodeResult convert_utf32_to_utf16be_with_errors(
        const char32_t* input, size_t length, char16_t* utf16_buffer) noexcept;


    /// Using native Endian, convert valid UTF-32 string into a UTF-16 string.
    ///
    /// This function assumes that the input string is valid UTF-32.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-32 string to convert
    /// @param length        the length of the string in 4-byte code units (char32_t)
    /// @param utf16_buffer   the pointer to a buffer that can hold the conversion
    /// UnicodeResult
    /// @return number of written code units; 0 if conversion is not possible
     [[nodiscard]] size_t convert_valid_utf32_to_utf16(
        const char32_t* input, size_t length, char16_t* utf16_buffer) noexcept;


    /// Convert valid UTF-32 string into UTF-16LE string.
    ///
    /// This function assumes that the input string is valid UTF-32.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-32 string to convert
    /// @param length        the length of the string in 4-byte code units (char32_t)
    /// @param utf16_buffer   the pointer to a buffer that can hold the conversion
    /// UnicodeResult
    /// @return number of written code units; 0 if conversion is not possible
     [[nodiscard]] size_t convert_valid_utf32_to_utf16le(
        const char32_t* input, size_t length, char16_t* utf16_buffer) noexcept;


    /// Convert valid UTF-32 string into UTF-16BE string.
    ///
    /// This function assumes that the input string is valid UTF-32.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-32 string to convert
    /// @param length        the length of the string in 4-byte code units (char32_t)
    /// @param utf16_buffer   the pointer to a buffer that can hold the conversion
    /// UnicodeResult
    /// @return number of written code units; 0 if conversion is not possible
     [[nodiscard]] size_t convert_valid_utf32_to_utf16be(
        const char32_t* input, size_t length, char16_t* utf16_buffer) noexcept;


    /// Compute the number of bytes that this UTF-32 string would require in UTF-8
    /// format.
    ///
    /// This function does not validate the input. It is acceptable to pass invalid
    /// UTF-32 strings but in such cases the UnicodeResult is implementation defined.
    ///
    /// @param input         the UTF-32 string to convert
    /// @param length        the length of the string in 4-byte code units (char32_t)
    /// @return the number of bytes required to encode the UTF-32 string as UTF-8
     [[nodiscard]] size_t utf8_length_from_utf32(const char32_t* input,
        size_t length) noexcept;


     /// Compute the number of two-byte code units that this UTF-32 string would
    /// require in UTF-16 format.
    ///
    /// This function does not validate the input. It is acceptable to pass invalid
    /// UTF-32 strings but in such cases the UnicodeResult is implementation defined.
    ///
    /// @param input         the UTF-32 string to convert
    /// @param length        the length of the string in 4-byte code units (char32_t)
    /// @return the number of bytes required to encode the UTF-32 string as UTF-16
     [[nodiscard]] size_t utf16_length_from_utf32(const char32_t* input,
        size_t length) noexcept;


    /// Surrogate code points are reserved for UTF-16 and are not Unicode scalar values,
    /// so they have no valid UTF-8 encoding. `convertCodePointToUTF8` doesn't check that,
    /// it encodes them as CESU-8, so callers must reject them beforehand.
    constexpr bool is_surrogate_code_point(char32_t code_point){
        return code_point >= 0xD800 && code_point <= 0xDFFF;
    }


    /// For Unicode code points 0 through 0x10FFFF, encode_utf32_to_utf8 writes
    /// out the UTF-8 encoding into buffer, and returns the number of chars
    /// it wrote.
    ///
    /// As described in https://tools.ietf.org/html/rfc3629#section-3 , the encodings
    /// are:
    ///    00 -     7F : 0xxxxxxx
    ///    80 -    7FF : 110xxxxx 10xxxxxx
    ///   800 -   FFFF : 1110xxxx 10xxxxxx 10xxxxxx
    /// 10000 - 10FFFF : 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    ///
    /// Values greater than 0x10FFFF are not supported and may or may not write
    /// characters into buffer, however never will more than kMaxEncodedUTF8Size
    /// bytes be written, regardless of the value of utf8_char.
    enum { kMaxEncodedUTF8Size = 4 };

    size_t encode_utf32_to_utf8(char* buffer, char32_t utf8_char);
    size_t encode_utf32_to_utf8(char* buffer, size_t out_length, char32_t utf8_char);

}  // namespace turbo
