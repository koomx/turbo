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

#include <turbo/hash/xx/engine/neon/interface.h>

#if KUMO_SIMD_NEON
namespace turbo::xxhash {



    typedef uint64x2_t xxh_aliasing_uint64x2_t XXH_ALIASING;

#ifndef XXH3_NEON_LANES
#if (defined(__aarch64__) || defined(__arm64__) || defined(_M_ARM64) || defined(_M_ARM64EC)) \
    && !defined(__APPLE__) && XXH_SIZE_OPT <= 0
#define XXH3_NEON_LANES 6
#else
#define XXH3_NEON_LANES XXH_ACC_NB
#endif
#endif

#if defined(__aarch64__) && defined(__GNUC__) && !defined(__clang__)
    KUMO_FORCE_INLINE uint64x2_t XXH_vld1q_u64(void const* ptr) {
        return *(xxh_aliasing_uint64x2_t const*)ptr;
    }
#else
    KUMO_FORCE_INLINE uint64x2_t XXH_vld1q_u64(void const* ptr) {
        return vreinterpretq_u64_u8(vld1q_u8((uint8_t const*)ptr));
    }
#endif

#if defined(__aarch64__) && defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 11
    KUMO_FORCE_INLINE uint64x2_t XXH_vmlal_low_u32(uint64x2_t acc, uint32x4_t lhs, uint32x4_t rhs) {
        __asm__("umlal   %0.2d, %1.2s, %2.2s" : "+w"(acc) : "w"(lhs), "w"(rhs));
        return acc;
    }
    KUMO_FORCE_INLINE uint64x2_t XXH_vmlal_high_u32(uint64x2_t acc, uint32x4_t lhs, uint32x4_t rhs) {
        return vmlal_high_u32(acc, lhs, rhs);
    }
#else
    KUMO_FORCE_INLINE uint64x2_t XXH_vmlal_low_u32(uint64x2_t acc, uint32x4_t lhs, uint32x4_t rhs) {
        return vmlal_u32(acc, vget_low_u32(lhs), vget_low_u32(rhs));
    }
    KUMO_FORCE_INLINE uint64x2_t XXH_vmlal_high_u32(uint64x2_t acc, uint32x4_t lhs, uint32x4_t rhs) {
        return vmlal_u32(acc, vget_high_u32(lhs), vget_high_u32(rhs));
    }
#endif

    KUMO_FORCE_INLINE void XXH3_accumulate_512_neon(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        KUMO_DASSERT((((size_t)acc) & 15) == 0);
        static_assert(XXH3_NEON_LANES > 0 && XXH3_NEON_LANES <= XXH_ACC_NB && XXH3_NEON_LANES % 2 == 0,
            "XXH3_NEON_LANES > 0 && XXH3_NEON_LANES <= XXH_ACC_NB && XXH3_NEON_LANES % 2 == 0");
        {
            xxh_aliasing_uint64x2_t* const xacc = (xxh_aliasing_uint64x2_t*)acc;
            uint8_t const* xinput = (const uint8_t*)input;
            uint8_t const* xsecret = (const uint8_t*)secret;

            size_t i;
#ifdef __wasm_simd128__
            KUMO_CCO_BARRIER(xsecret);
#endif
            for (i = XXH3_NEON_LANES; i < XXH_ACC_NB; i++) {
                XXH3_scalarRound(acc, input, secret, i);
            }
            i = 0;
            for (; i + 1 < XXH3_NEON_LANES / 2; i += 2) {
                uint64x2_t data_vec_1 = XXH_vld1q_u64(xinput + (i * 16));
                uint64x2_t data_vec_2 = XXH_vld1q_u64(xinput + ((i + 1) * 16));
                uint64x2_t key_vec_1 = XXH_vld1q_u64(xsecret + (i * 16));
                uint64x2_t key_vec_2 = XXH_vld1q_u64(xsecret + ((i + 1) * 16));
                uint64x2_t data_swap_1 = vextq_u64(data_vec_1, data_vec_1, 1);
                uint64x2_t data_swap_2 = vextq_u64(data_vec_2, data_vec_2, 1);
                uint64x2_t data_key_1 = veorq_u64(data_vec_1, key_vec_1);
                uint64x2_t data_key_2 = veorq_u64(data_vec_2, key_vec_2);

                uint32x4x2_t unzipped = vuzpq_u32(
                    vreinterpretq_u32_u64(data_key_1),
                    vreinterpretq_u32_u64(data_key_2));
                uint32x4_t data_key_lo = unzipped.val[0];
                uint32x4_t data_key_hi = unzipped.val[1];

                uint64x2_t sum_1 = XXH_vmlal_low_u32(data_swap_1, data_key_lo, data_key_hi);
                uint64x2_t sum_2 = XXH_vmlal_high_u32(data_swap_2, data_key_lo, data_key_hi);

                KUMO_CCO_BARRIER_CLANG_NEON(sum_1);
                KUMO_CCO_BARRIER_CLANG_NEON(sum_2);
                xacc[i] = vaddq_u64(xacc[i], sum_1);
                xacc[i + 1] = vaddq_u64(xacc[i + 1], sum_2);
            }
            for (; i < XXH3_NEON_LANES / 2; i++) {
                uint64x2_t data_vec = XXH_vld1q_u64(xinput + (i * 16));
                uint64x2_t key_vec = XXH_vld1q_u64(xsecret + (i * 16));
                uint64x2_t data_swap = vextq_u64(data_vec, data_vec, 1);
                uint64x2_t data_key = veorq_u64(data_vec, key_vec);
                uint32x2_t data_key_lo = vmovn_u64(data_key);
                uint32x2_t data_key_hi = vshrn_n_u64(data_key, 32);
                uint64x2_t sum = vmlal_u32(data_swap, data_key_lo, data_key_hi);
                KUMO_CCO_BARRIER_CLANG_NEON(sum);
                xacc[i] = vaddq_u64(xacc[i], sum);
            }
        }
    }

    KUMO_FORCE_INLINE void XXH3_accumulate_neon(uint64_t* KUMO_RESTRICT acc,
        const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret,
        size_t nbStripes) {
        size_t n;
        for (n = 0; n < nbStripes; n++) {
            const uint8_t* const in = input + n * XXH_STRIPE_LEN;
            XXH_PREFETCH(in + XXH_PREFETCH_DIST);
            XXH3_accumulate_512_neon(acc, in, secret + n * XXH_SECRET_CONSUME_RATE);
        }
    }

    KUMO_FORCE_INLINE void XXH3_scrambleAcc_neon(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT secret) {
        KUMO_DASSERT((((size_t)acc) & 15) == 0);

        {
            xxh_aliasing_uint64x2_t* xacc = (xxh_aliasing_uint64x2_t*)acc;
            uint8_t const* xsecret = (uint8_t const*)secret;

            size_t i;
#ifndef __wasm_simd128__
            uint32x2_t const kPrimeLo = vdup_n_u32(XXH_PRIME32_1);
            uint32x4_t const kPrimeHi = vreinterpretq_u32_u64(vdupq_n_u64((uint64_t)XXH_PRIME32_1 << 32));
#endif

            for (i = XXH3_NEON_LANES; i < XXH_ACC_NB; i++) {
                XXH3_scalarScrambleRound(acc, secret, i);
            }
            for (i = 0; i < XXH3_NEON_LANES / 2; i++) {
                uint64x2_t acc_vec = xacc[i];
                uint64x2_t shifted = vshrq_n_u64(acc_vec, 47);
                uint64x2_t data_vec = veorq_u64(acc_vec, shifted);

                uint64x2_t key_vec = XXH_vld1q_u64(xsecret + (i * 16));
                uint64x2_t data_key = veorq_u64(data_vec, key_vec);
#ifdef __wasm_simd128__
                xacc[i] = data_key * XXH_PRIME32_1;
#else
                uint32x4_t prod_hi = vmulq_u32(vreinterpretq_u32_u64(data_key), kPrimeHi);
                uint32x2_t data_key_lo = vmovn_u64(data_key);
                xacc[i] = vmlal_u32(vreinterpretq_u64_u32(prod_hi), data_key_lo, kPrimeLo);
#endif
            }
        }
    }


    void XXHashEngineNeon::init_custom_secret(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        XXH3_initCustomSecret_scalar(customSecret, seed64);
    }

    void XXHashEngineNeon::accumulate(uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret, size_t nbStripes) {
        XXH3_accumulate_neon(acc, input, secret, nbStripes);
    }

    void XXHashEngineNeon::scramble_acc(void* KUMO_RESTRICT acc, const void* secret) {
        XXH3_scrambleAcc_neon(acc, secret);
    }

    void XXHashEngineNeon::accumulate_512(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        XXH3_accumulate_512_neon(acc, input, secret);
    }

    static XXHashEngine* get_xxhash_neon_instance() {
        static XXHashEngineNeon ins;
        return &ins;
    }
} // namespace turbo::xxhash
#else
namespace turbo::xxhash {
    static XXHashEngine* get_xxhash_neon_instance() {
        return nullptr;
    }
} // namespace turbo::xxhash
#endif

namespace turbo::xxhash {
    IsaInfo get_xxhash_neon_info() {
        static IsaInfo ins = {
            KUMO_SIMD_NEON == 1,
            false,
            static_cast<uint32_t>(InstructionSet::NEON),
            "neon",
            get_xxhash_neon_instance(),
        };
        return ins;
    }
} // namespace turbo::xxhash
