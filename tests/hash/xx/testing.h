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

#include <array>
#include <cstddef>
#include <cstdint>
#include <turbo/hash/xx/common.h>

namespace turbo::xxtest {

    struct XXHTest32 {
        uint32_t len;
        uint32_t seed;
        uint32_t nresult;
    };

    struct XXHTest64 {
        uint64_t len;
        uint64_t seed;
        uint64_t nresult;
    };

    struct XXHTest128 {
        uint32_t len;
        uint64_t seed;
        XxHash128 Nresult;
    };

    struct XXHSecrets {
        uint32_t seedLen;
        uint32_t secretLen;
        std::array<uint8_t, 5> byte;
    };

    inline void fill_buffer(uint8_t* buf, size_t n) {
        uint64_t gen = 2654435761U;
        for (size_t i = 0; i < n; ++i) {
            buf[i] = static_cast<uint8_t>(gen >> 56);
            gen *= 11400714785074694797ULL;
        }
    }

}  // namespace turbo::xxtest
