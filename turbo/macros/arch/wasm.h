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

#if defined(__wasm__) || defined(__asmjs__)

#define KUMO_ARCH_WASM       1

#if defined(__wasm64__)
#define KUMO_ARCH_WASM64     1
#define KUMO_ARCH_WASM32     0
#else
#define KUMO_ARCH_WASM64     0
#define KUMO_ARCH_WASM32     1
#endif

#define KUMO_CACHELINE_SIZE 64

#define KUMO_SIMD_LEVEL      "NONE"

#if KUMO_ARCH_WASM64
#define KUMO_ARCH_NAME       "wasm64"
#else
#define KUMO_ARCH_NAME       "wasm32"
#endif
#else
#define KUMO_ARCH_WASM       0
#define KUMO_ARCH_WASM64     0
#define KUMO_ARCH_WASM32     0
#endif
