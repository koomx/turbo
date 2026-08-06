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

#if (defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)) && \
    !defined(_M_ARM64EC)

#define KUMO_ARCH_X86          1
#define KUMO_ARCH_ARM          0
#define KUMO_ARCH_RISCV        0
#define KUMO_ARCH_LOONGARCH    0
#define KUMO_ARCH_PPC          0
#define KUMO_ARCH_S390         0
#define KUMO_ARCH_MIPS         0
#define KUMO_ARCH_E2K          0
#define KUMO_ARCH_WASM         0

#define KUMO_ARCH_X86_64       1
#define KUMO_ARCH_X86_32       0
#define KUMO_ARCH_ARM64        0
#define KUMO_ARCH_ARM32        0
#define KUMO_ARCH_ARM64EC      0
#define KUMO_ARCH_RISCV64      0
#define KUMO_ARCH_RISCV32      0
#define KUMO_ARCH_LOONGARCH64  0
#define KUMO_ARCH_LOONGARCH32  0
#define KUMO_ARCH_PPC64        0
#define KUMO_ARCH_PPC32        0
#define KUMO_ARCH_PPC64LE      0
#define KUMO_ARCH_S390X        0
#define KUMO_ARCH_S390_31      0
#define KUMO_ARCH_MIPS64       0
#define KUMO_ARCH_MIPS32       0
#define KUMO_ARCH_WASM64       0
#define KUMO_ARCH_WASM32       0

#define KUMO_ARCH_32_BIT     0
#define KUMO_ARCH_64_BIT     1

#define KUMO_SIMD_SSE        1
#define KUMO_SIMD_SSE2       1
#if defined(__SSE3__)
#define KUMO_SIMD_SSE3         1
#else
#define KUMO_SIMD_SSE3         0
#endif
#if defined(__SSSE3__)
#define KUMO_SIMD_SSSE3        1
#else
#define KUMO_SIMD_SSSE3        0
#endif
#if defined(__SSE4_1__)
#define KUMO_SIMD_SSE4_1       1
#else
#define KUMO_SIMD_SSE4_1       0
#endif
#if defined(__SSE4_2__)
#define KUMO_SIMD_SSE4_2       1
#else
#define KUMO_SIMD_SSE4_2       0
#endif
#if defined(__AVX__)
#define KUMO_SIMD_AVX          1
#else
#define KUMO_SIMD_AVX          0
#endif
#if defined(__AVX2__)
#define KUMO_SIMD_AVX2         1
#else
#define KUMO_SIMD_AVX2         0
#endif
#if defined(__AVX512F__)
#define KUMO_SIMD_AVX512F      1
#else
#define KUMO_SIMD_AVX512F      0
#endif
#if defined(__AVX512BW__)
#define KUMO_SIMD_AVX512BW     1
#else
#define KUMO_SIMD_AVX512BW     0
#endif
#if defined(__AVX512VL__)
#define KUMO_SIMD_AVX512VL     1
#else
#define KUMO_SIMD_AVX512VL     0
#endif
#if defined(__AVX512DQ__)
#define KUMO_SIMD_AVX512DQ     1
#else
#define KUMO_SIMD_AVX512DQ     0
#endif
#if defined(__FMA__)
#define KUMO_SIMD_FMA          1
#else
#define KUMO_SIMD_FMA          0
#endif
#if defined(__BMI__)
#define KUMO_SIMD_BMI1         1
#else
#define KUMO_SIMD_BMI1         0
#endif
#if defined(__BMI2__)
#define KUMO_SIMD_BMI2         1
#else
#define KUMO_SIMD_BMI2         0
#endif
#if defined(__POPCNT__)
#define KUMO_SIMD_POPCNT       1
#else
#define KUMO_SIMD_POPCNT       0
#endif
#if defined(__LZCNT__)
#define KUMO_SIMD_LZCNT        1
#else
#define KUMO_SIMD_LZCNT        0
#endif
#if defined(__AES__)
#define KUMO_SIMD_AES          1
#else
#define KUMO_SIMD_AES          0
#endif
#if defined(__PCLMUL__)
#define KUMO_SIMD_PCLMUL       1
#else
#define KUMO_SIMD_PCLMUL       0
#endif
#define KUMO_SIMD_NEON         0
#define KUMO_SIMD_SVE          0
#define KUMO_SIMD_SVE2         0
#define KUMO_SIMD_RVV          0
#define KUMO_SIMD_LSX          0
#define KUMO_SIMD_LASX         0
#define KUMO_SIMD_ALTIVEC      0
#define KUMO_SIMD_VSX          0
#define KUMO_SIMD_CRYPTO       0

#define KUMO_CACHELINE_SIZE 64

#if defined(__AVX512F__)
#define KUMO_SIMD_LEVEL      "AVX512"
#elif defined(__AVX2__)
#define KUMO_SIMD_LEVEL      "AVX2"
#elif defined(__AVX__)
#define KUMO_SIMD_LEVEL      "AVX"
#elif defined(__SSE4_2__)
#define KUMO_SIMD_LEVEL      "SSE4_2"
#elif defined(__SSE4_1__)
#define KUMO_SIMD_LEVEL      "SSE4_1"
#elif defined(__SSSE3__)
#define KUMO_SIMD_LEVEL      "SSSE3"
#elif defined(__SSE3__)
#define KUMO_SIMD_LEVEL      "SSE3"
#elif defined(__SSE2__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2) || \
      defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
#define KUMO_SIMD_LEVEL      "SSE2"
#elif defined(__SSE__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1)
#define KUMO_SIMD_LEVEL      "SSE"
#else
#define KUMO_SIMD_LEVEL      "NONE"
#endif

#define KUMO_ARCH_NAME       "x86_64"

#endif
