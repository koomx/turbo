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

#include <turbo/hash/xx/engine/avx2/interface.h>

#if KUMO_SIMD_AVX2
#include <immintrin.h>

#if defined(__GNUC__) && !defined(__clang__) && defined(__OPTIMIZE__) && XXH_SIZE_OPT <= 0
#pragma GCC push_options
#pragma GCC optimize("-O2")
#endif

namespace turbo::xxhash {

    KUMO_FORCE_INLINE void XXH3_accumulate_512_avx2(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        KUMO_DASSERT((((size_t)acc) & 31) == 0);
        {
            __m256i* const xacc = (__m256i*)acc;
            const __m256i* const xinput = (const __m256i*)input;
            const __m256i* const xsecret = (const __m256i*)secret;

            size_t i;
            for (i = 0; i < XXH_STRIPE_LEN / sizeof(__m256i); i++) {
                __m256i const data_vec = _mm256_loadu_si256(xinput + i);
                __m256i const key_vec = _mm256_loadu_si256(xsecret + i);
                __m256i const data_key = _mm256_xor_si256(data_vec, key_vec);
                __m256i const data_key_lo = _mm256_srli_epi64(data_key, 32);
                __m256i const product = _mm256_mul_epu32(data_key, data_key_lo);
                __m256i const data_swap = _mm256_shuffle_epi32(data_vec, _MM_SHUFFLE(1, 0, 3, 2));
                __m256i const sum = _mm256_add_epi64(xacc[i], data_swap);
                xacc[i] = _mm256_add_epi64(product, sum);
            }
        }
    }

    KUMO_FORCE_INLINE void XXH3_accumulate_avx2(uint64_t* KUMO_RESTRICT acc,
        const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret,
        size_t nbStripes) {
        size_t n;
        for (n = 0; n < nbStripes; n++) {
            const uint8_t* const in = input + n * XXH_STRIPE_LEN;
            XXH_PREFETCH(in + XXH_PREFETCH_DIST);
            XXH3_accumulate_512_avx2(acc, in, secret + n * XXH_SECRET_CONSUME_RATE);
        }
    }

    KUMO_FORCE_INLINE void XXH3_scrambleAcc_avx2(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT secret) {
        KUMO_DASSERT((((size_t)acc) & 31) == 0);
        {
            __m256i* const xacc = (__m256i*)acc;
            const __m256i* const xsecret = (const __m256i*)secret;
            const __m256i prime32 = _mm256_set1_epi32((int)XXH_PRIME32_1);

            size_t i;
            for (i = 0; i < XXH_STRIPE_LEN / sizeof(__m256i); i++) {
                __m256i const acc_vec = xacc[i];
                __m256i const shifted = _mm256_srli_epi64(acc_vec, 47);
                __m256i const data_vec = _mm256_xor_si256(acc_vec, shifted);
                __m256i const key_vec = _mm256_loadu_si256(xsecret + i);
                __m256i const data_key = _mm256_xor_si256(data_vec, key_vec);

                __m256i const data_key_hi = _mm256_srli_epi64(data_key, 32);
                __m256i const prod_lo = _mm256_mul_epu32(data_key, prime32);
                __m256i const prod_hi = _mm256_mul_epu32(data_key_hi, prime32);
                xacc[i] = _mm256_add_epi64(prod_lo, _mm256_slli_epi64(prod_hi, 32));
            }
        }
    }

    KUMO_FORCE_INLINE void XXH3_initCustomSecret_avx2(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        static_assert((XXH_SECRET_DEFAULT_SIZE & 31) == 0, "(XXH_SECRET_DEFAULT_SIZE & 31) == 0");
        static_assert((XXH_SECRET_DEFAULT_SIZE / sizeof(__m256i)) == 6, "XXH_SECRET_DEFAULT_SIZE / sizeof(__m256i)) == 6");
        static_assert(XXH_SEC_ALIGN <= 64, "XXH_SEC_ALIGN <= 64");
        (void)(&turbo::little_endian::Store64);
        XXH_PREFETCH(customSecret);
        {
            __m256i const seed = _mm256_set_epi64x((int64_t)(0U - seed64), (int64_t)seed64, (int64_t)(0U - seed64), (int64_t)seed64);

            const __m256i* const src = (const __m256i*)((const void*)XXH3_kSecret);
            __m256i* dest = (__m256i*)customSecret;

#if defined(__GNUC__) || defined(__clang__)
            KUMO_CCO_BARRIER(dest);
#endif
            KUMO_DASSERT(((size_t)src & 31) == 0);
            KUMO_DASSERT(((size_t)dest & 31) == 0);

            dest[0] = _mm256_add_epi64(_mm256_load_si256(src + 0), seed);
            dest[1] = _mm256_add_epi64(_mm256_load_si256(src + 1), seed);
            dest[2] = _mm256_add_epi64(_mm256_load_si256(src + 2), seed);
            dest[3] = _mm256_add_epi64(_mm256_load_si256(src + 3), seed);
            dest[4] = _mm256_add_epi64(_mm256_load_si256(src + 4), seed);
            dest[5] = _mm256_add_epi64(_mm256_load_si256(src + 5), seed);
        }
    }

    void XXHashEngineAvx2::init_custom_secret(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        XXH3_initCustomSecret_avx2(customSecret, seed64);
    }

    void XXHashEngineAvx2::accumulate(uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret, size_t nbStripes) {
        XXH3_accumulate_avx2(acc, input, secret, nbStripes);
    }

    void XXHashEngineAvx2::scramble_acc(void* KUMO_RESTRICT acc, const void* secret) {
        XXH3_scrambleAcc_avx2(acc, secret);
    }

    void XXHashEngineAvx2::accumulate_512(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        XXH3_accumulate_512_avx2(acc, input, secret);
    }

    static XXHashEngine* get_xxhash_avx2_instance() {
        static XXHashEngineAvx2 ins;
        return &ins;
    }
} // namespace turbo::xxhash

#if defined(__GNUC__) && !defined(__clang__) && defined(__OPTIMIZE__) && XXH_SIZE_OPT <= 0
#pragma GCC pop_options
#endif

#else
namespace turbo::xxhash {
    static XXHashEngine* get_xxhash_avx2_instance() {
        return nullptr;
    }
} // namespace turbo::xxhash
#endif

namespace turbo::xxhash {
    IsaInfo get_xxhash_avx2_info() {
        static IsaInfo ins = {
            KUMO_SIMD_AVX2 == 1,
            false,
            static_cast<uint32_t>(InstructionSet::AVX2),
            "avx2",
            get_xxhash_avx2_instance(),
        };
        return ins;
    }
} // namespace turbo::xxhash
