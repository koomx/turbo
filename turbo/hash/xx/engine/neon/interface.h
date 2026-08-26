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

#include <turbo/arch/isa.h>
#include <turbo/hash/xx/engine/fallback/interface.h>
#include <turbo/hash/xx/interface.h>

#if KUMO_SIMD_NEON
#include <arm_neon.h>
#endif

namespace turbo::xxhash {

#if KUMO_SIMD_NEON

    class XXHashEngineNeon : public XXHashEngine {
        void init_custom_secret(void* KUMO_RESTRICT customSecret, uint64_t seed64) override;
        void accumulate(uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT input, const uint8_t* KUMO_RESTRICT secret, size_t nbStripes) override;
        void scramble_acc(void* KUMO_RESTRICT acc, const void* secret) override;
        void accumulate_512(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT input, const void* KUMO_RESTRICT secret) override;
    };

#endif /* KUMO_SIMD_NEON */

    IsaInfo get_xxhash_neon_info();
} // namespace turbo::xxhash
