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

#include <cstddef>
#include <turbo/hash/xx/isa_select.h>
#include <turbo/hash/xx/xxh3.h>

namespace turbo {

    void XXH128_canonicalFromHash(KUMO_ATTRIBUTE_NOESCAPE turbo::xxhash::XXH128_canonical_t* dst, turbo::xxhash::XXH128_hash_t hash) {
        static_assert(sizeof(turbo::xxhash::XXH128_canonical_t) == sizeof(turbo::xxhash::XXH128_hash_t), "sizeof(turbo::xxhash::XXH128_canonical_t) == sizeof(XXH128_hash_t)");
        if constexpr (KUMO_ENDIAN_LITTLE) {
            hash.high64 = turbo::byteswap(hash.high64);
            hash.low64 = turbo::byteswap(hash.low64);
        }
        memcpy(dst, &hash.high64, sizeof(hash.high64));
        memcpy((char*)dst + sizeof(hash.high64), &hash.low64, sizeof(hash.low64));
    }

    turbo::xxhash::XXH128_hash_t XXH128_hashFromCanonical(KUMO_ATTRIBUTE_NOESCAPE const turbo::xxhash::XXH128_canonical_t* src) {
        turbo::xxhash::XXH128_hash_t h;
        h.high64 = turbo::big_endian::Load64(src);
        h.low64 = turbo::big_endian::Load64(src->digest + 8);
        return h;
    }

    KUMO_DLL int XXH128_cmp(KUMO_ATTRIBUTE_NOESCAPE const void* h128_1, KUMO_ATTRIBUTE_NOESCAPE const void* h128_2) {
        turbo::xxhash::XXH128_hash_t const h1 = *(const turbo::xxhash::XXH128_hash_t*)h128_1;
        turbo::xxhash::XXH128_hash_t const h2 = *(const turbo::xxhash::XXH128_hash_t*)h128_2;
        int const hcmp = (h1.high64 > h2.high64) - (h2.high64 > h1.high64);
        /* note : bets that, in most cases, hash values are different */
        if (hcmp)
            return hcmp;
        return (h1.low64 > h2.low64) - (h2.low64 > h1.low64);
    }

    int XXH128_isEqual(turbo::xxhash::XXH128_hash_t h1, turbo::xxhash::XXH128_hash_t h2) {
        /* note : XXH128_hash_t is compact, it has no padding byte */
        return !(memcmp(&h1, &h2, sizeof(h1)));
    }

    KUMO_DLL xxhash::XXH_errorcode
    XXH3_generateSecret(KUMO_ATTRIBUTE_NOESCAPE uint8_t* secretBuffer, size_t secretSize, KUMO_ATTRIBUTE_NOESCAPE const uint8_t* customSeed, size_t customSeedSize) {
#if (XXH_DEBUGLEVEL >= 1)
        KUMO_DASSERT(secretBuffer != NULL);
        KUMO_DASSERT(secretSize >= XXH3_SECRET_SIZE_MIN);
#else
        /* production mode, assert() are disabled */
        if (secretBuffer == nullptr)
            return xxhash::XXH_ERROR;
        if (secretSize < XXH3_SECRET_SIZE_MIN)
            return xxhash::XXH_ERROR;
#endif

        if (customSeedSize == 0) {
            customSeed = turbo::xxhash::XXH3_kSecret;
            customSeedSize = XXH_SECRET_DEFAULT_SIZE;
        }
#if (XXH_DEBUGLEVEL >= 1)
        KUMO_DASSERT(customSeed != NULL);
#else
        if (customSeed == nullptr)
            return xxhash::XXH_ERROR;
#endif

        /* Fill secretBuffer with a copy of customSeed - repeat as needed */
        {
            size_t pos = 0;
            while (pos < secretSize) {
                size_t const toCopy = std::min((secretSize - pos), customSeedSize);
                memcpy((char*)secretBuffer + pos, customSeed, toCopy);
                pos += toCopy;
            }
        }

        {
            size_t const nbSeg16 = secretSize / 16;
            size_t n;
            xxhash::XXH128_canonical_t scrambler;
            XXH128_canonicalFromHash(&scrambler, xxhash128(customSeed, customSeedSize, 0));
            for (n = 0; n < nbSeg16; n++) {
                turbo::xxhash::XXH128_hash_t const h128 = xxhash128(reinterpret_cast<const uint8_t*>(&scrambler), sizeof(scrambler), n);
                turbo::xxhash::XXH3_combine16((char*)secretBuffer + n * 16, h128);
            }
            /* last segment */
            turbo::xxhash::XXH3_combine16((char*)secretBuffer + secretSize - 16, XXH128_hashFromCanonical(&scrambler));
        }
        return xxhash::XXH_OK;
    }

    void XXH3_generateSecret_fromSeed(KUMO_ATTRIBUTE_NOESCAPE uint8_t* secretBuffer, uint64_t seed) {
        alignas(XXH_SEC_ALIGN) uint8_t secret[XXH_SECRET_DEFAULT_SIZE];
        XXHashRegistry::get_best_isa()->init_custom_secret(secret, seed);
        KUMO_DASSERT(secretBuffer != NULL);
        memcpy(secretBuffer, secret, XXH_SECRET_DEFAULT_SIZE);
    }

    KUMO_DLL uint64_t xxhash64(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* data, size_t len) {
        return xxhash_64bits_with_seed(data, len, 0);
    }

    KUMO_DLL uint64_t xxhash_64bits_with_seed(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t len, uint64_t seed) {
        return XXHashRegistry::get_best_isa()->xxhash_64bits_with_seed(input, len, seed);
    }

    KUMO_DLL uint64_t xxhash_64bits_with_secret(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input,
        size_t len, KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize) {
        return XXHashRegistry::get_best_isa()->xxhash_64bits_with_secret(input, len, secret, secretSize);
    }

    KUMO_DLL turbo::xxhash::XXH128_hash_t xxhash128(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* data, size_t len, uint64_t seed) {
        return xxhash_128bits_with_seed(data,len,seed);
    }

    KUMO_DLL turbo::xxhash::XXH128_hash_t xxhash_128bits_with_seed(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t len, uint64_t seed) {
        KUMO_DASSERT(secretLen >= XXH3_SECRET_SIZE_MIN);
        return XXHashRegistry::get_best_isa()->xxhash_128bits_with_seed(input,len,seed);
        //     return XXH3_128bits_internal(input, len, seed,
        //     turbo::xxhash::XXH3_kSecret, sizeof(turbo::xxhash::XXH3_kSecret),
        //     XXH3_hashLong_128b_withSeed);
    }

    KUMO_DLL turbo::xxhash::XXH128_hash_t xxhash_128bits_with_secret(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input,
       size_t len, KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize) {
        return XXHashRegistry::get_best_isa()->xxhash_128bits_with_secret(input,len,secret,secretSize);
        //return XXH3_128bits_internal(input, len, 0,
        //    (const uint8_t*)secret, secretSize,
        //    XXH3_hashLong_128b_withSecret);
    }

    KUMO_DLL turbo::xxhash::XXH128_hash_t xxhash_128bits_with_secret_and_seed(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t len, KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize, uint64_t seed) {
        return XXHashRegistry::get_best_isa()->xxhash_128bits_with_secret_and_seed(input,len,secret,secretSize,seed);
    }

#ifndef XXH3_STREAM_USE_STACK
#if XXH_SIZE_OPT <= 0 && !defined(__clang__)
#define XXH3_STREAM_USE_STACK 1
#endif
#endif

    static void XXH3_reset_internal(XXH3_state_t* statePtr, uint64_t seed, const void* secret, size_t secretSize) {
        size_t const initStart = offsetof(XXH3_state_t, bufferedSize);
        size_t const initLength = offsetof(XXH3_state_t, nbStripesPerBlock) - initStart;
        KUMO_DASSERT(offsetof(XXH3_state_t, nbStripesPerBlock) > initStart);
        KUMO_DASSERT(statePtr != NULL);
        memset((char*)statePtr + initStart, 0, initLength);
        statePtr->acc[0] = XXH_PRIME32_3;
        statePtr->acc[1] = XXH_PRIME64_1;
        statePtr->acc[2] = XXH_PRIME64_2;
        statePtr->acc[3] = XXH_PRIME64_3;
        statePtr->acc[4] = XXH_PRIME64_4;
        statePtr->acc[5] = XXH_PRIME32_2;
        statePtr->acc[6] = XXH_PRIME64_5;
        statePtr->acc[7] = XXH_PRIME32_1;
        statePtr->seed = seed;
        statePtr->useSeed = (seed != 0);
        statePtr->extSecret = (const unsigned char*)secret;
        KUMO_DASSERT(secretSize >= XXH3_SECRET_SIZE_MIN);
        statePtr->secretLimit = secretSize - XXH_STRIPE_LEN;
        statePtr->nbStripesPerBlock = statePtr->secretLimit / XXH_SECRET_CONSUME_RATE;
    }

    static const uint8_t* XXH3_consumeStripes(uint64_t* KUMO_RESTRICT acc,
        size_t* KUMO_RESTRICT nbStripesSoFarPtr, size_t nbStripesPerBlock,
        const uint8_t* KUMO_RESTRICT input, size_t nbStripes,
        const uint8_t* KUMO_RESTRICT secret, size_t secretLimit) {
        turbo::xxhash::XXHashEngine* engine = XXHashRegistry::get_best_isa();
        const uint8_t* initialSecret = secret + *nbStripesSoFarPtr * XXH_SECRET_CONSUME_RATE;
        if (nbStripes >= (nbStripesPerBlock - *nbStripesSoFarPtr)) {
            size_t nbStripesThisIter = nbStripesPerBlock - *nbStripesSoFarPtr;
            do {
                engine->accumulate(acc, input, initialSecret, nbStripesThisIter);
                engine->scramble_acc(acc, secret + secretLimit);
                input += nbStripesThisIter * XXH_STRIPE_LEN;
                nbStripes -= nbStripesThisIter;
                nbStripesThisIter = nbStripesPerBlock;
                initialSecret = secret;
            } while (nbStripes >= nbStripesPerBlock);
            *nbStripesSoFarPtr = 0;
        }
        if (nbStripes > 0) {
            engine->accumulate(acc, input, initialSecret, nbStripes);
            input += nbStripes * XXH_STRIPE_LEN;
            *nbStripesSoFarPtr += nbStripes;
        }
        return input;
    }

    static xxhash::XXH_errorcode XXH3_update(XXH3_state_t* KUMO_RESTRICT const state,
        const uint8_t* KUMO_RESTRICT input, size_t len) {
        if (input == NULL) {
            KUMO_DASSERT(len == 0);
            return xxhash::XXH_OK;
        }

        KUMO_DASSERT(state != NULL);
        state->totalLen += len;

        KUMO_DASSERT(state->bufferedSize <= XXH3_INTERNALBUFFER_SIZE);
        if (len <= XXH3_INTERNALBUFFER_SIZE - state->bufferedSize) {
            memcpy(state->buffer + state->bufferedSize, input, len);
            state->bufferedSize += (uint32_t)len;
            return xxhash::XXH_OK;
        }

        {
            const uint8_t* const bEnd = input + len;
            const unsigned char* const secret = (state->extSecret == NULL) ? state->customSecret : state->extSecret;
#if defined(XXH3_STREAM_USE_STACK) && XXH3_STREAM_USE_STACK >= 1
            alignas(XXH_ACC_ALIGN) uint64_t acc[8];
            memcpy(acc, state->acc, sizeof(acc));
#else
            uint64_t* KUMO_RESTRICT const acc = state->acc;
#endif
            static_assert(XXH3_INTERNALBUFFER_SIZE % XXH_STRIPE_LEN == 0, "XXH3_INTERNALBUFFER_SIZE % XXH_STRIPE_LEN == 0");

            if (state->bufferedSize) {
                size_t const loadSize = XXH3_INTERNALBUFFER_SIZE - state->bufferedSize;
                memcpy(state->buffer + state->bufferedSize, input, loadSize);
                input += loadSize;
                XXH3_consumeStripes(acc,
                    &state->nbStripesSoFar, state->nbStripesPerBlock,
                    state->buffer, XXH3_INTERNALBUFFER_SIZE / XXH_STRIPE_LEN,
                    secret, state->secretLimit);
                state->bufferedSize = 0;
            }
            KUMO_DASSERT(input < bEnd);
            if (bEnd - input > XXH3_INTERNALBUFFER_SIZE) {
                size_t nbStripes = (size_t)(bEnd - 1 - input) / XXH_STRIPE_LEN;
                input = XXH3_consumeStripes(acc,
                    &state->nbStripesSoFar, state->nbStripesPerBlock,
                    input, nbStripes,
                    secret, state->secretLimit);
                memcpy(state->buffer + sizeof(state->buffer) - XXH_STRIPE_LEN, input - XXH_STRIPE_LEN, XXH_STRIPE_LEN);
            }
            KUMO_DASSERT(input < bEnd);
            KUMO_DASSERT(bEnd - input <= XXH3_INTERNALBUFFER_SIZE);
            KUMO_DASSERT(state->bufferedSize == 0);
            memcpy(state->buffer, input, (size_t)(bEnd - input));
            state->bufferedSize = (uint32_t)(bEnd - input);
#if defined(XXH3_STREAM_USE_STACK) && XXH3_STREAM_USE_STACK >= 1
            memcpy(state->acc, acc, sizeof(acc));
#endif
        }

        return xxhash::XXH_OK;
    }

    static void XXH3_digest_long(uint64_t* acc, const XXH3_state_t* state, const unsigned char* secret) {
        uint8_t lastStripe[XXH_STRIPE_LEN];
        const uint8_t* lastStripePtr;
        turbo::xxhash::XXHashEngine* engine = XXHashRegistry::get_best_isa();

        memcpy(acc, state->acc, sizeof(state->acc));
        if (state->bufferedSize >= XXH_STRIPE_LEN) {
            size_t const nbStripes = (state->bufferedSize - 1) / XXH_STRIPE_LEN;
            size_t nbStripesSoFar = state->nbStripesSoFar;
            XXH3_consumeStripes(acc,
                &nbStripesSoFar, state->nbStripesPerBlock,
                state->buffer, nbStripes,
                secret, state->secretLimit);
            lastStripePtr = state->buffer + state->bufferedSize - XXH_STRIPE_LEN;
        } else {
            size_t const catchupSize = XXH_STRIPE_LEN - state->bufferedSize;
            KUMO_DASSERT(state->bufferedSize > 0);
            memcpy(lastStripe, state->buffer + sizeof(state->buffer) - catchupSize, catchupSize);
            memcpy(lastStripe + catchupSize, state->buffer, state->bufferedSize);
            lastStripePtr = lastStripe;
        }
        engine->accumulate_512(acc, lastStripePtr, secret + state->secretLimit - 7);
    }

    KUMO_DLL xxhash::XXH_errorcode XXH3_64bits_reset_withSecret(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr,
        KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize) {
        if (statePtr == NULL)
            return xxhash::XXH_ERROR;
        XXH3_reset_internal(statePtr, 0, secret, secretSize);
        if (secret == NULL)
            return xxhash::XXH_ERROR;
        if (secretSize < XXH3_SECRET_SIZE_MIN)
            return xxhash::XXH_ERROR;
        return xxhash::XXH_OK;
    }

    KUMO_DLL xxhash::XXH_errorcode XXH3_64bits_reset_withSeed(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr, uint64_t seed) {
        if (statePtr == NULL)
            return xxhash::XXH_ERROR;
        if (seed == 0) {
            XXH3_reset_internal(statePtr, 0, turbo::xxhash::XXH3_kSecret, XXH_SECRET_DEFAULT_SIZE);
            return xxhash::XXH_OK;
        }
        if ((seed != statePtr->seed) || (statePtr->extSecret != NULL))
            XXHashRegistry::get_best_isa()->init_custom_secret(statePtr->customSecret, seed);
        XXH3_reset_internal(statePtr, seed, NULL, XXH_SECRET_DEFAULT_SIZE);
        return xxhash::XXH_OK;
    }

    KUMO_DLL xxhash::XXH_errorcode XXH3_64bits_update(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr,
        KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t length) {
        return XXH3_update(statePtr, (const uint8_t*)input, length);
    }

    KUMO_DLL uint64_t XXH3_64bits_digest(KUMO_ATTRIBUTE_NOESCAPE const XXH3_state_t* statePtr) {
        const unsigned char* const secret = (statePtr->extSecret == NULL) ? statePtr->customSecret : statePtr->extSecret;
        if (statePtr->totalLen > XXH3_MIDSIZE_MAX) {
            alignas(XXH_ACC_ALIGN) uint64_t acc[XXH_ACC_NB];
            XXH3_digest_long(acc, statePtr, secret);
            return turbo::xxhash::XXH3_finalizeLong_64b(acc, secret, (uint64_t)statePtr->totalLen);
        }
        if (statePtr->useSeed)
            return xxhash_64bits_with_seed(statePtr->buffer, (size_t)statePtr->totalLen, statePtr->seed);
        return xxhash_64bits_with_secret(statePtr->buffer, (size_t)(statePtr->totalLen),
            secret, statePtr->secretLimit + XXH_STRIPE_LEN);
    }

    KUMO_DLL xxhash::XXH_errorcode XXH3_128bits_reset_withSecret(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr,
        KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize) {
        return XXH3_64bits_reset_withSecret(statePtr, secret, secretSize);
    }

    KUMO_DLL xxhash::XXH_errorcode XXH3_128bits_reset_withSeed(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr, uint64_t seed) {
        return XXH3_64bits_reset_withSeed(statePtr, seed);
    }

    KUMO_DLL xxhash::XXH_errorcode XXH3_128bits_update(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr,
        KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t length) {
        return XXH3_64bits_update(statePtr, input, length);
    }

    KUMO_DLL turbo::xxhash::XXH128_hash_t XXH3_128bits_digest(KUMO_ATTRIBUTE_NOESCAPE const XXH3_state_t* statePtr) {
        const unsigned char* const secret = (statePtr->extSecret == NULL) ? statePtr->customSecret : statePtr->extSecret;
        if (statePtr->totalLen > XXH3_MIDSIZE_MAX) {
            alignas(XXH_ACC_ALIGN) uint64_t acc[XXH_ACC_NB];
            XXH3_digest_long(acc, statePtr, secret);
            KUMO_DASSERT(statePtr->secretLimit + XXH_STRIPE_LEN >= sizeof(acc) + XXH_SECRET_MERGEACCS_START);
            return turbo::xxhash::XXH3_finalizeLong_128b(acc, secret, statePtr->secretLimit + XXH_STRIPE_LEN, (uint64_t)statePtr->totalLen);
        }
        if (statePtr->useSeed)
            return xxhash_128bits_with_seed(statePtr->buffer, (size_t)statePtr->totalLen, statePtr->seed);
        return xxhash_128bits_with_secret(statePtr->buffer, (size_t)(statePtr->totalLen),
            secret, statePtr->secretLimit + XXH_STRIPE_LEN);
    }
} // namespace turbo
