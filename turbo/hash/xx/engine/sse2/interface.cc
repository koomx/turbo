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

#include <turbo/hash/xx/engine/sse2/interface.h>

#if KUMO_SIMD_SSE2
#include <emmintrin.h>

namespace turbo::xxhash {

    KUMO_FORCE_INLINE void XXH3_accumulate_512_sse2(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        KUMO_DASSERT((((size_t)acc) & 15) == 0);
        {
            __m128i* const xacc = (__m128i*)acc;
            const __m128i* const xinput = (const __m128i*)input;
            const __m128i* const xsecret = (const __m128i*)secret;

            size_t i;
            for (i = 0; i < XXH_STRIPE_LEN / sizeof(__m128i); i++) {
                __m128i const data_vec = _mm_loadu_si128(xinput + i);
                __m128i const key_vec = _mm_loadu_si128(xsecret + i);
                __m128i const data_key = _mm_xor_si128(data_vec, key_vec);
                __m128i const data_key_lo = _mm_shuffle_epi32(data_key, _MM_SHUFFLE(0, 3, 0, 1));
                __m128i const product = _mm_mul_epu32(data_key, data_key_lo);
                __m128i const data_swap = _mm_shuffle_epi32(data_vec, _MM_SHUFFLE(1, 0, 3, 2));
                __m128i const sum = _mm_add_epi64(xacc[i], data_swap);
                xacc[i] = _mm_add_epi64(product, sum);
            }
        }
    }

    KUMO_FORCE_INLINE void XXH3_accumulate_sse2(uint64_t* KUMO_RESTRICT acc,
        const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret,
        size_t nbStripes) {
        size_t n;
        for (n = 0; n < nbStripes; n++) {
            const uint8_t* const in = input + n * XXH_STRIPE_LEN;
            XXH_PREFETCH(in + XXH_PREFETCH_DIST);
            XXH3_accumulate_512_sse2(acc, in, secret + n * XXH_SECRET_CONSUME_RATE);
        }
    }

    KUMO_FORCE_INLINE void XXH3_scrambleAcc_sse2(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT secret) {
        KUMO_DASSERT((((size_t)acc) & 15) == 0);
        {
            __m128i* const xacc = (__m128i*)acc;
            const __m128i* const xsecret = (const __m128i*)secret;
            const __m128i prime32 = _mm_set1_epi32((int)XXH_PRIME32_1);

            size_t i;
            for (i = 0; i < XXH_STRIPE_LEN / sizeof(__m128i); i++) {
                __m128i const acc_vec = xacc[i];
                __m128i const shifted = _mm_srli_epi64(acc_vec, 47);
                __m128i const data_vec = _mm_xor_si128(acc_vec, shifted);
                __m128i const key_vec = _mm_loadu_si128(xsecret + i);
                __m128i const data_key = _mm_xor_si128(data_vec, key_vec);

                __m128i const data_key_hi = _mm_shuffle_epi32(data_key, _MM_SHUFFLE(0, 3, 0, 1));
                __m128i const prod_lo = _mm_mul_epu32(data_key, prime32);
                __m128i const prod_hi = _mm_mul_epu32(data_key_hi, prime32);
                xacc[i] = _mm_add_epi64(prod_lo, _mm_slli_epi64(prod_hi, 32));
            }
        }
    }

    KUMO_FORCE_INLINE void XXH3_initCustomSecret_sse2(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        static_assert((XXH_SECRET_DEFAULT_SIZE & 15) == 0, "(XXH_SECRET_DEFAULT_SIZE & 15) == 0");
        (void)(&turbo::little_endian::Store64);
        {
            int const nbRounds = XXH_SECRET_DEFAULT_SIZE / sizeof(__m128i);

#if defined(_MSC_VER) && defined(_M_IX86) && _MSC_VER <= 1900
            uint64_t const seed64_unsigned = (uint64_t)seed64;
            uint64_t const neg_seed64 = (uint64_t)(0ULL - seed64_unsigned);
            __m128i const seed = _mm_set_epi32(
                (int)(neg_seed64 >> 32),
                (int)(neg_seed64),
                (int)(seed64_unsigned >> 32),
                (int)(seed64_unsigned));
#else
            __m128i const seed = _mm_set_epi64x((int64_t)(0U - seed64), (int64_t)seed64);
#endif
            int i;

            const void* const src16 = XXH3_kSecret;
            __m128i* dst16 = (__m128i*)customSecret;
#if defined(__GNUC__) || defined(__clang__)
            KUMO_CCO_BARRIER(dst16);
#endif
            KUMO_DASSERT(((size_t)src16 & 15) == 0);
            KUMO_DASSERT(((size_t)dst16 & 15) == 0);

            for (i = 0; i < nbRounds; ++i) {
                dst16[i] = _mm_add_epi64(_mm_load_si128((const __m128i*)src16 + i), seed);
            }
        }
    }

    void XXHashEngineSse2::init_custom_secret(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        XXH3_initCustomSecret_sse2(customSecret, seed64);
    }

    void XXHashEngineSse2::accumulate(uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret, size_t nbStripes) {
        XXH3_accumulate_sse2(acc, input, secret, nbStripes);
    }

    void XXHashEngineSse2::scramble_acc(void* KUMO_RESTRICT acc, const void* secret) {
        XXH3_scrambleAcc_sse2(acc, secret);
    }

    void XXHashEngineSse2::accumulate_512(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        XXH3_accumulate_512_sse2(acc, input, secret);
    }

    static XXHashEngine* get_xxhash_sse2_instance() {
        static XXHashEngineSse2 ins;
        return &ins;
    }
} // namespace turbo::xxhash
#else
namespace turbo::xxhash {
    static XXHashEngine* get_xxhash_sse2_instance() {
        return nullptr;
    }
} // namespace turbo::xxhash
#endif

namespace turbo::xxhash {
    IsaInfo get_xxhash_sse2_info() {
        static IsaInfo ins = {
            KUMO_SIMD_SSE2 == 1,
            false,
            { kX86Sse2 },
            "sse2",
            get_xxhash_sse2_instance(),
        };
        return ins;
    }
} // namespace turbo::xxhash
