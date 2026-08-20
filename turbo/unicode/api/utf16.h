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

    /// Validate the ASCII string as a UTF-16 sequence.
    /// An UTF-16 sequence is considered an ASCII sequence
    /// if it could be converted to an ASCII string losslessly.
    ///
    /// Overridden by each implementation.
    ///
    /// @param buf the UTF-16 string to validate.
    /// @param len the length of the string in bytes.
    /// @return true if and only if the string is valid ASCII.
     [[nodiscard]] bool validate_utf16_as_ascii(const char16_t* buf,
        size_t len) noexcept;

    /// Validate the ASCII string as a UTF-16BE sequence.
    /// An UTF-16 sequence is considered an ASCII sequence
    /// if it could be converted to an ASCII string losslessly.
    ///
    /// Overridden by each implementation.
    ///
    /// @param buf the UTF-16BE string to validate.
    /// @param len the length of the string in bytes.
    /// @return true if and only if the string is valid ASCII.
     [[nodiscard]] bool validate_utf16be_as_ascii(const char16_t* buf,
        size_t len) noexcept;

    /// Validate the ASCII string as a UTF-16LE sequence.
    /// An UTF-16 sequence is considered an ASCII sequence
    /// if it could be converted to an ASCII string losslessly.
    ///
    /// Overridden by each implementation.
    ///
    /// @param buf the UTF-16LE string to validate.
    /// @param len the length of the string in bytes.
    /// @return true if and only if the string is valid ASCII.
     [[nodiscard]] bool validate_utf16le_as_ascii(const char16_t* buf,
        size_t len) noexcept;


    /// Using native endianness; Validate the UTF-16 string.
    /// This function may be best when you expect the input to be almost always
    /// valid. Otherwise, consider using validate_utf16_with_errors.
    ///
    /// Overridden by each implementation.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param buf the UTF-16 string to validate.
    /// @param len the length of the string in number of 2-byte code units
    /// (char16_t).
    /// @return true if and only if the string is valid UTF-16.
     [[nodiscard]] bool validate_utf16(const char16_t* buf,
        size_t len) noexcept;

    /// Validate the UTF-16LE string. This function may be best when you expect
    /// the input to be almost always valid. Otherwise, consider using
    /// validate_utf16le_with_errors.
    ///
    /// Overridden by each implementation.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param buf the UTF-16LE string to validate.
    /// @param len the length of the string in number of 2-byte code units
    /// (char16_t).
    /// @return true if and only if the string is valid UTF-16LE.
     [[nodiscard]] bool validate_utf16le(const char16_t* buf,
        size_t len) noexcept;

    /// Validate the UTF-16BE string. This function may be best when you expect
    /// the input to be almost always valid. Otherwise, consider using
    /// validate_utf16be_with_errors.
    ///
    /// Overridden by each implementation.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param buf the UTF-16BE string to validate.
    /// @param len the length of the string in number of 2-byte code units
    /// (char16_t).
    /// @return true if and only if the string is valid UTF-16BE.
     [[nodiscard]] bool validate_utf16be(const char16_t* buf,
        size_t len) noexcept;

    /// Using native endianness; Validate the UTF-16 string and stop on error.
    /// It might be faster than validate_utf16 when an error is expected to occur
    /// early.
    ///
    /// Overridden by each implementation.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param buf the UTF-16 string to validate.
    /// @param len the length of the string in number of 2-byte code units
    /// (char16_t).
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of code units validated if
    /// successful.
     [[nodiscard]] UnicodeResult validate_utf16_with_errors(const char16_t* buf,
        size_t len) noexcept;

    /// Validate the UTF-16LE string and stop on error. It might be faster than
    /// validate_utf16le when an error is expected to occur early.
    ///
    /// Overridden by each implementation.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param buf the UTF-16LE string to validate.
    /// @param len the length of the string in number of 2-byte code units
    /// (char16_t).
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of code units validated if
    /// successful.
     [[nodiscard]] UnicodeResult validate_utf16le_with_errors(const char16_t* buf,
        size_t len) noexcept;


    /// Validate the UTF-16BE string and stop on error. It might be faster than
    /// validate_utf16be when an error is expected to occur early.
    ///
    /// Overridden by each implementation.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param buf the UTF-16BE string to validate.
    /// @param len the length of the string in number of 2-byte code units
    /// (char16_t).
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of code units validated if
    /// successful.
     [[nodiscard]] UnicodeResult validate_utf16be_with_errors(const char16_t* buf,
        size_t len) noexcept;

    /// Fixes an ill-formed UTF-16LE string by replacing mismatched surrogates with
    /// the Unicode replacement character U+FFFD. If input and output points to
    /// different memory areas, the procedure copies string, and it's expected that
    /// output memory is at least as big as the input. It's also possible to set
    /// input equal output, that makes replacements an in-place operation.
    ///
    /// @param input the UTF-16LE string to correct.
    /// @param len the length of the string in number of 2-byte code units
    /// (char16_t).
    /// @param output the output buffer.
    void to_well_formed_utf16le(const char16_t* input, size_t len,
        char16_t* output) noexcept;

    /// Fixes an ill-formed UTF-16BE string by replacing mismatched surrogates with
    /// the Unicode replacement character U+FFFD. If input and output points to
    /// different memory areas, the procedure copies string, and it's expected that
    /// output memory is at least as big as the input. It's also possible to set
    /// input equal output, that makes replacements an in-place operation.
    ///
    /// @param input the UTF-16BE string to correct.
    /// @param len the length of the string in number of 2-byte code units
    /// (char16_t).
    /// @param output the output buffer.
    void to_well_formed_utf16be(const char16_t* input, size_t len,
        char16_t* output) noexcept;

    /// Fixes an ill-formed UTF-16 string by replacing mismatched surrogates with the
    /// Unicode replacement character U+FFFD. If input and output points to different
    /// memory areas, the procedure copies string, and it's expected that output
    /// memory is at least as big as the input. It's also possible to set input equal
    /// output, that makes replacements an in-place operation.
    ///
    /// @param input the UTF-16 string to correct.
    /// @param len the length of the string in number of 2-byte code units
    /// (char16_t).
    /// @param output the output buffer.
    void to_well_formed_utf16(const char16_t* input, size_t len,
        char16_t* output) noexcept;


    /// Using native endianness, convert possibly broken UTF-16 string into UTF-8
    /// string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16 string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf8_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units; 0 if input is not a valid UTF-16LE
    /// string
     [[nodiscard]] size_t convert_utf16_to_utf8(const char16_t* input,
        size_t length,
        char* utf8_buffer) noexcept;

    /// Using native endianness, convert possibly broken UTF-16 string into UTF-8
    /// string with output limit.
    ///
    /// We write as many characters as possible into the output buffer,
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// Using convert_utf16_to_utf8_safe instead of convert_utf16_to_utf8 comes with
    /// a significant penalty in some cases, being up to three times slower,
    /// especially on short inputs. If you have allocated the output buffer so that
    /// it contains utf8_length_from_utf16(input, length) bytes, then prefer
    /// convert_utf16_to_utf8.
    ///
    /// @param input         the UTF-16 string to convert
    /// @param length        the length of the string in 16-bit code units (char16_t)
    /// @param utf8_output  	the pointer to buffer that can hold conversion UnicodeResult
    /// @param utf8_len      the maximum output length
    /// @return the number of written char; 0 if conversion is not possible
     [[nodiscard]] size_t convert_utf16_to_utf8_safe(const char16_t* input,
        size_t length,
        char* utf8_output,
        size_t utf8_len) noexcept;

     /// Using native endianness, convert possibly broken UTF-16 string into Latin1
    /// string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16 string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param latin1_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units; 0 if input is not a valid UTF-16 string
    /// or if it cannot be represented as Latin1
     [[nodiscard]] size_t convert_utf16_to_latin1(
        const char16_t* input, size_t length, char* latin1_buffer) noexcept;

    /// Convert possibly broken UTF-16LE string into Latin1 string.
    /// If the string cannot be represented as Latin1, an error
    /// is returned.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16LE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param latin1_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units; 0 if input is not a valid UTF-16LE
    /// string or if it cannot be represented as Latin1
     [[nodiscard]] size_t convert_utf16le_to_latin1(
        const char16_t* input, size_t length, char* latin1_buffer) noexcept;


    /// Convert possibly broken UTF-16BE string into Latin1 string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16BE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param latin1_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units; 0 if input is not a valid UTF-16BE
    /// string or if it cannot be represented as Latin1
     [[nodiscard]] size_t convert_utf16be_to_latin1(
        const char16_t* input, size_t length, char* latin1_buffer) noexcept;

    /// Convert possibly broken UTF-16LE string into UTF-8 string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16LE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf8_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units; 0 if input is not a valid UTF-16LE
    /// string
     [[nodiscard]] size_t convert_utf16le_to_utf8(const char16_t* input,
        size_t length,
        char* utf8_buffer) noexcept;


    /// Convert possibly broken UTF-16BE string into UTF-8 string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16BE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf8_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units; 0 if input is not a valid UTF-16LE
    /// string
     [[nodiscard]] size_t convert_utf16be_to_utf8(const char16_t* input,
        size_t length,
        char* utf8_buffer) noexcept;

     /// Using native endianness, convert possibly broken UTF-16 string into Latin1
    /// string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16 string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param latin1_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of char written if
    /// successful.
     [[nodiscard]] UnicodeResult convert_utf16_to_latin1_with_errors(
        const char16_t* input, size_t length, char* latin1_buffer) noexcept;

    /// Convert possibly broken UTF-16LE string into Latin1 string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16LE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param latin1_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of char written if
    /// successful.
     [[nodiscard]] UnicodeResult convert_utf16le_to_latin1_with_errors(
        const char16_t* input, size_t length, char* latin1_buffer) noexcept;


    /// Convert possibly broken UTF-16BE string into Latin1 string.
    /// If the string cannot be represented as Latin1, an error
    /// is returned.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16BE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param latin1_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of char written if
    /// successful.
     [[nodiscard]] UnicodeResult convert_utf16be_to_latin1_with_errors(
        const char16_t* input, size_t length, char* latin1_buffer) noexcept;


     /// Using native endianness, convert possibly broken UTF-16 string into UTF-8
    /// string and stop on error.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16 string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf8_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of char written if
    /// successful.
     [[nodiscard]] UnicodeResult convert_utf16_to_utf8_with_errors(
        const char16_t* input, size_t length, char* utf8_buffer) noexcept;


    /// Convert possibly broken UTF-16LE string into UTF-8 string and stop on error.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16LE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf8_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of char written if
    /// successful.
     [[nodiscard]] UnicodeResult convert_utf16le_to_utf8_with_errors(
        const char16_t* input, size_t length, char* utf8_buffer) noexcept;

    /// Convert possibly broken UTF-16BE string into UTF-8 string and stop on error.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16BE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf8_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of char written if
    /// successful.
     [[nodiscard]] UnicodeResult convert_utf16be_to_utf8_with_errors(
        const char16_t* input, size_t length, char* utf8_buffer) noexcept;

    /// Convert possibly broken UTF-16LE string into UTF-8 string, replacing
    /// unpaired surrogates with the Unicode replacement character U+FFFD.
    ///
    /// This function always succeeds: unpaired surrogates are replaced with
    /// U+FFFD (3 bytes in UTF-8: 0xEF 0xBF 0xBD).
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16LE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf8_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units
     [[nodiscard]] size_t convert_utf16le_to_utf8_with_replacement(
        const char16_t* input, size_t length, char* utf8_buffer) noexcept;

    /// Convert possibly broken UTF-16BE string into UTF-8 string, replacing
    /// unpaired surrogates with the Unicode replacement character U+FFFD.
    ///
    /// This function always succeeds: unpaired surrogates are replaced with
    /// U+FFFD (3 bytes in UTF-8: 0xEF 0xBF 0xBD).
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16BE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf8_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units
     [[nodiscard]] size_t convert_utf16be_to_utf8_with_replacement(
        const char16_t* input, size_t length, char* utf8_buffer) noexcept;


    /// Convert possibly broken UTF-16 string (native endianness) into UTF-8 string,
    /// replacing unpaired surrogates with the Unicode replacement character U+FFFD.
    ///
    /// This function always succeeds: unpaired surrogates are replaced with
    /// U+FFFD (3 bytes in UTF-8: 0xEF 0xBF 0xBD).
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16 string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf8_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units
     [[nodiscard]] size_t convert_utf16_to_utf8_with_replacement(
        const char16_t* input, size_t length, char* utf8_buffer) noexcept;


     /// Using native endianness, convert valid UTF-16 string into UTF-8 string.
    ///
    /// This function assumes that the input string is valid UTF-16.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16 string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf8_buffer   the pointer to a buffer that can hold the conversion
    /// UnicodeResult
    /// @return number of written code units; 0 if conversion is not possible
     [[nodiscard]] size_t convert_valid_utf16_to_utf8(
        const char16_t* input, size_t length, char* utf8_buffer) noexcept;

    /// Using native endianness, convert UTF-16 string into Latin1 string.
    ///
    /// This function assumes that the input string is valid UTF-16 and that it can
    /// be represented as Latin1. If you violate this assumption, the UnicodeResult is
    /// implementation defined and may include system-dependent behavior such as
    /// crashes.
    ///
    /// This function is for expert users only and not part of our public API. Use
    /// convert_utf16_to_latin1 instead. The function may be removed from the library
    /// in the future.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16 string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param latin1_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units; 0 if conversion is not possible
     [[nodiscard]] size_t convert_valid_utf16_to_latin1(
        const char16_t* input, size_t length, char* latin1_buffer) noexcept;


    /// Convert valid UTF-16LE string into Latin1 string.
    ///
    /// This function assumes that the input string is valid UTF-16LE and that it can
    /// be represented as Latin1. If you violate this assumption, the UnicodeResult is
    /// implementation defined and may include system-dependent behavior such as
    /// crashes.
    ///
    /// This function is for expert users only and not part of our public API. Use
    /// convert_utf16le_to_latin1 instead. The function may be removed from the
    /// library in the future.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16LE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param latin1_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units; 0 if conversion is not possible
     [[nodiscard]] size_t convert_valid_utf16le_to_latin1(
        const char16_t* input, size_t length, char* latin1_buffer) noexcept;


    /// Convert valid UTF-16BE string into Latin1 string.
    ///
    /// This function assumes that the input string is valid UTF-16BE and that it can
    /// be represented as Latin1. If you violate this assumption, the UnicodeResult is
    /// implementation defined and may include system-dependent behavior such as
    /// crashes.
    ///
    /// This function is for expert users only and not part of our public API. Use
    /// convert_utf16be_to_latin1 instead. The function may be removed from the
    /// library in the future.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16BE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param latin1_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units; 0 if conversion is not possible
     [[nodiscard]] size_t convert_valid_utf16be_to_latin1(
        const char16_t* input, size_t length, char* latin1_buffer) noexcept;


    /// Convert valid UTF-16LE string into UTF-8 string.
    ///
    /// This function assumes that the input string is valid UTF-16LE
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16LE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf8_buffer   the pointer to a buffer that can hold the conversion
    /// UnicodeResult
    /// @return number of written code units; 0 if conversion is not possible
     [[nodiscard]] size_t convert_valid_utf16le_to_utf8(
        const char16_t* input, size_t length, char* utf8_buffer) noexcept;


    /// Convert valid UTF-16BE string into UTF-8 string.
    ///
    /// This function assumes that the input string is valid UTF-16BE.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16BE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf8_buffer   the pointer to a buffer that can hold the conversion
    /// UnicodeResult
    /// @return number of written code units; 0 if conversion is not possible
     [[nodiscard]] size_t convert_valid_utf16be_to_utf8(
        const char16_t* input, size_t length, char* utf8_buffer) noexcept;

     /// Using native endianness, convert possibly broken UTF-16 string into UTF-32
    /// string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16 string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf32_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units; 0 if input is not a valid UTF-16LE
    /// string
     [[nodiscard]] size_t convert_utf16_to_utf32(
        const char16_t* input, size_t length, char32_t* utf32_buffer) noexcept;


    /// Convert possibly broken UTF-16LE string into UTF-32 string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16LE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf32_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units; 0 if input is not a valid UTF-16LE
    /// string
     [[nodiscard]] size_t convert_utf16le_to_utf32(
        const char16_t* input, size_t length, char32_t* utf32_buffer) noexcept;

    /// Convert possibly broken UTF-16BE string into UTF-32 string.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16BE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf32_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return number of written code units; 0 if input is not a valid UTF-16LE
    /// string
     [[nodiscard]] size_t convert_utf16be_to_utf32(
        const char16_t* input, size_t length, char32_t* utf32_buffer) noexcept;

    /// Using native endianness, convert possibly broken UTF-16 string into
    /// UTF-32 string and stop on error.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16 string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf32_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of char32_t written if
    /// successful.
     [[nodiscard]] UnicodeResult convert_utf16_to_utf32_with_errors(
        const char16_t* input, size_t length, char32_t* utf32_buffer) noexcept;


    /// Convert possibly broken UTF-16LE string into UTF-32 string and stop on error.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16LE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf32_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of char32_t written if
    /// successful.
     [[nodiscard]] UnicodeResult convert_utf16le_to_utf32_with_errors(
        const char16_t* input, size_t length, char32_t* utf32_buffer) noexcept;


    /// Convert possibly broken UTF-16BE string into UTF-32 string and stop on error.
    ///
    /// During the conversion also validation of the input string is done.
    /// This function is suitable to work with inputs from untrusted sources.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16BE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf32_buffer   the pointer to buffer that can hold conversion UnicodeResult
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of char32_t written if
    /// successful.
     [[nodiscard]] UnicodeResult convert_utf16be_to_utf32_with_errors(
        const char16_t* input, size_t length, char32_t* utf32_buffer) noexcept;

    /// Using native endianness, convert valid UTF-16 string into UTF-32 string.
    ///
    /// This function assumes that the input string is valid UTF-16 (native
    /// endianness).
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16 string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf32_buffer   the pointer to a buffer that can hold the conversion
    /// UnicodeResult
    /// @return number of written code units; 0 if conversion is not possible
     [[nodiscard]] size_t convert_valid_utf16_to_utf32(
        const char16_t* input, size_t length, char32_t* utf32_buffer) noexcept;


    /// Convert valid UTF-16LE string into UTF-32 string.
    ///
    /// This function assumes that the input string is valid UTF-16LE.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16LE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf32_buffer   the pointer to a buffer that can hold the conversion
    /// UnicodeResult
    /// @return number of written code units; 0 if conversion is not possible
     [[nodiscard]] size_t convert_valid_utf16le_to_utf32(
        const char16_t* input, size_t length, char32_t* utf32_buffer) noexcept;


    /// Convert valid UTF-16BE string into UTF-32 string.
    ///
    /// This function assumes that the input string is valid UTF-16LE.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16BE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param utf32_buffer   the pointer to a buffer that can hold the conversion
    /// UnicodeResult
    /// @return number of written code units; 0 if conversion is not possible
     [[nodiscard]] size_t convert_valid_utf16be_to_utf32(
        const char16_t* input, size_t length, char32_t* utf32_buffer) noexcept;

    /// Using native endianness; Compute the number of bytes that this UTF-16
    /// string would require in UTF-8 format.
    ///
    /// This function does not validate the input. It is acceptable to pass invalid
    /// UTF-16 strings but in such cases the UnicodeResult is implementation defined.
    ///
    /// @param input         the UTF-16 string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @return the number of bytes required to encode the UTF-16LE string as UTF-8
     [[nodiscard]] size_t utf8_length_from_utf16(const char16_t* input,
        size_t length) noexcept;


    /// Using native endianness; compute the number of bytes that this UTF-16
    /// string would require in UTF-8 format even when the UTF-16LE content contains
    /// mismatched surrogates that have to be replaced by the replacement character
    /// (0xFFFD).
    ///
    /// @param input         the UTF-16 string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) where the count is the number of bytes required to
    /// encode the UTF-16 string as UTF-8, and the error code is either SUCCESS or
    /// SURROGATE. The count is correct regardless of the error field.
    /// When SURROGATE is returned, it does not indicate an error in the case of this
    /// function: it indicates that at least one surrogate has been encountered: the
    /// surrogates may be matched or not (thus this function does not validate). If
    /// the returned error code is SUCCESS, then the input contains no surrogate, is
    /// in the Basic Multilingual Plane, and is necessarily valid.
     [[nodiscard]] UnicodeResult utf8_length_from_utf16_with_replacement(
        const char16_t* input, size_t length) noexcept;

    /// Compute the number of bytes that this UTF-16LE string would require in UTF-8
    /// format.
    ///
    /// This function does not validate the input. It is acceptable to pass invalid
    /// UTF-16 strings but in such cases the UnicodeResult is implementation defined.
    ///
    /// @param input         the UTF-16LE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @return the number of bytes required to encode the UTF-16LE string as UTF-8
     [[nodiscard]] size_t utf8_length_from_utf16le(const char16_t* input,
        size_t length) noexcept;

    /// Compute the number of bytes that this UTF-16BE string would require in UTF-8
    /// format.
    ///
    /// This function does not validate the input. It is acceptable to pass invalid
    /// UTF-16 strings but in such cases the UnicodeResult is implementation defined.
    ///
    /// @param input         the UTF-16BE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @return the number of bytes required to encode the UTF-16BE string as UTF-8
     [[nodiscard]] size_t utf8_length_from_utf16be(const char16_t* input,
        size_t length) noexcept;


    /// Change the endianness of the input. Can be used to go from UTF-16LE to
    /// UTF-16BE or from UTF-16BE to UTF-16LE.
    ///
    /// This function does not validate the input.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16 string to process
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @param output        the pointer to a buffer that can hold the conversion
    /// UnicodeResult
    void change_endianness_utf16(const char16_t* input, size_t length,
        char16_t* output) noexcept;


    /// Count the number of code points (characters) in the string assuming that
    /// it is valid.
    ///
    /// This function assumes that the input string is valid UTF-16 (native
    /// endianness). It is acceptable to pass invalid UTF-16 strings but in such
    /// cases the UnicodeResult is implementation defined.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16 string to process
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @return number of code points
     [[nodiscard]] size_t count_utf16(const char16_t* input,
        size_t length) noexcept;

    /// Count the number of code points (characters) in the string assuming that
    /// it is valid.
    ///
    /// This function assumes that the input string is valid UTF-16LE.
    /// It is acceptable to pass invalid UTF-16 strings but in such cases
    /// the UnicodeResult is implementation defined.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16LE string to process
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @return number of code points
     [[nodiscard]] size_t count_utf16le(const char16_t* input,
        size_t length) noexcept;

    /// Count the number of code points (characters) in the string assuming that
    /// it is valid.
    ///
    /// This function assumes that the input string is valid UTF-16BE.
    /// It is acceptable to pass invalid UTF-16 strings but in such cases
    /// the UnicodeResult is implementation defined.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16BE string to process
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @return number of code points
     [[nodiscard]] size_t count_utf16be(const char16_t* input,
        size_t length) noexcept;

    /// Given a valid UTF-16BE string having a possibly truncated last character,
    /// this function checks the end of string. If the last character is truncated
    /// (or partial), then it returns a shorter length (shorter by 1 unit) so that
    /// the short UTF-16BE strings only contain complete characters. If there is no
    /// truncated character, the original length is returned.
    ///
    /// This function assumes that the input string is valid UTF-16BE, but possibly
    /// truncated.
    ///
    /// @param input         the UTF-16BE string to process
    /// @param length        the length of the string in bytes
    /// @return the length of the string in bytes, possibly shorter by 1 unit
     [[nodiscard]] size_t trim_partial_utf16be(const char16_t* input,
        size_t length);


    /// Given a valid UTF-16LE string having a possibly truncated last character,
    /// this function checks the end of string. If the last character is truncated
    /// (or partial), then it returns a shorter length (shorter by 1 unit) so that
    /// the short UTF-16LE strings only contain complete characters. If there is no
    /// truncated character, the original length is returned.
    ///
    /// This function assumes that the input string is valid UTF-16LE, but possibly
    /// truncated.
    ///
    /// @param input         the UTF-16LE string to process
    /// @param length        the length of the string in bytes
    /// @return the length of the string in unit, possibly shorter by 1 unit
     [[nodiscard]] size_t trim_partial_utf16le(const char16_t* input,
        size_t length);

    /// Given a valid UTF-16 string having a possibly truncated last character,
    /// this function checks the end of string. If the last character is truncated
    /// (or partial), then it returns a shorter length (shorter by 1 unit) so that
    /// the short UTF-16 strings only contain complete characters. If there is no
    /// truncated character, the original length is returned.
    ///
    /// This function assumes that the input string is valid UTF-16, but possibly
    /// truncated. We use the native endianness.
    ///
    /// @param input         the UTF-16 string to process
    /// @param length        the length of the string in bytes
    /// @return the length of the string in unit, possibly shorter by 1 unit
     [[nodiscard]] size_t trim_partial_utf16(const char16_t* input,
        size_t length);

    /// Compute the number of bytes that this UTF-16 string would require in Latin1
    /// format.
    ///
    /// @param length        the length of the string in Latin1 code units (char)
    /// @return the length of the string in Latin1 code units (char) required to
    /// encode the UTF-16 string as Latin1
    [[nodiscard]] KUMO_FORCE_INLINE  size_t
    latin1_length_from_utf16(size_t length) noexcept {
        return length;
    }


    /// Using native endianness; Compute the number of bytes that this UTF-16
    /// string would require in UTF-32 format.
    ///
    /// This function is equivalent to count_utf16.
    ///
    /// This function does not validate the input. It is acceptable to pass invalid
    /// UTF-16 strings but in such cases the UnicodeResult is implementation defined.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16 string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @return the number of bytes required to encode the UTF-16LE string as UTF-32
     [[nodiscard]] size_t utf32_length_from_utf16(const char16_t* input,
        size_t length) noexcept;



    /// Compute the number of bytes that this UTF-16LE string would require in UTF-32
    /// format.
    ///
    /// This function is equivalent to count_utf16le.
    ///
    /// This function does not validate the input. It is acceptable to pass invalid
    /// UTF-16 strings but in such cases the UnicodeResult is implementation defined.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16LE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @return the number of bytes required to encode the UTF-16LE string as UTF-32
     [[nodiscard]] size_t utf32_length_from_utf16le(const char16_t* input,
        size_t length) noexcept;



    /// Compute the number of bytes that this UTF-16BE string would require in UTF-32
    /// format.
    ///
    /// This function is equivalent to count_utf16be.
    ///
    /// This function does not validate the input. It is acceptable to pass invalid
    /// UTF-16 strings but in such cases the UnicodeResult is implementation defined.
    ///
    /// This function is not BOM-aware.
    ///
    /// @param input         the UTF-16BE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @return the number of bytes required to encode the UTF-16BE string as UTF-32
     [[nodiscard]] size_t utf32_length_from_utf16be(const char16_t* input,
        size_t length) noexcept;


    /// Compute the number of bytes that this UTF-16LE string would require in UTF-8
    /// format even when the UTF-16LE content contains mismatched surrogates
    /// that have to be replaced by the replacement character (0xFFFD).
    ///
    /// @param input         the UTF-16LE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) where the count is the number of bytes required to
    /// encode the UTF-16LE string as UTF-8, and the error code is either SUCCESS or
    /// SURROGATE. The count is correct regardless of the error field.
    /// When SURROGATE is returned, it does not indicate an error in the case of this
    /// function: it indicates that at least one surrogate has been encountered: the
    /// surrogates may be matched or not (thus this function does not validate). If
    /// the returned error code is SUCCESS, then the input contains no surrogate, is
    /// in the Basic Multilingual Plane, and is necessarily valid.
     [[nodiscard]] UnicodeResult utf8_length_from_utf16le_with_replacement(
        const char16_t* input, size_t length) noexcept;


    /// Compute the number of bytes that this UTF-16BE string would require in UTF-8
    /// format even when the UTF-16BE content contains mismatched surrogates
    /// that have to be replaced by the replacement character (0xFFFD).
    ///
    /// @param input         the UTF-16BE string to convert
    /// @param length        the length of the string in 2-byte code units (char16_t)
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) where the count is the number of bytes required to
    /// encode the UTF-16BE string as UTF-8, and the error code is either SUCCESS or
    /// SURROGATE. The count is correct regardless of the error field.
    /// When SURROGATE is returned, it does not indicate an error in the case of this
    /// function: it indicates that at least one surrogate has been encountered: the
    /// surrogates may be matched or not (thus this function does not validate). If
    /// the returned error code is SUCCESS, then the input contains no surrogate, is
    /// in the Basic Multilingual Plane, and is necessarily valid.
     [[nodiscard]] UnicodeResult utf8_length_from_utf16be_with_replacement(
        const char16_t* input, size_t length) noexcept;



}  // namespace turbo
