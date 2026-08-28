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

#if defined(__powerpc64__) || defined(__PPC64__) || defined(__powerpc__) || \
    defined(__ppc__) || defined(__PPC__)


#define KUMO_ARCH_PPC        1

#if defined(__powerpc64__) || defined(__PPC64__)
#define KUMO_ARCH_PPC64      1
#define KUMO_ARCH_PPC32      0
#if (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && \
     __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || \
    defined(_LITTLE_ENDIAN) || defined(__LITTLE_ENDIAN__)
#define KUMO_ARCH_PPC64LE    1
#else
#define KUMO_ARCH_PPC64LE    0
#endif
#else
#define KUMO_ARCH_PPC64      0
#define KUMO_ARCH_PPC32      1
#define KUMO_ARCH_PPC64LE    0
#endif

#if defined(__VEC__) || defined(__ALTIVEC__)
#define KUMO_SIMD_ALTIVEC    1
#else
#define KUMO_SIMD_ALTIVEC    0
#endif
#if defined(__VSX__)
#define KUMO_SIMD_VSX        1
#else
#define KUMO_SIMD_VSX        0
#endif
#if defined(__CRYPTO__)
#define KUMO_SIMD_CRYPTO     1
#else
#define KUMO_SIMD_CRYPTO     0
#endif
// Match TURBO_HAVE_ACCELERATED_AES on PPC: AltiVec/VEC + VSX + CRYPTO.
#if (defined(__VEC__) || defined(__ALTIVEC__)) && defined(__VSX__) && \
    defined(__CRYPTO__)
#define KUMO_SIMD_PPC_AES        1
#else
#define KUMO_SIMD_PPC_AES        0
#endif

// Match TURBO: 128 only for powerpc64; 32-bit PPC uses the default 64.
#if defined(__powerpc64__) || defined(__PPC64__)
#define KUMO_CACHELINE_SIZE 128
#else
#define KUMO_CACHELINE_SIZE 64
#endif

#if KUMO_SIMD_PPC_AES
#define KUMO_SIMD_LEVEL      "CRYPTO"
#elif KUMO_SIMD_VSX
#define KUMO_SIMD_LEVEL      "VSX"
#elif KUMO_SIMD_ALTIVEC
#define KUMO_SIMD_LEVEL      "ALTIVEC"
#else
#define KUMO_SIMD_LEVEL      "NONE"
#endif

#if KUMO_ARCH_PPC64LE
#define KUMO_ARCH_NAME       "PPC64LE"
#elif KUMO_ARCH_PPC64
#define KUMO_ARCH_NAME       "PPC64"
#else
#define KUMO_ARCH_NAME       "PPC32"
#endif
#else
#define KUMO_ARCH_PPC        0
#define KUMO_ARCH_PPC64      0
#define KUMO_ARCH_PPC32      0
#define KUMO_SIMD_CRYPTO     0
#define KUMO_ARCH_PPC64LE    0
#define KUMO_SIMD_ALTIVEC    0
#define KUMO_SIMD_PPC_AES        0
#define KUMO_SIMD_VSX        0
#endif
