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

#include <turbo/unicode/api/base64.h>
#include <turbo/unicode/engine/isa_select.h>
#include <turbo/unicode/api/base64_implementation.h>

namespace turbo {

     [[nodiscard]] size_t
    maximal_binary_length_from_base64(const char* input, size_t length) noexcept {
        return UnicodeRegistry::get_best_isa()->maximal_binary_length_from_base64(
            input, length);
    }

     [[nodiscard]] UnicodeResult base64_to_binary(
        const char* input, size_t length, char* output, Base64Options options,
        last_chunk_handling_options last_chunk_handling_options) noexcept {
        return UnicodeRegistry::get_best_isa()->base64_to_binary(
            input, length, output, options, last_chunk_handling_options);
    }

     [[nodiscard]] size_t maximal_binary_length_from_base64(
        const char16_t* input, size_t length) noexcept {
        return UnicodeRegistry::get_best_isa()->maximal_binary_length_from_base64(
            input, length);
    }

     [[nodiscard]] size_t binary_length_from_base64(const char* input,
        size_t length) noexcept {
        return UnicodeRegistry::get_best_isa()->binary_length_from_base64(input, length);
    }

     [[nodiscard]] size_t binary_length_from_base64(const char16_t* input,
        size_t length) noexcept {
        return UnicodeRegistry::get_best_isa()->binary_length_from_base64(input, length);
    }

     [[nodiscard]] UnicodeResult base64_to_binary(
        const char16_t* input, size_t length, char* output, Base64Options options,
        last_chunk_handling_options last_chunk_handling_options) noexcept {
        return UnicodeRegistry::get_best_isa()->base64_to_binary(
            input, length, output, options, last_chunk_handling_options);
    }

     [[nodiscard]] full_result base64_to_binary_details(
        const char* input, size_t length, char* output, Base64Options options,
        last_chunk_handling_options last_chunk_handling_options) noexcept {
        return UnicodeRegistry::get_best_isa()->base64_to_binary_details(
            input, length, output, options, last_chunk_handling_options);
    }

     [[nodiscard]] full_result base64_to_binary_details(
        const char16_t* input, size_t length, char* output, Base64Options options,
        last_chunk_handling_options last_chunk_handling_options) noexcept {
        return UnicodeRegistry::get_best_isa()->base64_to_binary_details(
            input, length, output, options, last_chunk_handling_options);
    }

     [[nodiscard]] UnicodeResult
    base64_to_binary_safe(const char* input, size_t length, char* output,
        size_t& outlen, Base64Options options,
        last_chunk_handling_options last_chunk_handling_options,
        bool decode_up_to_bad_char) noexcept {
        return base64_to_binary_safe_impl<char>(input, length, output, outlen,
            options, last_chunk_handling_options,
            decode_up_to_bad_char);
    }
     [[nodiscard]] UnicodeResult
    base64_to_binary_safe(const char16_t* input, size_t length, char* output,
        size_t& outlen, Base64Options options,
        last_chunk_handling_options last_chunk_handling_options,
        bool decode_up_to_bad_char) noexcept {
        return base64_to_binary_safe_impl<char16_t>(
            input, length, output, outlen, options, last_chunk_handling_options,
            decode_up_to_bad_char);
    }

    size_t binary_to_base64(const char* input, size_t length, char* output,
        Base64Options options) noexcept {
        return UnicodeRegistry::get_best_isa()->binary_to_base64(input, length, output,
            options);
    }

    size_t binary_to_base64_with_lines(const char* input, size_t length,
        char* output, size_t line_length,
        Base64Options options) noexcept {
        return UnicodeRegistry::get_best_isa()->binary_to_base64_with_lines(
            input, length, output, line_length, options);
    }

}  // namespace turbo
