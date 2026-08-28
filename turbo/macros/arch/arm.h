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

#if defined(__aarch64__) || defined(__arm64__) || defined(_M_ARM64) || \
    defined(_M_ARM64EC)
#define KUMO_ARCH_ARM        1
#define KUMO_ARCH_ARM64      1
#define KUMO_ARCH_ARM32      0
#if defined(_M_ARM64EC)
#define KUMO_ARCH_ARM64EC    1
#else
#define KUMO_ARCH_ARM64EC    0
#endif

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#if !(defined(__NVCC__) && defined(__CUDACC__) && defined(__CUDA_ARCH__))
#define KUMO_SIMD_NEON       1
#else
#define KUMO_SIMD_NEON       0
#endif
#else
// AArch64 always has NEON in host/user mode when ACLE does not define it.
#if !(defined(__NVCC__) && defined(__CUDACC__) && defined(__CUDA_ARCH__))
#define KUMO_SIMD_NEON       1
#else
#define KUMO_SIMD_NEON       0
#endif
#endif

#if defined(__ARM_FEATURE_SVE)
#define KUMO_SIMD_SVE        1
#else
#define KUMO_SIMD_SVE        0
#endif
#if defined(__ARM_FEATURE_SVE2)
#define KUMO_SIMD_SVE2       1
#else
#define KUMO_SIMD_SVE2       0
#endif

#if (defined(__ARM_FEATURE_CRYPTO) || defined(__ARM_FEATURE_AES)) && \
    !(defined(__NVCC__) && defined(__CUDACC__) && defined(__CUDA_ARCH__))
#define KUMO_SIMD_ARM_AES        1
#else
#define KUMO_SIMD_ARM_AES        0
#endif

#define KUMO_CACHELINE_SIZE 64

#if KUMO_SIMD_SVE2
#define KUMO_SIMD_LEVEL      "SVE2"
#elif KUMO_SIMD_SVE
#define KUMO_SIMD_LEVEL      "SVE"
#elif KUMO_SIMD_NEON
#define KUMO_SIMD_LEVEL      "NEON"
#else
#define KUMO_SIMD_LEVEL      "NONE"
#endif

#if defined(_M_ARM64EC)
#define KUMO_ARCH_NAME       "ARM64EC"
#else
#define KUMO_ARCH_NAME       "ARM64"
#endif


#elif (defined(__arm__) || defined(__ARMEL__) || defined(_M_ARM)) && \
    !defined(__aarch64__) && !defined(_M_ARM64) && !defined(_M_ARM64EC)
#define KUMO_ARCH_ARM        1
#define KUMO_ARCH_ARM64      0
#define KUMO_ARCH_ARM32      1

#if (defined(__ARM_NEON) || defined(__ARM_NEON__)) && \
    !(defined(__NVCC__) && defined(__CUDACC__) && defined(__CUDA_ARCH__))
#define KUMO_SIMD_NEON       1
#else
#define KUMO_SIMD_NEON       0
#endif

#if defined(__ARM_FEATURE_SVE)
#define KUMO_SIMD_SVE        1
#else
#define KUMO_SIMD_SVE        0
#endif
#if defined(__ARM_FEATURE_SVE2)
#define KUMO_SIMD_SVE2       1
#else
#define KUMO_SIMD_SVE2       0
#endif

#if (defined(__ARM_FEATURE_CRYPTO) || defined(__ARM_FEATURE_AES)) && \
    !(defined(__NVCC__) && defined(__CUDACC__) && defined(__CUDA_ARCH__))
#define KUMO_SIMD_ARM_AES        1
#else
#define KUMO_SIMD_ARM_AES        0
#endif

// Match KUMO_CACHELINE_SIZE in turbo/base/optimization.h.
#if defined(__ARM_ARCH_5T__)
#define KUMO_CACHELINE_SIZE 32
#elif defined(__ARM_ARCH_7A__)
#define KUMO_CACHELINE_SIZE 64
#else
#define KUMO_CACHELINE_SIZE 64
#endif

#if KUMO_SIMD_NEON
#define KUMO_SIMD_LEVEL      "NEON"
#else
#define KUMO_SIMD_LEVEL      "NONE"
#endif

#define KUMO_ARCH_NAME       "ARM32"

#else

#define KUMO_ARCH_ARM        0
#define KUMO_ARCH_ARM32      0
#define KUMO_ARCH_ARM64      0
#define KUMO_ARCH_ARM64EC    0
#define KUMO_SIMD_NEON       0
#define KUMO_SIMD_SVE        0
#define KUMO_SIMD_SVE2     0
#define KUMO_SIMD_ARM_AES        0

#endif

