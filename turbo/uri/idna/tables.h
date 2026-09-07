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

#include <cstdint>

namespace turbo::idna {

    enum direction : uint8_t {
        NONE,
        BN,
        CS,
        ES,
        ON,
        EN,
        L,
        R,
        NSM,
        AL,
        AN,
        ET,
        WS,
        RLO,
        LRO,
        PDF,
        RLE,
        RLI,
        FSI,
        PDI,
        LRI,
        B,
        S,
        LRE
    };

    struct directions {
        uint32_t start_code;
        uint32_t final_code;
        direction direct;
    };

    struct IdnaTables {
        static const uint32_t mappings[5164];
        static const uint32_t table[8000][2];

        static const uint8_t decomposition_index[4352];
        static const uint16_t decomposition_block[67][257];
        static const char32_t decomposition_data[9102];

        static const uint8_t canonical_combining_class_index[4352];
        static const uint8_t canonical_combining_class_block[67][256];

        static const uint8_t composition_index[4352];
        static const uint16_t composition_block[67][257];
        static const char32_t composition_data[1883];

        static const directions dir_table[];

    };
}  // namespace turbo::idna
