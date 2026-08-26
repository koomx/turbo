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

#include <turbo/hash/xx/common.h>

namespace turbo::xxhash {

    KUMO_FORCE_INLINE XXH128_hash_t XXH_mult64to128(uint64_t lhs, uint64_t rhs) {

#if (defined(__GNUC__) || defined(__clang__)) && !defined(__wasm__) \
        && defined(__SIZEOF_INT128__)                               \
    || (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128)

        __uint128_t const product = (__uint128_t)lhs * (__uint128_t)rhs;
        XXH128_hash_t r128;
        r128.low64 = (uint64_t)(product);
        r128.high64 = (uint64_t)(product >> 64);
        return r128;

#elif (defined(_M_X64) || defined(_M_IA64)) && !defined(_M_ARM64EC)

#ifndef _MSC_VER
#pragma intrinsic(_umul128)
#endif
        uint64_t product_high;
        uint64_t const product_low = _umul128(lhs, rhs, &product_high);
        XXH128_hash_t r128;
        r128.low64 = product_low;
        r128.high64 = product_high;
        return r128;

#elif defined(_M_ARM64) || defined(_M_ARM64EC)

#ifndef _MSC_VER
#pragma intrinsic(__umulh)
#endif
        XXH128_hash_t r128;
        r128.low64 = lhs * rhs;
        r128.high64 = __umulh(lhs, rhs);
        return r128;

#else

        /* First calculate all of the cross products. */
        uint64_t const lo_lo = XXH_mult32to64(lhs & 0xFFFFFFFF, rhs & 0xFFFFFFFF);
        uint64_t const hi_lo = XXH_mult32to64(lhs >> 32, rhs & 0xFFFFFFFF);
        uint64_t const lo_hi = XXH_mult32to64(lhs & 0xFFFFFFFF, rhs >> 32);
        uint64_t const hi_hi = XXH_mult32to64(lhs >> 32, rhs >> 32);

        /* Now add the products together. These will never overflow. */
        uint64_t const cross = (lo_lo >> 32) + (hi_lo & 0xFFFFFFFF) + lo_hi;
        uint64_t const upper = (hi_lo >> 32) + (cross >> 32) + hi_hi;
        uint64_t const lower = (cross << 32) | (lo_lo & 0xFFFFFFFF);

        XXH128_hash_t r128;
        r128.low64 = lower;
        r128.high64 = upper;
        return r128;
#endif
    }

    KUMO_FORCE_INLINE uint64_t XXH3_mul128_fold64(uint64_t lhs, uint64_t rhs) {
        XXH128_hash_t product = XXH_mult64to128(lhs, rhs);
        return product.low64 ^ product.high64;
    }

    KUMO_FORCE_INLINE uint64_t XXH3_mix16B(const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret, uint64_t seed64) {
#if defined(__GNUC__) && !defined(__clang__) /* GCC, not Clang */ \
    && defined(__i386__) && defined(__SSE2__) /* x86 + SSE2 */    \
    && !defined(XXH_ENABLE_AUTOVECTORIZE) /* Define to disable like XXH32 hack */
        KUMO_CCO_BARRIER(seed64);
#endif
        {
            uint64_t const input_lo = turbo::little_endian::Load64(input);
            uint64_t const input_hi = turbo::little_endian::Load64(input + 8);
            return XXH3_mul128_fold64(
                input_lo ^ (turbo::little_endian::Load64(secret) + seed64),
                input_hi ^ (turbo::little_endian::Load64(secret + 8) - seed64));
        }
    }

    KUMO_FORCE_INLINE uint64_t XXH3_len_9to16_64b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed) {
        KUMO_DASSERT(input != NULL);
        KUMO_DASSERT(secret != NULL);
        KUMO_DASSERT(9 <= len && len <= 16);
        {
            uint64_t const bitflip1 = (turbo::little_endian::Load64(secret + 24) ^ turbo::little_endian::Load64(secret + 32)) + seed;
            uint64_t const bitflip2 = (turbo::little_endian::Load64(secret + 40) ^ turbo::little_endian::Load64(secret + 48)) - seed;
            uint64_t const input_lo = turbo::little_endian::Load64(input) ^ bitflip1;
            uint64_t const input_hi = turbo::little_endian::Load64(input + len - 8) ^ bitflip2;
            uint64_t const acc = len
                + turbo::byteswap(input_lo) + input_hi
                + XXH3_mul128_fold64(input_lo, input_hi);
            return XXH3_avalanche(acc);
        }
    }

    KUMO_FORCE_INLINE uint64_t XXH3_len_4to8_64b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed) {
        KUMO_DASSERT(input != NULL);
        KUMO_DASSERT(secret != NULL);
        KUMO_DASSERT(4 <= len && len <= 8);
        seed ^= (uint64_t)turbo::byteswap((uint32_t)seed) << 32;
        {
            uint32_t const input1 = turbo::little_endian::Load32(input);
            uint32_t const input2 = turbo::little_endian::Load32(input + len - 4);
            uint64_t const bitflip = (turbo::little_endian::Load64(secret + 8) ^ turbo::little_endian::Load64(secret + 16)) - seed;
            uint64_t const input64 = input2 + (((uint64_t)input1) << 32);
            uint64_t const keyed = input64 ^ bitflip;
            return xxhash_scalar_rrmxmx(keyed, len);
        }
    }

    KUMO_FORCE_INLINE uint64_t
    XXH3_len_1to3_64b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed) {
        KUMO_DASSERT(input != NULL);
        KUMO_DASSERT(1 <= len && len <= 3);
        KUMO_DASSERT(secret != NULL);

        {
            uint8_t const c1 = input[0];
            uint8_t const c2 = input[len >> 1];
            uint8_t const c3 = input[len - 1];
            uint32_t const combined = ((uint32_t)c1 << 16) | ((uint32_t)c2 << 24)
                | ((uint32_t)c3 << 0) | ((uint32_t)len << 8);
            uint64_t const bitflip = (turbo::little_endian::Load32(secret) ^ turbo::little_endian::Load32(secret + 4)) + seed;
            uint64_t const keyed = (uint64_t)combined ^ bitflip;
            return xxhash_scalar_avalanche64(keyed);
        }
    }

    KUMO_FORCE_INLINE uint64_t XXH3_len_0to16_64b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed) {
        KUMO_DASSERT(len <= 16);
        {
            if (KUMO_LIKELY(len > 8))
                return XXH3_len_9to16_64b(input, len, secret, seed);
            if (KUMO_LIKELY(len >= 4))
                return XXH3_len_4to8_64b(input, len, secret, seed);
            if (len)
                return XXH3_len_1to3_64b(input, len, secret, seed);
            return xxhash_scalar_avalanche64(seed ^ (turbo::little_endian::Load64(secret + 56) ^ turbo::little_endian::Load64(secret + 64)));
        }
    }

    KUMO_FORCE_INLINE uint64_t
    XXH3_len_17to128_64b(const uint8_t* KUMO_RESTRICT input, size_t len,
        const uint8_t* KUMO_RESTRICT secret, size_t secretSize,
        uint64_t seed) {
        KUMO_DASSERT(secretSize >= XXH3_SECRET_SIZE_MIN);
        (void)secretSize;
        KUMO_DASSERT(16 < len && len <= 128);

        {
            uint64_t acc = len * XXH_PRIME64_1;
#if XXH_SIZE_OPT >= 1
            /* Smaller and cleaner, but slightly slower. */
            unsigned int i = (unsigned int)(len - 1) / 32;
            do {
                acc += XXH3_mix16B(input + 16 * i, secret + 32 * i, seed);
                acc += XXH3_mix16B(input + len - 16 * (i + 1), secret + 32 * i + 16, seed);
            } while (i-- != 0);
#else
            if (len > 32) {
                if (len > 64) {
                    if (len > 96) {
                        acc += XXH3_mix16B(input + 48, secret + 96, seed);
                        acc += XXH3_mix16B(input + len - 64, secret + 112, seed);
                    }
                    acc += XXH3_mix16B(input + 32, secret + 64, seed);
                    acc += XXH3_mix16B(input + len - 48, secret + 80, seed);
                }
                acc += XXH3_mix16B(input + 16, secret + 32, seed);
                acc += XXH3_mix16B(input + len - 32, secret + 48, seed);
            }
            acc += XXH3_mix16B(input + 0, secret + 0, seed);
            acc += XXH3_mix16B(input + len - 16, secret + 16, seed);
#endif
            return XXH3_avalanche(acc);
        }
    }

    KUMO_FORCE_INLINE uint64_t
    XXH3_len_129to240_64b(const uint8_t* KUMO_RESTRICT input, size_t len,
        const uint8_t* KUMO_RESTRICT secret, size_t secretSize,
        uint64_t seed) {
        KUMO_DASSERT(secretSize >= XXH3_SECRET_SIZE_MIN);
        (void)secretSize;
        KUMO_DASSERT(128 < len && len <= XXH3_MIDSIZE_MAX);

#define XXH3_MIDSIZE_STARTOFFSET 3
#define XXH3_MIDSIZE_LASTOFFSET 17

        {
            uint64_t acc = len * XXH_PRIME64_1;
            uint64_t acc_end;
            unsigned int const nbRounds = (unsigned int)len / 16;
            unsigned int i;
            KUMO_DASSERT(128 < len && len <= XXH3_MIDSIZE_MAX);
            for (i = 0; i < 8; i++) {
                acc += XXH3_mix16B(input + (16 * i), secret + (16 * i), seed);
            }
            /* last bytes */
            acc_end = XXH3_mix16B(input + len - 16, secret + XXH3_SECRET_SIZE_MIN - XXH3_MIDSIZE_LASTOFFSET, seed);
            KUMO_DASSERT(nbRounds >= 8);
            acc = XXH3_avalanche(acc);
#if defined(__clang__) /* Clang */                               \
    && (defined(__ARM_NEON) || defined(__ARM_NEON__)) /* NEON */ \
    && !defined(XXH_ENABLE_AUTOVECTORIZE) /* Define to disable */
#pragma clang loop vectorize(disable)
#endif
            for (i = 8; i < nbRounds; i++) {
                KUMO_CCO_BARRIER(acc);
                acc_end += XXH3_mix16B(input + (16 * i), secret + (16 * (i - 8)) + XXH3_MIDSIZE_STARTOFFSET, seed);
            }
            return XXH3_avalanche(acc + acc_end);
        }
    }

    KUMO_FORCE_INLINE XXH128_hash_t XXH3_len_9to16_128b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed) {
        KUMO_DASSERT(input != NULL);
        KUMO_DASSERT(secret != NULL);
        KUMO_DASSERT(9 <= len && len <= 16);
        {
            uint64_t const bitflipl = (turbo::little_endian::Load64(secret + 32) ^ turbo::little_endian::Load64(secret + 40)) - seed;
            uint64_t const bitfliph = (turbo::little_endian::Load64(secret + 48) ^ turbo::little_endian::Load64(secret + 56)) + seed;
            uint64_t const input_lo = turbo::little_endian::Load64(input);
            uint64_t input_hi = turbo::little_endian::Load64(input + len - 8);
            XXH128_hash_t m128 = turbo::xxhash::XXH_mult64to128(input_lo ^ input_hi ^ bitflipl, XXH_PRIME64_1);

            m128.low64 += (uint64_t)(len - 1) << 54;
            input_hi ^= bitfliph;

            if (sizeof(void*) < sizeof(uint64_t)) { /* 32-bit */

                m128.high64 += (input_hi & 0xFFFFFFFF00000000ULL) + XXH_mult32to64((uint32_t)input_hi, XXH_PRIME32_2);
            } else {

                m128.high64 += input_hi + XXH_mult32to64((uint32_t)input_hi, XXH_PRIME32_2 - 1);
            }

            m128.low64 ^= turbo::byteswap(m128.high64);

            { /* 128x64 multiply: h128 = m128 * XXH_PRIME64_2; */
                XXH128_hash_t h128 = turbo::xxhash::XXH_mult64to128(m128.low64, XXH_PRIME64_2);
                h128.high64 += m128.high64 * XXH_PRIME64_2;

                h128.low64 = XXH3_avalanche(h128.low64);
                h128.high64 = XXH3_avalanche(h128.high64);
                return h128;
            }
        }
    }

    KUMO_FORCE_INLINE XXH128_hash_t XXH3_len_1to3_128b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed) {
        /* A doubled version of 1to3_64b with different constants. */
        KUMO_DASSERT(input != NULL);
        KUMO_DASSERT(1 <= len && len <= 3);
        KUMO_DASSERT(secret != NULL);
        /*
         * len = 1: combinedl = { input[0], 0x01, input[0], input[0] }
         * len = 2: combinedl = { input[1], 0x02, input[0], input[1] }
         * len = 3: combinedl = { input[2], 0x03, input[0], input[1] }
         */
        {
            uint8_t const c1 = input[0];
            uint8_t const c2 = input[len >> 1];
            uint8_t const c3 = input[len - 1];
            uint32_t const combinedl = ((uint32_t)c1 << 16) | ((uint32_t)c2 << 24)
                | ((uint32_t)c3 << 0) | ((uint32_t)len << 8);
            uint32_t const combinedh = turbo::rotl(turbo::byteswap(combinedl), 13);
            uint64_t const bitflipl = (turbo::little_endian::Load32(secret) ^ turbo::little_endian::Load32(secret + 4)) + seed;
            uint64_t const bitfliph = (turbo::little_endian::Load32(secret + 8) ^ turbo::little_endian::Load32(secret + 12)) - seed;
            uint64_t const keyed_lo = (uint64_t)combinedl ^ bitflipl;
            uint64_t const keyed_hi = (uint64_t)combinedh ^ bitfliph;
            XXH128_hash_t h128;
            h128.low64 = xxhash_scalar_avalanche64(keyed_lo);
            h128.high64 = xxhash_scalar_avalanche64(keyed_hi);
            return h128;
        }
    }

    KUMO_FORCE_INLINE XXH128_hash_t XXH3_len_4to8_128b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed) {
        KUMO_DASSERT(input != NULL);
        KUMO_DASSERT(secret != NULL);
        KUMO_DASSERT(4 <= len && len <= 8);
        seed ^= (uint64_t)turbo::byteswap((uint32_t)seed) << 32;
        {
            uint32_t const input_lo = turbo::little_endian::Load32(input);
            uint32_t const input_hi = turbo::little_endian::Load32(input + len - 4);
            uint64_t const input_64 = input_lo + ((uint64_t)input_hi << 32);
            uint64_t const bitflip = (turbo::little_endian::Load64(secret + 16) ^ turbo::little_endian::Load64(secret + 24)) + seed;
            uint64_t const keyed = input_64 ^ bitflip;

            /* Shift len to the left to ensure it is even, this avoids even multiplies. */
            XXH128_hash_t m128 = turbo::xxhash::XXH_mult64to128(keyed, XXH_PRIME64_1 + (len << 2));

            m128.high64 += (m128.low64 << 1);
            m128.low64 ^= (m128.high64 >> 3);

            m128.low64 = xxhash_scalar_xorshift64(m128.low64, 35);
            m128.low64 *= turbo::xxhash::PRIME_MX2;
            m128.low64 = xxhash_scalar_xorshift64(m128.low64, 28);
            m128.high64 = XXH3_avalanche(m128.high64);
            return m128;
        }
    }

    KUMO_FORCE_INLINE XXH128_hash_t XXH3_len_0to16_128b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed) {
        KUMO_DASSERT(len <= 16);
        {
            if (len > 8)
                return XXH3_len_9to16_128b(input, len, secret, seed);
            if (len >= 4)
                return XXH3_len_4to8_128b(input, len, secret, seed);
            if (len)
                return XXH3_len_1to3_128b(input, len, secret, seed);
            {
                XXH128_hash_t h128;
                uint64_t const bitflipl = turbo::little_endian::Load64(secret + 64) ^ turbo::little_endian::Load64(secret + 72);
                uint64_t const bitfliph = turbo::little_endian::Load64(secret + 80) ^ turbo::little_endian::Load64(secret + 88);
                h128.low64 = xxhash_scalar_avalanche64(seed ^ bitflipl);
                h128.high64 = xxhash_scalar_avalanche64(seed ^ bitfliph);
                return h128;
            }
        }
    }

    KUMO_FORCE_INLINE XXH128_hash_t XXH128_mix32B(XXH128_hash_t acc, const uint8_t* input_1, const uint8_t* input_2,
        const uint8_t* secret, uint64_t seed) {
        acc.low64 += turbo::xxhash::XXH3_mix16B(input_1, secret + 0, seed);
        acc.low64 ^= turbo::little_endian::Load64(input_2) + turbo::little_endian::Load64(input_2 + 8);
        acc.high64 += turbo::xxhash::XXH3_mix16B(input_2, secret + 16, seed);
        acc.high64 ^= turbo::little_endian::Load64(input_1) + turbo::little_endian::Load64(input_1 + 8);
        return acc;
    }

    KUMO_FORCE_INLINE XXH128_hash_t XXH3_len_17to128_128b(const uint8_t* KUMO_RESTRICT input, size_t len,
        const uint8_t* KUMO_RESTRICT secret, size_t secretSize,
        uint64_t seed) {
        KUMO_DASSERT(secretSize >= XXH3_SECRET_SIZE_MIN);
        (void)secretSize;
        KUMO_DASSERT(16 < len && len <= 128);

        {
            XXH128_hash_t acc;
            acc.low64 = len * XXH_PRIME64_1;
            acc.high64 = 0;

#if XXH_SIZE_OPT >= 1
            {
                /* Smaller, but slightly slower. */
                unsigned int i = (unsigned int)(len - 1) / 32;
                do {
                    acc = XXH128_mix32B(acc, input + 16 * i, input + len - 16 * (i + 1), secret + 32 * i, seed);
                } while (i-- != 0);
            }
#else
            if (len > 32) {
                if (len > 64) {
                    if (len > 96) {
                        acc = XXH128_mix32B(acc, input + 48, input + len - 64, secret + 96, seed);
                    }
                    acc = XXH128_mix32B(acc, input + 32, input + len - 48, secret + 64, seed);
                }
                acc = XXH128_mix32B(acc, input + 16, input + len - 32, secret + 32, seed);
            }
            acc = XXH128_mix32B(acc, input, input + len - 16, secret, seed);
#endif
            {
                XXH128_hash_t h128;
                h128.low64 = acc.low64 + acc.high64;
                h128.high64 = (acc.low64 * XXH_PRIME64_1)
                    + (acc.high64 * XXH_PRIME64_4)
                    + ((len - seed) * XXH_PRIME64_2);
                h128.low64 = XXH3_avalanche(h128.low64);
                h128.high64 = (uint64_t)0 - XXH3_avalanche(h128.high64);
                return h128;
            }
        }
    }

    KUMO_FORCE_INLINE XXH128_hash_t XXH3_len_129to240_128b(const uint8_t* KUMO_RESTRICT input, size_t len,
        const uint8_t* KUMO_RESTRICT secret, size_t secretSize,
        uint64_t seed) {
        KUMO_DASSERT(secretSize >= XXH3_SECRET_SIZE_MIN);
        (void)secretSize;
        KUMO_DASSERT(128 < len && len <= XXH3_MIDSIZE_MAX);

        {
            XXH128_hash_t acc;
            unsigned i;
            acc.low64 = len * XXH_PRIME64_1;
            acc.high64 = 0;

            for (i = 32; i < 160; i += 32) {
                acc = XXH128_mix32B(acc,
                    input + i - 32,
                    input + i - 16,
                    secret + i - 32,
                    seed);
            }
            acc.low64 = XXH3_avalanche(acc.low64);
            acc.high64 = XXH3_avalanche(acc.high64);

            for (i = 160; i <= len; i += 32) {
                acc = XXH128_mix32B(acc,
                    input + i - 32,
                    input + i - 16,
                    secret + XXH3_MIDSIZE_STARTOFFSET + i - 160,
                    seed);
            }
            /* last bytes */
            acc = XXH128_mix32B(acc,
                input + len - 16,
                input + len - 32,
                secret + XXH3_SECRET_SIZE_MIN - XXH3_MIDSIZE_LASTOFFSET - 16,
                (uint64_t)0 - seed);

            {
                XXH128_hash_t h128;
                h128.low64 = acc.low64 + acc.high64;
                h128.high64 = (acc.low64 * XXH_PRIME64_1)
                    + (acc.high64 * XXH_PRIME64_4)
                    + ((len - seed) * XXH_PRIME64_2);
                h128.low64 = XXH3_avalanche(h128.low64);
                h128.high64 = (uint64_t)0 - XXH3_avalanche(h128.high64);
                return h128;
            }
        }
    }

    KUMO_FORCE_INLINE uint64_t XXH3_mix2Accs(const uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT secret) {
        return turbo::xxhash::XXH3_mul128_fold64(
            acc[0] ^ turbo::little_endian::Load64(secret),
            acc[1] ^ turbo::little_endian::Load64(secret + 8));
    }

    KUMO_FORCE_INLINE uint64_t XXH3_mergeAccs(const uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT secret, uint64_t start) {
        uint64_t result64 = start;
        size_t i = 0;

        for (i = 0; i < 4; i++) {
            result64 += XXH3_mix2Accs(acc + 2 * i, secret + 16 * i);
#if defined(__clang__) /* Clang */                               \
    && (defined(__arm__) || defined(__thumb__)) /* ARMv7 */      \
    && (defined(__ARM_NEON) || defined(__ARM_NEON__)) /* NEON */ \
    && !defined(XXH_ENABLE_AUTOVECTORIZE) /* Define to disable */
            KUMO_CCO_BARRIER(result64);
#endif
        }

        return XXH3_avalanche(result64);
    }

    KUMO_FORCE_INLINE uint64_t XXH3_finalizeLong_64b(const uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT secret, uint64_t len) {
        return turbo::xxhash::XXH3_mergeAccs(acc, secret + XXH_SECRET_MERGEACCS_START, len * XXH_PRIME64_1);
    }
    KUMO_FORCE_INLINE turbo::xxhash::XXH128_hash_t XXH3_finalizeLong_128b(const uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT secret, size_t secretSize, uint64_t len) {
        turbo::xxhash::XXH128_hash_t h128;
        h128.low64 = XXH3_finalizeLong_64b(acc, secret, len);
        h128.high64 = turbo::xxhash::XXH3_mergeAccs(acc, secret + secretSize - XXH_STRIPE_LEN - XXH_SECRET_MERGEACCS_START,
            ~(len * XXH_PRIME64_2));
        return h128;
    }

} // namespace turbo::xxhash
