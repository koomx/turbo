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


// KUMO_OPTION_USE_STD_SOURCE_LOCATION
//
// This option controls whether turbo::SourceLocation is implemented as an alias
// to the turbo::SourceLocation type, or as an independent implementation.
//
// A value of 0 means to use library's implementation.  This requires only C++17
// support, and is expected to run on every toolchain we support, and to
// properly capture source location information on every toolchain that supports
// the necessary built-ins (such as `__builtin_LINE`).
//
// A value of 1 means to use aliases.  This requires that all code using library
// is built in C++20 mode or later.
//
// A value of 2 means to detect the C++ version being used to compile library,
// and use an alias only if working turbo::SourceLocation types are available.
// This option is useful when you are building your program from source.  It
// should not be used otherwise -- for example, if you are distributing library
// in a binary package manager -- since in mode 2, they will name different
// types, with different mangled names and binary layout, depending on the
// compiler flags passed by the end user.
//
// User code should not inspect this macro.  To check in the preprocessor if
// the source location type is an alias of turbo::SourceLocation type, use the
// feature macro KUMO_USES_STD_SOURCE_LOCATION.
//

#ifndef KUMO_OPTION_USE_STD_SOURCE_LOCATION
#define KUMO_OPTION_USE_STD_SOURCE_LOCATION 2
#elif KUMO_OPTION_USE_STD_SOURCE_LOCATION != 0 && KUMO_OPTION_USE_STD_SOURCE_LOCATION != 1 && KUMO_OPTION_USE_STD_SOURCE_LOCATION != 2
#error KUMO_OPTION_HARDENED must be 0 or 1 or 2
#endif


// KUMO_OPTION_HARDENED
//
// This option enables a "hardened" build in release mode (in this context,
// release mode is defined as a build where the `NDEBUG` macro is defined).
//
// A value of 0 means that "hardened" mode is not enabled.
//
// A value of 1 means that "hardened" mode is enabled with all checks.
//
// A value of 2 means that "hardened" mode is partially enabled, with
// only a subset of checks chosen to minimize performance impact.
//
// Hardened builds have additional security checks enabled when `NDEBUG` is
// defined. Defining `NDEBUG` is normally used to turn `assert()` macro into a
// no-op, as well as disabling other bespoke program consistency checks. By
// defining KUMO_OPTION_HARDENED to 1, a select set of checks remain enabled in
// release mode. These checks guard against programming errors that may lead to
// security vulnerabilities. In release mode, when one of these programming
// errors is encountered, the program will immediately abort, possibly without
// any attempt at logging.
//
// The checks enabled by this option are not free; they do incur runtime cost.
//
// The checks enabled by this option are always active when `NDEBUG` is not
// defined, even in the case when KUMO_OPTION_HARDENED is defined to 0. The
// checks enabled by this option may abort the program in a different way and
// log additional information when `NDEBUG` is not defined.
#ifndef KUMO_OPTION_HARDENED
#define KUMO_OPTION_HARDENED 0
#elif KUMO_OPTION_HARDENED != 0 && KUMO_OPTION_HARDENED != 1 && KUMO_OPTION_HARDENED != 2
#error KUMO_OPTION_HARDENED must be 0 or 1 or 2
#endif


// KUMO_OPTION_USE_STD_ORDERING
//
// This option controls whether turbo::{partial,weak,strong}_ordering are
// implemented as aliases to the std:: ordering types, or as an independent
// implementation.
//
// A value of 0 means to use library's implementation.  This is expected to
// work on every toolchain we support.
//
// A value of 1 means to use aliases.  This requires that all code using library
// is built in C++20 mode or later.
//
// A value of 2 means to detect the C++ version being used to compile library,
// and use an alias only if working std:: ordering types are available.  This
// option is useful when you are building your program from source.  It should
// not be used otherwise -- for example, if you are distributing library in a
// binary package manager -- since in mode 2, they will name different types,
// with different mangled names and binary layout, depending on the compiler
// flags passed by the end user.
//
// User code should not inspect this macro.  To check in the preprocessor if
// the ordering types are aliases of std:: ordering types, use the feature macro
// KUMO_USES_STD_ORDERING.
#ifndef KUMO_OPTION_USE_STD_ORDERING
#define KUMO_OPTION_USE_STD_ORDERING 2
#elif KUMO_OPTION_USE_STD_ORDERING != 0 && KUMO_OPTION_USE_STD_ORDERING != 1 && KUMO_OPTION_USE_STD_ORDERING != 2
#error KUMO_OPTION_USE_STD_ORDERING must be 0 or 1 or 2
#endif

#ifndef KUMO_BUILD_DLL
#define KUMO_BUILD_DLL 0
#elif KUMO_BUILD_DLL != 0 && KUMO_BUILD_DLL != 1
#error KUMO_BUILD_DLL must be 0 or 1
#endif

#ifndef KUMO_CONSUME_DLL
#define KUMO_CONSUME_DLL 0
#elif KUMO_CONSUME_DLL != 0 && KUMO_CONSUME_DLL != 1
#error KUMO_CONSUME_DLL must be 0 or 1
#endif


#ifndef KUMO_BUILD_TEST_DLL
#define KUMO_BUILD_TEST_DLL 0
#elif KUMO_BUILD_TEST_DLL != 0 && KUMO_BUILD_TEST_DLL != 1
#error KUMO_BUILD_TEST_DLL must be 0 or 1
#endif

#ifndef KUMO_CONSUME_TEST_DLL
#define KUMO_CONSUME_TEST_DLL 0
#elif KUMO_CONSUME_TEST_DLL != 0 && KUMO_CONSUME_TEST_DLL != 1
#error KUMO_CONSUME_TEST_DLL must be 0 or 1
#endif

// KUMO_OPTION_INLINE_HW_ACCEL_STRATEGY
//
// This option controls whether library is allowed to use non-portable
// hardware-accelerated implementations in headers (where they are typically
// inlined into the caller's translation unit).
//
// Using such optimizations in headers can lead to One Definition Rule (ODR)
// violations if different translation units are built with different CPU
// architecture flags (e.g., -march=native vs -march=generic) and linked
// together.
//
// A value of 0 means to use the portable software implementation in headers.
// This provides the best ODR guarantees when linking code built with
// inconsistent flags, but may be slower.
//
// A value of 1 means that the implementation requires the use of a
// hardware-accelerated implementation. This requires the compiler environment
// to support these instructions; otherwise, the build will fail.
//
// A value of 2 means to select the best available implementation based on
// the compiler flags, but can't guarantee ODR safety if translation units are
// built with inconsistent flags.
//
// User code should not inspect this macro.
#define KUMO_OPTION_INLINE_HW_ACCEL_STRATEGY 0
