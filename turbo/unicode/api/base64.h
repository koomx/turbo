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
#include <turbo/unicode/api/base64_tables.h>
#include <turbo/unicode/scalar/base64.h>

namespace turbo {

    inline std::string_view to_string(Base64Options options) {
        switch (options) {
        case base64_default:
            return "base64_default";
        case base64_url:
            return "base64_url";
        case base64_reverse_padding:
            return "base64_reverse_padding";
        case base64_url_with_padding:
            return "base64_url_with_padding";
        case base64_default_accept_garbage:
            return "base64_default_accept_garbage";
        case base64_url_accept_garbage:
            return "base64_url_accept_garbage";
        case base64_default_or_url:
            return "base64_default_or_url";
        case base64_default_or_url_accept_garbage:
            return "base64_default_or_url_accept_garbage";
        }
        return "<unknown>";
    }

    inline std::string_view to_string(last_chunk_handling_options options) {
        switch (options) {
        case loose:
            return "loose";
        case strict:
            return "strict";
        case stop_before_partial:
            return "stop_before_partial";
        case only_full_chunks:
            return "only_full_chunks";
        }
        return "<unknown>";
    }

    /// Provide the maximal binary length in bytes given the base64 input.
    /// As long as the input does not contain ignorable characters (e.g., ASCII
    /// spaces or linefeed characters), the UnicodeResult is exact. In particular, the
    /// function checks for padding characters.
    ///
    /// The function is fast (constant time). It checks up to two characters at
    /// the end of the string. The input is not otherwise validated or read.
    ///
    /// @param input         the base64 input to process
    /// @param length        the length of the base64 input in bytes
    /// @return maximum number of binary bytes
     [[nodiscard]] size_t
    maximal_binary_length_from_base64(const char* input, size_t length) noexcept;


    /// Provide the maximal binary length in bytes given the base64 input.
    /// As long as the input does not contain ignorable characters (e.g., ASCII
    /// spaces or linefeed characters), the UnicodeResult is exact. In particular, the
    /// function checks for padding characters.
    ///
    /// The function is fast (constant time). It checks up to two characters at
    /// the end of the string. The input is not otherwise validated or read.
    ///
    /// @param input         the base64 input to process, in ASCII stored as 16-bit
    /// units
    /// @param length        the length of the base64 input in 16-bit units
    /// @return maximal number of binary bytes
     [[nodiscard]] size_t maximal_binary_length_from_base64(
        const char16_t* input, size_t length) noexcept;


    /// Compute the binary length from a base64 input.
    /// This function is useful for base64 inputs that may contain ASCII whitespaces
    /// (such as line breaks). For such inputs, the UnicodeResult is exact, and for any
    /// inputs the UnicodeResult can be used to size the output buffer passed to
    /// `base64_to_binary`.
    ///
    /// The function ignores whitespace and does not require padding characters
    /// ('=').
    ///
    /// @param input         the base64 input to process
    /// @param length        the length of the base64 input in bytes
    /// @return number of binary bytes
     [[nodiscard]] size_t binary_length_from_base64(const char* input,
        size_t length) noexcept;

    /// Compute the binary length from a base64 input.
    /// This function is useful for base64 inputs that may contain ASCII whitespaces
    /// (such as line breaks). For such inputs, the UnicodeResult is exact, and for any
    /// inputs the UnicodeResult can be used to size the output buffer passed to
    /// `base64_to_binary`.
    ///
    /// The function ignores whitespace and does not require padding characters
    /// ('=').
    ///
    /// @param input         the base64 input to process, in ASCII stored as 16-bit
    /// units
    /// @param length        the length of the base64 input in 16-bit units
    /// @return number of binary bytes
     [[nodiscard]] size_t binary_length_from_base64(const char16_t* input,
        size_t length) noexcept;

    /// Convert a base64 input to a binary output.
    ///
    /// This function follows the WHATWG forgiving-base64 format, which means that it
    /// will ignore any ASCII spaces in the input. You may provide a padded input
    /// (with one or two equal signs at the end) or an unpadded input (without any
    /// equal signs at the end).
    ///
    /// See https://infra.spec.whatwg.org/#forgiving-base64-decode
    ///
    /// This function will fail in case of invalid input. When last_chunk_options =
    /// loose, there are two possible reasons for failure: the input contains a
    /// number of base64 characters that when divided by 4, leaves a single remainder
    /// character (BASE64_INPUT_REMAINDER), or the input contains a character that is
    /// not a valid base64 character (INVALID_BASE64_CHARACTER).
    ///
    /// When the error is INVALID_BASE64_CHARACTER, r.count contains the index in the
    /// input where the invalid character was found. When the error is
    /// BASE64_INPUT_REMAINDER, then r.count contains the number of bytes decoded.
    ///
    /// The default option (turbo::base64_default) expects the characters `+` and
    /// `/` as part of its alphabet. The URL option (turbo::base64_url) expects the
    /// characters `-` and `_` as part of its alphabet.
    ///
    /// The padding (`=`) is validated if present. There may be at most two padding
    /// characters at the end of the input. If there are any padding characters, the
    /// total number of characters (excluding spaces but including padding
    /// characters) must be divisible by four.
    ///
    /// You should call this function with a buffer that is at least
    /// maximal_binary_length_from_base64(input, length) bytes long. If you fail to
    /// provide that much space, the function may cause a buffer overflow.
    ///
    /// Advanced users may want to tailor how the last chunk is handled. By default,
    /// we use a loose (forgiving) approach but we also support a strict approach
    /// as well as a stop_before_partial approach, as per the following proposal:
    ///
    /// https://tc39.es/proposal-arraybuffer-base64/spec/#sec-frombase64
    ///
    /// @param input         the base64 string to process
    /// @param length        the length of the string in bytes
    /// @param output        the pointer to a buffer that can hold the conversion
    /// UnicodeResult (should be at least maximal_binary_length_from_base64(input, length)
    /// bytes long).
    /// @param options       the base64 options to use, usually base64_default or
    /// base64_url, and base64_default by default.
    /// @param last_chunk_options the last chunk handling options,
    /// last_chunk_handling_options::loose by default
    /// but can also be last_chunk_handling_options::strict or
    /// last_chunk_handling_options::stop_before_partial.
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in bytes) if any, or the number of bytes written if successful.
     [[nodiscard]] UnicodeResult base64_to_binary(
        const char* input, size_t length, char* output,
        Base64Options options = base64_default,
        last_chunk_handling_options last_chunk_options = loose) noexcept;

    /// Provide the base64 length in bytes given the length of a binary input.
    ///
    /// @param length        the length of the input in bytes
    /// @param options       the base64 options to use (default: base64_default)
    /// @return number of base64 bytes
    [[nodiscard]] inline  size_t base64_length_from_binary(
        size_t length, Base64Options options = base64_default) noexcept {
        return scalar::base64::base64_length_from_binary(length, options);
    }

    /// Provide the base64 length in bytes given the length of a binary input,
    /// taking into account line breaks.
    ///
    /// @param length        the length of the input in bytes
    /// @param options       the base64 options to use (default: base64_default)
    /// @param line_length   the length of lines, must be at least 4 (otherwise it is
    /// interpreted as 4),
    /// @return number of base64 bytes
    [[nodiscard]] inline  size_t
    base64_length_from_binary_with_lines(
        size_t length, Base64Options options = base64_default,
        size_t line_length = default_line_length) noexcept {
        return scalar::base64::base64_length_from_binary_with_lines(length, options,
            line_length);
    }

    /// Convert a binary input to a base64 output.
    ///
    /// The default option (turbo::base64_default) uses the characters `+` and `/`
    /// as part of its alphabet. Further, it adds padding (`=`) at the end of the
    /// output to ensure that the output length is a multiple of four.
    ///
    /// The URL option (turbo::base64_url) uses the characters `-` and `_` as part
    /// of its alphabet. No padding is added at the end of the output.
    ///
    /// This function always succeeds.
    ///
    /// @param input         the binary to process
    /// @param length        the length of the input in bytes
    /// @param output        the pointer to a buffer that can hold the conversion
    /// UnicodeResult (should be at least base64_length_from_binary(length) bytes long)
    /// @param options       the base64 options to use, can be base64_default or
    /// base64_url, is base64_default by default.
    /// @return number of written bytes, will be equal to
    /// base64_length_from_binary(length, options)
    size_t binary_to_base64(const char* input, size_t length, char* output,
        Base64Options options = base64_default) noexcept;

    /// Convert a binary input to a base64 output with line breaks.
    ///
    /// The default option (turbo::base64_default) uses the characters `+` and `/`
    /// as part of its alphabet. Further, it adds padding (`=`) at the end of the
    /// output to ensure that the output length is a multiple of four.
    ///
    /// The URL option (turbo::base64_url) uses the characters `-` and `_` as part
    /// of its alphabet. No padding is added at the end of the output.
    ///
    /// This function always succeeds.
    ///
    /// @param input         the binary to process
    /// @param length        the length of the input in bytes
    /// @param output        the pointer to a buffer that can hold the conversion
    /// UnicodeResult (should be at least base64_length_from_binary_with_lines(length,
    /// options, line_length) bytes long)
    /// @param line_length   the length of lines, must be at least 4 (otherwise it is
    /// interpreted as 4),
    /// @param options       the base64 options to use, can be base64_default or
    /// base64_url, is base64_default by default.
    /// @return number of written bytes, will be equal to
    /// base64_length_from_binary_with_lines(length, options)
    size_t
    binary_to_base64_with_lines(const char* input, size_t length, char* output,
        size_t line_length = turbo::default_line_length,
        Base64Options options = base64_default) noexcept;

    /// Convert a base64 input to a binary output.
    ///
    /// This function follows the WHATWG forgiving-base64 format, which means that it
    /// will ignore any ASCII spaces in the input. You may provide a padded input
    /// (with one or two equal signs at the end) or an unpadded input (without any
    /// equal signs at the end).
    ///
    /// See https://infra.spec.whatwg.org/#forgiving-base64-decode
    ///
    /// This function will fail in case of invalid input. When last_chunk_options =
    /// loose, there are two possible reasons for failure: the input contains a
    /// number of base64 characters that when divided by 4, leaves a single remainder
    /// character (BASE64_INPUT_REMAINDER), or the input contains a character that is
    /// not a valid base64 character (INVALID_BASE64_CHARACTER).
    ///
    /// When the error is INVALID_BASE64_CHARACTER, r.count contains the index in the
    /// input where the invalid character was found. When the error is
    /// BASE64_INPUT_REMAINDER, then r.count contains the number of bytes decoded.
    ///
    /// The default option (turbo::base64_default) expects the characters `+` and
    /// `/` as part of its alphabet. The URL option (turbo::base64_url) expects the
    /// characters `-` and `_` as part of its alphabet.
    ///
    /// The padding (`=`) is validated if present. There may be at most two padding
    /// characters at the end of the input. If there are any padding characters, the
    /// total number of characters (excluding spaces but including padding
    /// characters) must be divisible by four.
    ///
    /// You should call this function with a buffer that is at least
    /// maximal_binary_length_from_base64(input, length) bytes long. If you fail
    /// to provide that much space, the function may cause a buffer overflow.
    ///
    /// Advanced users may want to tailor how the last chunk is handled. By default,
    /// we use a loose (forgiving) approach but we also support a strict approach
    /// as well as a stop_before_partial approach, as per the following proposal:
    ///
    /// https://tc39.es/proposal-arraybuffer-base64/spec/#sec-frombase64
    ///
    /// @param input         the base64 string to process, in ASCII stored as 16-bit
    /// units
    /// @param length        the length of the string in 16-bit units
    /// @param output        the pointer to a buffer that can hold the conversion
    /// UnicodeResult (should be at least maximal_binary_length_from_base64(input, length)
    /// bytes long).
    /// @param options       the base64 options to use, can be base64_default or
    /// base64_url, is base64_default by default.
    /// @param last_chunk_options the last chunk handling options,
    /// last_chunk_handling_options::loose by default
    /// but can also be last_chunk_handling_options::strict or
    /// last_chunk_handling_options::stop_before_partial.
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and position of the
    /// INVALID_BASE64_CHARACTER error (in the input in units) if any, or the number
    /// of bytes written if successful.
     [[nodiscard]] UnicodeResult
    base64_to_binary(const char16_t* input, size_t length, char* output,
        Base64Options options = base64_default,
        last_chunk_handling_options last_chunk_options = last_chunk_handling_options::loose) noexcept;


    /// Convert a base64 input to a binary output while returning more details
    /// than base64_to_binary.
    ///
    /// This function follows the WHATWG forgiving-base64 format, which means that it
    /// will ignore any ASCII spaces in the input. You may provide a padded input
    /// (with one or two equal signs at the end) or an unpadded input (without any
    /// equal signs at the end).
    ///
    /// See https://infra.spec.whatwg.org/#forgiving-base64-decode
    ///
    /// Unlike base64_to_binary, this function returns a full_result with both
    /// input_count and output_count, so you always know how much input was consumed
    /// and how much output was written. There are three cases where the input may
    /// not be fully consumed:
    ///
    /// 1. stop_before_partial: When last_chunk_options is set to
    ///    stop_before_partial, any incomplete 4-character group at the end of the
    ///    input is left unconsumed. This is useful for streaming/chunked decoding
    ///    where you can carry over the unconsumed input to the next chunk.
    ///
    /// 2. INVALID_BASE64_CHARACTER: The input contains a character that is not a
    ///    valid base64 character. In this case, input_count indicates where the
    ///    invalid character was found.
    ///
    /// 3. BASE64_INPUT_REMAINDER: When last_chunk_options is loose, the input
    ///    contains a number of base64 characters that, when divided by 4, leaves
    ///    a single remainder character (which cannot encode any bytes).
    ///
    /// You should call this function with a buffer that is at least
    /// maximal_binary_length_from_base64(input, length) bytes long. If you fail to
    /// provide that much space, the function may cause a buffer overflow.
    ///
    /// @param input         the base64 string to process
    /// @param length        the length of the string in bytes
    /// @param output        the pointer to a buffer that can hold the conversion
    /// UnicodeResult (should be at least maximal_binary_length_from_base64(input, length)
    /// bytes long).
    /// @param options       the base64 options to use, can be base64_default or
    /// base64_url, is base64_default by default.
    /// @param last_chunk_options the last chunk handling options,
    /// last_chunk_handling_options::loose by default
    /// but can also be last_chunk_handling_options::strict or
    /// last_chunk_handling_options::stop_before_partial.
    /// @return a full_result struct (of type turbo::full_result containing the
    /// three fields error, input_count and output_count).
     [[nodiscard]] full_result
    base64_to_binary_details(const char* input, size_t length, char* output,
        Base64Options options = base64_default,
        last_chunk_handling_options last_chunk_options = last_chunk_handling_options::loose) noexcept;


    /// Convert a base64 input to a binary output while returning more details
    /// than base64_to_binary.
    ///
    /// This function follows the WHATWG forgiving-base64 format, which means that it
    /// will ignore any ASCII spaces in the input. You may provide a padded input
    /// (with one or two equal signs at the end) or an unpadded input (without any
    /// equal signs at the end).
    ///
    /// See https://infra.spec.whatwg.org/#forgiving-base64-decode
    ///
    /// Unlike base64_to_binary, this function returns a full_result with both
    /// input_count and output_count, so you always know how much input was consumed
    /// and how much output was written. There are three cases where the input may
    /// not be fully consumed:
    ///
    /// 1. stop_before_partial: When last_chunk_options is set to
    ///    stop_before_partial, any incomplete 4-character group at the end of the
    ///    input is left unconsumed. This is useful for streaming/chunked decoding
    ///    where you can carry over the unconsumed input to the next chunk.
    ///
    /// 2. INVALID_BASE64_CHARACTER: The input contains a character that is not a
    ///    valid base64 character. In this case, input_count indicates where the
    ///    invalid character was found.
    ///
    /// 3. BASE64_INPUT_REMAINDER: When last_chunk_options is loose, the input
    ///    contains a number of base64 characters that, when divided by 4, leaves
    ///    a single remainder character (which cannot encode any bytes).
    ///
    /// You should call this function with a buffer that is at least
    /// maximal_binary_length_from_base64(input, length) bytes long. If you fail to
    /// provide that much space, the function may cause a buffer overflow.
    ///
    /// @param input         the base64 string to process, in ASCII stored as 16-bit
    /// units
    /// @param length        the length of the string in 16-bit units
    /// @param output        the pointer to a buffer that can hold the conversion
    /// UnicodeResult (should be at least maximal_binary_length_from_base64(input, length)
    /// bytes long).
    /// @param options       the base64 options to use, can be base64_default or
    /// base64_url, is base64_default by default.
    /// @param last_chunk_options the last chunk handling options,
    /// last_chunk_handling_options::loose by default
    /// but can also be last_chunk_handling_options::strict or
    /// last_chunk_handling_options::stop_before_partial.
    /// @return a full_result struct (of type turbo::full_result containing the
    /// three fields error, input_count and output_count).
     [[nodiscard]] full_result
    base64_to_binary_details(const char16_t* input, size_t length, char* output,
        Base64Options options = base64_default,
        last_chunk_handling_options last_chunk_options = last_chunk_handling_options::loose) noexcept;

    /// Check if a character is an ignorable base64 character.
    /// Checking a large input, character by character, is not computationally
    /// efficient.
    ///
    /// @param input         the character to check
    /// @param options       the base64 options to use, is base64_default by default.
    /// @return true if the character is an ignorable base64 character, false
    /// otherwise.
     [[nodiscard]] KUMO_FORCE_INLINE  bool
    base64_ignorable(char input, Base64Options options = base64_default) noexcept {
        return scalar::base64::is_ignorable(input, options);
    }
     [[nodiscard]] KUMO_FORCE_INLINE  bool
    base64_ignorable(char16_t input,
        Base64Options options = base64_default) noexcept {
        return scalar::base64::is_ignorable(input, options);
    }

    /// Check if a character is a valid base64 character.
    /// Checking a large input, character by character, is not computationally
    /// efficient.
    /// Note that padding characters are not considered valid base64 characters in
    /// this context, nor are spaces.
    ///
    /// @param input         the character to check
    /// @param options       the base64 options to use, is base64_default by default.
    /// @return true if the character is a base64 character, false otherwise.
     [[nodiscard]] KUMO_FORCE_INLINE  bool
    base64_valid(char input, Base64Options options = base64_default) noexcept {
        return scalar::base64::is_base64(input, options);
    }
     [[nodiscard]] KUMO_FORCE_INLINE  bool
    base64_valid(char16_t input, Base64Options options = base64_default) noexcept {
        return scalar::base64::is_base64(input, options);
    }

    /// Check if a character is a valid base64 character or the padding character
    /// ('='). Checking a large input, character by character, is not computationally
    /// efficient.
    ///
    /// @param input         the character to check
    /// @param options       the base64 options to use, is base64_default by default.
    /// @return true if the character is a base64 character, false otherwise.
     [[nodiscard]] KUMO_FORCE_INLINE  bool
    base64_valid_or_padding(char input,
        Base64Options options = base64_default) noexcept {
        return scalar::base64::is_base64_or_padding(input, options);
    }
     [[nodiscard]] KUMO_FORCE_INLINE  bool
    base64_valid_or_padding(char16_t input,
        Base64Options options = base64_default) noexcept {
        return scalar::base64::is_base64_or_padding(input, options);
    }

    /// Convert a base64 input to a binary output.
    ///
    /// This function follows the WHATWG forgiving-base64 format, which means that it
    /// will ignore any ASCII spaces in the input. You may provide a padded input
    /// (with one or two equal signs at the end) or an unpadded input (without any
    /// equal signs at the end).
    ///
    /// See https://infra.spec.whatwg.org/#forgiving-base64-decode
    ///
    /// This function will fail in case of invalid input. When last_chunk_options =
    /// loose, there are three possible reasons for failure: the input contains a
    /// number of base64 characters that when divided by 4, leaves a single remainder
    /// character (BASE64_INPUT_REMAINDER), the input contains a character that is
    /// not a valid base64 character (INVALID_BASE64_CHARACTER), or the output buffer
    /// is too small (OUTPUT_BUFFER_TOO_SMALL).
    ///
    /// When OUTPUT_BUFFER_TOO_SMALL, we return both the number of bytes written
    /// and the number of units processed, see description of the parameters and
    /// returned value.
    ///
    /// When the error is INVALID_BASE64_CHARACTER, r.count contains the index in the
    /// input where the invalid character was found. When the error is
    /// BASE64_INPUT_REMAINDER, then r.count contains the number of bytes decoded.
    ///
    /// The default option (turbo::base64_default) expects the characters `+` and
    /// `/` as part of its alphabet. The URL option (turbo::base64_url) expects the
    /// characters `-` and `_` as part of its alphabet.
    ///
    /// The padding (`=`) is validated if present. There may be at most two padding
    /// characters at the end of the input. If there are any padding characters, the
    /// total number of characters (excluding spaces but including padding
    /// characters) must be divisible by four.
    ///
    /// The INVALID_BASE64_CHARACTER cases are considered fatal and you are expected
    /// to discard the output unless the parameter decode_up_to_bad_char is set to
    /// true. In that case, the function will decode up to the first invalid
    /// character. Extra padding characters ('=') are considered invalid characters.
    ///
    /// Advanced users may want to tailor how the last chunk is handled. By default,
    /// we use a loose (forgiving) approach but we also support a strict approach
    /// as well as a stop_before_partial approach, as per the following proposal:
    ///
    /// https://tc39.es/proposal-arraybuffer-base64/spec/#sec-frombase64
    ///
    /// The base64_to_binary_safe function has negligible overhead compared with
    /// base64_to_binary in the absence of ignorable characters; however, on short
    /// inputs containing ignorable characters, it can be up to three times slower.
    ///
    /// @param input         the base64 string to process, in ASCII stored as 8-bit
    /// or 16-bit units
    /// @param length        the length of the string in 8-bit or 16-bit units.
    /// @param output        the pointer to a buffer that can hold the conversion
    /// UnicodeResult.
    /// @param outlen        the number of bytes that can be written in the output
    /// buffer. Upon return, it is modified to reflect how many bytes were written.
    /// @param options       the base64 options to use, can be base64_default or
    /// base64_url, is base64_default by default.
    /// @param last_chunk_options the last chunk handling options,
    /// last_chunk_handling_options::loose by default
    /// but can also be last_chunk_handling_options::strict or
    /// last_chunk_handling_options::stop_before_partial.
    /// @param decode_up_to_bad_char if true, the function will decode up to the
    /// first invalid character. By default (false), it is assumed that the output
    /// buffer is to be discarded. When there are multiple errors in the input,
    /// using decode_up_to_bad_char might trigger a different error.
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and position of the
    /// INVALID_BASE64_CHARACTER error (in the input in units) if any, or the number
    /// of units processed if successful.
     [[nodiscard]] UnicodeResult
    base64_to_binary_safe(const char* input, size_t length, char* output,
        size_t& outlen, Base64Options options = base64_default,
        last_chunk_handling_options last_chunk_options = last_chunk_handling_options::loose,
        bool decode_up_to_bad_char = false) noexcept;
    // the span overload has moved to the bottom of the file

     [[nodiscard]] UnicodeResult
    base64_to_binary_safe(const char16_t* input, size_t length, char* output,
        size_t& outlen, Base64Options options = base64_default,
        last_chunk_handling_options last_chunk_options = last_chunk_handling_options::loose,
        bool decode_up_to_bad_char = false) noexcept;
    // span overload moved to bottom of file

}  // namespace turbo
