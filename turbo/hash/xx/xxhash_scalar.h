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

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <turbo/bits/bits.h>
#include <turbo/macros/macros.h>

namespace turbo {

    [[nodiscard]] KUMO_DLL uint32_t xxhash32_scalar(const uint8_t* input, size_t len, uint32_t seed = 0);

    [[nodiscard]] KUMO_DLL uint64_t xxhash64_scalar(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t len, uint64_t seed = 0);

    [[nodiscard]] inline bool xxhash32_scalar_canonical(uint32_t hash, uint8_t* out, size_t len) {
        if (len < sizeof(uint32_t)) {
            return false;
        }
        auto bh = turbo::big_endian::from_host(hash);
        memcpy(out, &bh, sizeof(uint32_t));
        return true;
    }

    [[nodiscard]] inline std::array<uint8_t, 4> xxhash32_scalar_canonical(uint32_t hash) {
        auto bh = turbo::big_endian::from_host(hash);
        std::array<uint8_t, 4> dst;
        memcpy(dst.data(), &bh, sizeof(uint32_t));
        return dst;
    }

    [[nodiscard]] inline std::optional<uint32_t> xxhash32_scalar_from__canonical(const uint8_t* src, size_t len) {
        if (len < sizeof(uint32_t)) {
            return std::nullopt;
        }
        uint32_t hh;
        memcpy(&hh, src, sizeof(uint32_t));
        return turbo::big_endian::from_host(hh);
    }

    [[nodiscard]] inline uint32_t xxhash32_scalar_from__canonical(std::array<uint8_t, 4> src) {
        uint32_t hh;
        memcpy(&hh, src.data(), sizeof(uint32_t));
        return turbo::big_endian::from_host(hh);
    }

    struct XXH32_state_t {
        uint32_t total_len_32; /*!< Total length hashed, modulo 2^32 */
        uint32_t large_len; /*!< Whether the hash is >= 16 (handles @ref total_len_32 overflow) */
        uint32_t acc[4]; /*!< Accumulator lanes */
        unsigned char buffer[16]; /*!< Internal buffer for partial reads. */
        uint32_t bufferedSize; /*!< Amount of data in @ref buffer */
        uint32_t reserved; /*!< Reserved field. Do not read nor write to it. */

        void reset(uint32_t seed = 0);
        void update(const uint8_t* input, size_t len);

        uint32_t digest();
    };

    struct XXH64_state_s {
        uint64_t total_len; /*!< Total length hashed. This is always 64-bit. */
        uint64_t acc[4]; /*!< Accumulator lanes */
        unsigned char buffer[32]; /*!< Internal buffer for partial reads.. */
        uint32_t bufferedSize; /*!< Amount of data in @ref buffer */
        uint32_t reserved32; /*!< Reserved field, needed for padding anyways*/
        uint64_t reserved64; /*!< Reserved field. Do not read or write to it. */

        void reset(uint64_t seed = 0);
        void update(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t len);

        uint64_t digest();
    };

} // namespace turbo
