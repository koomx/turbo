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
//////////////////////////////////////////////////////////////////////////////////
/// File: config.h
/// -----------------------------------------------------------------------------
/// turbo config
#pragma once

// Included for the __GLIBC_PREREQ macro used below.
#include <limits.h>

// Included for the _STLPORT_VERSION macro used below.
#if defined(__cplusplus)
#include <cstddef>
#endif


// Included for the __GLIBC__ macro (or similar macros on other systems).
#include <turbo/macros/macros.h>

#ifdef __cplusplus
// Included for __GLIBCXX__, _LIBCPP_VERSION
#include <cstddef>
#endif  // __cplusplus


#if KUMO_CPLUSPLUS_LANG >= 202002L
// Include library feature test macros.
#include <version>
#endif

#if defined(__APPLE__)
// Included for TARGET_OS_IPHONE, __IPHONE_OS_VERSION_MIN_REQUIRED,
// __IPHONE_8_0.
#include <Availability.h>
#include <TargetConditionals.h>
#endif


// -----------------------------------------------------------------------------
// Operating System Check
// -----------------------------------------------------------------------------

#if defined(__CYGWIN__)
#error "Cygwin is not supported."
#endif

// -----------------------------------------------------------------------------
// Toolchain Check
// -----------------------------------------------------------------------------

// We support Visual Studio 2022 (MSVC++ 17.0) and later.
// This minimum will go up.
#if defined(_MSC_VER) && _MSC_VER < 1930 && !defined(__clang__)
#error "This package requires Visual Studio 2022 (MSVC++ 17.0) or higher."
#endif

// We support GCC 9 and later.
// This minimum will go up.
#if defined(__GNUC__) && !defined(__clang__)
#if __GNUC__ < 9
#error "This package requires GCC 10 or higher."
#endif
#endif

// We support Apple Xcode clang 4.2.1 (version 421.11.65) and later.
// This corresponds to Apple Xcode version 4.5.
// This minimum will go up.
#if defined(__apple_build_version__) && __apple_build_version__ < 4211165
#error "This package requires __apple_build_version__ of 4211165 or higher."
#endif

// -----------------------------------------------------------------------------
// C++ Version Check
// -----------------------------------------------------------------------------

// Enforce C++17 as the minimum.
#if defined(_MSVC_LANG)
#if _MSVC_LANG < 201703L
#error "C++ versions less than C++17 are not supported."
#endif  // _MSVC_LANG < 201703L
#elif defined(__cplusplus)
#if __cplusplus < 201703L
#error "C++ versions less than C++17 are not supported."
#endif  // __cplusplus < 201703L
#endif

// -----------------------------------------------------------------------------
// Standard Library Check
// -----------------------------------------------------------------------------

#if defined(_STLPORT_VERSION)
#error "STLPort is not supported."
#endif

// -----------------------------------------------------------------------------
// `char` Size Check
// -----------------------------------------------------------------------------

// Abseil currently assumes CHAR_BIT == 8. If you would like to use Abseil on a
// platform where this is not the case, please provide us with the details about
// your platform so we can consider relaxing this requirement.
#if CHAR_BIT != 8
#error "Turbo assumes CHAR_BIT == 8."
#endif

// -----------------------------------------------------------------------------
// `int` Size Check
// -----------------------------------------------------------------------------

// Abseil currently assumes that an int is 4 bytes. If you would like to use
// Abseil on a platform where this is not the case, please provide us with the
// details about your platform so we can consider relaxing this requirement.
#if INT_MAX < 2147483647
#error "Abseil assumes that int is at least 4 bytes. "
#endif


#define TURBO_INTERNAL_C_SYMBOL(x) x


// TURBO_INTERNAL_MANGLED_NS
// TURBO_INTERNAL_MANGLED_BACKREFERENCE
//
// Internal macros for building up mangled names in our internal fork of CCTZ.
// This implementation detail is only needed and provided for the MSVC build.
//
// These macros both expand to string literals.  TURBO_INTERNAL_MANGLED_NS is
// the mangled spelling of the `turbo` namespace, and
// TURBO_INTERNAL_MANGLED_BACKREFERENCE is a back-reference integer representing
// the proper count to skip past the CCTZ fork namespace names.  (This number
// is one larger when there is an inline namespace name to skip.)
#if defined(_MSC_VER)
#define TURBO_INTERNAL_MANGLED_NS "turbo"
#define TURBO_INTERNAL_MANGLED_BACKREFERENCE "5"
#endif
