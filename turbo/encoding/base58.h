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

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <functional>
#include <turbo/macros/macros.h>


namespace turbo {


    /// The generic (variable-length) Base58 encoder and decoder use a big-integer base conversion
    /// whose cost is quadratic in the input length. For large inputs this can run for a very long
    /// time, so they accept an optional `check_cancellation` callback that is invoked periodically;
    /// it is expected to throw if the query has been cancelled or exceeded its time limit.
    size_t base58_encode(const uint8_t* src, size_t src_length, uint8_t* dst, const std::function<void()>& check_cancellation = { });
    std::optional<size_t> base58_decode(const uint8_t* src, size_t src_length, uint8_t* dst, const std::function<void()>& check_cancellation = { });

    /// Maximum base58-encoded lengths for fixed-size inputs.
    /// A 32-byte value uses 9 intermediate digits of radix 58^5, producing at most
    /// 9*5 = 45 raw base58 digits; the leading digit is always zero, so max output is 44.
    /// Similarly, 64 bytes use 18 intermediate digits: 18*5 = 90, minus 2 guaranteed
    /// leading zeros, giving max output 88.
    constexpr auto BASE58_ENCODED_32_LEN = 44UL;
    constexpr auto BASE58_ENCODED_64_LEN = 88UL;

    size_t base58_encode32(const uint8_t* src, uint8_t* dst);
    size_t base58_encode64(const uint8_t* src, uint8_t* dst);
    std::optional<size_t> base58_decode32(const uint8_t* src, size_t src_length, uint8_t* dst);
    std::optional<size_t> base58_decode64(const uint8_t* src, size_t src_length, uint8_t* dst);

    namespace encoding_internal {
        size_t base58_encode32_fd(const uint8_t* src, uint8_t* dst);
        size_t base58_encode64_fd(const uint8_t* src, uint8_t* dst);
        std::optional<size_t> base58_decode32_fd(const uint8_t* src, size_t src_length, uint8_t* dst);
        std::optional<size_t> base58_decode64_fd(const uint8_t* src, size_t src_length, uint8_t* dst);
    }
}  // namespace turbo
