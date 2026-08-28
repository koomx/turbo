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

#include <turbo/hash/xx/engine/fallback/interface.h>

namespace turbo::xxhash {

    void XXHashEngineScalar::init_custom_secret(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        xxhash_init_custom_secret_scalar(customSecret, seed64);
    }

    void XXHashEngineScalar::accumulate(uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret, size_t nbStripes) {
        xxhash_accumulate_scalar<xxhash::kDefaultXxhPrefetchDist>(acc, input, secret, nbStripes);
    }

    void XXHashEngineScalar::scramble_acc(void* KUMO_RESTRICT acc, const void* secret) {
        xxhash_scramble_acc_scalar(acc, secret);
    }

    void XXHashEngineScalar::accumulate_512(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        xxhash_accumulate_512_scalar(acc, input, secret);
    }

    static XXHashEngineScalar* get_xxhash_fallback_instance() {
        static XXHashEngineScalar ins;
        return &ins;
    }

    IsaInfo get_xxhash_fallback_info() {
        static IsaInfo ins = {
            true,
            true,
            {},
            "fallback",
            get_xxhash_fallback_instance(),
        };
        return ins;
    }
} // namespace turbo::xxhash
