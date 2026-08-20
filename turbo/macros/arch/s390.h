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

#if defined(__s390x__) || defined(__s390__)

#define KUMO_ARCH_X86        0
#define KUMO_ARCH_ARM        0
#define KUMO_ARCH_RISCV      0
#define KUMO_ARCH_LOONGARCH  0
#define KUMO_ARCH_PPC        0
#define KUMO_ARCH_S390       1
#define KUMO_ARCH_MIPS       0
#define KUMO_ARCH_E2K        0
#define KUMO_ARCH_WASM       0

#define KUMO_ARCH_X86_64     0
#define KUMO_ARCH_X86_32     0
#define KUMO_ARCH_ARM64      0
#define KUMO_ARCH_ARM32      0
#define KUMO_ARCH_ARM64EC    0
#define KUMO_ARCH_RISCV64    0
#define KUMO_ARCH_RISCV32    0
#define KUMO_ARCH_LOONGARCH64 0
#define KUMO_ARCH_LOONGARCH32 0
#define KUMO_ARCH_PPC64      0
#define KUMO_ARCH_PPC32      0
#define KUMO_ARCH_PPC64LE    0
#if defined(__s390x__)
#define KUMO_ARCH_S390X      1
#define KUMO_ARCH_S390_31    0
#define KUMO_ARCH_32_BIT     0
#define KUMO_ARCH_64_BIT     1
#else
#define KUMO_ARCH_S390X      0
#define KUMO_ARCH_S390_31    1
#define KUMO_ARCH_32_BIT     1
#define KUMO_ARCH_64_BIT     0
#endif
#define KUMO_ARCH_MIPS64     0
#define KUMO_ARCH_MIPS32     0
#define KUMO_ARCH_WASM64     0
#define KUMO_ARCH_WASM32     0

#define KUMO_SIMD_SSE          0
#define KUMO_SIMD_SSE2         0
#define KUMO_SIMD_SSE3         0
#define KUMO_SIMD_SSSE3        0
#define KUMO_SIMD_SSE4_1       0
#define KUMO_SIMD_SSE4_2       0
#define KUMO_SIMD_AVX          0
#define KUMO_SIMD_AVX2         0
#define KUMO_SIMD_AVX512F      0
#define KUMO_SIMD_AVX512BW     0
#define KUMO_SIMD_AVX512VL     0
#define KUMO_SIMD_AVX512DQ     0
#define KUMO_SIMD_AVX512IFMA   0
#define KUMO_SIMD_AVX512CD     0
#define KUMO_SIMD_AVX512VBMI   0
#define KUMO_SIMD_AVX512VBMI2  0
#define KUMO_SIMD_AVX512VNNI   0
#define KUMO_SIMD_AVX512BITALG 0
#define KUMO_SIMD_AVX512VPOPCNTDQ 0
#define KUMO_SIMD_FMA          0
#define KUMO_SIMD_BMI1         0
#define KUMO_SIMD_BMI2         0
#define KUMO_SIMD_POPCNT       0
#define KUMO_SIMD_LZCNT        0
#define KUMO_SIMD_NEON         0
#define KUMO_SIMD_SVE          0
#define KUMO_SIMD_SVE2         0
#define KUMO_SIMD_AES          0
#define KUMO_SIMD_PCLMUL       0
#define KUMO_SIMD_RVV          0
#define KUMO_SIMD_LSX          0
#define KUMO_SIMD_LASX         0
#define KUMO_SIMD_ALTIVEC      0
#define KUMO_SIMD_VSX          0
#define KUMO_SIMD_CRYPTO       0

// Match TURBO default (optimization.h falls through to 64).
#define KUMO_CACHELINE_SIZE 64

#define KUMO_SIMD_LEVEL      "NONE"

#if KUMO_ARCH_S390X
#define KUMO_ARCH_NAME       "s390x"
#else
#define KUMO_ARCH_NAME       "s390"
#endif

#endif
