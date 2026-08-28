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

#define KUMO_ARCH_S390       1

#if defined(__s390x__)
#define KUMO_ARCH_S390X      1
#define KUMO_ARCH_S390_31    0
#else
#define KUMO_ARCH_S390X      0
#define KUMO_ARCH_S390_31    1
#endif

// Match TURBO default (optimization.h falls through to 64).
#define KUMO_CACHELINE_SIZE 64

#define KUMO_SIMD_LEVEL      "NONE"

#if KUMO_ARCH_S390X
#define KUMO_ARCH_NAME       "s390x"
#else
#define KUMO_ARCH_NAME       "s390"
#endif
#else
#define KUMO_ARCH_S390X      0
#define KUMO_ARCH_S390_31    0
#define KUMO_ARCH_S390       0
#endif
