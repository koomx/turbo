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

    KUMO_DLL void xxhash_generate_secret(KUMO_ATTRIBUTE_NOESCAPE uint8_t* secretBuffer, size_t secretSize,
        KUMO_ATTRIBUTE_NOESCAPE const uint8_t* customSeed, size_t customSeedSize);

    KUMO_DLL void xxhash_generate_secret_from_seed(KUMO_ATTRIBUTE_NOESCAPE uint8_t* secretBuffer, uint64_t seed);

    /////////////////
    /// 64
    KUMO_DLL uint64_t xxhash64(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* data, size_t len);

    KUMO_DLL uint64_t xxhash_64bits_with_seed(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t len, uint64_t seed);

    KUMO_DLL uint64_t xxhash_64bits_with_secret(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input,
        size_t len, KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize);

    /////////////////
    /// 128
    KUMO_DLL XxHash128 xxhash128(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* data, size_t len, uint64_t seed);

    KUMO_DLL XxHash128 xxhash_128bits_with_seed(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t len, uint64_t seed);

    KUMO_DLL XxHash128 xxhash_128bits_with_secret(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input,
        size_t len, KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize);

    KUMO_DLL XxHash128 xxhash_128bits_with_secret_and_seed(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input,
        size_t len, KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize, uint64_t seed);

    class XxHashState64 : public XxHashStateCore {
    public:
        XxHashState64(uint64_t sd = 0) {
            reset_with_seed(sd);
        }

        XxHashState64(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize) {
            reset_with_secret(secret, secretSize);
        }

        void reset_with_seed(uint64_t seed);

        void reset_with_secret(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize);

        void update(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t length);

        uint64_t digest();

    };

    class XxHashState128 : public XxHashStateCore {
    public:
        XxHashState128(uint64_t sd = 0) {
            reset_with_seed(sd);
        }

        XxHashState128(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize) {
            reset_with_secret(secret, secretSize);
        }

        void reset_with_seed(uint64_t seed);

        void reset_with_secret(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* secret, size_t secretSize);

        void update(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t length);

        XxHash128 digest();
    };

} // namespace turbo
