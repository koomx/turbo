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

#include <turbo/bits/bits.h>
#include <turbo/hash/xx/common.h>
#include <turbo/hash/xx/engine/fallback/interface.h>
#include <turbo/hash/xx/scalar.h>

namespace turbo {

    KUMO_DLL int XXH128_isEqual(turbo::xxhash::XXH128_hash_t h1, turbo::xxhash::XXH128_hash_t h2);

    KUMO_DLL int XXH128_cmp(KUMO_ATTRIBUTE_NOESCAPE const void* h128_1, KUMO_ATTRIBUTE_NOESCAPE const void* h128_2);

    KUMO_DLL void XXH128_canonicalFromHash(KUMO_ATTRIBUTE_NOESCAPE turbo::xxhash::XXH128_canonical_t* dst, turbo::xxhash::XXH128_hash_t hash);

    KUMO_DLL turbo::xxhash::XXH128_hash_t XXH128_hashFromCanonical(KUMO_ATTRIBUTE_NOESCAPE const turbo::xxhash::XXH128_canonical_t* src);

    KUMO_DLL xxhash::XXH_errorcode XXH3_generateSecret(KUMO_ATTRIBUTE_NOESCAPE uint8_t* secretBuffer, size_t secretSize,
        KUMO_ATTRIBUTE_NOESCAPE const uint8_t* customSeed, size_t customSeedSize);

    KUMO_DLL void XXH3_generateSecret_fromSeed(KUMO_ATTRIBUTE_NOESCAPE uint8_t* secretBuffer, uint64_t seed);

    /////////////////
    /// 64
    KUMO_DLL uint64_t xxhash64(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* data, size_t len);

    KUMO_DLL uint64_t xxhash_64bits_with_seed(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t len, uint64_t seed);

    KUMO_DLL uint64_t xxhash_64bits_with_secret(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input,
        size_t len, KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize);

    /////////////////
    /// 128
    KUMO_DLL turbo::xxhash::XXH128_hash_t xxhash128(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* data, size_t len, uint64_t seed);

    KUMO_DLL turbo::xxhash::XXH128_hash_t xxhash_128bits_with_seed(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t len, uint64_t seed);

    KUMO_DLL turbo::xxhash::XXH128_hash_t xxhash_128bits_with_secret(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input,
        size_t len, KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize);

    KUMO_DLL turbo::xxhash::XXH128_hash_t xxhash_128bits_with_secret_and_seed(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input,
        size_t len, KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize, uint64_t seed);


    struct XXH3_state_t {
        alignas(64) uint64_t acc[8];
        /*!< The 8 accumulators. See @ref XXH32_state_s::acc and @ref XXH64_state_s::acc */
        alignas(64) unsigned char customSecret[XXH3_SECRET_DEFAULT_SIZE];
        /*!< Used to store a custom secret generated from a seed. */
        alignas(64) unsigned char buffer[XXH3_INTERNALBUFFER_SIZE];
        /*!< The internal buffer. @see XXH32_state_s::mem32 */
        uint32_t bufferedSize;
        /*!< The amount of memory in @ref buffer, @see XXH32_state_s::memsize */
        uint32_t useSeed;
        /*!< Reserved field. Needed for padding on 64-bit. */
        size_t nbStripesSoFar;
        /*!< Number or stripes processed. */
        uint64_t totalLen;
        /*!< Total length hashed. 64-bit even on 32-bit targets. */
        size_t nbStripesPerBlock;
        /*!< Number of stripes per block. */
        size_t secretLimit;
        /*!< Size of @ref customSecret or @ref extSecret */
        uint64_t seed;
        /*!< Seed for _withSeed variants. Must be zero otherwise, @see XXH3_INITSTATE() */
        uint64_t reserved64;
        /*!< Reserved field. */
        const unsigned char* extSecret;
        /*!< Reference to an external secret for the _withSecret variants, NULL
         *   for other variants. */
        /* note: there may be some padding at the end due to alignment on 64 bytes */
    };

    KUMO_DLL xxhash::XXH_errorcode XXH3_64bits_reset_withSeed(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr, uint64_t seed);

    KUMO_DLL xxhash::XXH_errorcode XXH3_64bits_reset_withSecret(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr,
        KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize);

    KUMO_DLL xxhash::XXH_errorcode XXH3_64bits_update(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr,
        KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t length);

    KUMO_DLL uint64_t XXH3_64bits_digest(KUMO_ATTRIBUTE_NOESCAPE const XXH3_state_t* statePtr);

    KUMO_DLL xxhash::XXH_errorcode XXH3_128bits_reset_withSeed(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr, uint64_t seed);

    KUMO_DLL xxhash::XXH_errorcode XXH3_128bits_reset_withSecret(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr,
        KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize);

    KUMO_DLL xxhash::XXH_errorcode XXH3_128bits_update(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr,
        KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t length);

    KUMO_DLL turbo::xxhash::XXH128_hash_t XXH3_128bits_digest(KUMO_ATTRIBUTE_NOESCAPE const XXH3_state_t* statePtr);


} // namespace turbo
