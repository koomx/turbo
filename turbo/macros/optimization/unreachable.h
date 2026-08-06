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
#include <stdlib.h>

#include <turbo/macros/compiler/compiler.h>
#include <turbo/macros/option.h>
#ifdef __cplusplus
// Included for std::unreachable()
#include <utility>
#endif  // __cplusplus

// `KUMO_INTERNAL_IMMEDIATE_ABORT_IMPL()` aborts the program in the fastest
// possible way, with no attempt at logging. One use is to implement hardening
// aborts with KUMO_OPTION_HARDENED.  Since this is an internal symbol, it
// should not be used directly outside of Abseil.
#if KUMO_HAVE_BUILTIN(__builtin_trap) || \
    (defined(__GNUC__) && !defined(__clang__))
#define KUMO_INTERNAL_IMMEDIATE_ABORT_IMPL() __builtin_trap()
#else
#define KUMO_INTERNAL_IMMEDIATE_ABORT_IMPL() abort()
#endif

// `KUMO_INTERNAL_UNREACHABLE_IMPL()` is the platform specific directive to
// indicate that a statement is unreachable, and to allow the compiler to
// optimize accordingly. Clients should use `KUMO_UNREACHABLE()`, which is
// defined below.
#if defined(__cplusplus) && defined(__cpp_lib_unreachable) && \
    __cpp_lib_unreachable >= 202202L
#define KUMO_INTERNAL_UNREACHABLE_IMPL() std::unreachable()
#elif defined(__GNUC__) || KUMO_HAVE_BUILTIN(__builtin_unreachable)
#define KUMO_INTERNAL_UNREACHABLE_IMPL() __builtin_unreachable()
#elif KUMO_HAVE_BUILTIN(__builtin_assume)
#define KUMO_INTERNAL_UNREACHABLE_IMPL() __builtin_assume(0)
#elif defined(_MSC_VER)
#define KUMO_INTERNAL_UNREACHABLE_IMPL() __assume(0)
#else
#define KUMO_INTERNAL_UNREACHABLE_IMPL() ((void)0)
#endif

// `KUMO_UNREACHABLE()` is an unreachable statement.  A program which reaches
// one has undefined behavior, and the compiler may optimize accordingly.
#if (KUMO_OPTION_HARDENED == 1 || KUMO_OPTION_HARDENED == 2) && defined(NDEBUG)
// Abort in hardened mode to avoid dangerous undefined behavior.
#define KUMO_UNREACHABLE()                \
  do {                                    \
    KUMO_INTERNAL_IMMEDIATE_ABORT_IMPL(); \
    KUMO_INTERNAL_UNREACHABLE_IMPL();     \
  } while (0)
#else
// The assert only fires in debug mode to aid in debugging.
// When NDEBUG is defined, reaching KUMO_UNREACHABLE() is undefined behavior.
#define KUMO_UNREACHABLE()                       \
  do {                                           \
    /* NOLINTNEXTLINE: misc-static-assert */     \
    assert(0 && "KUMO_UNREACHABLE reached");     \
    KUMO_INTERNAL_UNREACHABLE_IMPL();            \
  } while (0)
#endif
