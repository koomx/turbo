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

#include <turbo/unicode/utf_c.h>
#include <turbo/unicode/engine/implementation.h>

static unicode_result to_c_result(const turbo::UnicodeResult& r) {
    unicode_result out;
    out.error = static_cast<unicode_error_code>(r.error);
    out.count = r.count;
    return out;
}

/* The C wrapper depends on the library features. Only expose the C API
   when all relevant feature is enabled. This helps the
   single-header generator to omit the C wrapper when features are
   disabled. */
// clang-format off
// clang-format on
extern "C" {

bool unicode_validate_utf8(const char* buf, size_t len) {
    return turbo::validate_utf8(buf, len);
}

unicode_result unicode_validate_utf8_with_errors(const char* buf, size_t len) {
    return to_c_result(turbo::validate_utf8_with_errors(buf, len));
}

unicode_encoding_type unicode_autodetect_encoding(const char* input,
    size_t length) {
    return static_cast<unicode_encoding_type>(
        turbo::autodetect_encoding(input, length));
}

int unicode_detect_encodings(const char* input, size_t length) {
    return turbo::detect_encodings(input, length);
}

bool unicode_validate_ascii(const char* buf, size_t len) {
    return turbo::validate_ascii(buf, len);
}
unicode_result unicode_validate_ascii_with_errors(const char* buf, size_t len) {
    return to_c_result(turbo::validate_ascii_with_errors(buf, len));
}

bool unicode_validate_utf16_as_ascii(const char16_t* buf, size_t len) {
    return turbo::validate_utf16_as_ascii(buf, len);
}
bool unicode_validate_utf16be_as_ascii(const char16_t* buf, size_t len) {
    return turbo::validate_utf16be_as_ascii(buf, len);
}
bool unicode_validate_utf16le_as_ascii(const char16_t* buf, size_t len) {
    return turbo::validate_utf16le_as_ascii(buf, len);
}

bool unicode_validate_utf16(const char16_t* buf, size_t len) {
    return turbo::validate_utf16(buf, len);
}
bool unicode_validate_utf16le(const char16_t* buf, size_t len) {
    return turbo::validate_utf16le(buf, len);
}
bool unicode_validate_utf16be(const char16_t* buf, size_t len) {
    return turbo::validate_utf16be(buf, len);
}
unicode_result unicode_validate_utf16_with_errors(const char16_t* buf,
    size_t len) {
    return to_c_result(turbo::validate_utf16_with_errors(buf, len));
}
unicode_result unicode_validate_utf16le_with_errors(const char16_t* buf,
    size_t len) {
    return to_c_result(turbo::validate_utf16le_with_errors(buf, len));
}
unicode_result unicode_validate_utf16be_with_errors(const char16_t* buf,
    size_t len) {
    return to_c_result(turbo::validate_utf16be_with_errors(buf, len));
}

bool unicode_validate_utf32(const char32_t* buf, size_t len) {
    return turbo::validate_utf32(buf, len);
}
unicode_result unicode_validate_utf32_with_errors(const char32_t* buf,
    size_t len) {
    return to_c_result(turbo::validate_utf32_with_errors(buf, len));
}

void unicode_to_well_formed_utf16le(const char16_t* input, size_t len,
    char16_t* output) {
    turbo::to_well_formed_utf16le(input, len, output);
}
void unicode_to_well_formed_utf16be(const char16_t* input, size_t len,
    char16_t* output) {
    turbo::to_well_formed_utf16be(input, len, output);
}
void unicode_to_well_formed_utf16(const char16_t* input, size_t len,
    char16_t* output) {
    turbo::to_well_formed_utf16(input, len, output);
}

size_t unicode_count_utf16(const char16_t* input, size_t length) {
    return turbo::count_utf16(input, length);
}
size_t unicode_count_utf16le(const char16_t* input, size_t length) {
    return turbo::count_utf16le(input, length);
}
size_t unicode_count_utf16be(const char16_t* input, size_t length) {
    return turbo::count_utf16be(input, length);
}
size_t unicode_count_utf8(const char* input, size_t length) {
    return turbo::count_utf8(input, length);
}

size_t unicode_utf8_length_from_latin1(const char* input, size_t length) {
    return turbo::utf8_length_from_latin1(input, length);
}
size_t unicode_latin1_length_from_utf8(const char* input, size_t length) {
    return turbo::latin1_length_from_utf8(input, length);
}
size_t unicode_latin1_length_from_utf16(size_t length) {
    return turbo::latin1_length_from_utf16(length);
}
size_t unicode_latin1_length_from_utf32(size_t length) {
    return turbo::latin1_length_from_utf32(length);
}
size_t unicode_utf16_length_from_utf8(const char* input, size_t length) {
    return turbo::utf16_length_from_utf8(input, length);
}
size_t unicode_utf32_length_from_utf8(const char* input, size_t length) {
    return turbo::utf32_length_from_utf8(input, length);
}
size_t unicode_utf8_length_from_utf16(const char16_t* input, size_t length) {
    return turbo::utf8_length_from_utf16(input, length);
}
size_t unicode_utf8_length_from_utf32(const char32_t* input, size_t length) {
    return turbo::utf8_length_from_utf32(input, length);
}
unicode_result
unicode_utf8_length_from_utf16_with_replacement(const char16_t* input,
    size_t length) {
    return to_c_result(
        turbo::utf8_length_from_utf16_with_replacement(input, length));
}
size_t unicode_utf8_length_from_utf16le(const char16_t* input, size_t length) {
    return turbo::utf8_length_from_utf16le(input, length);
}
size_t unicode_utf8_length_from_utf16be(const char16_t* input, size_t length) {
    return turbo::utf8_length_from_utf16be(input, length);
}
unicode_result
unicode_utf8_length_from_utf16le_with_replacement(const char16_t* input,
    size_t length) {
    return to_c_result(
        turbo::utf8_length_from_utf16le_with_replacement(input, length));
}
unicode_result
unicode_utf8_length_from_utf16be_with_replacement(const char16_t* input,
    size_t length) {
    return to_c_result(
        turbo::utf8_length_from_utf16be_with_replacement(input, length));
}

/* Conversions: latin1 <-> utf8, utf8 <-> utf16/utf32, utf16 <-> utf8, etc. */
size_t unicode_convert_latin1_to_utf8(const char* input, size_t length,
    char* output) {
    return turbo::convert_latin1_to_utf8(input, length, output);
}

size_t unicode_convert_latin1_to_utf8_safe(const char* input, size_t length,
    char* output, size_t utf8_len) {
    return turbo::convert_latin1_to_utf8_safe(input, length, output, utf8_len);
}
size_t unicode_convert_latin1_to_utf16le(const char* input, size_t length,
    char16_t* output) {
    return turbo::convert_latin1_to_utf16le(input, length, output);
}
size_t unicode_convert_latin1_to_utf16be(const char* input, size_t length,
    char16_t* output) {
    return turbo::convert_latin1_to_utf16be(input, length, output);
}
size_t unicode_convert_latin1_to_utf16(const char* input, size_t length,
    char16_t* output) {
    return turbo::convert_latin1_to_utf16(input, length, output);
}
size_t unicode_convert_latin1_to_utf32(const char* input, size_t length,
    char32_t* output) {
    return turbo::convert_latin1_to_utf32(input, length, output);
}

size_t unicode_convert_utf8_to_latin1(const char* input, size_t length,
    char* output) {
    return turbo::convert_utf8_to_latin1(input, length, output);
}
size_t unicode_convert_utf8_to_utf16le(const char* input, size_t length,
    char16_t* output) {
    return turbo::convert_utf8_to_utf16le(input, length, output);
}
size_t unicode_convert_utf8_to_utf16(const char* input, size_t length,
    char16_t* output) {
    return turbo::convert_utf8_to_utf16(input, length, output);
}
size_t unicode_convert_utf8_to_utf16be(const char* input, size_t length,
    char16_t* output) {
    return turbo::convert_utf8_to_utf16be(input, length, output);
}
size_t unicode_convert_utf8_to_utf32(const char* input, size_t length,
    char32_t* output) {
    return turbo::convert_utf8_to_utf32(input, length, output);
}
unicode_result unicode_convert_utf8_to_latin1_with_errors(const char* input,
    size_t length,
    char* output) {
    return to_c_result(
        turbo::convert_utf8_to_latin1_with_errors(input, length, output));
}
unicode_result unicode_convert_utf8_to_utf16_with_errors(const char* input,
    size_t length,
    char16_t* output) {
    return to_c_result(
        turbo::convert_utf8_to_utf16_with_errors(input, length, output));
}
unicode_result unicode_convert_utf8_to_utf16le_with_errors(const char* input,
    size_t length,
    char16_t* output) {
    return to_c_result(
        turbo::convert_utf8_to_utf16le_with_errors(input, length, output));
}
unicode_result unicode_convert_utf8_to_utf16be_with_errors(const char* input,
    size_t length,
    char16_t* output) {
    return to_c_result(
        turbo::convert_utf8_to_utf16be_with_errors(input, length, output));
}
unicode_result unicode_convert_utf8_to_utf32_with_errors(const char* input,
    size_t length,
    char32_t* output) {
    return to_c_result(
        turbo::convert_utf8_to_utf32_with_errors(input, length, output));
}

/* Conversions assuming valid input */
size_t unicode_convert_valid_utf8_to_latin1(const char* input, size_t length,
    char* output) {
    return turbo::convert_valid_utf8_to_latin1(input, length, output);
}
size_t unicode_convert_valid_utf8_to_utf16le(const char* input, size_t length,
    char16_t* output) {
    return turbo::convert_valid_utf8_to_utf16le(input, length, output);
}
size_t unicode_convert_valid_utf8_to_utf16be(const char* input, size_t length,
    char16_t* output) {
    return turbo::convert_valid_utf8_to_utf16be(input, length, output);
}
size_t unicode_convert_valid_utf8_to_utf32(const char* input, size_t length,
    char32_t* output) {
    return turbo::convert_valid_utf8_to_utf32(input, length, output);
}

/* UTF-16 -> UTF-8 and related conversions */
size_t unicode_convert_utf16_to_utf8(const char16_t* input, size_t length,
    char* output) {
    return turbo::convert_utf16_to_utf8(input, length, output);
}
size_t unicode_convert_utf16_to_utf8_safe(const char16_t* input, size_t length,
    char* output, size_t utf8_len) {
    return turbo::convert_utf16_to_utf8_safe(input, length, output, utf8_len);
}
size_t unicode_convert_utf16_to_latin1(const char16_t* input, size_t length,
    char* output) {
    return turbo::convert_utf16_to_latin1(input, length, output);
}
size_t unicode_convert_utf16le_to_latin1(const char16_t* input, size_t length,
    char* output) {
    return turbo::convert_utf16le_to_latin1(input, length, output);
}
size_t unicode_convert_utf16be_to_latin1(const char16_t* input, size_t length,
    char* output) {
    return turbo::convert_utf16be_to_latin1(input, length, output);
}
unicode_result
unicode_convert_utf16_to_latin1_with_errors(const char16_t* input,
    size_t length, char* output) {
    return to_c_result(
        turbo::convert_utf16_to_latin1_with_errors(input, length, output));
}
unicode_result
unicode_convert_utf16le_to_latin1_with_errors(const char16_t* input,
    size_t length, char* output) {
    return to_c_result(
        turbo::convert_utf16le_to_latin1_with_errors(input, length, output));
}
unicode_result
unicode_convert_utf16be_to_latin1_with_errors(const char16_t* input,
    size_t length, char* output) {
    return to_c_result(
        turbo::convert_utf16be_to_latin1_with_errors(input, length, output));
}

unicode_result unicode_convert_utf16_to_utf8_with_errors(const char16_t* input,
    size_t length,
    char* output) {
    return to_c_result(
        turbo::convert_utf16_to_utf8_with_errors(input, length, output));
}
unicode_result
unicode_convert_utf16le_to_utf8_with_errors(const char16_t* input,
    size_t length, char* output) {
    return to_c_result(
        turbo::convert_utf16le_to_utf8_with_errors(input, length, output));
}
unicode_result
unicode_convert_utf16be_to_utf8_with_errors(const char16_t* input,
    size_t length, char* output) {
    return to_c_result(
        turbo::convert_utf16be_to_utf8_with_errors(input, length, output));
}

size_t unicode_convert_utf16le_to_utf8(const char16_t* input, size_t length,
    char* output) {
    return turbo::convert_utf16le_to_utf8(input, length, output);
}
size_t unicode_convert_utf16be_to_utf8(const char16_t* input, size_t length,
    char* output) {
    return turbo::convert_utf16be_to_utf8(input, length, output);
}

size_t unicode_convert_utf16_to_utf8_with_replacement(const char16_t* input,
    size_t length,
    char* output) {
    return turbo::convert_utf16_to_utf8_with_replacement(input, length, output);
}
size_t unicode_convert_utf16le_to_utf8_with_replacement(const char16_t* input,
    size_t length,
    char* output) {
    return turbo::convert_utf16le_to_utf8_with_replacement(input, length,
        output);
}
size_t unicode_convert_utf16be_to_utf8_with_replacement(const char16_t* input,
    size_t length,
    char* output) {
    return turbo::convert_utf16be_to_utf8_with_replacement(input, length,
        output);
}

size_t unicode_convert_valid_utf16_to_utf8(const char16_t* input, size_t length,
    char* output) {
    return turbo::convert_valid_utf16_to_utf8(input, length, output);
}
size_t unicode_convert_valid_utf16_to_latin1(const char16_t* input,
    size_t length, char* output) {
    return turbo::convert_valid_utf16_to_latin1(input, length, output);
}
size_t unicode_convert_valid_utf16le_to_latin1(const char16_t* input,
    size_t length, char* output) {
    return turbo::convert_valid_utf16le_to_latin1(input, length, output);
}
size_t unicode_convert_valid_utf16be_to_latin1(const char16_t* input,
    size_t length, char* output) {
    return turbo::convert_valid_utf16be_to_latin1(input, length, output);
}

size_t unicode_convert_valid_utf16le_to_utf8(const char16_t* input,
    size_t length, char* output) {
    return turbo::convert_valid_utf16le_to_utf8(input, length, output);
}
size_t unicode_convert_valid_utf16be_to_utf8(const char16_t* input,
    size_t length, char* output) {
    return turbo::convert_valid_utf16be_to_utf8(input, length, output);
}

/* UTF-16 <-> UTF-32 conversions */
size_t unicode_convert_utf16_to_utf32(const char16_t* input, size_t length,
    char32_t* output) {
    return turbo::convert_utf16_to_utf32(input, length, output);
}
size_t unicode_convert_utf16le_to_utf32(const char16_t* input, size_t length,
    char32_t* output) {
    return turbo::convert_utf16le_to_utf32(input, length, output);
}
size_t unicode_convert_utf16be_to_utf32(const char16_t* input, size_t length,
    char32_t* output) {
    return turbo::convert_utf16be_to_utf32(input, length, output);
}
unicode_result unicode_convert_utf16_to_utf32_with_errors(const char16_t* input,
    size_t length,
    char32_t* output) {
    return to_c_result(
        turbo::convert_utf16_to_utf32_with_errors(input, length, output));
}
unicode_result
unicode_convert_utf16le_to_utf32_with_errors(const char16_t* input,
    size_t length, char32_t* output) {
    return to_c_result(
        turbo::convert_utf16le_to_utf32_with_errors(input, length, output));
}
unicode_result
unicode_convert_utf16be_to_utf32_with_errors(const char16_t* input,
    size_t length, char32_t* output) {
    return to_c_result(
        turbo::convert_utf16be_to_utf32_with_errors(input, length, output));
}

/* Valid UTF-16 conversions */
size_t unicode_convert_valid_utf16_to_utf32(const char16_t* input,
    size_t length, char32_t* output) {
    return turbo::convert_valid_utf16_to_utf32(input, length, output);
}
size_t unicode_convert_valid_utf16le_to_utf32(const char16_t* input,
    size_t length, char32_t* output) {
    return turbo::convert_valid_utf16le_to_utf32(input, length, output);
}
size_t unicode_convert_valid_utf16be_to_utf32(const char16_t* input,
    size_t length, char32_t* output) {
    return turbo::convert_valid_utf16be_to_utf32(input, length, output);
}

/* UTF-32 -> ... conversions */
size_t unicode_convert_utf32_to_utf8(const char32_t* input, size_t length,
    char* output) {
    return turbo::convert_utf32_to_utf8(input, length, output);
}
unicode_result unicode_convert_utf32_to_utf8_with_errors(const char32_t* input,
    size_t length,
    char* output) {
    return to_c_result(
        turbo::convert_utf32_to_utf8_with_errors(input, length, output));
}
size_t unicode_convert_valid_utf32_to_utf8(const char32_t* input, size_t length,
    char* output) {
    return turbo::convert_valid_utf32_to_utf8(input, length, output);
}

size_t unicode_convert_utf32_to_utf16(const char32_t* input, size_t length,
    char16_t* output) {
    return turbo::convert_utf32_to_utf16(input, length, output);
}
size_t unicode_convert_utf32_to_utf16le(const char32_t* input, size_t length,
    char16_t* output) {
    return turbo::convert_utf32_to_utf16le(input, length, output);
}
size_t unicode_convert_utf32_to_utf16be(const char32_t* input, size_t length,
    char16_t* output) {
    return turbo::convert_utf32_to_utf16be(input, length, output);
}
unicode_result
unicode_convert_utf32_to_latin1_with_errors(const char32_t* input,
    size_t length, char* output) {
    return to_c_result(
        turbo::convert_utf32_to_latin1_with_errors(input, length, output));
}

/* --- find helpers --- */
const char* unicode_find(const char* start, const char* end, char character) {
    return turbo::find_token(start, end, character);
}
const char16_t* unicode_find_utf16(const char16_t* start, const char16_t* end,
    char16_t character) {
    return turbo::find_token(start, end, character);
}

/* --- base64 helpers --- */
size_t unicode_maximal_binary_length_from_base64(const char* input,
    size_t length) {
    return turbo::maximal_binary_length_from_base64(input, length);
}
size_t unicode_maximal_binary_length_from_base64_utf16(const char16_t* input,
    size_t length) {
    return turbo::maximal_binary_length_from_base64(input, length);
}

unicode_result unicode_base64_to_binary(
    const char* input, size_t length, char* output,
    unicode_base64_options options,
    unicode_last_chunk_handling_options last_chunk_options) {
    return to_c_result(turbo::base64_to_binary(
        input, length, output, static_cast<turbo::Base64Options>(options),
        static_cast<turbo::last_chunk_handling_options>(last_chunk_options)));
}
unicode_result unicode_base64_to_binary_utf16(
    const char16_t* input, size_t length, char* output,
    unicode_base64_options options,
    unicode_last_chunk_handling_options last_chunk_options) {
    return to_c_result(turbo::base64_to_binary(
        input, length, output, static_cast<turbo::Base64Options>(options),
        static_cast<turbo::last_chunk_handling_options>(last_chunk_options)));
}

size_t unicode_base64_length_from_binary(size_t length,
    unicode_base64_options options) {
    return turbo::base64_length_from_binary(
        length, static_cast<turbo::Base64Options>(options));
}
size_t unicode_base64_length_from_binary_with_lines(
    size_t length, unicode_base64_options options, size_t line_length) {
    return turbo::base64_length_from_binary_with_lines(
        length, static_cast<turbo::Base64Options>(options), line_length);
}

size_t unicode_binary_to_base64(const char* input, size_t length, char* output,
    unicode_base64_options options) {
    return turbo::binary_to_base64(
        input, length, output, static_cast<turbo::Base64Options>(options));
}
size_t unicode_binary_to_base64_with_lines(const char* input, size_t length,
    char* output, size_t line_length,
    unicode_base64_options options) {
    return turbo::binary_to_base64_with_lines(
        input, length, output, line_length,
        static_cast<turbo::Base64Options>(options));
}

unicode_result unicode_base64_to_binary_safe(
    const char* input, size_t length, char* output, size_t* outlen,
    unicode_base64_options options,
    unicode_last_chunk_handling_options last_chunk_options,
    bool decode_up_to_bad_char) {
    size_t local_out = outlen ? *outlen : 0;
    turbo::UnicodeResult r = turbo::base64_to_binary_safe(
        input, length, output, local_out,
        static_cast<turbo::Base64Options>(options),
        static_cast<turbo::last_chunk_handling_options>(last_chunk_options),
        decode_up_to_bad_char);
    if (outlen)
        *outlen = local_out;
    return to_c_result(r);
}
unicode_result unicode_base64_to_binary_safe_utf16(
    const char16_t* input, size_t length, char* output, size_t* outlen,
    unicode_base64_options options,
    unicode_last_chunk_handling_options last_chunk_options,
    bool decode_up_to_bad_char) {
    size_t local_out = outlen ? *outlen : 0;
    turbo::UnicodeResult r = turbo::base64_to_binary_safe(
        input, length, output, local_out,
        static_cast<turbo::Base64Options>(options),
        static_cast<turbo::last_chunk_handling_options>(last_chunk_options),
        decode_up_to_bad_char);
    if (outlen)
        *outlen = local_out;
    return to_c_result(r);
}

static unicode_full_result to_c_full_result(const turbo::full_result& r) {
    unicode_full_result out;
    out.error = static_cast<unicode_error_code>(r.error);
    out.input_count = r.input_count;
    out.output_count = r.output_count;
    return out;
}

unicode_full_result unicode_base64_to_binary_details(
    const char* input, size_t length, char* output,
    unicode_base64_options options,
    unicode_last_chunk_handling_options last_chunk_options) {
    return to_c_full_result(turbo::base64_to_binary_details(
        input, length, output, static_cast<turbo::Base64Options>(options),
        static_cast<turbo::last_chunk_handling_options>(last_chunk_options)));
}
unicode_full_result unicode_base64_to_binary_details_utf16(
    const char16_t* input, size_t length, char* output,
    unicode_base64_options options,
    unicode_last_chunk_handling_options last_chunk_options) {
    return to_c_full_result(turbo::base64_to_binary_details(
        input, length, output, static_cast<turbo::Base64Options>(options),
        static_cast<turbo::last_chunk_handling_options>(last_chunk_options)));
}

bool unicode_base64_valid(char input, unicode_base64_options options) {
    return turbo::base64_valid(input,
        static_cast<turbo::Base64Options>(options));
}
bool unicode_base64_valid_utf16(char16_t input,
    unicode_base64_options options) {
    return turbo::base64_valid(input,
        static_cast<turbo::Base64Options>(options));
}

} // extern "C"
// clang-format off
// clang-format on
