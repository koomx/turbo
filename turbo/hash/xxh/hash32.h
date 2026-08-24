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

#include <turbo/bits/bits.h>
#include <turbo/hash/xxh/config.h>
#include <turbo/hash/xxh/scalar.h>
#include <turbo/hash/xxh/types.h>

namespace turbo {

    /// @internal
    /// @brief Structure for XXH32 streaming API.
    ///
    /// @note This is only defined when @ref XXH_STATIC_LINKING_ONLY,
    /// @ref XXH_INLINE_ALL, or @ref XXH_IMPLEMENTATION is defined. Otherwise it is
    /// an opaque type. This allows fields to safely be changed.
    ///
    /// Typedef'd to @ref XXH32_state_t.
    /// Do not access the members of this struct directly.
    /// @see XXH64_state_s, XXH3_state_s
    struct XXH32_state_t {
        /// Total length hashed, modulo 2^32
        uint32_t total_len_32;
        /// Whether the hash is >= 16 (handles @ref total_len_32 overflow)
        uint32_t large_len;
        /// Accumulator lanes
        uint32_t acc[4];
        /// Internal buffer for partial reads
        unsigned char buffer[16];
        /// Amount of data in @ref buffer
        uint32_t bufferedSize;
        /// Reserved field. Do not read nor write to it.
        uint32_t reserved;

        explicit XXH32_state_t(uint32_t seed = 0) {
            reset(seed);
        }
        XXH32_state_t(const XXH32_state_t& rhs) {
            memcpy(this, &rhs, sizeof(XXH32_state_t));
        }
        XXH32_state_t& operator=(const XXH32_state_t& rhs) {
            if (&rhs == this) {
                return *this;
            }
            memcpy(this, &rhs, sizeof(XXH32_state_t));
            return *this;
        }

        void clear() {
            memset(this, 0, sizeof(XXH32_state_t));
        }

        void reset(uint32_t seed);

        void update(const uint8_t* input, size_t length);

        uint32_t digest();

    private:
        /// @internal
        /// @brief Sets up the initial accumulator state for XXH32().
        KUMO_FORCE_INLINE void init_accs(uint32_t seed) {
            acc[0] = seed + XXH_PRIME32_1 + XXH_PRIME32_2;
            acc[1] = seed + XXH_PRIME32_2;
            acc[2] = seed + 0;
            acc[3] = seed - XXH_PRIME32_1;
        }

        /// @internal
        /// @brief Normal stripe processing routine.
        ///
        /// This shuffles the bits so that any bit from @p input impacts several bits in
        /// @p acc.
        ///
        /// @param acc The accumulator lane.
        /// @param input The stripe of input to mix.
        /// @return The mixed accumulator lane.
        static uint32_t acc_round(uint32_t acc, uint32_t input) {
            acc += input * XXH_PRIME32_2;
            acc = turbo::rotl(acc, 13);
            acc *= XXH_PRIME32_1;
#if (defined(__SSE4_1__) || defined(__aarch64__) || defined(__wasm_simd128__)) && !defined(XXH_ENABLE_AUTOVECTORIZE)
            /// UGLY HACK:
            /// A compiler fence is used to prevent GCC and Clang from
            /// autovectorizing the XXH32 loop (pragmas and attributes don't work for some
            /// reason) without globally disabling SSE4.1.
            ///
            /// The reason we want to avoid vectorization is because despite working on
            /// 4 integers at a time, there are multiple factors slowing XXH32 down on
            /// SSE4:
            /// - There's a ridiculous amount of lag from pmulld (10 cycles of latency on
            ///   newer chips!) making it slightly slower to multiply four integers at
            ///   once compared to four integers independently. Even when pmulld was
            ///   fastest, Sandy/Ivy Bridge, it is still not worth it to go into SSE
            ///   just to multiply unless doing a long operation.
            ///
            /// - Four instructions are required to rotate,
            ///      movqda tmp,  v // not required with VEX encoding
            ///      pslld  tmp, 13 // tmp <<= 13
            ///      psrld  v,   19 // x >>= 19
            ///      por    v,  tmp // x |= tmp
            ///   compared to one for scalar:
            ///      roll   v, 13    // reliably fast across the board
            ///      shldl  v, v, 13 // Sandy Bridge and later prefer this for some reason
            ///
            /// - Instruction level parallelism is actually more beneficial here because
            ///   the SIMD actually serializes this operation: While v1 is rotating, v2
            ///   can load data, while v3 can multiply. SSE forces them to operate
            ///   together.
            ///
            /// This is also enabled on AArch64, as Clang is *very aggressive* in vectorizing
            /// the loop. NEON is only faster on the A53, and with the newer cores, it is less
            /// than half the speed.
            ///
            /// Additionally, this is used on WASM SIMD128 because it JITs to the same
            /// SIMD instructions and has the same issue.
            KUMO_CCO_BARRIER(acc);
#endif
            return acc;
        }

        /// @internal
        /// @brief Consumes a block of data for XXH32().
        ///
        /// @return the end input pointer.
        KUMO_FORCE_INLINE const uint8_t* consume_long(uint8_t const* KUMO_RESTRICT input, size_t len) {
            const uint8_t* const bEnd = input + len;
            const uint8_t* const limit = bEnd - 15;
            KUMO_DASSERT(input != nullptr);
            KUMO_DASSERT(len >= 16);
            do {
                acc[0] = acc_round(acc[0], little_endian::Load32(input));
                input += 4;
                acc[1] = acc_round(acc[1], little_endian::Load32(input));
                input += 4;
                acc[2] = acc_round(acc[2], little_endian::Load32(input));
                input += 4;
                acc[3] = acc_round(acc[3], little_endian::Load32(input));
                input += 4;
            } while (input < limit);

            return input;
        }

        static uint32_t avalanche(uint32_t hash) {
            hash ^= hash >> 15;
            hash *= XXH_PRIME32_2;
            hash ^= hash >> 13;
            hash *= XXH_PRIME32_3;
            hash ^= hash >> 16;
            return hash;
        }

        KUMO_ATTRIBUTE_PURE_FUNCTION uint32_t merge_accs() const;

        KUMO_ATTRIBUTE_PURE_FUNCTION static uint32_t finalize(uint32_t hash, const uint8_t* ptr, size_t len);
    };

    inline uint32_t xxhash32(const uint8_t* input, size_t len, uint32_t seed = 0) {
        XXH32_state_t state(seed);
        state.update(input, len);
        return state.digest();
    }

} // namespace turbo
