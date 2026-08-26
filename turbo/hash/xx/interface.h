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
#include <turbo/hash/xx/scalar.h>

namespace turbo::xxhash {

    class XXHashEngine {
    public:
        virtual void init_custom_secret(void* KUMO_RESTRICT customSecret, uint64_t seed64) = 0;

        virtual void accumulate(uint64_t* KUMO_RESTRICT, const uint8_t* KUMO_RESTRICT, const uint8_t* KUMO_RESTRICT, size_t) = 0;

        virtual  void scramble_acc(void* KUMO_RESTRICT, const void*) = 0;

        virtual  void accumulate_512(void* KUMO_RESTRICT acc,const void* KUMO_RESTRICT input,const void* KUMO_RESTRICT secret) = 0;
    public:

        uint64_t xxhash_64bits_with_secret(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input,
            size_t len, KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize) {
            if (len <= XXH3_MIDSIZE_MAX)
                return XXH3_64bits_short(input, len, 0, secret, secretSize);
            return XXH3_hashLong_64b_internal(input, len, secret, secretSize);
        }

        KUMO_FORCE_INLINE uint64_t xxhash_64bits_with_seed(const void* KUMO_RESTRICT input, size_t len, uint64_t seed64) {
            if (len <= XXH3_MIDSIZE_MAX)
                return XXH3_64bits_short(input, len, seed64, turbo::xxhash::XXH3_kSecret, sizeof(turbo::xxhash::XXH3_kSecret));
            if (seed64 == 0)
                return XXH3_hashLong_64b_internal(input, len,
                    turbo::xxhash::XXH3_kSecret, sizeof(turbo::xxhash::XXH3_kSecret));
            {
                alignas(XXH_SEC_ALIGN) uint8_t secret[XXH_SECRET_DEFAULT_SIZE];
                init_custom_secret(secret, seed64);
                return XXH3_hashLong_64b_internal(input, len, (const uint8_t*)secret, sizeof(secret));
            }
        }

        turbo::xxhash::XXH128_hash_t xxhash_128bits_with_secret(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input,
            size_t len, KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize) {
            if (len <= XXH3_MIDSIZE_MAX)
                return XXH3_128bits_short(input, len, 0, secret, secretSize);
            return XXH3_hashLong_128b_internal(input, len, secret, secretSize);
        }

        KUMO_FORCE_INLINE turbo::xxhash::XXH128_hash_t xxhash_128bits_with_seed(const void* KUMO_RESTRICT input, size_t len, uint64_t seed64) {
            if (len <= XXH3_MIDSIZE_MAX)
                return XXH3_128bits_short(input, len, seed64, turbo::xxhash::XXH3_kSecret, sizeof(turbo::xxhash::XXH3_kSecret));
            if (seed64 == 0)
                return XXH3_hashLong_128b_internal(input, len,
                    turbo::xxhash::XXH3_kSecret, sizeof(turbo::xxhash::XXH3_kSecret));
            {
                alignas(XXH_SEC_ALIGN) uint8_t secret[XXH_SECRET_DEFAULT_SIZE];
                init_custom_secret(secret, seed64);
                return XXH3_hashLong_128b_internal(input, len, (const uint8_t*)secret, sizeof(secret));
            }
        }

        KUMO_FORCE_INLINE turbo::xxhash::XXH128_hash_t xxhash_128bits_with_secret_and_seed(
            KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t len,
            KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize, uint64_t seed) {
            if (len <= XXH3_MIDSIZE_MAX)
                return XXH3_128bits_short(input, len, seed, turbo::xxhash::XXH3_kSecret, sizeof(turbo::xxhash::XXH3_kSecret));
            return XXH3_hashLong_128b_internal(input, len, secret, secretSize);
        }
    private:

        KUMO_FORCE_INLINE uint64_t XXH3_64bits_short(const void* input, size_t len,
            uint64_t seed64, const uint8_t* secret, size_t secretLen) {
            KUMO_DASSERT(secretLen >= XXH3_SECRET_SIZE_MIN);
            if (len <= 16)
                return XXH3_len_0to16_64b((const uint8_t*)input, len, secret, seed64);
            if (len <= 128)
                return XXH3_len_17to128_64b((const uint8_t*)input, len, secret, secretLen, seed64);
            return XXH3_len_129to240_64b((const uint8_t*)input, len, secret, secretLen, seed64);
        }

        KUMO_FORCE_INLINE turbo::xxhash::XXH128_hash_t XXH3_128bits_short(const void* input, size_t len,
            uint64_t seed64, const uint8_t* secret, size_t secretLen) {
            KUMO_DASSERT(secretLen >= XXH3_SECRET_SIZE_MIN);
            if (len <= 16)
                return XXH3_len_0to16_128b((const uint8_t*)input, len, secret, seed64);
            if (len <= 128)
                return XXH3_len_17to128_128b((const uint8_t*)input, len, secret, secretLen, seed64);
            return XXH3_len_129to240_128b((const uint8_t*)input, len, secret, secretLen, seed64);
        }


        KUMO_FORCE_INLINE void
        XXH3_hashLong_internal_loop(uint64_t* KUMO_RESTRICT acc,
            const uint8_t* KUMO_RESTRICT input, size_t len,
            const uint8_t* KUMO_RESTRICT secret, size_t secretSize) {
            size_t const nbStripesPerBlock = (secretSize - XXH_STRIPE_LEN) / XXH_SECRET_CONSUME_RATE;
            size_t const block_len = XXH_STRIPE_LEN * nbStripesPerBlock;
            size_t const nb_blocks = (len - 1) / block_len;

            size_t n;

            KUMO_DASSERT(secretSize >= XXH3_SECRET_SIZE_MIN);

            for (n = 0; n < nb_blocks; n++) {
                accumulate(acc, input + n * block_len, secret, nbStripesPerBlock);
                scramble_acc(acc, secret + secretSize - XXH_STRIPE_LEN);
            }

            /* last partial block */
            KUMO_DASSERT(len > XXH_STRIPE_LEN);
            {
                size_t const nbStripes = ((len - 1) - (block_len * nb_blocks)) / XXH_STRIPE_LEN;
                KUMO_DASSERT(nbStripes <= (secretSize / XXH_SECRET_CONSUME_RATE));
                accumulate(acc, input + nb_blocks * block_len, secret, nbStripes);

                /* last stripe */
                {
                    const uint8_t* const p = input + len - XXH_STRIPE_LEN;
#define XXH_SECRET_LASTACC_START 7 /* not aligned on 8, last secret is different from acc & scrambler */
                    accumulate_512(acc, p, secret + secretSize - XXH_STRIPE_LEN - XXH_SECRET_LASTACC_START);
                }
            }
        }

        KUMO_FORCE_INLINE uint64_t XXH3_hashLong_64b_internal(const void* KUMO_RESTRICT input, size_t len,
            const uint8_t* KUMO_RESTRICT secret, size_t secretSize) {
            alignas(XXH_ACC_ALIGN) uint64_t acc[XXH_ACC_NB] = XXH3_INIT_ACC;

            XXH3_hashLong_internal_loop(acc, (const uint8_t*)input, len, secret, secretSize);

            static_assert(sizeof(acc) == 64, "sizeof(acc) == 64");
            KUMO_DASSERT(secretSize >= sizeof(acc) + XXH_SECRET_MERGEACCS_START);
            return turbo::xxhash::XXH3_finalizeLong_64b(acc, secret, (uint64_t)len);
        }

        KUMO_FORCE_INLINE turbo::xxhash::XXH128_hash_t XXH3_hashLong_128b_internal(const void* KUMO_RESTRICT input, size_t len,
            const uint8_t* KUMO_RESTRICT secret, size_t secretSize) {
            alignas(XXH_ACC_ALIGN) uint64_t acc[XXH_ACC_NB] = XXH3_INIT_ACC;

            XXH3_hashLong_internal_loop(acc, (const uint8_t*)input, len, secret, secretSize);

            /* converge into final hash */
            static_assert(sizeof(acc) == 64, "sizeof(acc) == 64");
            KUMO_DASSERT(secretSize >= sizeof(acc) + XXH_SECRET_MERGEACCS_START);
            return turbo::xxhash::XXH3_finalizeLong_128b(acc, secret, secretSize, (uint64_t)len);
        }

    };
} // namespace turbo::xxhash
