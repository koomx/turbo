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

#define KUMO_ARCH_X86_64       1
#define KUMO_ARCH_X86_32       1

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
#if defined(__AVX512IFMA__)
#define KUMO_SIMD_AVX512IFMA   1
#else
#define KUMO_SIMD_AVX512IFMA   0
#endif
#if defined(__AVX512CD__)
#define KUMO_SIMD_AVX512CD     1
#else
#define KUMO_SIMD_AVX512CD     0
#endif
#if defined(__AVX512VBMI__)
#define KUMO_SIMD_AVX512VBMI   1
#else
#define KUMO_SIMD_AVX512VBMI   0
#endif
#if defined(__AVX512VBMI2__)
#define KUMO_SIMD_AVX512VBMI2  1
#else
#define KUMO_SIMD_AVX512VBMI2  0
#endif
#if defined(__AVX512VNNI__)
#define KUMO_SIMD_AVX512VNNI   1
#else
#define KUMO_SIMD_AVX512VNNI   0
#endif
#if defined(__AVX512BITALG__)
#define KUMO_SIMD_AVX512BITALG 1
#else
#define KUMO_SIMD_AVX512BITALG 0
#endif
#if defined(__AVX512VPOPCNTDQ__)
#define KUMO_SIMD_AVX512VPOPCNTDQ 1
#else
#define KUMO_SIMD_AVX512VPOPCNTDQ 0
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
#define KUMO_SIMD_X86_AES          1
#else
#define KUMO_SIMD_X86_AES          0
#endif
#if defined(__PCLMUL__)
#define KUMO_SIMD_PCLMUL       1
#else
#define KUMO_SIMD_PCLMUL       0
#endif

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

#elif defined(__i386__) || defined(__i386) || defined(_M_IX86)

#define KUMO_ARCH_X86          1
#define KUMO_ARCH_X86_64       0
#define KUMO_ARCH_X86_32       1

#if defined(__SSE__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1)
#define KUMO_SIMD_SSE        1
#else
#define KUMO_SIMD_SSE        0
#endif
#if defined(__SSE2__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#define KUMO_SIMD_SSE2       1
#else
#define KUMO_SIMD_SSE2       0
#endif
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
#if defined(__AVX512IFMA__)
#define KUMO_SIMD_AVX512IFMA   1
#else
#define KUMO_SIMD_AVX512IFMA   0
#endif
#if defined(__AVX512CD__)
#define KUMO_SIMD_AVX512CD     1
#else
#define KUMO_SIMD_AVX512CD     0
#endif
#if defined(__AVX512VBMI__)
#define KUMO_SIMD_AVX512VBMI   1
#else
#define KUMO_SIMD_AVX512VBMI   0
#endif
#if defined(__AVX512VBMI2__)
#define KUMO_SIMD_AVX512VBMI2  1
#else
#define KUMO_SIMD_AVX512VBMI2  0
#endif
#if defined(__AVX512VNNI__)
#define KUMO_SIMD_AVX512VNNI   1
#else
#define KUMO_SIMD_AVX512VNNI   0
#endif
#if defined(__AVX512BITALG__)
#define KUMO_SIMD_AVX512BITALG 1
#else
#define KUMO_SIMD_AVX512BITALG 0
#endif
#if defined(__AVX512VPOPCNTDQ__)
#define KUMO_SIMD_AVX512VPOPCNTDQ 1
#else
#define KUMO_SIMD_AVX512VPOPCNTDQ 0
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
#define KUMO_SIMD_X86_AES          1
#else
#define KUMO_SIMD_X86_AES          0
#endif
#if defined(__PCLMUL__)
#define KUMO_SIMD_PCLMUL       1
#else
#define KUMO_SIMD_PCLMUL       0
#endif

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

#define KUMO_ARCH_NAME       "x86"
#else
#define KUMO_ARCH_X86          0
#define KUMO_ARCH_X86_64       0
#define KUMO_ARCH_X86_32       0
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
#define KUMO_SIMD_X86_AES          0
#define KUMO_SIMD_PCLMUL       0
#endif

#if KUMO_ARCH_X86
#if defined(__PRFCHW__) || defined(__3dNOW__) || defined(__3dNOW_A__)
#define KUMO_SIMD_PRFCHW 1
#else
#define KUMO_SIMD_PRFCHW 0
#endif
#if defined(__3dNOW__) || defined(__3dNOW_A__)
#define KUMO_SIMD_PREFETCH 1
#else
#define KUMO_SIMD_PREFETCH 0
#endif
#if defined(__PREFETCHWT1__)
#define KUMO_SIMD_PREFETCHWT1 1
#else
#define KUMO_SIMD_PREFETCHWT1 0
#endif
#if defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_16) || defined(__CX16__) || \
    defined(_MSC_VER)
#define KUMO_SIMD_CX16 1
#else
#define KUMO_SIMD_CX16 0
#endif
#if defined(__MOVBE__)
#define KUMO_SIMD_MOVBE 1
#else
#define KUMO_SIMD_MOVBE 0
#endif
#if defined(__XSAVE__)
#define KUMO_SIMD_XSAVE 1
#else
#define KUMO_SIMD_XSAVE 0
#endif
#if defined(__F16C__)
#define KUMO_SIMD_F16C 1
#else
#define KUMO_SIMD_F16C 0
#endif
#if defined(__RDRND__)
#define KUMO_SIMD_RDRND 1
#else
#define KUMO_SIMD_RDRND 0
#endif
#if defined(__RDSEED__)
#define KUMO_SIMD_RDSEED 1
#else
#define KUMO_SIMD_RDSEED 0
#endif
#if defined(__FSGSBASE__)
#define KUMO_SIMD_FSGSBASE 1
#else
#define KUMO_SIMD_FSGSBASE 0
#endif
#if defined(__SHA__)
#define KUMO_SIMD_SHA 1
#else
#define KUMO_SIMD_SHA 0
#endif
#if defined(__ADX__)
#define KUMO_SIMD_ADX 1
#else
#define KUMO_SIMD_ADX 0
#endif
#if defined(__CLFLUSHOPT__)
#define KUMO_SIMD_CLFLUSHOPT 1
#else
#define KUMO_SIMD_CLFLUSHOPT 0
#endif
#if defined(__CLWB__)
#define KUMO_SIMD_CLWB 1
#else
#define KUMO_SIMD_CLWB 0
#endif
#if defined(__CLZERO__)
#define KUMO_SIMD_CLZERO 1
#else
#define KUMO_SIMD_CLZERO 0
#endif
#if defined(__RDTSCP__)
#define KUMO_SIMD_RDTSCP 1
#else
#define KUMO_SIMD_RDTSCP 0
#endif
#if defined(__RDPID__)
#define KUMO_SIMD_RDPID 1
#else
#define KUMO_SIMD_RDPID 0
#endif
#if defined(__HLE__)
#define KUMO_SIMD_HLE 1
#else
#define KUMO_SIMD_HLE 0
#endif
#if defined(__RTM__)
#define KUMO_SIMD_RTM 1
#else
#define KUMO_SIMD_RTM 0
#endif
#if defined(__MPX__)
#define KUMO_SIMD_MPX 1
#else
#define KUMO_SIMD_MPX 0
#endif
#if defined(__SSE4A__)
#define KUMO_SIMD_SSE4A 1
#else
#define KUMO_SIMD_SSE4A 0
#endif
#if defined(__FMA4__)
#define KUMO_SIMD_FMA4 1
#else
#define KUMO_SIMD_FMA4 0
#endif
#if defined(__XOP__)
#define KUMO_SIMD_XOP 1
#else
#define KUMO_SIMD_XOP 0
#endif
#if defined(__TBM__)
#define KUMO_SIMD_TBM 1
#else
#define KUMO_SIMD_TBM 0
#endif
#if defined(__LWP__)
#define KUMO_SIMD_LWP 1
#else
#define KUMO_SIMD_LWP 0
#endif
#if defined(__VAES__)
#define KUMO_SIMD_VAES 1
#else
#define KUMO_SIMD_VAES 0
#endif
#if defined(__VPCLMULQDQ__)
#define KUMO_SIMD_VPCLMUL 1
#else
#define KUMO_SIMD_VPCLMUL 0
#endif
#if defined(__GFNI__)
#define KUMO_SIMD_GFNI 1
#else
#define KUMO_SIMD_GFNI 0
#endif
#if defined(__AVXVNNI__)
#define KUMO_SIMD_AVXVNNI 1
#else
#define KUMO_SIMD_AVXVNNI 0
#endif
#if defined(__AVX512PF__)
#define KUMO_SIMD_AVX512PF 1
#else
#define KUMO_SIMD_AVX512PF 0
#endif
#if defined(__AVX512ER__)
#define KUMO_SIMD_AVX512ER 1
#else
#define KUMO_SIMD_AVX512ER 0
#endif
#if defined(__AVX512BF16__)
#define KUMO_SIMD_AVX512BF16 1
#else
#define KUMO_SIMD_AVX512BF16 0
#endif
#if defined(__AVX512FP16__)
#define KUMO_SIMD_AVX512FP16 1
#else
#define KUMO_SIMD_AVX512FP16 0
#endif
#if defined(__AVX512VP2INTERSECT__)
#define KUMO_SIMD_AVX512VP2 1
#else
#define KUMO_SIMD_AVX512VP2 0
#endif
#if defined(__AMX_TILE__)
#define KUMO_SIMD_AMX_TILE 1
#else
#define KUMO_SIMD_AMX_TILE 0
#endif
#if defined(__AMX_INT8__)
#define KUMO_SIMD_AMX_INT8 1
#else
#define KUMO_SIMD_AMX_INT8 0
#endif
#if defined(__AMX_BF16__)
#define KUMO_SIMD_AMX_BF16 1
#else
#define KUMO_SIMD_AMX_BF16 0
#endif
#if defined(__AMX_FP16__)
#define KUMO_SIMD_AMX_FP16 1
#else
#define KUMO_SIMD_AMX_FP16 0
#endif
#else
#define KUMO_SIMD_PRFCHW 0
#define KUMO_SIMD_PREFETCH 0
#define KUMO_SIMD_PREFETCHWT1 0
#define KUMO_SIMD_CX16 0
#define KUMO_SIMD_MOVBE 0
#define KUMO_SIMD_XSAVE 0
#define KUMO_SIMD_F16C 0
#define KUMO_SIMD_RDRND 0
#define KUMO_SIMD_RDSEED 0
#define KUMO_SIMD_FSGSBASE 0
#define KUMO_SIMD_SHA 0
#define KUMO_SIMD_ADX 0
#define KUMO_SIMD_CLFLUSHOPT 0
#define KUMO_SIMD_CLWB 0
#define KUMO_SIMD_CLZERO 0
#define KUMO_SIMD_RDTSCP 0
#define KUMO_SIMD_RDPID 0
#define KUMO_SIMD_HLE 0
#define KUMO_SIMD_RTM 0
#define KUMO_SIMD_MPX 0
#define KUMO_SIMD_SSE4A 0
#define KUMO_SIMD_FMA4 0
#define KUMO_SIMD_XOP 0
#define KUMO_SIMD_TBM 0
#define KUMO_SIMD_LWP 0
#define KUMO_SIMD_VAES 0
#define KUMO_SIMD_VPCLMUL 0
#define KUMO_SIMD_GFNI 0
#define KUMO_SIMD_AVXVNNI 0
#define KUMO_SIMD_AVX512PF 0
#define KUMO_SIMD_AVX512ER 0
#define KUMO_SIMD_AVX512BF16 0
#define KUMO_SIMD_AVX512FP16 0
#define KUMO_SIMD_AVX512VP2 0
#define KUMO_SIMD_AMX_TILE 0
#define KUMO_SIMD_AMX_INT8 0
#define KUMO_SIMD_AMX_BF16 0
#define KUMO_SIMD_AMX_FP16 0
#endif
