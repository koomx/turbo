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

#include <algorithm>
#include <turbo/hash/xx/common.h>
#include <turbo/hash/xx/scalar.h>

namespace turbo::xxhash {

    class XXHashEngine {
    protected:
        virtual void init_custom_secret(void* KUMO_RESTRICT customSecret, uint64_t seed64) = 0;

        virtual void accumulate(uint64_t* KUMO_RESTRICT, const uint8_t* KUMO_RESTRICT, const uint8_t* KUMO_RESTRICT, size_t) = 0;

        virtual  void scramble_acc(void* KUMO_RESTRICT, const void*) = 0;

        virtual  void accumulate_512(void* KUMO_RESTRICT acc,const void* KUMO_RESTRICT input,const void* KUMO_RESTRICT secret) = 0;
    public:

        const uint8_t* consume_stripes(uint64_t* KUMO_RESTRICT acc,
            size_t* KUMO_RESTRICT nbStripesSoFarPtr, size_t nbStripesPerBlock,
            const uint8_t* KUMO_RESTRICT input, size_t nbStripes,
            const uint8_t* KUMO_RESTRICT secret, size_t secretLimit);

        void internal_64bits_reset_with_seed(KUMO_ATTRIBUTE_NOESCAPE XxHashStateCore* statePtr, uint64_t seed);

        void internal_64bits_reset_with_secret(KUMO_ATTRIBUTE_NOESCAPE XxHashStateCore* statePtr,
                KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize);

        void internal_update(XxHashStateCore* KUMO_RESTRICT const state,const uint8_t* KUMO_RESTRICT input, size_t len);

        void internal_digest_long(uint64_t* acc, const XxHashStateCore* state, const unsigned char* secret);

        uint64_t xxhash_64bits_with_secret(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input,
            size_t len, KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize) {
            if (len <= kXxh3MidsizeMax)
                return xxhash_64bits_short(input, len, 0, secret, secretSize);
            return xxhash_hashLong_64b_internal(input, len, secret, secretSize);
        }

        KUMO_FORCE_INLINE uint64_t xxhash_64bits_with_seed(const void* KUMO_RESTRICT input, size_t len, uint64_t seed64) {
            if (len <= kXxh3MidsizeMax)
                return xxhash_64bits_short(input, len, seed64, turbo::xxhash::kXxhSecret, sizeof(turbo::xxhash::kXxhSecret));
            if (seed64 == 0)
                return xxhash_hashLong_64b_internal(input, len,
                    turbo::xxhash::kXxhSecret, sizeof(turbo::xxhash::kXxhSecret));
            {
                alignas(KUMO_CACHELINE_SIZE) uint8_t secret[kXxhSecretDefaultSize];
                init_custom_secret(secret, seed64);
                return xxhash_hashLong_64b_internal(input, len, (const uint8_t*)secret, sizeof(secret));
            }
        }

        XxHash128 xxhash_128bits_with_secret(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input,
            size_t len, KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize) {
            if (len <= kXxh3MidsizeMax)
                return xxhash_128bits_short(input, len, 0, secret, secretSize);
            return xxhash_hashLong_128b_internal(input, len, secret, secretSize);
        }

        KUMO_FORCE_INLINE XxHash128 xxhash_128bits_with_seed(const void* KUMO_RESTRICT input, size_t len, uint64_t seed64) {
            if (len <= kXxh3MidsizeMax)
                return xxhash_128bits_short(input, len, seed64, turbo::xxhash::kXxhSecret, sizeof(turbo::xxhash::kXxhSecret));
            if (seed64 == 0)
                return xxhash_hashLong_128b_internal(input, len,
                    turbo::xxhash::kXxhSecret, sizeof(turbo::xxhash::kXxhSecret));
            {
                alignas(KUMO_CACHELINE_SIZE) uint8_t secret[kXxhSecretDefaultSize];
                init_custom_secret(secret, seed64);
                return xxhash_hashLong_128b_internal(input, len, (const uint8_t*)secret, sizeof(secret));
            }
        }

        KUMO_FORCE_INLINE XxHash128 xxhash_128bits_with_secret_and_seed(
            KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t len,
            KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize, uint64_t seed) {
            if (len <= kXxh3MidsizeMax)
                return xxhash_128bits_short(input, len, seed, turbo::xxhash::kXxhSecret, sizeof(turbo::xxhash::kXxhSecret));
            return xxhash_hashLong_128b_internal(input, len, secret, secretSize);
        }

        void xxhash_generate_secret(uint8_t* secretBuffer, size_t secretSize,
            const uint8_t* customSeed, size_t customSeedSize) {
            KUMO_DASSERT(secretBuffer != nullptr);
            KUMO_DASSERT(secretSize >= kXxh3SecretSizeMin);
            if (customSeedSize == 0) {
                customSeed = turbo::xxhash::kXxhSecret;
                customSeedSize = kXxhSecretDefaultSize;
            }
            KUMO_DASSERT(customSeed != nullptr);
            {
                size_t pos = 0;
                while (pos < secretSize) {
                    size_t const toCopy = std::min((secretSize - pos), customSeedSize);
                    memcpy(reinterpret_cast<char*>(secretBuffer) + pos, customSeed, toCopy);
                    pos += toCopy;
                }
            }
            {
                size_t const nbSeg16 = secretSize / 16;
                std::array<uint8_t, 16> scrambler;
                xxhash_128bits_with_seed(customSeed, customSeedSize, 0).write(scrambler);
                for (size_t n = 0; n < nbSeg16; n++) {
                    XxHash128 const h128 = xxhash_128bits_with_seed(scrambler.data(), scrambler.size(), n);
                    turbo::xxhash::xxhash_combine16_add64(reinterpret_cast<char*>(secretBuffer) + n * 16, h128);
                }
                turbo::xxhash::xxhash_combine16_add64(reinterpret_cast<char*>(secretBuffer) + secretSize - 16, XxHash128::read(scrambler));
            }
        }

        void xxhash_generate_secret_from_seed(uint8_t* secretBuffer, uint64_t seed) {
            alignas(KUMO_CACHELINE_SIZE) uint8_t secret[kXxhSecretDefaultSize];
            init_custom_secret(secret, seed);
            KUMO_DASSERT(secretBuffer != nullptr);
            memcpy(secretBuffer, secret, kXxhSecretDefaultSize);
        }
    private:

        KUMO_FORCE_INLINE uint64_t xxhash_64bits_short(const void* input, size_t len,
            uint64_t seed64, const uint8_t* secret, size_t secretLen) {
            KUMO_DASSERT(secretLen >= kXxh3SecretSizeMin);
            if (len <= 16)
                return xxhash_len_0to16_64b((const uint8_t*)input, len, secret, seed64);
            if (len <= 128)
                return xxhash_len_17to128_64b((const uint8_t*)input, len, secret, secretLen, seed64);
            return xxhash_len_129to240_64b((const uint8_t*)input, len, secret, secretLen, seed64);
        }

        KUMO_FORCE_INLINE XxHash128 xxhash_128bits_short(const void* input, size_t len,
            uint64_t seed64, const uint8_t* secret, size_t secretLen) {
            KUMO_DASSERT(secretLen >= kXxh3SecretSizeMin);
            if (len <= 16)
                return xxhash_len_0to16_128b((const uint8_t*)input, len, secret, seed64);
            if (len <= 128)
                return xxhash_len_17to128_128b((const uint8_t*)input, len, secret, secretLen, seed64);
            return xxhash_len_129to240_128b((const uint8_t*)input, len, secret, secretLen, seed64);
        }


        KUMO_FORCE_INLINE void
        xxhash_hash_long_internal_loop(uint64_t* KUMO_RESTRICT acc,
            const uint8_t* KUMO_RESTRICT input, size_t len,
            const uint8_t* KUMO_RESTRICT secret, size_t secretSize) {
            size_t const nbStripesPerBlock = (secretSize - kXxhStripeLen) / kXxhSecretConsumeRate;
            size_t const block_len = kXxhStripeLen * nbStripesPerBlock;
            size_t const nb_blocks = (len - 1) / block_len;

            size_t n;

            KUMO_DASSERT(secretSize >= kXxh3SecretSizeMin);

            for (n = 0; n < nb_blocks; n++) {
                accumulate(acc, input + n * block_len, secret, nbStripesPerBlock);
                scramble_acc(acc, secret + secretSize - kXxhStripeLen);
            }

            /* last partial block */
            KUMO_DASSERT(len > kXxhStripeLen);
            {
                size_t const nbStripes = ((len - 1) - (block_len * nb_blocks)) / kXxhStripeLen;
                KUMO_DASSERT(nbStripes <= (secretSize / kXxhSecretConsumeRate));
                accumulate(acc, input + nb_blocks * block_len, secret, nbStripes);

                /* last stripe */
                {
                    const uint8_t* const p = input + len - kXxhStripeLen;
                    accumulate_512(acc, p, secret + secretSize - kXxhStripeLen - kXxhashSecretLastAccStart);
                }
            }
        }

        KUMO_FORCE_INLINE uint64_t xxhash_hashLong_64b_internal(const void* KUMO_RESTRICT input, size_t len,
            const uint8_t* KUMO_RESTRICT secret, size_t secretSize) {
            alignas(KUMO_CACHELINE_SIZE) auto acc = kXxhashAcc;

            xxhash_hash_long_internal_loop(acc.data(), (const uint8_t*)input, len, secret, secretSize);

            static_assert(sizeof(acc) == 64, "sizeof(acc) == 64");
            KUMO_DASSERT(secretSize >= sizeof(acc) + kXxhSecretMergeAccsStart);
            return turbo::xxhash::xxhash_finalize_long_64b(acc.data(), secret, (uint64_t)len);
        }

        KUMO_FORCE_INLINE XxHash128 xxhash_hashLong_128b_internal(const void* KUMO_RESTRICT input, size_t len,
            const uint8_t* KUMO_RESTRICT secret, size_t secretSize) {
            alignas(KUMO_CACHELINE_SIZE) auto acc = kXxhashAcc;

            xxhash_hash_long_internal_loop(acc.data(), (const uint8_t*)input, len, secret, secretSize);

            /* converge into final hash */
            static_assert(sizeof(acc) == 64, "sizeof(acc) == 64");
            KUMO_DASSERT(secretSize >= sizeof(acc) + kXxhSecretMergeAccsStart);
            return turbo::xxhash::xxhash_finalize_long_128b(acc.data(), secret, secretSize, (uint64_t)len);
        }

    };
} // namespace turbo::xxhash
