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

#include <turbo/macros/utility/compiler_check.h>
#include <turbo/macros/compiler/compiler.h>
#include <turbo/macros/have/base.h>

// KUMO_HAVE_EXCEPTIONS
//
// Checks whether the compiler both supports and enables exceptions. Always
// defined to 0 or 1 (use #if). In C mode this is always 0.
//
// Generally, when KUMO_HAVE_EXCEPTIONS is 0:
//
// * Code using `throw` and `try` may not compile.
// * The `noexcept` specifier will still compile and behave as normal.
// * The `noexcept` operator may still return `false`.
//
// For further details, consult the compiler's documentation.
#ifdef KUMO_HAVE_EXCEPTIONS
#error KUMO_HAVE_EXCEPTIONS cannot be directly set.
#elif !defined(__cplusplus)
#define KUMO_HAVE_EXCEPTIONS 0
#elif KUMO_INTERNAL_HAVE_MIN_CLANG_VERSION(3, 6)
// Clang >= 3.6
#if KUMO_HAVE_FEATURE(cxx_exceptions)
#define KUMO_HAVE_EXCEPTIONS 1
#else
#define KUMO_HAVE_EXCEPTIONS 0
#endif
#elif defined(__clang__)
// Clang < 3.6
// http://releases.llvm.org/3.6.0/tools/clang/docs/ReleaseNotes.html#the-exceptions-macro
#if defined(__EXCEPTIONS) && KUMO_HAVE_FEATURE(cxx_exceptions)
#define KUMO_HAVE_EXCEPTIONS 1
#else
#define KUMO_HAVE_EXCEPTIONS 0
#endif
#elif !(defined(__GNUC__) && !defined(__cpp_exceptions)) && \
    !(defined(_MSC_VER) && !defined(_CPPUNWIND))
#define KUMO_HAVE_EXCEPTIONS 1
#else
#define KUMO_HAVE_EXCEPTIONS 0
#endif
