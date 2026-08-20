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

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __has_include
#if __has_include(<uchar.h>)
#include <uchar.h>
#else // __has_include(<uchar.h>)
#define char16_t uint16_t
#define char32_t uint32_t
#endif // __has_include(<uchar.h>)
#else // __has_include(<uchar.h>)
#define char16_t uint16_t
#define char32_t uint32_t
#endif // __has_include

#ifdef __cplusplus
extern "C" {
#endif

/* C-friendly subset of simdutf errors */
typedef enum unicode_error_code {
    UNICODE_ERROR_SUCCESS = 0,
    UNICODE_ERROR_HEADER_BITS,
    UNICODE_ERROR_TOO_SHORT,
    UNICODE_ERROR_TOO_LONG,
    UNICODE_ERROR_OVERLONG,
    UNICODE_ERROR_TOO_LARGE,
    UNICODE_ERROR_SURROGATE,
    UNICODE_ERROR_INVALID_BASE64_CHARACTER,
    UNICODE_ERROR_BASE64_INPUT_REMAINDER,
    UNICODE_ERROR_BASE64_EXTRA_BITS,
    UNICODE_ERROR_OUTPUT_BUFFER_TOO_SMALL,
    UNICODE_ERROR_OTHER
} unicode_error_code;

typedef struct unicode_result {
    unicode_error_code error;
    size_t count; /* position of error or number of code units validated */
} unicode_result;

typedef struct unicode_full_result {
    unicode_error_code error;
    size_t input_count; /* number of input units consumed */
    size_t output_count; /* number of output bytes written */
} unicode_full_result;

typedef enum unicode_encoding_type {
    UNICODE_ENCODING_UNSPECIFIED = 0,
    UNICODE_ENCODING_UTF8 = 1,
    UNICODE_ENCODING_UTF16_LE = 2,
    UNICODE_ENCODING_UTF16_BE = 4,
    UNICODE_ENCODING_UTF32_LE = 8,
    UNICODE_ENCODING_UTF32_BE = 16
} unicode_encoding_type;

/* Validate UTF-8: returns true iff input is valid UTF-8 */
bool unicode_validate_utf8(const char* buf, size_t len);

/* Validate UTF-8 with detailed result */
unicode_result unicode_validate_utf8_with_errors(const char* buf, size_t len);

/* Encoding detection */
unicode_encoding_type unicode_autodetect_encoding(const char* input,
    size_t length);
int unicode_detect_encodings(const char* input, size_t length);

/* ASCII validation */
bool unicode_validate_ascii(const char* buf, size_t len);
unicode_result unicode_validate_ascii_with_errors(const char* buf, size_t len);

/* UTF-16 ASCII checks */
bool unicode_validate_utf16_as_ascii(const char16_t* buf, size_t len);
bool unicode_validate_utf16be_as_ascii(const char16_t* buf, size_t len);
bool unicode_validate_utf16le_as_ascii(const char16_t* buf, size_t len);

/* UTF-16/UTF-8/UTF-32 validation (native/endian-specific) */
bool unicode_validate_utf16(const char16_t* buf, size_t len);
bool unicode_validate_utf16le(const char16_t* buf, size_t len);
bool unicode_validate_utf16be(const char16_t* buf, size_t len);
unicode_result unicode_validate_utf16_with_errors(const char16_t* buf,
    size_t len);
unicode_result unicode_validate_utf16le_with_errors(const char16_t* buf,
    size_t len);
unicode_result unicode_validate_utf16be_with_errors(const char16_t* buf,
    size_t len);

bool unicode_validate_utf32(const char32_t* buf, size_t len);
unicode_result unicode_validate_utf32_with_errors(const char32_t* buf,
    size_t len);

/* to_well_formed UTF-16 helpers */
void unicode_to_well_formed_utf16le(const char16_t* input, size_t len,
    char16_t* output);
void unicode_to_well_formed_utf16be(const char16_t* input, size_t len,
    char16_t* output);
void unicode_to_well_formed_utf16(const char16_t* input, size_t len,
    char16_t* output);

/* Counting */
size_t unicode_count_utf16(const char16_t* input, size_t length);
size_t unicode_count_utf16le(const char16_t* input, size_t length);
size_t unicode_count_utf16be(const char16_t* input, size_t length);
size_t unicode_count_utf8(const char* input, size_t length);

/* Length estimators */
size_t unicode_utf8_length_from_latin1(const char* input, size_t length);
size_t unicode_latin1_length_from_utf8(const char* input, size_t length);
size_t unicode_latin1_length_from_utf16(size_t length);
size_t unicode_latin1_length_from_utf32(size_t length);
size_t unicode_utf16_length_from_utf8(const char* input, size_t length);
size_t unicode_utf32_length_from_utf8(const char* input, size_t length);
size_t unicode_utf8_length_from_utf16(const char16_t* input, size_t length);
size_t unicode_utf8_length_from_utf32(const char32_t* input, size_t length);
unicode_result
unicode_utf8_length_from_utf16_with_replacement(const char16_t* input,
    size_t length);
size_t unicode_utf8_length_from_utf16le(const char16_t* input, size_t length);
size_t unicode_utf8_length_from_utf16be(const char16_t* input, size_t length);
unicode_result
unicode_utf8_length_from_utf16le_with_replacement(const char16_t* input,
    size_t length);
unicode_result
unicode_utf8_length_from_utf16be_with_replacement(const char16_t* input,
    size_t length);

/* Conversions: latin1 <-> utf8, utf8 <-> utf16/utf32, utf16 <-> utf8, etc. */
size_t unicode_convert_latin1_to_utf8(const char* input, size_t length,
    char* output);
size_t unicode_convert_latin1_to_utf8_safe(const char* input, size_t length,
    char* output, size_t utf8_len);
size_t unicode_convert_latin1_to_utf16le(const char* input, size_t length,
    char16_t* output);
size_t unicode_convert_latin1_to_utf16be(const char* input, size_t length,
    char16_t* output);
size_t unicode_convert_latin1_to_utf16(const char* input, size_t length,
    char16_t* output);
size_t unicode_convert_latin1_to_utf32(const char* input, size_t length,
    char32_t* output);

size_t unicode_convert_utf8_to_latin1(const char* input, size_t length,
    char* output);
size_t unicode_convert_utf8_to_utf16le(const char* input, size_t length,
    char16_t* output);
size_t unicode_convert_utf8_to_utf16be(const char* input, size_t length,
    char16_t* output);
size_t unicode_convert_utf8_to_utf16(const char* input, size_t length,
    char16_t* output);

size_t unicode_convert_utf8_to_utf32(const char* input, size_t length,
    char32_t* output);
unicode_result unicode_convert_utf8_to_latin1_with_errors(const char* input,
    size_t length,
    char* output);
unicode_result unicode_convert_utf8_to_utf16_with_errors(const char* input,
    size_t length,
    char16_t* output);
unicode_result unicode_convert_utf8_to_utf16le_with_errors(const char* input,
    size_t length,
    char16_t* output);
unicode_result unicode_convert_utf8_to_utf16be_with_errors(const char* input,
    size_t length,
    char16_t* output);
unicode_result unicode_convert_utf8_to_utf32_with_errors(const char* input,
    size_t length,
    char32_t* output);

/* Conversions assuming valid input */
size_t unicode_convert_valid_utf8_to_latin1(const char* input, size_t length,
    char* output);
size_t unicode_convert_valid_utf8_to_utf16le(const char* input, size_t length,
    char16_t* output);
size_t unicode_convert_valid_utf8_to_utf16be(const char* input, size_t length,
    char16_t* output);
size_t unicode_convert_valid_utf8_to_utf32(const char* input, size_t length,
    char32_t* output);

/* UTF-16 -> UTF-8 and related conversions */
size_t unicode_convert_utf16_to_utf8(const char16_t* input, size_t length,
    char* output);
size_t unicode_convert_utf16le_to_utf8(const char16_t* input, size_t length,
    char* output);
size_t unicode_convert_utf16be_to_utf8(const char16_t* input, size_t length,
    char* output);
size_t unicode_convert_utf16_to_utf8_safe(const char16_t* input, size_t length,
    char* output, size_t utf8_len);
size_t unicode_convert_utf16_to_latin1(const char16_t* input, size_t length,
    char* output);
size_t unicode_convert_utf16le_to_latin1(const char16_t* input, size_t length,
    char* output);
size_t unicode_convert_utf16be_to_latin1(const char16_t* input, size_t length,
    char* output);
unicode_result
unicode_convert_utf16_to_latin1_with_errors(const char16_t* input,
    size_t length, char* output);
unicode_result
unicode_convert_utf16le_to_latin1_with_errors(const char16_t* input,
    size_t length, char* output);
unicode_result
unicode_convert_utf16be_to_latin1_with_errors(const char16_t* input,
    size_t length, char* output);

unicode_result unicode_convert_utf16_to_utf8_with_errors(const char16_t* input,
    size_t length,
    char* output);
unicode_result
unicode_convert_utf16le_to_utf8_with_errors(const char16_t* input,
    size_t length, char* output);
unicode_result
unicode_convert_utf16be_to_utf8_with_errors(const char16_t* input,
    size_t length, char* output);

/* Convert possibly broken UTF-16 to UTF-8, replacing each unpaired surrogate
   with U+FFFD (EF BF BD). These always succeed and return the number of bytes
   written. Size the output buffer with the matching
   unicode_utf8_length_from_utf16*_with_replacement function. */
size_t unicode_convert_utf16_to_utf8_with_replacement(const char16_t* input,
    size_t length,
    char* output);
size_t unicode_convert_utf16le_to_utf8_with_replacement(const char16_t* input,
    size_t length,
    char* output);
size_t unicode_convert_utf16be_to_utf8_with_replacement(const char16_t* input,
    size_t length,
    char* output);

size_t unicode_convert_valid_utf16_to_utf8(const char16_t* input, size_t length,
    char* output);
size_t unicode_convert_valid_utf16_to_latin1(const char16_t* input,
    size_t length, char* output);
size_t unicode_convert_valid_utf16le_to_latin1(const char16_t* input,
    size_t length, char* output);
size_t unicode_convert_valid_utf16be_to_latin1(const char16_t* input,
    size_t length, char* output);

size_t unicode_convert_valid_utf16le_to_utf8(const char16_t* input,
    size_t length, char* output);
size_t unicode_convert_valid_utf16be_to_utf8(const char16_t* input,
    size_t length, char* output);

/* UTF-16 <-> UTF-32 conversions */
size_t unicode_convert_utf16_to_utf32(const char16_t* input, size_t length,
    char32_t* output);
size_t unicode_convert_utf16le_to_utf32(const char16_t* input, size_t length,
    char32_t* output);
size_t unicode_convert_utf16be_to_utf32(const char16_t* input, size_t length,
    char32_t* output);
unicode_result unicode_convert_utf16_to_utf32_with_errors(const char16_t* input,
    size_t length,
    char32_t* output);
unicode_result
unicode_convert_utf16le_to_utf32_with_errors(const char16_t* input,
    size_t length, char32_t* output);
unicode_result
unicode_convert_utf16be_to_utf32_with_errors(const char16_t* input,
    size_t length, char32_t* output);

/* Valid UTF-16 conversions */
size_t unicode_convert_valid_utf16_to_utf32(const char16_t* input,
    size_t length, char32_t* output);
size_t unicode_convert_valid_utf16le_to_utf32(const char16_t* input,
    size_t length, char32_t* output);
size_t unicode_convert_valid_utf16be_to_utf32(const char16_t* input,
    size_t length, char32_t* output);

/* UTF-32 -> ... conversions */
size_t unicode_convert_utf32_to_utf8(const char32_t* input, size_t length,
    char* output);
unicode_result unicode_convert_utf32_to_utf8_with_errors(const char32_t* input,
    size_t length,
    char* output);
size_t unicode_convert_valid_utf32_to_utf8(const char32_t* input, size_t length,
    char* output);

size_t unicode_convert_utf32_to_utf16(const char32_t* input, size_t length,
    char16_t* output);
size_t unicode_convert_utf32_to_utf16le(const char32_t* input, size_t length,
    char16_t* output);
size_t unicode_convert_utf32_to_utf16be(const char32_t* input, size_t length,
    char16_t* output);
unicode_result
unicode_convert_utf32_to_latin1_with_errors(const char32_t* input,
    size_t length, char* output);

/* --- Find helpers --- */
const char* unicode_find(const char* start, const char* end, char character);
const char16_t* unicode_find_utf16(const char16_t* start, const char16_t* end,
    char16_t character);

/* --- Base64 enums and helpers --- */
typedef enum unicode_base64_options {
    UNICODE_BASE64_DEFAULT = 0,
    UNICODE_BASE64_URL = 1,
    UNICODE_BASE64_DEFAULT_NO_PADDING = 2,
    UNICODE_BASE64_URL_WITH_PADDING = 3,
    UNICODE_BASE64_DEFAULT_ACCEPT_GARBAGE = 4,
    UNICODE_BASE64_URL_ACCEPT_GARBAGE = 5,
    UNICODE_BASE64_DEFAULT_OR_URL = 8,
    UNICODE_BASE64_DEFAULT_OR_URL_ACCEPT_GARBAGE = 12
} unicode_base64_options;

typedef enum unicode_last_chunk_handling_options {
    UNICODE_LAST_CHUNK_LOOSE = 0,
    UNICODE_LAST_CHUNK_STRICT = 1,
    UNICODE_LAST_CHUNK_STOP_BEFORE_PARTIAL = 2,
    UNICODE_LAST_CHUNK_ONLY_FULL_CHUNKS = 3
} unicode_last_chunk_handling_options;

/* maximal binary length estimators */
size_t unicode_maximal_binary_length_from_base64(const char* input,
    size_t length);
size_t unicode_maximal_binary_length_from_base64_utf16(const char16_t* input,
    size_t length);

/* base64 decoding/encoding */
unicode_result unicode_base64_to_binary(
    const char* input, size_t length, char* output,
    unicode_base64_options options,
    unicode_last_chunk_handling_options last_chunk_options);
unicode_result unicode_base64_to_binary_utf16(
    const char16_t* input, size_t length, char* output,
    unicode_base64_options options,
    unicode_last_chunk_handling_options last_chunk_options);

size_t unicode_base64_length_from_binary(size_t length,
    unicode_base64_options options);
size_t unicode_base64_length_from_binary_with_lines(
    size_t length, unicode_base64_options options, size_t line_length);

size_t unicode_binary_to_base64(const char* input, size_t length, char* output,
    unicode_base64_options options);
size_t unicode_binary_to_base64_with_lines(const char* input, size_t length,
    char* output, size_t line_length,
    unicode_base64_options options);

/* safe decoding that provides an in/out outlen parameter */
unicode_result unicode_base64_to_binary_safe(
    const char* input, size_t length, char* output, size_t* outlen,
    unicode_base64_options options,
    unicode_last_chunk_handling_options last_chunk_options,
    bool decode_up_to_bad_char);
unicode_result unicode_base64_to_binary_safe_utf16(
    const char16_t* input, size_t length, char* output, size_t* outlen,
    unicode_base64_options options,
    unicode_last_chunk_handling_options last_chunk_options,
    bool decode_up_to_bad_char);

/* detailed decoding returning input_count and output_count */
unicode_full_result unicode_base64_to_binary_details(
    const char* input, size_t length, char* output,
    unicode_base64_options options,
    unicode_last_chunk_handling_options last_chunk_options);
unicode_full_result unicode_base64_to_binary_details_utf16(
    const char16_t* input, size_t length, char* output,
    unicode_base64_options options,
    unicode_last_chunk_handling_options last_chunk_options);

/* single-character base64 validation */
bool unicode_base64_valid(char input, unicode_base64_options options);
bool unicode_base64_valid_utf16(char16_t input, unicode_base64_options options);

#ifdef __cplusplus
} /* extern "C" */
#endif

