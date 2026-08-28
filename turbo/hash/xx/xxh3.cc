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

    KUMO_DLL void
    xxhash_generate_secret(KUMO_ATTRIBUTE_NOESCAPE uint8_t* secretBuffer, size_t secretSize,
        KUMO_ATTRIBUTE_NOESCAPE const uint8_t* customSeed, size_t customSeedSize) {
        XXHashRegistry::get_best_isa()->xxhash_generate_secret(secretBuffer, secretSize, customSeed, customSeedSize);
    }

    KUMO_DLL void xxhash_generate_secret_from_seed(KUMO_ATTRIBUTE_NOESCAPE uint8_t* secretBuffer, uint64_t seed) {
        XXHashRegistry::get_best_isa()->xxhash_generate_secret_from_seed(secretBuffer, seed);
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

    KUMO_DLL XxHash128 xxhash128(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* data, size_t len, uint64_t seed) {
        return xxhash_128bits_with_seed(data,len,seed);
    }

    KUMO_DLL XxHash128 xxhash_128bits_with_seed(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t len, uint64_t seed) {
        return XXHashRegistry::get_best_isa()->xxhash_128bits_with_seed(input,len,seed);
    }

    KUMO_DLL XxHash128 xxhash_128bits_with_secret(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input,
       size_t len, KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize) {
        return XXHashRegistry::get_best_isa()->xxhash_128bits_with_secret(input,len,secret,secretSize);
    }

    KUMO_DLL XxHash128 xxhash_128bits_with_secret_and_seed(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t len, KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize, uint64_t seed) {
        return XXHashRegistry::get_best_isa()->xxhash_128bits_with_secret_and_seed(input,len,secret,secretSize,seed);
    }


    void XxHashState64::reset_with_seed(uint64_t seed) {
        XXHashRegistry::get_best_isa()->internal_64bits_reset_with_seed(this, seed);
    }

    void XxHashState64::reset_with_secret(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize) {
        XXHashRegistry::get_best_isa()->internal_64bits_reset_with_secret(this, secret,secretSize);
    }

    void XxHashState64::update(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t length) {
        XXHashRegistry::get_best_isa()->internal_update(this, (const uint8_t*)input, length);
    }


    uint64_t XxHashState64::digest() {
        const unsigned char* const secret = (ext_secret == nullptr) ? custom_secret : ext_secret;
        if (totalLen > xxhash::kXxh3MidsizeMax) {
            alignas(KUMO_CACHELINE_SIZE) uint64_t acc[xxhash::kXxhAccNb];
            XXHashRegistry::get_best_isa()->internal_digest_long(acc, this, secret);
            return turbo::xxhash::xxhash_finalize_long_64b(acc, secret, (uint64_t)totalLen);
        }
        if (use_seed)
            return xxhash_64bits_with_seed(buffer, (size_t)totalLen, seed);
        return xxhash_64bits_with_secret(buffer, (size_t)(totalLen),
            secret, secret_limit + xxhash::kXxhStripeLen);
    }



    void XxHashState128::reset_with_secret(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize) {
        XXHashRegistry::get_best_isa()->internal_64bits_reset_with_secret(this, secret, secretSize);
    }

    void XxHashState128::reset_with_seed(uint64_t seed) {
        XXHashRegistry::get_best_isa()->internal_64bits_reset_with_seed(this, seed);
    }

    void XxHashState128::update(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t length) {
        XXHashRegistry::get_best_isa()->internal_update(this, input, length);
    }

    KUMO_DLL XxHash128 XxHashState128::digest() {
        const unsigned char* const secret = (ext_secret == nullptr) ? custom_secret : ext_secret;
        if (totalLen > xxhash::kXxh3MidsizeMax) {
            alignas(KUMO_CACHELINE_SIZE) uint64_t acc[xxhash::kXxhAccNb];
            XXHashRegistry::get_best_isa()->internal_digest_long(acc, this, secret);
            KUMO_DASSERT(secret_limit + xxhash::kXxhStripeLen >= sizeof(acc) + xxhash::kXxhSecretMergeAccsStart);
            return turbo::xxhash::xxhash_finalize_long_128b(acc, secret, secret_limit + xxhash::kXxhStripeLen, (uint64_t)totalLen);
        }
        if (use_seed)
            return xxhash_128bits_with_seed(buffer, (size_t)totalLen, seed);
        return xxhash_128bits_with_secret(buffer, (size_t)(totalLen),
            secret, secret_limit + xxhash::kXxhStripeLen);
    }
} // namespace turbo
