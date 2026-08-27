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

#include <turbo/hash/xx/engine/avx512/interface.h>

#if KUMO_SIMD_AVX512F
#include <immintrin.h>

namespace turbo::xxhash {

    KUMO_FORCE_INLINE void XXH3_accumulate_512_avx512(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        __m512i* const xacc = (__m512i*)acc;
        KUMO_DASSERT((((size_t)acc) & 63) == 0);
        static_assert(XXH_STRIPE_LEN == sizeof(__m512i), "XXH_STRIPE_LEN == sizeof(__m512i)");

        {
            __m512i const data_vec = _mm512_loadu_si512(input);
            __m512i const key_vec = _mm512_loadu_si512(secret);
            __m512i const data_key = _mm512_xor_si512(data_vec, key_vec);
            __m512i const data_key_lo = _mm512_srli_epi64(data_key, 32);
            __m512i const product = _mm512_mul_epu32(data_key, data_key_lo);
            __m512i const data_swap = _mm512_shuffle_epi32(data_vec, (_MM_PERM_ENUM)_MM_SHUFFLE(1, 0, 3, 2));
            __m512i const sum = _mm512_add_epi64(*xacc, data_swap);
            *xacc = _mm512_add_epi64(product, sum);
        }
    }

    KUMO_FORCE_INLINE void XXH3_accumulate_avx512(uint64_t* KUMO_RESTRICT acc,
        const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret,
        size_t nbStripes) {
        size_t n;
        for (n = 0; n < nbStripes; n++) {
            const uint8_t* const in = input + n * XXH_STRIPE_LEN;
            XXH_PREFETCH(in + XXH_PREFETCH_DIST);
            XXH3_accumulate_512_avx512(acc, in, secret + n * XXH_SECRET_CONSUME_RATE);
        }
    }

    KUMO_FORCE_INLINE void XXH3_scrambleAcc_avx512(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT secret) {
        KUMO_DASSERT((((size_t)acc) & 63) == 0);
        static_assert(XXH_STRIPE_LEN == sizeof(__m512i), "XXH_STRIPE_LEN == sizeof(__m512i)");
        {
            __m512i* const xacc = (__m512i*)acc;
            const __m512i prime32 = _mm512_set1_epi32((int)XXH_PRIME32_1);

            __m512i const acc_vec = *xacc;
            __m512i const shifted = _mm512_srli_epi64(acc_vec, 47);
            __m512i const key_vec = _mm512_loadu_si512(secret);
            __m512i const data_key = _mm512_ternarylogic_epi32(key_vec, acc_vec, shifted, 0x96);

            __m512i const data_key_hi = _mm512_srli_epi64(data_key, 32);
            __m512i const prod_lo = _mm512_mul_epu32(data_key, prime32);
            __m512i const prod_hi = _mm512_mul_epu32(data_key_hi, prime32);
            *xacc = _mm512_add_epi64(prod_lo, _mm512_slli_epi64(prod_hi, 32));
        }
    }

    KUMO_FORCE_INLINE void XXH3_initCustomSecret_avx512(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        static_assert((XXH_SECRET_DEFAULT_SIZE & 63) == 0, "(XXH_SECRET_DEFAULT_SIZE & 63) == 0");
        KUMO_DASSERT(((size_t)customSecret & 63) == 0);
        (void)(&turbo::little_endian::Store64);
        {
            int const nbRounds = XXH_SECRET_DEFAULT_SIZE / sizeof(__m512i);
            __m512i const seed_pos = _mm512_set1_epi64((int64_t)seed64);
            __m512i const seed = _mm512_mask_sub_epi64(seed_pos, 0xAA, _mm512_set1_epi8(0), seed_pos);

            const __m512i* const src = (const __m512i*)((const void*)XXH3_kSecret);
            __m512i* const dest = (__m512i*)customSecret;
            int i;
            KUMO_DASSERT(((size_t)src & 63) == 0);
            KUMO_DASSERT(((size_t)dest & 63) == 0);
            for (i = 0; i < nbRounds; ++i) {
                dest[i] = _mm512_add_epi64(_mm512_load_si512(src + i), seed);
            }
        }
    }

    void XXHashEngineAvx512::init_custom_secret(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        XXH3_initCustomSecret_avx512(customSecret, seed64);
    }

    void XXHashEngineAvx512::accumulate(uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret, size_t nbStripes) {
        XXH3_accumulate_avx512(acc, input, secret, nbStripes);
    }

    void XXHashEngineAvx512::scramble_acc(void* KUMO_RESTRICT acc, const void* secret) {
        XXH3_scrambleAcc_avx512(acc, secret);
    }

    void XXHashEngineAvx512::accumulate_512(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        XXH3_accumulate_512_avx512(acc, input, secret);
    }

    static XXHashEngine* get_xxhash_avx512_instance() {
        static XXHashEngineAvx512 ins;
        return &ins;
    }
} // namespace turbo::xxhash
#else
namespace turbo::xxhash {
    static XXHashEngine* get_xxhash_avx512_instance() {
        return nullptr;
    }
} // namespace turbo::xxhash
#endif

namespace turbo::xxhash {
    IsaInfo get_xxhash_avx512_info() {
        static IsaInfo ins = {
            KUMO_SIMD_AVX512F == 1,
            false,
            { kX86Avx512F },
            "avx512",
            get_xxhash_avx512_instance(),
        };
        return ins;
    }
} // namespace turbo::xxhash
