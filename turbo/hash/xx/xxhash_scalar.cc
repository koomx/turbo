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

#include <turbo/bits/bits.h>
#include <turbo/hash/xx/common.h>
#include <turbo/hash/xx/xxhash_scalar.h>
#include <cstdint>

namespace turbo {

    ////////////////////////////////////////////////////////////////////////
    /// 32 bits

    KUMO_FORCE_INLINE void XXH32_initAccs(uint32_t* acc, uint32_t seed) {
        KUMO_DASSERT(acc != NULL);
        acc[0] = seed + XXH_PRIME32_1 + XXH_PRIME32_2;
        acc[1] = seed + XXH_PRIME32_2;
        acc[2] = seed + 0;
        acc[3] = seed - XXH_PRIME32_1;
    }

    static uint32_t XXH32_avalanche(uint32_t hash) {
        hash ^= hash >> 15;
        hash *= XXH_PRIME32_2;
        hash ^= hash >> 13;
        hash *= XXH_PRIME32_3;
        hash ^= hash >> 16;
        return hash;
    }

    static uint32_t XXH32_round(uint32_t acc, uint32_t input) {
        acc += input * XXH_PRIME32_2;
        acc = rotl(acc, 13);
        acc *= XXH_PRIME32_1;
#if (defined(__SSE4_1__) || defined(__aarch64__) || defined(__wasm_simd128__)) && !defined(XXH_ENABLE_AUTOVECTORIZE)
        /*
         * UGLY HACK:
         * A compiler fence is used to prevent GCC and Clang from
         * autovectorizing the XXH32 loop (pragmas and attributes don't work for some
         * reason) without globally disabling SSE4.1.
         *
         * The reason we want to avoid vectorization is because despite working on
         * 4 integers at a time, there are multiple factors slowing XXH32 down on
         * SSE4:
         * - There's a ridiculous amount of lag from pmulld (10 cycles of latency on
         *   newer chips!) making it slightly slower to multiply four integers at
         *   once compared to four integers independently. Even when pmulld was
         *   fastest, Sandy/Ivy Bridge, it is still not worth it to go into SSE
         *   just to multiply unless doing a long operation.
         *
         * - Four instructions are required to rotate,
         *      movqda tmp,  v // not required with VEX encoding
         *      pslld  tmp, 13 // tmp <<= 13
         *      psrld  v,   19 // x >>= 19
         *      por    v,  tmp // x |= tmp
         *   compared to one for scalar:
         *      roll   v, 13    // reliably fast across the board
         *      shldl  v, v, 13 // Sandy Bridge and later prefer this for some reason
         *
         * - Instruction level parallelism is actually more beneficial here because
         *   the SIMD actually serializes this operation: While v1 is rotating, v2
         *   can load data, while v3 can multiply. SSE forces them to operate
         *   together.
         *
         * This is also enabled on AArch64, as Clang is *very aggressive* in vectorizing
         * the loop. NEON is only faster on the A53, and with the newer cores, it is less
         * than half the speed.
         *
         * Additionally, this is used on WASM SIMD128 because it JITs to the same
         * SIMD instructions and has the same issue.
         */
        KUMO_CCO_BARRIER(acc);
#endif
        return acc;
    }

    KUMO_FORCE_INLINE const uint8_t*
    XXH32_consumeLong(
        uint32_t* KUMO_RESTRICT acc,
        uint8_t const* KUMO_RESTRICT input,
        size_t len) {
        const uint8_t* const bEnd = input + len;
        const uint8_t* const limit = bEnd - 15;
        KUMO_DASSERT(acc != NULL);
        KUMO_DASSERT(input != NULL);
        KUMO_DASSERT(len >= 16);
        do {
            acc[0] = XXH32_round(acc[0], turbo::little_endian::Load32(input));
            input += 4;
            acc[1] = XXH32_round(acc[1], turbo::little_endian::Load32(input));
            input += 4;
            acc[2] = XXH32_round(acc[2], turbo::little_endian::Load32(input));
            input += 4;
            acc[3] = XXH32_round(acc[3], turbo::little_endian::Load32(input));
            input += 4;
        } while (input < limit);

        return input;
    }

    KUMO_FORCE_INLINE uint32_t XXH32_mergeAccs(const uint32_t* acc) {
        KUMO_DASSERT(acc != NULL);
        return rotl(acc[0], 1) + rotl(acc[1], 7)
            + rotl(acc[2], 12) + rotl(acc[3], 18);
    }

    static uint32_t XXH32_finalize(uint32_t hash, const uint8_t* ptr, size_t len) {
#define XXH_PROCESS1                           \
    do {                                       \
        hash += (*ptr++) * XXH_PRIME32_5;      \
        hash = rotl(hash, 11) * XXH_PRIME32_1; \
    } while (0)

#define XXH_PROCESS4                                               \
    do {                                                           \
        hash += turbo::little_endian::Load32(ptr) * XXH_PRIME32_3; \
        ptr += 4;                                                  \
        hash = rotl(hash, 17) * XXH_PRIME32_4;                     \
    } while (0)

        if (ptr == NULL)
            KUMO_DASSERT(len == 0);

        /* Compact rerolled version; generally faster */
        if (!XXH32_ENDJMP) {
            len &= 15;
            while (len >= 4) {
                XXH_PROCESS4;
                len -= 4;
            }
            while (len > 0) {
                XXH_PROCESS1;
                --len;
            }
            return XXH32_avalanche(hash);
        } else {
            switch (len & 15) /* or switch(bEnd - p) */ {
            case 12:
                XXH_PROCESS4;
                KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
            case 8:
                XXH_PROCESS4;
                KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
            case 4:
                XXH_PROCESS4;
                return XXH32_avalanche(hash);

            case 13:
                XXH_PROCESS4;
                KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
            case 9:
                XXH_PROCESS4;
                KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
            case 5:
                XXH_PROCESS4;
                XXH_PROCESS1;
                return XXH32_avalanche(hash);

            case 14:
                XXH_PROCESS4;
                KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
            case 10:
                XXH_PROCESS4;
                KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
            case 6:
                XXH_PROCESS4;
                XXH_PROCESS1;
                XXH_PROCESS1;
                return XXH32_avalanche(hash);

            case 15:
                XXH_PROCESS4;
                KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
            case 11:
                XXH_PROCESS4;
                KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
            case 7:
                XXH_PROCESS4;
                KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
            case 3:
                XXH_PROCESS1;
                KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
            case 2:
                XXH_PROCESS1;
                KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
            case 1:
                XXH_PROCESS1;
                KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
            case 0:
                return XXH32_avalanche(hash);
            }
            KUMO_DASSERT(0);
            return hash; /* reaching this point is deemed impossible */
        }
    }

#ifdef XXH_OLD_NAMES
#define PROCESS1 XXH_PROCESS1
#define PROCESS4 XXH_PROCESS4
#else
#undef XXH_PROCESS1
#undef XXH_PROCESS4
#endif

    KUMO_FORCE_INLINE uint32_t XXH32_endian_align(const uint8_t* input, size_t len, uint32_t seed) {
        uint32_t h32;
        KUMO_DASSERT(input == nullptr ? len == 0 : true);
        if (len >= 16) {
            uint32_t acc[4];
            XXH32_initAccs(acc, seed);

            input = XXH32_consumeLong(acc, input, len);

            h32 = XXH32_mergeAccs(acc);
        } else {
            h32 = seed + XXH_PRIME32_5;
        }

        h32 += (uint32_t)len;

        return XXH32_finalize(h32, input, len & 15);
    }

    uint32_t xxhash32_scalar(const uint8_t* input, size_t len, uint32_t seed) {
        if constexpr (turbo::xxhash::kXxhForceAlignCheck) {
            if ((((size_t)input) & 3) == 0) { /* Input is 4-bytes aligned, leverage the speed benefit */
                return XXH32_endian_align(input, len, seed);
            }
        }

        return XXH32_endian_align(input, len, seed);
    }

    /*! @ingroup XXH32_family */
    void XXH32_state_t::reset(uint32_t seed) {
        KUMO_DASSERT(statePtr != NULL);
        memset(this, 0, sizeof(*this));
        XXH32_initAccs(acc, seed);
    }

    void XXH32_state_t::update(const uint8_t* input, size_t len) {
        if (input == NULL) {
            KUMO_DASSERT(len == 0);
            return;
        }

        total_len_32 += (uint32_t)len;
        large_len |= (uint32_t)((len >= 16) | (total_len_32 >= 16));

        KUMO_DASSERT(bufferedSize < sizeof(buffer));
        if (len < sizeof(buffer) - bufferedSize) { /* fill in tmp buffer */
            memcpy(buffer + bufferedSize, input, len);
            bufferedSize += (uint32_t)len;
            return;
        }

        {
            const uint8_t* xinput = (const uint8_t*)input;
            const uint8_t* const bEnd = xinput + len;

            if (bufferedSize) { /* non-empty buffer: complete first */
                memcpy(buffer + bufferedSize, xinput, sizeof(buffer) - bufferedSize);
                xinput += sizeof(buffer) - bufferedSize;
                /* then process one round */
                (void)XXH32_consumeLong(acc, buffer, sizeof(buffer));
                bufferedSize = 0;
            }

            KUMO_DASSERT(xinput <= bEnd);
            if ((size_t)(bEnd - xinput) >= sizeof(buffer)) {
                /* Process the remaining data */
                xinput = XXH32_consumeLong(acc, xinput, (size_t)(bEnd - xinput));
            }

            if (xinput < bEnd) {
                /* Copy the leftover to the tmp buffer */
                memcpy(buffer, xinput, (size_t)(bEnd - xinput));
                bufferedSize = (unsigned)(bEnd - xinput);
            }
        }
    }

    uint32_t XXH32_state_t::digest() {
        uint32_t h32;

        if (large_len) {
            h32 = XXH32_mergeAccs(acc);
        } else {
            h32 = acc[2] /* == seed */ + XXH_PRIME32_5;
        }

        h32 += total_len_32;

        return XXH32_finalize(h32, buffer, bufferedSize);
    }
    ////////////////////////////////////////////////////////////////////////
    /// 64 bits

    static uint64_t XXH64_round(uint64_t acc, uint64_t input) {
        acc += input * XXH_PRIME64_2;
        acc = rotl(acc, 31);
        acc *= XXH_PRIME64_1;
#if (defined(__AVX512F__)) && !defined(XXH_ENABLE_AUTOVECTORIZE)
        KUMO_CCO_BARRIER(acc);
#endif
        return acc;
    }

    static uint64_t XXH64_mergeRound(uint64_t acc, uint64_t val) {
        val = XXH64_round(0, val);
        acc ^= val;
        acc = acc * XXH_PRIME64_1 + XXH_PRIME64_4;
        return acc;
    }

    KUMO_FORCE_INLINE void
    XXH64_initAccs(uint64_t* acc, uint64_t seed) {
        KUMO_DASSERT(acc != NULL);
        acc[0] = seed + XXH_PRIME64_1 + XXH_PRIME64_2;
        acc[1] = seed + XXH_PRIME64_2;
        acc[2] = seed + 0;
        acc[3] = seed - XXH_PRIME64_1;
    }

    KUMO_FORCE_INLINE const uint8_t*
    XXH64_consumeLong(
        uint64_t* KUMO_RESTRICT acc,
        uint8_t const* KUMO_RESTRICT input,
        size_t len) {
        const uint8_t* const bEnd = input + len;
        const uint8_t* const limit = bEnd - 31;
        KUMO_DASSERT(acc != NULL);
        KUMO_DASSERT(input != NULL);
        KUMO_DASSERT(len >= 32);
        do {
            /* reroll on 32-bit */
            if (sizeof(void*) < sizeof(uint64_t)) {
                size_t i;
                for (i = 0; i < 4; i++) {
                    acc[i] = XXH64_round(acc[i], turbo::little_endian::Load64(input));
                    input += 8;
                }
            } else {
                acc[0] = XXH64_round(acc[0], turbo::little_endian::Load64(input));
                input += 8;
                acc[1] = XXH64_round(acc[1], turbo::little_endian::Load64(input));
                input += 8;
                acc[2] = XXH64_round(acc[2], turbo::little_endian::Load64(input));
                input += 8;
                acc[3] = XXH64_round(acc[3], turbo::little_endian::Load64(input));
                input += 8;
            }
        } while (input < limit);

        return input;
    }

    KUMO_FORCE_INLINE uint64_t
    XXH64_mergeAccs(const uint64_t* acc) {
        KUMO_DASSERT(acc != NULL);
        {
            uint64_t h64 = rotl(acc[0], 1) + rotl(acc[1], 7)
                + rotl(acc[2], 12) + rotl(acc[3], 18);
            /* reroll on 32-bit */
            if (sizeof(void*) < sizeof(uint64_t)) {
                size_t i;
                for (i = 0; i < 4; i++) {
                    h64 = XXH64_mergeRound(h64, acc[i]);
                }
            } else {
                h64 = XXH64_mergeRound(h64, acc[0]);
                h64 = XXH64_mergeRound(h64, acc[1]);
                h64 = XXH64_mergeRound(h64, acc[2]);
                h64 = XXH64_mergeRound(h64, acc[3]);
            }
            return h64;
        }
    }

    static uint64_t
    XXH64_finalize(uint64_t hash, const uint8_t* ptr, size_t len) {
        if (ptr == NULL)
            KUMO_DASSERT(len == 0);
        len &= 31;
        while (len >= 8) {
            uint64_t const k1 = XXH64_round(0, turbo::little_endian::Load64(ptr));
            ptr += 8;
            hash ^= k1;
            hash = rotl(hash, 27) * XXH_PRIME64_1 + XXH_PRIME64_4;
            len -= 8;
        }
        if (len >= 4) {
            hash ^= (uint64_t)(turbo::little_endian::Load32(ptr)) * XXH_PRIME64_1;
            ptr += 4;
            hash = rotl(hash, 23) * XXH_PRIME64_2 + XXH_PRIME64_3;
            len -= 4;
        }
        while (len > 0) {
            hash ^= (*ptr++) * XXH_PRIME64_5;
            hash = rotl(hash, 11) * XXH_PRIME64_1;
            --len;
        }
        return xxhash::xxhash_scalar_avalanche64(hash);
    }

#undef XXH_PROCESS1_64
#undef XXH_PROCESS4_64
#undef XXH_PROCESS8_64

    KUMO_FORCE_INLINE uint64_t
    XXH64_endian_align(const uint8_t* input, size_t len, uint64_t seed) {
        uint64_t h64;
        if (input == NULL)
            KUMO_DASSERT(len == 0);

        if (len >= 32) { /* Process a large block of data */
            uint64_t acc[4];
            XXH64_initAccs(acc, seed);

            input = XXH64_consumeLong(acc, input, len);

            h64 = XXH64_mergeAccs(acc);
        } else {
            h64 = seed + XXH_PRIME64_5;
        }

        h64 += (uint64_t)len;

        return XXH64_finalize(h64, input, len);
    }

    KUMO_DLL uint64_t xxhash64_scalar(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t len, uint64_t seed) {
        if constexpr (turbo::xxhash::kXxhForceAlignCheck) {
            if ((((size_t)input) & 7) == 0) {
                return XXH64_endian_align(input, len, seed);
            }
        }

        return XXH64_endian_align(input, len, seed);
    }

    /*! @ingroup XXH64_family */
    void XXH64_state_s::reset(uint64_t seed) {
        KUMO_DASSERT(statePtr != NULL);
        memset(this, 0, sizeof(*this));
        XXH64_initAccs(this->acc, seed);
    }

    void XXH64_state_s::update(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t len) {
        if (input == NULL) {
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

            if (bufferedSize) { /* non-empty buffer => complete first */
                memcpy(buffer + bufferedSize, xinput, sizeof(buffer) - bufferedSize);
                xinput += sizeof(buffer) - bufferedSize;
                /* and process one round */
                (void)XXH64_consumeLong(acc, buffer, sizeof(buffer));
                bufferedSize = 0;
            }

            KUMO_DASSERT(xinput <= bEnd);
            if ((size_t)(bEnd - xinput) >= sizeof(buffer)) {
                /* Process the remaining data */
                xinput = XXH64_consumeLong(acc, xinput, (size_t)(bEnd - xinput));
            }

            if (xinput < bEnd) {
                /* Copy the leftover to the tmp buffer */
                memcpy(buffer, xinput, (size_t)(bEnd - xinput));
                bufferedSize = (unsigned)(bEnd - xinput);
            }
        }
    }

    /*! @ingroup XXH64_family */
    uint64_t XXH64_state_s::digest() {
        uint64_t h64;

        if (total_len >= 32) {
            h64 = XXH64_mergeAccs(acc);
        } else {
            h64 = acc[2] /*seed*/ + XXH_PRIME64_5;
        }

        h64 += (uint64_t)total_len;

        return XXH64_finalize(h64, buffer, (size_t)total_len);
    }
} // namespace turbo
