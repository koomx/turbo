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

    constexpr size_t default_line_length = 76; ///< default line length for base64 encoding with lines

    // base64_options are used to specify the base64 encoding options.
    // ASCII spaces are ' ', '\t', '\n', '\r', '\f'
    // garbage characters are characters that are not part of the base64 alphabet
    // nor ASCII spaces.
    constexpr uint64_t base64_reverse_padding = 2; /* modifier for base64_default and base64_url */
    enum base64_options : uint64_t {
        base64_default = 0, /* standard base64 format (with padding) */
        base64_url = 1, /* base64url format (no padding) */
        base64_default_no_padding = base64_default | base64_reverse_padding, /* standard base64 format without padding */
        base64_url_with_padding = base64_url | base64_reverse_padding, /* base64url with padding */
        base64_default_accept_garbage = 4, /* standard base64 format accepting garbage characters, the input stops
                                              with the first '=' if any */
        base64_url_accept_garbage = 5, /* base64url format accepting garbage characters, the input stops with
                                          the first '=' if any */
        base64_default_or_url = 8, /* standard/base64url hybrid format (only meaningful for decoding!) */
        base64_default_or_url_accept_garbage = 12, /* standard/base64url hybrid format accepting garbage characters
                                                      (only meaningful for decoding!), the input stops with the first '='
                                                      if any */
    };

    // last_chunk_handling_options are used to specify the handling of the last
    // chunk in base64 decoding.
    // https://tc39.es/proposal-arraybuffer-base64/spec/#sec-frombase64
    enum last_chunk_handling_options : uint64_t {
        loose = 0, /* standard base64 format, decode partial final chunk */
        strict = 1, /* error when the last chunk is partial, 2 or 3 chars, and
                       unpadded, or non-zero bit padding */
        stop_before_partial = 2, /* if the last chunk is partial, ignore it (no error) */
        only_full_chunks = 3 /* only decode full blocks (4 base64 characters, no padding) */
    };

    inline simdutf_constexpr23 bool
    is_partial(last_chunk_handling_options options) {
        return (options == stop_before_partial) || (options == only_full_chunks);
    }

    namespace detail {
        simdutf_warn_unused const char* find(const char* start, const char* end,
            char character) noexcept;
        simdutf_warn_unused const char16_t*
        find(const char16_t* start, const char16_t* end, char16_t character) noexcept;
    } // namespace detail

    /// Find the first occurrence of a character in a string. If the character is
    /// not found, return a pointer to the end of the string.
    /// @param start        the start of the string
    /// @param end          the end of the string
    /// @param character    the character to find
    /// @return a pointer to the first occurrence of the character in the string,
    /// or a pointer to the end of the string if the character is not found.
    simdutf_warn_unused simdutf_really_inline simdutf_constexpr23 const char*
    find(const char* start, const char* end, char character) noexcept {
#if SIMDUTF_CPLUSPLUS23
        if consteval {
            for (; start != end; ++start)
                if (*start == character)
                    return start;
            return end;
        } else
#endif
        {
            return detail::find(start, end, character);
        }
    }
    simdutf_warn_unused simdutf_really_inline simdutf_constexpr23 const char16_t*
    find(const char16_t* start, const char16_t* end, char16_t character) noexcept {
        // implementation note: this is repeated instead of a template, to ensure
        // the api is still a function and compiles without concepts
#if SIMDUTF_CPLUSPLUS23
        if consteval {
            for (; start != end; ++start)
                if (*start == character)
                    return start;
            return end;
        } else
#endif
        {
            return detail::find(start, end, character);
        }
    }
} // namespace turbo

#include <turbo/unicode/engine/base64_tables.h>
#include <turbo/unicode/engine/scalar/base64.h>

namespace turbo {

    inline std::string_view to_string(base64_options options) {
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
    /// spaces or linefeed characters), the result is exact. In particular, the
    /// function checks for padding characters.
    ///
    /// The function is fast (constant time). It checks up to two characters at
    /// the end of the string. The input is not otherwise validated or read.
    ///
    /// @param input         the base64 input to process
    /// @param length        the length of the base64 input in bytes
    /// @return maximum number of binary bytes
    simdutf_warn_unused size_t
    maximal_binary_length_from_base64(const char* input, size_t length) noexcept;


    /// Provide the maximal binary length in bytes given the base64 input.
    /// As long as the input does not contain ignorable characters (e.g., ASCII
    /// spaces or linefeed characters), the result is exact. In particular, the
    /// function checks for padding characters.
    ///
    /// The function is fast (constant time). It checks up to two characters at
    /// the end of the string. The input is not otherwise validated or read.
    ///
    /// @param input         the base64 input to process, in ASCII stored as 16-bit
    /// units
    /// @param length        the length of the base64 input in 16-bit units
    /// @return maximal number of binary bytes
    simdutf_warn_unused size_t maximal_binary_length_from_base64(
        const char16_t* input, size_t length) noexcept;


    /// Compute the binary length from a base64 input.
    /// This function is useful for base64 inputs that may contain ASCII whitespaces
    /// (such as line breaks). For such inputs, the result is exact, and for any
    /// inputs the result can be used to size the output buffer passed to
    /// `base64_to_binary`.
    ///
    /// The function ignores whitespace and does not require padding characters
    /// ('=').
    ///
    /// @param input         the base64 input to process
    /// @param length        the length of the base64 input in bytes
    /// @return number of binary bytes
    simdutf_warn_unused size_t binary_length_from_base64(const char* input,
        size_t length) noexcept;

    /// Compute the binary length from a base64 input.
    /// This function is useful for base64 inputs that may contain ASCII whitespaces
    /// (such as line breaks). For such inputs, the result is exact, and for any
    /// inputs the result can be used to size the output buffer passed to
    /// `base64_to_binary`.
    ///
    /// The function ignores whitespace and does not require padding characters
    /// ('=').
    ///
    /// @param input         the base64 input to process, in ASCII stored as 16-bit
    /// units
    /// @param length        the length of the base64 input in 16-bit units
    /// @return number of binary bytes
    simdutf_warn_unused size_t binary_length_from_base64(const char16_t* input,
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
    /// result (should be at least maximal_binary_length_from_base64(input, length)
    /// bytes long).
    /// @param options       the base64 options to use, usually base64_default or
    /// base64_url, and base64_default by default.
    /// @param last_chunk_options the last chunk handling options,
    /// last_chunk_handling_options::loose by default
    /// but can also be last_chunk_handling_options::strict or
    /// last_chunk_handling_options::stop_before_partial.
    /// @return a result pair struct (of type turbo::result containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in bytes) if any, or the number of bytes written if successful.
    simdutf_warn_unused result base64_to_binary(
        const char* input, size_t length, char* output,
        base64_options options = base64_default,
        last_chunk_handling_options last_chunk_options = loose) noexcept;

    /// Provide the base64 length in bytes given the length of a binary input.
    ///
    /// @param length        the length of the input in bytes
    /// @param options       the base64 options to use (default: base64_default)
    /// @return number of base64 bytes
    inline simdutf_warn_unused simdutf_constexpr23 size_t base64_length_from_binary(
        size_t length, base64_options options = base64_default) noexcept {
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
    inline simdutf_warn_unused simdutf_constexpr23 size_t
    base64_length_from_binary_with_lines(
        size_t length, base64_options options = base64_default,
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
    /// result (should be at least base64_length_from_binary(length) bytes long)
    /// @param options       the base64 options to use, can be base64_default or
    /// base64_url, is base64_default by default.
    /// @return number of written bytes, will be equal to
    /// base64_length_from_binary(length, options)
    size_t binary_to_base64(const char* input, size_t length, char* output,
        base64_options options = base64_default) noexcept;

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
    /// result (should be at least base64_length_from_binary_with_lines(length,
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
        base64_options options = base64_default) noexcept;

#if SIMDUTF_ATOMIC_REF
    /// Convert a binary input to a base64 output, using atomic accesses.
    /// This function comes with a potentially significant performance
    /// penalty, but it may be useful in some cases where the input
    /// buffers are shared between threads, to avoid undefined
    /// behavior in case of data races.
    ///
    /// The function is for advanced users. Its main use case is when
    /// to silence sanitizer warnings. We have no documented use case
    /// where this function is actually necessary in terms of practical correctness.
    ///
    /// This function is only available when simdutf is compiled with
    /// C++20 support and __cpp_lib_atomic_ref >= 201806L. You may check
    /// the availability of this function by checking the macro
    /// SIMDUTF_ATOMIC_REF.
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
    /// This function is considered experimental. It is not tested by default
    /// (see the CMake option SIMDUTF_ATOMIC_BASE64_TESTS) nor is it fuzz tested.
    /// It is not documented in the public API documentation (README). It is
    /// offered on a best effort basis. We rely on the community for further
    /// testing and feedback.
    ///
    /// @brief atomic_binary_to_base64
    /// @param input         the binary to process
    /// @param length        the length of the input in bytes
    /// @param output        the pointer to a buffer that can hold the conversion
    /// result (should be at least base64_length_from_binary(length) bytes long)
    /// @param options       the base64 options to use, can be base64_default or
    /// base64_url, is base64_default by default.
    /// @return number of written bytes, will be equal to
    /// base64_length_from_binary(length, options)
    size_t
    atomic_binary_to_base64(const char* input, size_t length, char* output,
        base64_options options = base64_default) noexcept;
#endif // SIMDUTF_ATOMIC_REF

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
    /// result (should be at least maximal_binary_length_from_base64(input, length)
    /// bytes long).
    /// @param options       the base64 options to use, can be base64_default or
    /// base64_url, is base64_default by default.
    /// @param last_chunk_options the last chunk handling options,
    /// last_chunk_handling_options::loose by default
    /// but can also be last_chunk_handling_options::strict or
    /// last_chunk_handling_options::stop_before_partial.
    /// @return a result pair struct (of type turbo::result containing the two
    /// fields error and count) with an error code and position of the
    /// INVALID_BASE64_CHARACTER error (in the input in units) if any, or the number
    /// of bytes written if successful.
    simdutf_warn_unused result
    base64_to_binary(const char16_t* input, size_t length, char* output,
        base64_options options = base64_default,
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
    /// result (should be at least maximal_binary_length_from_base64(input, length)
    /// bytes long).
    /// @param options       the base64 options to use, can be base64_default or
    /// base64_url, is base64_default by default.
    /// @param last_chunk_options the last chunk handling options,
    /// last_chunk_handling_options::loose by default
    /// but can also be last_chunk_handling_options::strict or
    /// last_chunk_handling_options::stop_before_partial.
    /// @return a full_result struct (of type turbo::full_result containing the
    /// three fields error, input_count and output_count).
    simdutf_warn_unused full_result
    base64_to_binary_details(const char* input, size_t length, char* output,
        base64_options options = base64_default,
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
    /// result (should be at least maximal_binary_length_from_base64(input, length)
    /// bytes long).
    /// @param options       the base64 options to use, can be base64_default or
    /// base64_url, is base64_default by default.
    /// @param last_chunk_options the last chunk handling options,
    /// last_chunk_handling_options::loose by default
    /// but can also be last_chunk_handling_options::strict or
    /// last_chunk_handling_options::stop_before_partial.
    /// @return a full_result struct (of type turbo::full_result containing the
    /// three fields error, input_count and output_count).
    simdutf_warn_unused full_result
    base64_to_binary_details(const char16_t* input, size_t length, char* output,
        base64_options options = base64_default,
        last_chunk_handling_options last_chunk_options = last_chunk_handling_options::loose) noexcept;

    /// Check if a character is an ignorable base64 character.
    /// Checking a large input, character by character, is not computationally
    /// efficient.
    ///
    /// @param input         the character to check
    /// @param options       the base64 options to use, is base64_default by default.
    /// @return true if the character is an ignorable base64 character, false
    /// otherwise.
    simdutf_warn_unused simdutf_really_inline simdutf_constexpr23 bool
    base64_ignorable(char input, base64_options options = base64_default) noexcept {
        return scalar::base64::is_ignorable(input, options);
    }
    simdutf_warn_unused simdutf_really_inline simdutf_constexpr23 bool
    base64_ignorable(char16_t input,
        base64_options options = base64_default) noexcept {
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
    simdutf_warn_unused simdutf_really_inline simdutf_constexpr23 bool
    base64_valid(char input, base64_options options = base64_default) noexcept {
        return scalar::base64::is_base64(input, options);
    }
    simdutf_warn_unused simdutf_really_inline simdutf_constexpr23 bool
    base64_valid(char16_t input, base64_options options = base64_default) noexcept {
        return scalar::base64::is_base64(input, options);
    }

    /// Check if a character is a valid base64 character or the padding character
    /// ('='). Checking a large input, character by character, is not computationally
    /// efficient.
    ///
    /// @param input         the character to check
    /// @param options       the base64 options to use, is base64_default by default.
    /// @return true if the character is a base64 character, false otherwise.
    simdutf_warn_unused simdutf_really_inline simdutf_constexpr23 bool
    base64_valid_or_padding(char input,
        base64_options options = base64_default) noexcept {
        return scalar::base64::is_base64_or_padding(input, options);
    }
    simdutf_warn_unused simdutf_really_inline simdutf_constexpr23 bool
    base64_valid_or_padding(char16_t input,
        base64_options options = base64_default) noexcept {
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
    /// result.
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
    /// @return a result pair struct (of type turbo::result containing the two
    /// fields error and count) with an error code and position of the
    /// INVALID_BASE64_CHARACTER error (in the input in units) if any, or the number
    /// of units processed if successful.
    simdutf_warn_unused result
    base64_to_binary_safe(const char* input, size_t length, char* output,
        size_t& outlen, base64_options options = base64_default,
        last_chunk_handling_options last_chunk_options = last_chunk_handling_options::loose,
        bool decode_up_to_bad_char = false) noexcept;
    // the span overload has moved to the bottom of the file

    simdutf_warn_unused result
    base64_to_binary_safe(const char16_t* input, size_t length, char* output,
        size_t& outlen, base64_options options = base64_default,
        last_chunk_handling_options last_chunk_options = last_chunk_handling_options::loose,
        bool decode_up_to_bad_char = false) noexcept;
    // span overload moved to bottom of file

#if SIMDUTF_ATOMIC_REF
    /// Convert a base64 input to a binary output with a size limit and using atomic
    /// operations.
    ///
    /// Like `base64_to_binary_safe` but using atomic operations, this function is
    /// thread-safe for concurrent memory access, allowing the output
    /// buffers to be shared between threads without undefined behavior in case of
    /// data races.
    ///
    /// This function comes with a potentially significant performance penalty, but
    /// is useful when thread safety is needed during base64 decoding.
    ///
    /// This function is only available when simdutf is compiled with
    /// C++20 support and __cpp_lib_atomic_ref >= 201806L. You may check
    /// the availability of this function by checking the macro
    /// SIMDUTF_ATOMIC_REF.
    ///
    /// This function is considered experimental. It is not tested by default
    /// (see the CMake option SIMDUTF_ATOMIC_BASE64_TESTS) nor is it fuzz tested.
    /// It is not documented in the public API documentation (README). It is
    /// offered on a best effort basis. We rely on the community for further
    /// testing and feedback.
    ///
    /// @param input         the base64 input to decode
    /// @param length        the length of the input in bytes
    /// @param output        the pointer to buffer that can hold the conversion
    /// result
    /// @param outlen        the number of bytes that can be written in the output
    /// buffer. Upon return, it is modified to reflect how many bytes were written.
    /// @param options       the base64 options to use (default, url, etc.)
    /// @param last_chunk_options the last chunk handling options (loose, strict,
    /// stop_before_partial)
    /// @param decode_up_to_bad_char if true, the function will decode up to the
    /// first invalid character. By default (false), it is assumed that the output
    /// buffer is to be discarded. When there are multiple errors in the input,
    /// using decode_up_to_bad_char might trigger a different error.
    /// @return a result struct with an error code and count indicating error
    /// position or success
    simdutf_warn_unused result atomic_base64_to_binary_safe(
        const char* input, size_t length, char* output, size_t& outlen,
        base64_options options = base64_default,
        last_chunk_handling_options last_chunk_options = last_chunk_handling_options::loose,
        bool decode_up_to_bad_char = false) noexcept;
    simdutf_warn_unused result atomic_base64_to_binary_safe(
        const char16_t* input, size_t length, char* output, size_t& outlen,
        base64_options options = base64_default,
        last_chunk_handling_options last_chunk_options = loose,
        bool decode_up_to_bad_char = false) noexcept;

#endif // SIMDUTF_ATOMIC_REF

}  // namespace turbo
