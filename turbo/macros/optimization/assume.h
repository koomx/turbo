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

#include <assert.h>

#include <turbo/macros/compiler/compiler.h>
#include <turbo/macros/optimization/like.h>

// KUMO_ASSUME(cond)
//
// Informs the compiler that a condition is always true and that it can assume
// it to be true for optimization purposes.
//
// WARNING: If the condition is false, the program can produce undefined and
// potentially dangerous behavior.
//
// In !NDEBUG mode, the condition is checked with an assert().
//
// NOTE: The expression must not have side effects, as it may only be evaluated
// in some compilation modes and not others. Some compilers may issue a warning
// if the compiler cannot prove the expression has no side effects. For example,
// the expression should not use a function call since the compiler cannot prove
// that a function call does not have side effects.
//
// Example:
//
//   int x = ...;
//   KUMO_ASSUME(x >= 0);
//   // The compiler can optimize the division to a simple right shift using the
//   // assumption specified above.
//   int y = x / 16;
//
#if !defined(NDEBUG)
#ifdef __cplusplus
#define KUMO_ASSUME(cond) \
  (KUMO_LIKELY((cond)) ? void() : assert(false && #cond))  // NOLINT
#else
#define KUMO_ASSUME(cond) \
  (KUMO_LIKELY((cond)) ? (void)0 : (assert(0 && #cond), (void)0))
#endif
#elif KUMO_HAVE_BUILTIN(__builtin_assume)
#define KUMO_ASSUME(cond) __builtin_assume(cond)
#elif defined(_MSC_VER)
#define KUMO_ASSUME(cond) __assume(cond)
#elif defined(__cplusplus) && defined(__cpp_lib_unreachable) && \
    __cpp_lib_unreachable >= 202202L
#define KUMO_ASSUME(cond) ((cond) ? void() : std::unreachable())
#elif defined(__GNUC__) || KUMO_HAVE_BUILTIN(__builtin_unreachable)
#ifdef __cplusplus
#define KUMO_ASSUME(cond) ((cond) ? void() : __builtin_unreachable())
#else
#define KUMO_ASSUME(cond) ((cond) ? (void)0 : __builtin_unreachable())
#endif
#elif defined(__cplusplus) && __cplusplus >= 202002L
// Unimplemented. Uses the same definition as KUMO_ASSERT in the NDEBUG case.
#define KUMO_ASSUME(expr) (decltype((expr) ? void() : void())())
#elif defined(__cplusplus)
#define KUMO_ASSUME(expr) (false ? ((expr) ? void() : void()) : void())
#else
#define KUMO_ASSUME(cond) ((void)(cond))
#endif
