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

#include <turbo/hash/xx/interface.h>

namespace turbo::xxhash {

    const uint8_t* XXHashEngine::consume_stripes(uint64_t* KUMO_RESTRICT acc,
        size_t* KUMO_RESTRICT nbStripesSoFarPtr, size_t nb_stripes_per_block,
        const uint8_t* KUMO_RESTRICT input, size_t nbStripes,
        const uint8_t* KUMO_RESTRICT secret, size_t secret_limit) {
        const uint8_t* initialSecret = secret + *nbStripesSoFarPtr * kXxhSecretConsumeRate;
        if (nbStripes >= (nb_stripes_per_block - *nbStripesSoFarPtr)) {
            size_t nbStripesThisIter = nb_stripes_per_block - *nbStripesSoFarPtr;
            do {
                accumulate(acc, input, initialSecret, nbStripesThisIter);
                scramble_acc(acc, secret + secret_limit);
                input += nbStripesThisIter * kXxhStripeLen;
                nbStripes -= nbStripesThisIter;
                nbStripesThisIter = nb_stripes_per_block;
                initialSecret = secret;
            } while (nbStripes >= nb_stripes_per_block);
            *nbStripesSoFarPtr = 0;
        }
        if (nbStripes > 0) {
            accumulate(acc, input, initialSecret, nbStripes);
            input += nbStripes * kXxhStripeLen;
            *nbStripesSoFarPtr += nbStripes;
        }
        return input;
    }

    void XXHashEngine::internal_64bits_reset_with_seed(KUMO_ATTRIBUTE_NOESCAPE XxHashStateCore* statePtr, uint64_t seed) {
        if (seed == 0) {
            statePtr->reset(0, turbo::xxhash::kXxhSecret, kXxhSecretDefaultSize);
            return;
        }
        if ((seed != statePtr->seed) || (statePtr->ext_secret != nullptr))
            init_custom_secret(statePtr->custom_secret, seed);
        statePtr->reset(seed, nullptr, kXxhSecretDefaultSize);
    }

    void XXHashEngine::internal_64bits_reset_with_secret(KUMO_ATTRIBUTE_NOESCAPE XxHashStateCore* statePtr,
        KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize) {
        KUMO_DASSERT(secret != nullptr);
        KUMO_DASSERT(secretSize >= kXxh3SecretSizeMin);
        statePtr->reset(0, secret, secretSize);
    }

      void XXHashEngine::internal_update(XxHashStateCore* KUMO_RESTRICT const state,
        const uint8_t* KUMO_RESTRICT input, size_t len) {
        if (input == nullptr) {
            KUMO_DASSERT(len == 0);
            return;
        }

        KUMO_DASSERT(state != nullptr);
        state->totalLen += len;

        KUMO_DASSERT(state->buffered_size <= kXxh3InternalBufferSize);
        if (len <= kXxh3InternalBufferSize - state->buffered_size) {
            memcpy(state->buffer + state->buffered_size, input, len);
            state->buffered_size += (uint32_t)len;
            return;
        }

        {
            const uint8_t* const bEnd = input + len;
            const unsigned char* const secret = (state->ext_secret == nullptr) ? state->custom_secret : state->ext_secret;
#if XXHASH_STREAM_USE_STACK >= 1
            alignas(KUMO_CACHELINE_SIZE) uint64_t acc[8];
            memcpy(acc, state->acc, sizeof(acc));
#else
            uint64_t* KUMO_RESTRICT const acc = state->acc;
#endif
            static_assert(kXxh3InternalBufferSize % kXxhStripeLen == 0, "kXxh3InternalBufferSize % kXxhStripeLen == 0");

            if (state->buffered_size) {
                size_t const loadSize = kXxh3InternalBufferSize - state->buffered_size;
                memcpy(state->buffer + state->buffered_size, input, loadSize);
                input += loadSize;
                consume_stripes(acc,
                    &state->nb_stripes_so_far, state->nb_stripes_per_block,
                    state->buffer, kXxh3InternalBufferSize / kXxhStripeLen,
                    secret, state->secret_limit);
                state->buffered_size = 0;
            }
            KUMO_DASSERT(input < bEnd);
            if (bEnd - input > kXxh3InternalBufferSize) {
                size_t nbStripes = (size_t)(bEnd - 1 - input) / kXxhStripeLen;
                input = consume_stripes(acc,
                    &state->nb_stripes_so_far, state->nb_stripes_per_block,
                    input, nbStripes,
                    secret, state->secret_limit);
                memcpy(state->buffer + sizeof(state->buffer) - kXxhStripeLen, input - kXxhStripeLen, kXxhStripeLen);
            }
            KUMO_DASSERT(input < bEnd);
            KUMO_DASSERT(bEnd - input <= kXxh3InternalBufferSize);
            KUMO_DASSERT(state->buffered_size == 0);
            memcpy(state->buffer, input, (size_t)(bEnd - input));
            state->buffered_size = (uint32_t)(bEnd - input);
#if XXHASH_STREAM_USE_STACK >= 1
            memcpy(state->acc, acc, sizeof(acc));
#endif
        }

    }


    void XXHashEngine::internal_digest_long(uint64_t* acc, const XxHashStateCore* state, const unsigned char* secret) {
        uint8_t lastStripe[kXxhStripeLen];
        const uint8_t* lastStripePtr;

        memcpy(acc, state->acc, sizeof(state->acc));
        if (state->buffered_size >= kXxhStripeLen) {
            size_t const nbStripes = (state->buffered_size - 1) / kXxhStripeLen;
            size_t nb_stripes_so_far = state->nb_stripes_so_far;
            consume_stripes(acc,
                &nb_stripes_so_far, state->nb_stripes_per_block,
                state->buffer, nbStripes,
                secret, state->secret_limit);
            lastStripePtr = state->buffer + state->buffered_size - kXxhStripeLen;
        } else {
            size_t const catchupSize = kXxhStripeLen - state->buffered_size;
            KUMO_DASSERT(state->buffered_size > 0);
            memcpy(lastStripe, state->buffer + sizeof(state->buffer) - catchupSize, catchupSize);
            memcpy(lastStripe + catchupSize, state->buffer, state->buffered_size);
            lastStripePtr = lastStripe;
        }
        accumulate_512(acc, lastStripePtr, secret + state->secret_limit - 7);
    }


}  // namespace turbo::xxhash

