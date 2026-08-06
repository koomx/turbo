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
// -----------------------------------------------------------------------------
// File: cache_line.h
// -----------------------------------------------------------------------------
//
// Cache line alignment macros.  KUMO_CACHELINE_SIZE is defined by each
// architecture header in <xmacros/arch/>; this file provides the alignment
// annotation macro that uses it.

#pragma once

#include <turbo/macros/arch/arch.h>

// ---------------------------------------------------------------------------
// KUMO_CACHELINE_ALIGNED
//
// Aligns a variable or struct to the L1 cache line boundary.
//
//   alignas(KUMO_CACHELINE_ALIGNED) int counter;
//   struct KUMO_CACHELINE_ALIGNED AlignedData { int x; };
// ---------------------------------------------------------------------------

// Match KUMO_CACHELINE_ALIGNED: attribute on GCC/Clang/MSVC, else empty.
#if defined(__clang__) || defined(__GNUC__)
#define KUMO_CACHELINE_ALIGNED  __attribute__((aligned(KUMO_CACHELINE_SIZE)))
#elif defined(_MSC_VER)
#define KUMO_CACHELINE_ALIGNED  __declspec(align(KUMO_CACHELINE_SIZE))
#else
#define KUMO_CACHELINE_ALIGNED
#endif
