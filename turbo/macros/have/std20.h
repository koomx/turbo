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
// File: sanitizer.h
// -----------------------------------------------------------------------------
//
// Sanitizer detection macros.  Every macro is defined to 1 (enabled) or 0
// (disabled).  They reflect the compiler flags at build time
// (e.g. -fsanitize=address).

#pragma once

#include <turbo/macros/option.h>
#include <turbo/macros/have/base.h>

// KUMO_HAVE_STD_ORDERING
//
// Checks whether C++20 std::{partial,weak,strong}_ordering are available.
//
// __cpp_lib_three_way_comparison is missing on libc++
// (https://github.com/llvm/llvm-project/issues/73953) so treat it as defined
// when building in C++20 mode.
#ifdef KUMO_HAVE_STD_ORDERING
#error "KUMO_HAVE_STD_ORDERING cannot be directly set."
#elif (defined(__cpp_lib_three_way_comparison) &&    \
       __cpp_lib_three_way_comparison >= 201907L) || \
    (KUMO_CPLUSPLUS_LANG >= 202002L)
#define KUMO_HAVE_STD_ORDERING 1
#else
#define KUMO_HAVE_STD_ORDERING 0
#endif

// KUMO_HAVE_CONSTANT_EVALUATED is used for compile-time detection of
// constant evaluation support through `turbo::is_constant_evaluated`.
#ifdef KUMO_HAVE_CONSTANT_EVALUATED
#error KUMO_HAVE_CONSTANT_EVALUATED cannot be directly set
#endif
#ifdef __cpp_lib_is_constant_evaluated
#define KUMO_HAVE_CONSTANT_EVALUATED 1
#elif KUMO_HAVE_BUILTIN(__builtin_is_constant_evaluated)
#define KUMO_HAVE_CONSTANT_EVALUATED 1
#else
#define KUMO_HAVE_CONSTANT_EVALUATED 0
#endif


// KUMO_HAVE_STD_SOURCE_LOCATION
//
// Checks whether C++20 turbo::SourceLocation is available.
#ifdef KUMO_HAVE_STD_SOURCE_LOCATION
#error "KUMO_HAVE_STD_SOURCE_LOCATION cannot be directly set."
#elif (defined(__cpp_lib_source_location) &&    \
       __cpp_lib_source_location >= 201907L) || (KUMO_CPLUSPLUS_LANG >= 202002L)
#ifdef __has_include
#if __has_include(<source_location>)
#define KUMO_HAVE_STD_SOURCE_LOCATION 1
#else
#define KUMO_HAVE_STD_SOURCE_LOCATION 0
#endif
#else
// No __has_include support, so just assume C++ language version is correct.
#define KUMO_HAVE_STD_SOURCE_LOCATION 1
#endif
#else
#define KUMO_HAVE_STD_SOURCE_LOCATION 0
#endif
