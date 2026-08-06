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
// File: optimization.h
// -----------------------------------------------------------------------------
//
// Portable macros for performance optimization — branch prediction hints,
// compiler assumptions, unreachable-code markers, and tail-call suppression.

#pragma once

#include <assert.h>

#ifdef __cplusplus
#include <utility>  // std::unreachable
#endif

#include <turbo/macros/optimization/like.h>
#include <turbo/macros/optimization/assume.h>
#include <turbo/macros/optimization/unreachable.h>
#include <turbo/macros/optimization/unique_name.h>

// KUMO_BLOCK_TAIL_CALL_OPTIMIZATION
//
// Instructs the compiler to avoid optimizing tail-call recursion. This macro is
// useful when you wish to preserve the existing function order within a stack
// trace for logging, debugging, or profiling purposes.
//
// Example:
//
//   int f() {
//     int result = g();
//     KUMO_BLOCK_TAIL_CALL_OPTIMIZATION();
//     return result;
//   }
#if defined(__clang__)
// Clang will not tail call given inline volatile assembly.
#define KUMO_BLOCK_TAIL_CALL_OPTIMIZATION() __asm__ __volatile__("")
#elif defined(__GNUC__)
// GCC will not tail call given inline volatile assembly.
#define KUMO_BLOCK_TAIL_CALL_OPTIMIZATION() __asm__ __volatile__("")
#elif defined(_MSC_VER)
#include <intrin.h>
// The __nop() intrinsic blocks the optimisation.
#define KUMO_BLOCK_TAIL_CALL_OPTIMIZATION() __nop()
#else
#define KUMO_BLOCK_TAIL_CALL_OPTIMIZATION() \
  do {                                      \
    volatile int kumo_block_tail_call_x_ = 0; \
    (void)kumo_block_tail_call_x_;          \
  } while (0)
#endif

