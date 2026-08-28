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

#pragma once

#if defined(__riscv)

#define KUMO_ARCH_RISCV      1
#if __riscv_xlen == 64
#define KUMO_ARCH_RISCV64    1
#define KUMO_ARCH_RISCV32    0
#elif __riscv_xlen == 32
#define KUMO_ARCH_RISCV64    0
#define KUMO_ARCH_RISCV32    1
#else
#error "unsupported __riscv_xlen"
#endif

#if defined(__riscv_v) || defined(__riscv_vector)
#define KUMO_SIMD_RVV        1
#else
#define KUMO_SIMD_RVV        0
#endif

#define KUMO_CACHELINE_SIZE 64

#if KUMO_SIMD_RVV
#define KUMO_SIMD_LEVEL      "RVV"
#else
#define KUMO_SIMD_LEVEL      "NONE"
#endif

#if KUMO_ARCH_RISCV64
#define KUMO_ARCH_NAME       "RISCV64"
#else
#define KUMO_ARCH_NAME       "RISCV32"
#endif

#else
#define KUMO_ARCH_RISCV      0
#define KUMO_ARCH_RISCV64    0
#define KUMO_ARCH_RISCV32    0
#define KUMO_SIMD_RVV        0
#endif
