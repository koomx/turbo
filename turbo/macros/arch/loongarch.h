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

#if defined(__loongarch64) || defined(__loongarch__)

#define KUMO_ARCH_LOONGARCH  1

#if defined(__loongarch64) || (defined(__loongarch_grlen) && __loongarch_grlen == 64)
#define KUMO_ARCH_LOONGARCH64 1
#define KUMO_ARCH_LOONGARCH32 0
#else
#define KUMO_ARCH_LOONGARCH64 0
#define KUMO_ARCH_LOONGARCH32 1
#endif

#if defined(__LSX__)
#define KUMO_SIMD_LSX        1
#else
#define KUMO_SIMD_LSX        0
#endif
#if defined(__LASX__)
#define KUMO_SIMD_LASX       1
#else
#define KUMO_SIMD_LASX       0
#endif

#define KUMO_CACHELINE_SIZE 64

#if KUMO_SIMD_LASX
#define KUMO_SIMD_LEVEL      "LASX"
#elif KUMO_SIMD_LSX
#define KUMO_SIMD_LEVEL      "LSX"
#else
#define KUMO_SIMD_LEVEL      "NONE"
#endif

#if KUMO_ARCH_LOONGARCH64
#define KUMO_ARCH_NAME       "LoongArch64"
#else
#define KUMO_ARCH_NAME       "LoongArch32"
#endif
#else
#define KUMO_ARCH_LOONGARCH  0
#define KUMO_ARCH_LOONGARCH64 0
#define KUMO_ARCH_LOONGARCH32 0
#define KUMO_SIMD_LSX        0
#define KUMO_SIMD_LASX       0

#endif
