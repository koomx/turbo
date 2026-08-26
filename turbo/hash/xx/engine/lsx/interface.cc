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

#include <turbo/hash/xx/engine/lsx/interface.h>

#if KUMO_SIMD_LSX
#include <lsxintrin.h>

#define _LSX_SHUFFLE(z, y, x, w) (((z) << 6) | ((y) << 4) | ((x) << 2) | (w))

namespace turbo::xxhash {

    KUMO_FORCE_INLINE void XXH3_accumulate_512_lsx(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        KUMO_DASSERT((((size_t)acc) & 15) == 0);
        {
            __m128i* const xacc = (__m128i*)acc;
            const __m128i* const xinput = (const __m128i*)input;
            const __m128i* const xsecret = (const __m128i*)secret;
            size_t i;

            for (i = 0; i < XXH_STRIPE_LEN / sizeof(__m128i); i++) {
                __m128i const data_vec = __lsx_vld(xinput + i, 0);
                __m128i const key_vec = __lsx_vld(xsecret + i, 0);
                __m128i const data_key = __lsx_vxor_v(data_vec, key_vec);
                __m128i const data_key_lo = __lsx_vsrli_d(data_key, 32);
                __m128i const product = __lsx_vmulwev_d_wu(data_key, data_key_lo);
                __m128i const data_swap = __lsx_vshuf4i_w(data_vec, _LSX_SHUFFLE(1, 0, 3, 2));
                __m128i const sum = __lsx_vadd_d(xacc[i], data_swap);
                xacc[i] = __lsx_vadd_d(product, sum);
            }
        }
    }

    KUMO_FORCE_INLINE void XXH3_accumulate_lsx(uint64_t* KUMO_RESTRICT acc,
        const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret,
        size_t nbStripes) {
        size_t n;
        for (n = 0; n < nbStripes; n++) {
            const uint8_t* const in = input + n * XXH_STRIPE_LEN;
            XXH_PREFETCH(in + XXH_PREFETCH_DIST);
            XXH3_accumulate_512_lsx(acc, in, secret + n * XXH_SECRET_CONSUME_RATE);
        }
    }

    KUMO_FORCE_INLINE void XXH3_scrambleAcc_lsx(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT secret) {
        KUMO_DASSERT((((size_t)acc) & 15) == 0);
        {
            __m128i* const xacc = (__m128i*)acc;
            const __m128i* const xsecret = (const __m128i*)secret;
            const __m128i prime32 = __lsx_vreplgr2vr_d(XXH_PRIME32_1);
            size_t i;

            for (i = 0; i < XXH_STRIPE_LEN / sizeof(__m128i); i++) {
                __m128i const acc_vec = xacc[i];
                __m128i const shifted = __lsx_vsrli_d(acc_vec, 47);
                __m128i const data_vec = __lsx_vxor_v(acc_vec, shifted);
                __m128i const key_vec = __lsx_vld(xsecret + i, 0);
                __m128i const data_key = __lsx_vxor_v(data_vec, key_vec);
                xacc[i] = __lsx_vmul_d(data_key, prime32);
            }
        }
    }

    void XXHashEngineLsx::init_custom_secret(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        XXH3_initCustomSecret_scalar(customSecret, seed64);
    }

    void XXHashEngineLsx::accumulate(uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret, size_t nbStripes) {
        XXH3_accumulate_lsx(acc, input, secret, nbStripes);
    }

    void XXHashEngineLsx::scramble_acc(void* KUMO_RESTRICT acc, const void* secret) {
        XXH3_scrambleAcc_lsx(acc, secret);
    }

    void XXHashEngineLsx::accumulate_512(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        XXH3_accumulate_512_lsx(acc, input, secret);
    }

    static XXHashEngine* get_xxhash_lsx_instance() {
        static XXHashEngineLsx ins;
        return &ins;
    }
} // namespace turbo::xxhash
#else
namespace turbo::xxhash {
    static XXHashEngine* get_xxhash_lsx_instance() {
        return nullptr;
    }
} // namespace turbo::xxhash
#endif

namespace turbo::xxhash {
    IsaInfo get_xxhash_lsx_info() {
        static IsaInfo ins = {
            KUMO_SIMD_LSX == 1,
            false,
            static_cast<uint32_t>(InstructionSet::LSX),
            "lsx",
            get_xxhash_lsx_instance(),
        };
        return ins;
    }
} // namespace turbo::xxhash
