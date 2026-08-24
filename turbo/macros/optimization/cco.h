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

#pragma once

//////////////////////////////////////////////////////////////////////////
/// Compiler control optimization
/// KUMO_CCO_*
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/// @def KUMO_CCO_BARRIER(var)
/// @brief Prevents unwanted compiler optimizations for @p var.
///
/// Uses an empty GCC inline assembly statement with a register constraint
/// which forces @p var into a general purpose register (e.g. eax, ebx, ecx
/// on x86) and marks it as modified.
///
/// This prevents autovectorization and constant folding where we want
/// explicit control via intrinsics.
///
#if defined(__GNUC__) || defined(__clang__)
#  define KUMO_CCO_BARRIER(var) do { __asm__("" : "+r" (var)); } while(0)
#else
#  define KUMO_CCO_BARRIER(var) ((void)0)
#endif



//////////////////////////////////////////////////////////////////////////
/// @brief Whether to prioritize code size over speed.
///
/// When enabled (set to 1), the library will use compact loops and avoid
/// loop unrolling to reduce the binary size. This is useful for embedded
/// systems or environments with strict size constraints.
///
/// When disabled (set to 0), the library will prioritize execution speed
/// by using loop unrolling and other performance optimizations.
///
/// By default, this macro is automatically set to 1 when compiling with
/// -Os or -Oz (optimize for size) on GCC or Clang. Users can override
/// this by defining KUMO_CCO_SIZE_OPT before including the header.
///
/// @note This macro is intended for compile-time optimization control.
///       Changing it after inclusion has no effect.
///
/// @example
///   // Force size-optimized build:
///   #define KUMO_CCO_SIZE_OPT 1
///   #include <kumo/macros/macros.h>
///
///   // Force speed-optimized build:
///   #define KUMO_CCO_SIZE_OPT 0
///   #include <kumo/macros/macros.h>
#ifndef KUMO_CCO_SIZE_OPT
   /// default to 1 for -Os or -Oz
#  if (defined(__GNUC__) || defined(__clang__)) && defined(__OPTIMIZE_SIZE__)
#    define KUMO_CCO_SIZE_OPT 1
#  else
#    define KUMO_CCO_SIZE_OPT 0
#  endif
#elif KUMO_CCO_SIZE_OPT !=0 && KUMO_CCO_SIZE_OPT != 1
#error "KUMO_CCO_SIZE_OPT must be defined as 0 or 1"
#endif
