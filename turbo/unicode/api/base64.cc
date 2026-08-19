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
#include <turbo/unicode/engine/backend_select.h>
#include <turbo/unicode/api/base64_implementation.h>

namespace turbo {

#if SIMDUTF_ATOMIC_REF
    template <typename char_type>
    simdutf_warn_unused result atomic_base64_to_binary_safe_impl(
        const char_type* input, size_t length, char* output, size_t& outlen,
        base64_options options,
        last_chunk_handling_options last_chunk_handling_options,
        bool decode_up_to_bad_char) noexcept {
#if defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
        // We use a smaller buffer during fuzzing to more easily detect bugs.
        constexpr size_t buffer_size = 128;
#else
        // Arbitrary block sizes: 4KB for input.
        constexpr size_t buffer_size = 4096;
#endif
        std::array<char, buffer_size> temp_buffer;
        const char_type* const input_init = input;
        size_t actual_out = 0;
        bool last_chunk = false;
        const size_t length_init = length;
        result r;
        while (!last_chunk) {
            last_chunk |= (temp_buffer.size() >= outlen - actual_out);
            size_t temp_outlen = (detail::min)(temp_buffer.size(), outlen - actual_out);
            r = base64_to_binary_safe(input, length, temp_buffer.data(), temp_outlen,
                options, last_chunk_handling_options,
                decode_up_to_bad_char);
            // We processed r.count characters of input.
            // We wrote temp_outlen bytes to temp_buffer.
            // If there is no ignorable characters,
            // we should expect that values/4.0*3 == temp_outlen,
            // except maybe at the tail end of the string.

            //
            // We are assuming that when r.error == error_code::OUTPUT_BUFFER_TOO_SMALL,
            // we truncate the results so that a number of base64 characters divisible
            // by four is processed.
            //

            //
            // We wrote temp_outlen bytes to temp_buffer.
            // We need to copy them to output.
            // Copy with relaxed atomic operations to the output
            simdutf_log_assert(temp_outlen <= outlen - actual_out,
                "Output buffer is too small");
            simdutf_log_assert(temp_outlen <= temp_buffer.size(),
                "Output buffer is too small");

            turbo::scalar::memcpy_atomic_write(output + actual_out,
                temp_buffer.data(), temp_outlen);
            actual_out += temp_outlen;
            length -= r.count;
            input += r.count;

            if (r.error != error_code::OUTPUT_BUFFER_TOO_SMALL) {
                break;
            }
        }
        if (size_t(input - input_init) != length_init) {
            // We did not process all input characters. In such case, we
            // should not end with an ignorable character. See
            // https://tc39.es/proposal-arraybuffer-base64/spec/#sec-frombase64
            while (input > input_init && base64_ignorable(*(input - 1), options)) {
                --input;
            }
        }
        outlen = actual_out;
        return { r.error, size_t(input - input_init) };
    }

    simdutf_warn_unused result atomic_base64_to_binary_safe(
        const char* input, size_t length, char* output, size_t& outlen,
        base64_options options,
        last_chunk_handling_options last_chunk_handling_options,
        bool decode_up_to_bad_char) noexcept {
        return atomic_base64_to_binary_safe_impl<char>(
            input, length, output, outlen, options, last_chunk_handling_options,
            decode_up_to_bad_char);
    }
    simdutf_warn_unused result atomic_base64_to_binary_safe(
        const char16_t* input, size_t length, char* output, size_t& outlen,
        base64_options options,
        last_chunk_handling_options last_chunk_handling_options,
        bool decode_up_to_bad_char) noexcept {
        return atomic_base64_to_binary_safe_impl<char16_t>(
            input, length, output, outlen, options, last_chunk_handling_options,
            decode_up_to_bad_char);
    }
#endif // SIMDUTF_ATOMIC_REF

    simdutf_warn_unused size_t
    maximal_binary_length_from_base64(const char* input, size_t length) noexcept {
        return get_default_implementation()->maximal_binary_length_from_base64(
            input, length);
    }

    simdutf_warn_unused result base64_to_binary(
        const char* input, size_t length, char* output, base64_options options,
        last_chunk_handling_options last_chunk_handling_options) noexcept {
        return get_default_implementation()->base64_to_binary(
            input, length, output, options, last_chunk_handling_options);
    }

    simdutf_warn_unused size_t maximal_binary_length_from_base64(
        const char16_t* input, size_t length) noexcept {
        return get_default_implementation()->maximal_binary_length_from_base64(
            input, length);
    }

    simdutf_warn_unused size_t binary_length_from_base64(const char* input,
        size_t length) noexcept {
        return get_default_implementation()->binary_length_from_base64(input, length);
    }

    simdutf_warn_unused size_t binary_length_from_base64(const char16_t* input,
        size_t length) noexcept {
        return get_default_implementation()->binary_length_from_base64(input, length);
    }

    simdutf_warn_unused result base64_to_binary(
        const char16_t* input, size_t length, char* output, base64_options options,
        last_chunk_handling_options last_chunk_handling_options) noexcept {
        return get_default_implementation()->base64_to_binary(
            input, length, output, options, last_chunk_handling_options);
    }

    simdutf_warn_unused full_result base64_to_binary_details(
        const char* input, size_t length, char* output, base64_options options,
        last_chunk_handling_options last_chunk_handling_options) noexcept {
        return get_default_implementation()->base64_to_binary_details(
            input, length, output, options, last_chunk_handling_options);
    }

    simdutf_warn_unused full_result base64_to_binary_details(
        const char16_t* input, size_t length, char* output, base64_options options,
        last_chunk_handling_options last_chunk_handling_options) noexcept {
        return get_default_implementation()->base64_to_binary_details(
            input, length, output, options, last_chunk_handling_options);
    }

    // moved to implementation.h
    // simdutf_warn_unused bool base64_ignorable(char input,
    //                                           base64_options options) noexcept
    // simdutf_warn_unused bool base64_ignorable(char16_t input,
    //                                           base64_options options) noexcept
    // simdutf_warn_unused bool base64_valid(char input,
    //                                       base64_options options) noexcept
    // simdutf_warn_unused bool base64_valid(char16_t input,
    //                                       base64_options options) noexcept
    // simdutf_warn_unused bool
    // base64_valid_or_padding(char input, base64_options options) noexcept
    // simdutf_warn_unused bool
    // base64_valid_or_padding(char16_t input, base64_options options) noexcept

    // base64_to_binary_safe_impl is moved to
    // include/simdutf/base64_implementation.h

#if SIMDUTF_ATOMIC_REF
    size_t atomic_binary_to_base64(const char* input, size_t length, char* output,
        base64_options options) noexcept {
        size_t retval = 0;
#if defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
        // We use a smaller buffer during fuzzing to more easily detect bugs.
        constexpr size_t input_block_size = 128 * 3;
#else
        // Arbitrary block sizes: 3KB for input which produces 4KB in output.
        constexpr size_t input_block_size = 1024 * 3;
#endif
        std::array<char, input_block_size> inbuf;
        for (size_t i = 0; i < length; i += input_block_size) {
            const size_t current_block_size = detail::min(input_block_size, length - i);
            turbo::scalar::memcpy_atomic_read(inbuf.data(), input + i,
                current_block_size);
            const size_t written = binary_to_base64(inbuf.data(), current_block_size,
                output + retval, options);
            retval += written;
        }
        return retval;
    }
#endif // SIMDUTF_ATOMIC_REF


    simdutf_warn_unused result
    base64_to_binary_safe(const char* input, size_t length, char* output,
        size_t& outlen, base64_options options,
        last_chunk_handling_options last_chunk_handling_options,
        bool decode_up_to_bad_char) noexcept {
        return base64_to_binary_safe_impl<char>(input, length, output, outlen,
            options, last_chunk_handling_options,
            decode_up_to_bad_char);
    }
    simdutf_warn_unused result
    base64_to_binary_safe(const char16_t* input, size_t length, char* output,
        size_t& outlen, base64_options options,
        last_chunk_handling_options last_chunk_handling_options,
        bool decode_up_to_bad_char) noexcept {
        return base64_to_binary_safe_impl<char16_t>(
            input, length, output, outlen, options, last_chunk_handling_options,
            decode_up_to_bad_char);
    }

    size_t binary_to_base64(const char* input, size_t length, char* output,
        base64_options options) noexcept {
        return get_default_implementation()->binary_to_base64(input, length, output,
            options);
    }

    size_t binary_to_base64_with_lines(const char* input, size_t length,
        char* output, size_t line_length,
        base64_options options) noexcept {
        return get_default_implementation()->binary_to_base64_with_lines(
            input, length, output, line_length, options);
    }

}  // namespace turbo
