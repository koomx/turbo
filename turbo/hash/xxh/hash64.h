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

#include <turbo/hash/xxh/config.h>
#include <turbo/hash/xxh/types.h>
#include <turbo/hash/xxh/scalar.h>
#include <turbo/bits/bits.h>

namespace turbo {

    /// @internal
    /// @brief Structure for XXH64 streaming API.
    ///
    /// @note This is only defined when @ref XXH_STATIC_LINKING_ONLY,
    /// @ref XXH_INLINE_ALL, or @ref XXH_IMPLEMENTATION is defined. Otherwise it is
    /// an opaque type. This allows fields to safely be changed.
    ///
    /// Typedef'd to @ref XXH64_state_t.
    /// Do not access the members of this struct directly.
    /// @see XXH32_state_s, XXH3_state_s
    struct XXH64_state_t {
        /// Total length hashed. This is always 64-bit.
        uint64_t total_len;
        /// Accumulator lanes
        uint64_t acc[4];
        /// Internal buffer for partial reads
        unsigned char buffer[32];
        /// Amount of data in @ref buffer
        uint32_t bufferedSize;
        /// Reserved field, needed for padding anyway
        uint32_t reserved32;
        /// Reserved field. Do not read or write to it.
        uint64_t reserved64;


        explicit XXH64_state_t(uint64_t seed = 0) {
            reset(seed);
        }
        XXH64_state_t(const XXH64_state_t& rhs) {
            memcpy(this, &rhs, sizeof(XXH64_state_t));
        }
        XXH64_state_t& operator=(const XXH64_state_t& rhs) {
            if (&rhs == this) {
                return *this;
            }
            memcpy(this, &rhs, sizeof(XXH64_state_t));
            return *this;
        }

        void clear() {
            memset(this, 0, sizeof(XXH64_state_t));
        }


        void reset(uint64_t seed);

        void update(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t length);

        uint64_t digest();

    public:

    private:
        KUMO_FORCE_INLINE void init_accs(uint64_t seed) {
            acc[0] = seed + XXH_PRIME64_1 + XXH_PRIME64_2;
            acc[1] = seed + XXH_PRIME64_2;
            acc[2] = seed + 0;
            acc[3] = seed - XXH_PRIME64_1;
        }

        KUMO_FORCE_INLINE const uint8_t* consume_long(uint8_t const* KUMO_RESTRICT input,size_t len);

        static uint64_t acc_round(uint64_t acc, uint64_t input) {
            acc += input * XXH_PRIME64_2;
            acc = turbo::rotl(acc, 31);
            acc *= XXH_PRIME64_1;
#if (defined(__AVX512F__)) && !defined(XXH_ENABLE_AUTOVECTORIZE)
            /*
             * DISABLE AUTOVECTORIZATION:
             * A compiler fence is used to prevent GCC and Clang from
             * autovectorizing the XXH64 loop (pragmas and attributes don't work for some
             * reason) without globally disabling AVX512.
             *
             * Autovectorization of XXH64 tends to be detrimental,
             * though the exact outcome may change depending on exact cpu and compiler version.
             * For information, it has been reported as detrimental for Skylake-X,
             * but possibly beneficial for Zen4.
             *
             * The default is to disable auto-vectorization,
             * but you can select to enable it instead using `XXH_ENABLE_AUTOVECTORIZE` build variable.
             */
            KUMO_CCO_BARRIER(acc);
#endif
            return acc;
        }

        uint64_t merge_round(uint64_t acc, uint64_t val) {
            val = acc_round(0, val);
            acc ^= val;
            acc = acc * XXH_PRIME64_1 + XXH_PRIME64_4;
            return acc;
        }

        KUMO_ATTRIBUTE_PURE_FUNCTION KUMO_FORCE_INLINE uint64_t merge_accs() {
            KUMO_DASSERT(acc != nullptr);
            {
                uint64_t h64 = turbo::rotl(acc[0], 1) + turbo::rotl(acc[1], 7)
                    + turbo::rotl(acc[2], 12) + turbo::rotl(acc[3], 18);
                /* reroll on 32-bit */
                if (sizeof(void*) < sizeof(uint64_t)) {
                    size_t i;
                    for (i = 0; i < 4; i++) {
                        h64 = merge_round(h64, acc[i]);
                    }
                } else {
                    h64 = merge_round(h64, acc[0]);
                    h64 = merge_round(h64, acc[1]);
                    h64 = merge_round(h64, acc[2]);
                    h64 = merge_round(h64, acc[3]);
                }
                return h64;
            }
        }

        KUMO_ATTRIBUTE_PURE_FUNCTION static uint64_t finalize(uint64_t hash, const uint8_t* ptr, size_t len);
    };


    /*! @ingroup XXH64_family */
    KUMO_DLL uint64_t xxhash64(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t len, uint64_t seed) {
        XXH64_state_t state(seed);
        state.update(input, len);
        return state.digest();
    }

}  // namespace turbo
