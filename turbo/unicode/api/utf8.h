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
#include <turbo/unicode/engine/compiler_check.h>
#include <turbo/unicode/engine/encoding_types.h>
#include <turbo/unicode/engine/error.h>

namespace turbo {

    /// Validate the UTF-8 string. This function may be best when you expect
    /// the input to be almost always valid. Otherwise, consider using
    /// validate_utf8_with_errors.
    ///
    /// Overridden by each implementation.
    ///
    /// @param buf the UTF-8 string to validate.
    /// @param len the length of the string in bytes.
    /// @return true if and only if the string is valid UTF-8.
    simdutf_warn_unused bool validate_utf8(const char* buf, size_t len) noexcept;


    /// Validate the UTF-8 string and stop on error.
    ///
    /// Overridden by each implementation.
    ///
    /// @param buf the UTF-8 string to validate.
    /// @param len the length of the string in bytes.
    /// @return a result pair struct (of type turbo::result containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of code units validated if
    /// successful.
    simdutf_warn_unused result validate_utf8_with_errors(const char* buf,
        size_t len) noexcept;

    /// Convert possibly broken UTF-8 string into latin1 string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// @param input         the UTF-8 string to convert
    /// @param length        the length of the string in bytes
    /// @param latin1_output  the pointer to buffer that can hold conversion result
    /// @return the number of written char; 0 if the input was not valid UTF-8 string
    /// or if it cannot be represented as Latin1
    simdutf_warn_unused size_t convert_utf8_to_latin1(const char* input,
        size_t length,
        char* latin1_output) noexcept;

    /// Using native endianness, convert possibly broken UTF-8 string into a UTF-16
    /// string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// @param input         the UTF-8 string to convert
    /// @param length        the length of the string in bytes
    /// @param utf16_output  the pointer to buffer that can hold conversion result
    /// @return the number of written char16_t; 0 if the input was not valid UTF-8
    /// string
    simdutf_warn_unused size_t convert_utf8_to_utf16(
        const char* input, size_t length, char16_t* utf16_output) noexcept;

    /// Convert possibly broken UTF-8 string into UTF-16LE string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// @param input         the UTF-8 string to convert
    /// @param length        the length of the string in bytes
    /// @param utf16_output  the pointer to buffer that can hold conversion result
    /// @return the number of written char16_t; 0 if the input was not valid UTF-8
    /// string
    simdutf_warn_unused size_t convert_utf8_to_utf16le(
        const char* input, size_t length, char16_t* utf16_output) noexcept;


    /// Convert possibly broken UTF-8 string into UTF-16BE string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// @param input         the UTF-8 string to convert
    /// @param length        the length of the string in bytes
    /// @param utf16_output  the pointer to buffer that can hold conversion result
    /// @return the number of written char16_t; 0 if the input was not valid UTF-8
    /// string
    simdutf_warn_unused size_t convert_utf8_to_utf16be(
        const char* input, size_t length, char16_t* utf16_output) noexcept;

    /// Convert possibly broken UTF-8 string into latin1 string with errors.
    /// If the string cannot be represented as Latin1, an error
    /// code is returned.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// @param input         the UTF-8 string to convert
    /// @param length        the length of the string in bytes
    /// @param latin1_output  the pointer to buffer that can hold conversion result
    /// @return a result pair struct (of type turbo::result containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of code units validated if
    /// successful.
    simdutf_warn_unused result convert_utf8_to_latin1_with_errors(
        const char* input, size_t length, char* latin1_output) noexcept;


    /// Using native endianness, convert possibly broken UTF-8 string into UTF-16
    /// string and stop on error.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// @param input         the UTF-8 string to convert
    /// @param length        the length of the string in bytes
    /// @param utf16_output  the pointer to buffer that can hold conversion result
    /// @return a result pair struct (of type turbo::result containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of char16_t written if
    /// successful.
    simdutf_warn_unused result convert_utf8_to_utf16_with_errors(
        const char* input, size_t length, char16_t* utf16_output) noexcept;


    /// Convert possibly broken UTF-8 string into UTF-16LE string and stop on error.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// @param input         the UTF-8 string to convert
    /// @param length        the length of the string in bytes
    /// @param utf16_output  the pointer to buffer that can hold conversion result
    /// @return a result pair struct (of type turbo::result containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of char16_t written if
    /// successful.
    simdutf_warn_unused result convert_utf8_to_utf16le_with_errors(
        const char* input, size_t length, char16_t* utf16_output) noexcept;

    /// Convert possibly broken UTF-8 string into UTF-16BE string and stop on error.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// @param input         the UTF-8 string to convert
    /// @param length        the length of the string in bytes
    /// @param utf16_output  the pointer to buffer that can hold conversion result
    /// @return a result pair struct (of type turbo::result containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of char16_t written if
    /// successful.
    simdutf_warn_unused result convert_utf8_to_utf16be_with_errors(
        const char* input, size_t length, char16_t* utf16_output) noexcept;


    /// Convert possibly broken UTF-8 string into UTF-32 string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// @param input         the UTF-8 string to convert
    /// @param length        the length of the string in bytes
    /// @param utf32_output  the pointer to buffer that can hold conversion result
    /// @return the number of written char32_t; 0 if the input was not valid UTF-8
    /// string
    simdutf_warn_unused size_t convert_utf8_to_utf32(
        const char* input, size_t length, char32_t* utf32_output) noexcept;


    /// Convert possibly broken UTF-8 string into UTF-32 string and stop on error.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// @param input         the UTF-8 string to convert
    /// @param length        the length of the string in bytes
    /// @param utf32_output  the pointer to buffer that can hold conversion result
    /// @return a result pair struct (of type turbo::result containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of char32_t written if
    /// successful.
    simdutf_warn_unused result convert_utf8_to_utf32_with_errors(
        const char* input, size_t length, char32_t* utf32_output) noexcept;


    /// Convert valid UTF-8 string into latin1 string.
    ///
    /// This function assumes that the input string is valid UTF-8 and that it can be
    /// represented as Latin1. If you violate this assumption, the result is
    /// implementation defined and may include system-dependent behavior such as
    /// crashes.
    ///
    /// This function is for expert users only and not part of our public API. Use
    /// convert_utf8_to_latin1 instead. The function may be removed from the library
    /// in the future.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-8 string to convert
    /// @param length        the length of the string in bytes
    /// @param latin1_output  the pointer to buffer that can hold conversion result
    /// @return the number of written char; 0 if the input was not valid UTF-8 string
    simdutf_warn_unused size_t convert_valid_utf8_to_latin1(
        const char* input, size_t length, char* latin1_output) noexcept;


    /// Using native endianness, convert valid UTF-8 string into a UTF-16 string.
    ///
    /// This function assumes that the input string is valid UTF-8.
    ///
    /// @param input         the UTF-8 string to convert
    /// @param length        the length of the string in bytes
    /// @param utf16_buffer  the pointer to buffer that can hold conversion result
    /// @return the number of written char16_t
    simdutf_warn_unused size_t convert_valid_utf8_to_utf16(
        const char* input, size_t length, char16_t* utf16_buffer) noexcept;


    /// Convert valid UTF-8 string into UTF-16LE string.
    ///
    /// This function assumes that the input string is valid UTF-8.
    ///
    /// @param input         the UTF-8 string to convert
    /// @param length        the length of the string in bytes
    /// @param utf16_buffer  the pointer to buffer that can hold conversion result
    /// @return the number of written char16_t
    simdutf_warn_unused size_t convert_valid_utf8_to_utf16le(
        const char* input, size_t length, char16_t* utf16_buffer) noexcept;


    /// Convert valid UTF-8 string into UTF-16BE string.
    ///
    /// This function assumes that the input string is valid UTF-8.
    ///
    /// @param input         the UTF-8 string to convert
    /// @param length        the length of the string in bytes
    /// @param utf16_buffer  the pointer to buffer that can hold conversion result
    /// @return the number of written char16_t
    simdutf_warn_unused size_t convert_valid_utf8_to_utf16be(
        const char* input, size_t length, char16_t* utf16_buffer) noexcept;


    /// Convert valid UTF-8 string into UTF-32 string.
    ///
    /// This function assumes that the input string is valid UTF-8.
    ///
    /// @param input         the UTF-8 string to convert
    /// @param length        the length of the string in bytes
    /// @param utf32_buffer  the pointer to buffer that can hold conversion result
    /// @return the number of written char32_t
    simdutf_warn_unused size_t convert_valid_utf8_to_utf32(
        const char* input, size_t length, char32_t* utf32_buffer) noexcept;


    /// Compute the number of 2-byte code units that this UTF-8 string would require
    /// in UTF-16LE format.
    ///
    /// This function does not validate the input. It is acceptable to pass invalid
    /// UTF-8 strings but in such cases the result is implementation defined.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-8 string to process
    /// @param length        the length of the string in bytes
    /// @return the number of char16_t code units required to encode the UTF-8 string
    /// as UTF-16LE
    simdutf_warn_unused size_t utf16_length_from_utf8(const char* input,
        size_t length) noexcept;

    /// Compute the number of 4-byte code units that this UTF-8 string would require
    /// in UTF-32 format.
    ///
    /// This function is equivalent to count_utf8
    ///
    /// This function does not validate the input. It is acceptable to pass invalid
    /// UTF-8 strings but in such cases the result is implementation defined.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-8 string to process
    /// @param length        the length of the string in bytes
    /// @return the number of char32_t code units required to encode the UTF-8 string
    /// as UTF-32
    simdutf_warn_unused size_t utf32_length_from_utf8(const char* input,
        size_t length) noexcept;

    /// Count the number of code points (characters) in the string assuming that
    /// it is valid.
    ///
    /// This function assumes that the input string is valid UTF-8.
    /// It is acceptable to pass invalid UTF-8 strings but in such cases
    /// the result is implementation defined.
    ///
    /// @param input         the UTF-8 string to process
    /// @param length        the length of the string in bytes
    /// @return number of code points
    simdutf_warn_unused size_t count_utf8(const char* input,
        size_t length) noexcept;


    /// Given a valid UTF-8 string having a possibly truncated last character,
    /// this function checks the end of string. If the last character is truncated
    /// (or partial), then it returns a shorter length (shorter by 1 to 3 bytes) so
    /// that the short UTF-8 strings only contain complete characters. If there is no
    /// truncated character, the original length is returned.
    ///
    /// This function assumes that the input string is valid UTF-8, but possibly
    /// truncated.
    ///
    /// @param input         the UTF-8 string to process
    /// @param length        the length of the string in bytes
    /// @return the length of the string in bytes, possibly shorter by 1 to 3 bytes
    simdutf_warn_unused size_t trim_partial_utf8(const char* input, size_t length);


    /// Compute the number of bytes that this UTF-8 string would require in Latin1
    /// format.
    ///
    /// This function does not validate the input. It is acceptable to pass invalid
    /// UTF-8 strings but in such cases the result is implementation defined.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-8 string to convert
    /// @param length        the length of the string in byte
    /// @return the number of bytes required to encode the UTF-8 string as Latin1
    simdutf_warn_unused size_t latin1_length_from_utf8(const char* input,
        size_t length) noexcept;


}  // namespace turbo
