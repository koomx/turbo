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

#include <cstdint>
#include <turbo/bits/bits.h>
#include <turbo/hash/xx/common.h>
#include <turbo/hash/xx/xxhash_scalar.h>

namespace turbo {

    ////////////////////////////////////////////////////////////////////////
    /// 32 bits

    KUMO_FORCE_INLINE void xxhash32_initAccs(uint32_t* acc, uint32_t seed) {
        KUMO_DASSERT(acc != nullptr);
        acc[0] = seed + xxhash::kXxhPrime32_1 + xxhash::kXxhPrime32_2;
        acc[1] = seed + xxhash::kXxhPrime32_2;
        acc[2] = seed + 0;
        acc[3] = seed - xxhash::kXxhPrime32_1;
    }

    static uint32_t xxhash32_avalanche(uint32_t hash) {
        hash ^= hash >> 15;
        hash *= xxhash::kXxhPrime32_2;
        hash ^= hash >> 13;
        hash *= xxhash::kXxhPrime32_3;
        hash ^= hash >> 16;
        return hash;
    }

    static uint32_t xxhash32_round(uint32_t acc, uint32_t input) {
        acc += input * xxhash::kXxhPrime32_2;
        acc = rotl(acc, 13);
        acc *= xxhash::kXxhPrime32_1;
#if (defined(__SSE4_1__) || defined(__aarch64__) || defined(__wasm_simd128__))
        ///
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
        ///
        KUMO_CCO_BARRIER(acc);
#endif
        return acc;
    }

    KUMO_FORCE_INLINE const uint8_t*
    xxhash32_consumeLong(
        uint32_t* KUMO_RESTRICT acc,
        uint8_t const* KUMO_RESTRICT input,
        size_t len) {
        const uint8_t* const bEnd = input + len;
        const uint8_t* const limit = bEnd - 15;
        KUMO_DASSERT(acc != nullptr);
        KUMO_DASSERT(input != nullptr);
        KUMO_DASSERT(len >= 16);
        do {
            acc[0] = xxhash32_round(acc[0], turbo::little_endian::Load32(input));
            input += 4;
            acc[1] = xxhash32_round(acc[1], turbo::little_endian::Load32(input));
            input += 4;
            acc[2] = xxhash32_round(acc[2], turbo::little_endian::Load32(input));
            input += 4;
            acc[3] = xxhash32_round(acc[3], turbo::little_endian::Load32(input));
            input += 4;
        } while (input < limit);

        return input;
    }

    KUMO_FORCE_INLINE uint32_t xxhash32_mergeAccs(const uint32_t* acc) {
        KUMO_DASSERT(acc != nullptr);
        return rotl(acc[0], 1) + rotl(acc[1], 7)
            + rotl(acc[2], 12) + rotl(acc[3], 18);
    }

    static uint32_t xxhash32_finalize(uint32_t hash, const uint8_t* ptr, size_t len) {
#define XXHASH_PROCESS1                                   \
    do {                                               \
        hash += (*ptr++) * xxhash::kXxhPrime32_5;      \
        hash = rotl(hash, 11) * xxhash::kXxhPrime32_1; \
    } while (0)

#define XXHASH_PROCESS4                                                       \
    do {                                                                   \
        hash += turbo::little_endian::Load32(ptr) * xxhash::kXxhPrime32_3; \
        ptr += 4;                                                          \
        hash = rotl(hash, 17) * xxhash::kXxhPrime32_4;                     \
    } while (0)

        if (ptr == nullptr)
            KUMO_DASSERT(len == 0);

        /// Compact rerolled version; generally faster
        len &= 15;
        while (len >= 4) {
            XXHASH_PROCESS4;
            len -= 4;
        }
        while (len > 0) {
            XXHASH_PROCESS1;
            --len;
        }
        return xxhash32_avalanche(hash);
    }

    KUMO_FORCE_INLINE uint32_t xxhash32_endian_align(const uint8_t* input, size_t len, uint32_t seed) {
        uint32_t h32;
        KUMO_DASSERT(input == nullptr ? len == 0 : true);
        if (len >= 16) {
            uint32_t acc[4];
            xxhash32_initAccs(acc, seed);

            input = xxhash32_consumeLong(acc, input, len);

            h32 = xxhash32_mergeAccs(acc);
        } else {
            h32 = seed + xxhash::kXxhPrime32_5;
        }

        h32 += (uint32_t)len;

        return xxhash32_finalize(h32, input, len & 15);
    }

    uint32_t xxhash32_scalar(const uint8_t* input, size_t len, uint32_t seed) {
        return xxhash32_endian_align(input, len, seed);
    }

    /// @ingroup xxhash32_family
    void ScalarHash32::reset(uint32_t seed) {
        memset(this, 0, sizeof(*this));
        xxhash32_initAccs(acc, seed);
    }

    void ScalarHash32::update(const uint8_t* input, size_t len) {
        if (input == nullptr) {
            KUMO_DASSERT(len == 0);
            return;
        }

        total_len_32 += (uint32_t)len;
        large_len |= (uint32_t)((len >= 16) | (total_len_32 >= 16));

        KUMO_DASSERT(bufferedSize < sizeof(buffer));
        if (len < sizeof(buffer) - bufferedSize) {
            /// fill in tmp buffer
            memcpy(buffer + bufferedSize, input, len);
            bufferedSize += (uint32_t)len;
            return;
        }

        {
            const uint8_t* xinput = (const uint8_t*)input;
            const uint8_t* const bEnd = xinput + len;

            if (bufferedSize) {
                /// non-empty buffer: complete first
                memcpy(buffer + bufferedSize, xinput, sizeof(buffer) - bufferedSize);
                xinput += sizeof(buffer) - bufferedSize;
                /// then process one round
                (void)xxhash32_consumeLong(acc, buffer, sizeof(buffer));
                bufferedSize = 0;
            }

            KUMO_DASSERT(xinput <= bEnd);
            if ((size_t)(bEnd - xinput) >= sizeof(buffer)) {
                /// Process the remaining data
                xinput = xxhash32_consumeLong(acc, xinput, (size_t)(bEnd - xinput));
            }

            if (xinput < bEnd) {
                /// Copy the leftover to the tmp buffer
                memcpy(buffer, xinput, (size_t)(bEnd - xinput));
                bufferedSize = (unsigned)(bEnd - xinput);
            }
        }
    }

    uint32_t ScalarHash32::digest() {
        uint32_t h32;

        if (large_len) {
            h32 = xxhash32_mergeAccs(acc);
        } else {
            h32 = acc[2]  + xxhash::kXxhPrime32_5;
        }

        h32 += total_len_32;

        return xxhash32_finalize(h32, buffer, bufferedSize);
    }
    ////////////////////////////////////////////////////////////////////////
    /// 64 bits

    static uint64_t xxhash64_round(uint64_t acc, uint64_t input) {
        acc += input * xxhash::kXxhPrime64_2;
        acc = rotl(acc, 31);
        acc *= xxhash::kXxhPrime64_1;
#if (defined(__AVX512F__))
        KUMO_CCO_BARRIER(acc);
#endif
        return acc;
    }

    static uint64_t xxhash64_mergeRound(uint64_t acc, uint64_t val) {
        val = xxhash64_round(0, val);
        acc ^= val;
        acc = acc * xxhash::kXxhPrime64_1 + xxhash::kXxhPrime64_4;
        return acc;
    }

    KUMO_FORCE_INLINE void
    xxhash64_initAccs(uint64_t* acc, uint64_t seed) {
        KUMO_DASSERT(acc != nullptr);
        acc[0] = seed + xxhash::kXxhPrime64_1 + xxhash::kXxhPrime64_2;
        acc[1] = seed + xxhash::kXxhPrime64_2;
        acc[2] = seed + 0;
        acc[3] = seed - xxhash::kXxhPrime64_1;
    }

    KUMO_FORCE_INLINE const uint8_t*
    xxhash64_consumeLong(
        uint64_t* KUMO_RESTRICT acc,
        uint8_t const* KUMO_RESTRICT input,
        size_t len) {
        const uint8_t* const bEnd = input + len;
        const uint8_t* const limit = bEnd - 31;
        KUMO_DASSERT(acc != nullptr);
        KUMO_DASSERT(input != nullptr);
        KUMO_DASSERT(len >= 32);
        do {
            /// reroll on 32-bit
            if (sizeof(void*) < sizeof(uint64_t)) {
                size_t i;
                for (i = 0; i < 4; i++) {
                    acc[i] = xxhash64_round(acc[i], turbo::little_endian::Load64(input));
                    input += 8;
                }
            } else {
                acc[0] = xxhash64_round(acc[0], turbo::little_endian::Load64(input));
                input += 8;
                acc[1] = xxhash64_round(acc[1], turbo::little_endian::Load64(input));
                input += 8;
                acc[2] = xxhash64_round(acc[2], turbo::little_endian::Load64(input));
                input += 8;
                acc[3] = xxhash64_round(acc[3], turbo::little_endian::Load64(input));
                input += 8;
            }
        } while (input < limit);

        return input;
    }

    KUMO_FORCE_INLINE uint64_t
    xxhash64_mergeAccs(const uint64_t* acc) {
        KUMO_DASSERT(acc != nullptr);
        {
            uint64_t h64 = rotl(acc[0], 1) + rotl(acc[1], 7)
                + rotl(acc[2], 12) + rotl(acc[3], 18);
            /// reroll on 32-bit
            if (sizeof(void*) < sizeof(uint64_t)) {
                size_t i;
                for (i = 0; i < 4; i++) {
                    h64 = xxhash64_mergeRound(h64, acc[i]);
                }
            } else {
                h64 = xxhash64_mergeRound(h64, acc[0]);
                h64 = xxhash64_mergeRound(h64, acc[1]);
                h64 = xxhash64_mergeRound(h64, acc[2]);
                h64 = xxhash64_mergeRound(h64, acc[3]);
            }
            return h64;
        }
    }

    static uint64_t
    xxhash64_finalize(uint64_t hash, const uint8_t* ptr, size_t len) {
        if (ptr == nullptr)
            KUMO_DASSERT(len == 0);
        len &= 31;
        while (len >= 8) {
            uint64_t const k1 = xxhash64_round(0, turbo::little_endian::Load64(ptr));
            ptr += 8;
            hash ^= k1;
            hash = rotl(hash, 27) * xxhash::kXxhPrime64_1 + xxhash::kXxhPrime64_4;
            len -= 8;
        }
        if (len >= 4) {
            hash ^= (uint64_t)(turbo::little_endian::Load32(ptr)) * xxhash::kXxhPrime64_1;
            ptr += 4;
            hash = rotl(hash, 23) * xxhash::kXxhPrime64_2 + xxhash::kXxhPrime64_3;
            len -= 4;
        }
        while (len > 0) {
            hash ^= (*ptr++) * xxhash::kXxhPrime64_5;
            hash = rotl(hash, 11) * xxhash::kXxhPrime64_1;
            --len;
        }
        return xxhash::xxhash_scalar_avalanche64(hash);
    }

#undef XXHASH_PROCESS1
#undef XXHASH_PROCESS4

    static  uint64_t xxhash_endian_align(const uint8_t* input, size_t len, uint64_t seed) {
        uint64_t h64;
        if (input == nullptr)
            KUMO_DASSERT(len == 0);

        if (len >= 32) {
            /// Process a large block of data
            uint64_t acc[4];
            xxhash64_initAccs(acc, seed);

            input = xxhash64_consumeLong(acc, input, len);

            h64 = xxhash64_mergeAccs(acc);
        } else {
            h64 = seed + xxhash::kXxhPrime64_5;
        }

        h64 += (uint64_t)len;

        return xxhash64_finalize(h64, input, len);
    }

    KUMO_DLL uint64_t xxhash64_scalar(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t len, uint64_t seed) {
        return xxhash_endian_align(input, len, seed);
    }

    /// @ingroup xxhash64_family
    void ScalarHash64::reset(uint64_t seed) {
        memset(this, 0, sizeof(*this));
        xxhash64_initAccs(this->acc, seed);
    }

    void ScalarHash64::update(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t len) {
        if (input == nullptr) {
            KUMO_DASSERT(len == 0);
            return;
        }

        total_len += len;

        KUMO_DASSERT(bufferedSize <= sizeof(buffer));
        if (len < sizeof(buffer) - bufferedSize) {
            /// fill in tmp buffer
            memcpy(buffer + bufferedSize, input, len);
            bufferedSize += (uint64_t)len;
            return;
        }

        {
            const uint8_t* xinput = input;
            const uint8_t* const bEnd = xinput + len;

            if (bufferedSize) {
                /// non-empty buffer => complete first
                memcpy(buffer + bufferedSize, xinput, sizeof(buffer) - bufferedSize);
                xinput += sizeof(buffer) - bufferedSize;
                /// and process one round
                (void)xxhash64_consumeLong(acc, buffer, sizeof(buffer));
                bufferedSize = 0;
            }

            KUMO_DASSERT(xinput <= bEnd);
            if ((size_t)(bEnd - xinput) >= sizeof(buffer)) {
                /// Process the remaining data
                xinput = xxhash64_consumeLong(acc, xinput, (size_t)(bEnd - xinput));
            }

            if (xinput < bEnd) {
                /// Copy the leftover to the tmp buffer
                memcpy(buffer, xinput, (size_t)(bEnd - xinput));
                bufferedSize = (unsigned)(bEnd - xinput);
            }
        }
    }

    /// @ingroup xxhash64_family
    uint64_t ScalarHash64::digest() {
        uint64_t h64;

        if (total_len >= 32) {
            h64 = xxhash64_mergeAccs(acc);
        } else {
            h64 = acc[2] + xxhash::kXxhPrime64_5;
        }

        h64 += (uint64_t)total_len;

        return xxhash64_finalize(h64, buffer, (size_t)total_len);
    }
} // namespace turbo
