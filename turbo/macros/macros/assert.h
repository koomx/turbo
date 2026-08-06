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
// File: assert.h
// -----------------------------------------------------------------------------
//
// Debug and always-active assertion macros.  All macros are C/C++ compatible.
//
// KUMO_DASSERT(expr)           — debug only, abort on failure
// KUMO_ASSERT(expr)            — always active (unless NDEBUG), abort on failure
// KUMO_HARDENING_ASSERT(expr)  — also active under NDEBUG when hardened
//
// On failure, the expression string is passed to assert() / hardening abort.

#pragma once

#include <assert.h>
#include <stdlib.h>

#include <turbo/macros/have/exception.h>
#include <turbo/macros/option.h>
#include <turbo/macros/optimization/like.h>
#include <turbo/macros/optimization/unreachable.h>

// KUMO_BAD_CALL_IF()
//
// Used on a function overload to trap bad calls: any call that matches the
// overload will cause a compile-time error. This macro uses a clang-specific
// "enable_if" attribute, as described at
// https://clang.llvm.org/docs/AttributeReference.html#enable-if
//
// Overloads which use this macro should be bracketed by
// `#ifdef KUMO_BAD_CALL_IF`.
//
// Example:
//
//   int isdigit(int c);
//   #ifdef KUMO_BAD_CALL_IF
//   int isdigit(int c)
//     KUMO_BAD_CALL_IF(c <= -1 || c > 255,
//                       "'c' must have the value of an unsigned char or EOF");
//   #endif // KUMO_BAD_CALL_IF
#ifdef __cplusplus
#if KUMO_HAVE_ATTRIBUTE(enable_if)
#define KUMO_BAD_CALL_IF(expr, msg) \
  __attribute__((enable_if(expr, "Bad call trap"), unavailable(msg)))
#endif
#endif  // __cplusplus

// `KUMO_INTERNAL_HARDENING_ABORT()` controls how `KUMO_HARDENING_ASSERT()`
// aborts the program in release mode (when NDEBUG is defined). The
// implementation should abort the program as quickly as possible and ideally it
// should not be possible to ignore the abort request.
#if defined(__CUDACC__) || defined(__CUDA_ARCH__) || defined(__CUDA__) || \
    !defined(__cplusplus)
#define KUMO_INTERNAL_HARDENING_ABORT()   \
  do {                                    \
    KUMO_INTERNAL_IMMEDIATE_ABORT_IMPL(); \
    KUMO_INTERNAL_UNREACHABLE_IMPL();     \
  } while (0)
#else
namespace turbo {
namespace base_internal {
#if KUMO_HAVE_CPP_ATTRIBUTE(clang::nomerge)
[[clang::nomerge]]  // Needed when this function is not inlined
#endif
[[noreturn]] inline void HardeningAbort() {
#if KUMO_HAVE_CPP_ATTRIBUTE(clang::nomerge)
  [[clang::nomerge]]  // Needed when this function is inlined
#endif
  KUMO_INTERNAL_IMMEDIATE_ABORT_IMPL();
  KUMO_INTERNAL_UNREACHABLE_IMPL();
}
}  // namespace base_internal
}  // namespace turbo

#define KUMO_INTERNAL_HARDENING_ABORT() ::turbo::base_internal::HardeningAbort()
#endif

// KUMO_INTERNAL_UNEVALUATED()
//
// Expands into a no-op expression that contains the given expression. Used to
// avoid unused-variable warnings in configurations that don't need to evaluate
// the given expression (e.g., NDEBUG).
#ifdef __cplusplus
#if KUMO_CPLUSPLUS_LANG >= 202002L
// We use `decltype` here to avoid generating unnecessary code that the
// optimizer then has to optimize away.
// This not only improves compilation performance by reducing codegen bloat
// and optimization work, but also guarantees fast run-time performance without
// having to rely on the optimizer.
#define KUMO_INTERNAL_UNEVALUATED(expr) (decltype((void)(expr))())
#else
// Pre-C++20, lambdas can't be inside unevaluated operands, so we're forced to
// rely on the optimizer.
#define KUMO_INTERNAL_UNEVALUATED(expr) (false ? (void)(expr) : void())
#endif
#else  // !__cplusplus
// sizeof does not evaluate its operand (except VLAs).
#define KUMO_INTERNAL_UNEVALUATED(expr) ((void)sizeof((expr), 0))
#endif  // __cplusplus

// KUMO_ASSERT()
//
// In C++11, `assert` can't be used portably within constexpr functions.
// `assert` also generates spurious unused-symbol warnings.
// KUMO_ASSERT functions as a runtime assert but works in C++11 constexpr
// functions, and maintains references to symbols.  Example:
//
// constexpr double Divide(double a, double b) {
//   return KUMO_ASSERT(b != 0), a / b;
// }
//
// This macro is inspired by
// https://akrzemi1.wordpress.com/2017/05/18/asserts-in-constexpr-functions/
//
// In C, the macro is an expression that aborts via assert() on failure.
#if defined(NDEBUG)
#ifdef __cplusplus
#define KUMO_ASSERT(expr) KUMO_INTERNAL_UNEVALUATED((expr) ? void() : void())
#else
#define KUMO_ASSERT(expr) KUMO_INTERNAL_UNEVALUATED(expr)
#endif
#elif defined(__cplusplus)
#define KUMO_ASSERT(expr)                   \
  (KUMO_LIKELY((expr)) ? static_cast<void>(0) \
                       : assert(false && #expr))  // NOLINT
#else
#define KUMO_ASSERT(expr) \
  (KUMO_LIKELY((expr)) ? (void)0 : (assert(0 && #expr), (void)0))
#endif

// KUMO_HARDENING_ASSERT()
//
// `KUMO_HARDENING_ASSERT()` is like `KUMO_ASSERT()`, but used to implement
// runtime assertions that should be enabled in hardened builds even when
// `NDEBUG` is defined.
//
// When `NDEBUG` is not defined, `KUMO_HARDENING_ASSERT()` is identical to
// `KUMO_ASSERT()`.
//
// See `KUMO_OPTION_HARDENED` in `turbo/macros/option.h` for more information on
// hardened mode.
#if (KUMO_OPTION_HARDENED == 1 || KUMO_OPTION_HARDENED == 2) && defined(NDEBUG)
#define KUMO_HARDENING_ASSERT(expr)     \
  do {                                  \
    if (!KUMO_LIKELY((expr))) {         \
      KUMO_INTERNAL_HARDENING_ABORT();  \
    }                                   \
  } while (0)
#else
#define KUMO_HARDENING_ASSERT(expr) KUMO_ASSERT(expr)
#endif

// KUMO_HARDENING_ASSERT_SLOW()
//
// Like `KUMO_HARDENING_ASSERT()`, but specifically for assertions whose
// predicates are too slow to be enabled in many applications.
//
// When `NDEBUG` is not defined, `KUMO_HARDENING_ASSERT_SLOW()` is identical to
// `KUMO_ASSERT()`.
//
// See `KUMO_OPTION_HARDENED` in `turbo/macros/option.h` for more information on
// hardened mode.
#if KUMO_OPTION_HARDENED == 1 && defined(NDEBUG)
#define KUMO_HARDENING_ASSERT_SLOW(expr) KUMO_HARDENING_ASSERT(expr)
#else
#define KUMO_HARDENING_ASSERT_SLOW(expr) KUMO_ASSERT(expr)
#endif

#if !defined(NDEBUG)
#define KUMO_DASSERT KUMO_ASSERT
#else
#define KUMO_DASSERT(expr) ((void)0)
#endif

// ---------------------------------------------------------------------------
// Comparison convenience macros
// ---------------------------------------------------------------------------

#define KUMO_DASSERT_EQ(a, b) KUMO_DASSERT((a) == (b))
#define KUMO_DASSERT_NE(a, b) KUMO_DASSERT((a) != (b))
#define KUMO_DASSERT_LT(a, b) KUMO_DASSERT((a) < (b))
#define KUMO_DASSERT_LE(a, b) KUMO_DASSERT((a) <= (b))
#define KUMO_DASSERT_GT(a, b) KUMO_DASSERT((a) > (b))
#define KUMO_DASSERT_GE(a, b) KUMO_DASSERT((a) >= (b))

#define KUMO_ASSERT_EQ(a, b) KUMO_ASSERT((a) == (b))
#define KUMO_ASSERT_NE(a, b) KUMO_ASSERT((a) != (b))
#define KUMO_ASSERT_LT(a, b) KUMO_ASSERT((a) < (b))
#define KUMO_ASSERT_LE(a, b) KUMO_ASSERT((a) <= (b))
#define KUMO_ASSERT_GT(a, b) KUMO_ASSERT((a) > (b))
#define KUMO_ASSERT_GE(a, b) KUMO_ASSERT((a) >= (b))

// ---------------------------------------------------------------------------
// Pointer convenience macros
// ---------------------------------------------------------------------------

#define KUMO_DASSERT_NULL(p) KUMO_DASSERT((p) == ((void*)0))
#define KUMO_DASSERT_NOT_NULL(p) KUMO_DASSERT((p) != ((void*)0))
#define KUMO_ASSERT_NULL(p) KUMO_ASSERT((p) == ((void*)0))
#define KUMO_ASSERT_NOT_NULL(p) KUMO_ASSERT((p) != ((void*)0))

// Exception try/catch helpers (C++ only for real exceptions).
#if defined(__cplusplus) && KUMO_HAVE_EXCEPTIONS
#define KUMO_INTERNAL_TRY try
#define KUMO_INTERNAL_CATCH_ANY catch (...)
#define KUMO_INTERNAL_RETHROW \
  do {                        \
    throw;                    \
  } while (0)
#else
#define KUMO_INTERNAL_TRY if (1)
#define KUMO_INTERNAL_CATCH_ANY else if (0)
#define KUMO_INTERNAL_RETHROW \
  do {                        \
  } while (0)
#endif
