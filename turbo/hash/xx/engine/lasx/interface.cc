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

#include <turbo/hash/xx/engine/lasx/interface.h>

#if KUMO_SIMD_LASX
#include <lasxintrin.h>

#define _LASX_SHUFFLE(z, y, x, w) (((z) << 6) | ((y) << 4) | ((x) << 2) | (w))

namespace turbo::xxhash {

    KUMO_FORCE_INLINE void xxhash_accumulate_512_lasx(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        KUMO_DASSERT((((size_t)acc) & 31) == 0);
        {
            size_t i;
            __m256i* const xacc = (__m256i*)acc;
            const __m256i* const xinput = (const __m256i*)input;
            const __m256i* const xsecret = (const __m256i*)secret;

            for (i = 0; i < kXxhStripeLen / sizeof(__m256i); i++) {
                __m256i const data_vec = __lasx_xvld(xinput + i, 0);
                __m256i const key_vec = __lasx_xvld(xsecret + i, 0);
                __m256i const data_key = __lasx_xvxor_v(data_vec, key_vec);
                __m256i const data_key_lo = __lasx_xvsrli_d(data_key, 32);
                __m256i const product = __lasx_xvmulwev_d_wu(data_key, data_key_lo);
                __m256i const data_swap = __lasx_xvshuf4i_w(data_vec, _LASX_SHUFFLE(1, 0, 3, 2));
                __m256i const sum = __lasx_xvadd_d(xacc[i], data_swap);
                xacc[i] = __lasx_xvadd_d(product, sum);
            }
        }
    }

    KUMO_FORCE_INLINE void xxhash_accumulate_lasx(uint64_t* KUMO_RESTRICT acc,
        const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret,
        size_t nbStripes) {
        size_t n;
        for (n = 0; n < nbStripes; n++) {
            const uint8_t* const in = input + n * kXxhStripeLen;
            prefetch_to_local_cache(in + kDefaultXxhPrefetchDist);
            xxhash_accumulate_512_lasx(acc, in, secret + n * kXxhSecretConsumeRate);
        }
    }

    KUMO_FORCE_INLINE void xxhash_scramble_acc_lasx(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT secret) {
        KUMO_DASSERT((((size_t)acc) & 31) == 0);
        {
            __m256i* const xacc = (__m256i*)acc;
            const __m256i* const xsecret = (const __m256i*)secret;
            const __m256i prime32 = __lasx_xvreplgr2vr_d(kXxhPrime32_1);
            size_t i;

            for (i = 0; i < kXxhStripeLen / sizeof(__m256i); i++) {
                __m256i const acc_vec = xacc[i];
                __m256i const shifted = __lasx_xvsrli_d(acc_vec, 47);
                __m256i const data_vec = __lasx_xvxor_v(acc_vec, shifted);
                __m256i const key_vec = __lasx_xvld(xsecret + i, 0);
                __m256i const data_key = __lasx_xvxor_v(data_vec, key_vec);
                xacc[i] = __lasx_xvmul_d(data_key, prime32);
            }
        }
    }

    void XXHashEngineLasx::init_custom_secret(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        xxhash_init_custom_secret_scalar(customSecret, seed64);
    }

    void XXHashEngineLasx::accumulate(uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret, size_t nbStripes) {
        xxhash_accumulate_lasx(acc, input, secret, nbStripes);
    }

    void XXHashEngineLasx::scramble_acc(void* KUMO_RESTRICT acc, const void* secret) {
        xxhash_scramble_acc_lasx(acc, secret);
    }

    void XXHashEngineLasx::accumulate_512(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        xxhash_accumulate_512_lasx(acc, input, secret);
    }

    static XXHashEngine* get_xxhash_lasx_instance() {
        static XXHashEngineLasx ins;
        return &ins;
    }
} // namespace turbo::xxhash
#else
namespace turbo::xxhash {
    static XXHashEngine* get_xxhash_lasx_instance() {
        return nullptr;
    }
} // namespace turbo::xxhash
#endif

namespace turbo::xxhash {
    IsaInfo get_xxhash_lasx_info() {
        static IsaInfo ins = {
            KUMO_SIMD_LASX == 1,
            false,
            { kLoongLsx, kLoongLasx },
            "lasx",
            get_xxhash_lasx_instance(),
        };
        return ins;
    }
} // namespace turbo::xxhash
