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

#include <turbo/arch/isa.h>
#include <turbo/hash/xx/interface.h>

namespace turbo::xxhash {

    KUMO_FORCE_INLINE void XXH3_scalarRound(void* KUMO_RESTRICT acc,
        void const* KUMO_RESTRICT input,
        void const* KUMO_RESTRICT secret,
        size_t lane) {
        uint64_t* xacc = (uint64_t*)acc;
        uint8_t const* xinput = (uint8_t const*)input;
        uint8_t const* xsecret = (uint8_t const*)secret;
        KUMO_DASSERT(lane < XXH_ACC_NB);
        KUMO_DASSERT(((size_t)acc & (XXH_ACC_ALIGN - 1)) == 0);
        {
            uint64_t const data_val = turbo::little_endian::Load64(xinput + lane * 8);
            uint64_t const data_key = data_val ^ turbo::little_endian::Load64(xsecret + lane * 8);
            xacc[lane ^ 1] += data_val; /* swap adjacent lanes */
            xacc[lane] = turbo::xxhash::XXH_mult32to64_add64(data_key /* & 0xFFFFFFFF */, data_key >> 32, xacc[lane]);
        }
    }

    KUMO_FORCE_INLINE void XXH3_accumulate_512_scalar(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        size_t i;
        /* ARM GCC refuses to unroll this loop, resulting in a 24% slowdown on ARMv6. */
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 8                       \
    && (defined(__arm__) || defined(__thumb2__))                                    \
    && defined(__ARM_FEATURE_UNALIGNED) /* no unaligned access just wastes bytes */ \
    && XXH_SIZE_OPT <= 0
#pragma GCC unroll 8
#endif
        for (i = 0; i < XXH_ACC_NB; i++) {
            XXH3_scalarRound(acc, input, secret, i);
        }
    }

    KUMO_FORCE_INLINE void XXH3_accumulate_scalar(uint64_t* KUMO_RESTRICT acc,
        const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret,
        size_t nbStripes) {
        size_t n;
        for (n = 0; n < nbStripes; n++) {
            const uint8_t* const in = input + n * XXH_STRIPE_LEN;
            XXH_PREFETCH(in + XXH_PREFETCH_DIST);
            XXH3_accumulate_512_scalar(
                acc,
                in,
                secret + n * XXH_SECRET_CONSUME_RATE);
        }
    }

    KUMO_FORCE_INLINE void
    XXH3_scalarScrambleRound(void* KUMO_RESTRICT acc,
        void const* KUMO_RESTRICT secret,
        size_t lane) {
        uint64_t* const xacc = (uint64_t*)acc; /* presumed aligned */
        const uint8_t* const xsecret = (const uint8_t*)secret; /* no alignment restriction */
        KUMO_DASSERT((((size_t)acc) & (XXH_ACC_ALIGN - 1)) == 0);
        KUMO_DASSERT(lane < XXH_ACC_NB);
        {
            uint64_t const key64 = turbo::little_endian::Load64(xsecret + lane * 8);
            uint64_t acc64 = xacc[lane];
            acc64 = turbo::xxhash::xxhash_scalar_xorshift64(acc64, 47);
            acc64 ^= key64;
            acc64 *= XXH_PRIME32_1;
            xacc[lane] = acc64;
        }
    }

    /*!
     * @internal
     * @brief Scrambles the accumulators after a large chunk has been read
     */
    KUMO_FORCE_INLINE void
    XXH3_scrambleAcc_scalar(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT secret) {
        size_t i;
        for (i = 0; i < XXH_ACC_NB; i++) {
            XXH3_scalarScrambleRound(acc, secret, i);
        }
    }

    KUMO_FORCE_INLINE void
    XXH3_initCustomSecret_scalar(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        const uint8_t* kSecretPtr = turbo::xxhash::XXH3_kSecret;
        static_assert((XXH_SECRET_DEFAULT_SIZE & 15) == 0, "(XXH_SECRET_DEFAULT_SIZE & 15) == 0");

#if defined(__GNUC__) && defined(__aarch64__)

        KUMO_CCO_BARRIER(kSecretPtr);
#endif
        {
            int const nbRounds = XXH_SECRET_DEFAULT_SIZE / 16;
            int i;
            for (i = 0; i < nbRounds; i++) {
                uint64_t lo = turbo::little_endian::Load64(kSecretPtr + 16 * i) + seed64;
                uint64_t hi = turbo::little_endian::Load64(kSecretPtr + 16 * i + 8) - seed64;
                turbo::little_endian::Store64((uint8_t*)customSecret + 16 * i, lo);
                turbo::little_endian::Store64((uint8_t*)customSecret + 16 * i + 8, hi);
            }
        }
    }

    class XXHashEngineScalar : public XXHashEngine {
        void init_custom_secret(void* KUMO_RESTRICT customSecret, uint64_t seed64) override;
        void accumulate(uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT input, const uint8_t* KUMO_RESTRICT secret, size_t nbStripes) override;
        void scramble_acc(void* KUMO_RESTRICT acc, const void* secret) override;
        void accumulate_512(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT input, const void* KUMO_RESTRICT secret) override;
    };

    IsaInfo get_xxhash_fallback_info();
} // namespace turbo::xxhash
