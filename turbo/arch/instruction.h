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

namespace turbo {

    enum InstructionSet {
        DEFAULT = 0x0,
        NEON = 0x1,
        AVX2 = 0x4,
        SSE42 = 0x8,
        PCLMULQDQ = 0x10,
        BMI1 = 0x20,
        BMI2 = 0x40,
        ALTIVEC = 0x80,
        AVX512F = 0x100,
        AVX512DQ = 0x200,
        AVX512IFMA = 0x400,
        AVX512PF = 0x800,
        AVX512ER = 0x1000,
        AVX512CD = 0x2000,
        AVX512BW = 0x4000,
        AVX512VL = 0x8000,
        AVX512VBMI2 = 0x10000,
        AVX512VPOPCNTDQ = 0x2000,
        RVV = 0x4000,
        ZVBB = 0x8000,
        LSX = 0x40000,
        LASX = 0x80000,
    };

}  // namespace turbo
