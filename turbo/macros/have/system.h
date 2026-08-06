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
// File: have_ext.h
// -----------------------------------------------------------------------------
//
// Extended feature-detection macros — C++-standard-library traits, language
// features, POSIX availability, and version-comparison helpers.
//
// Every object-like macro is defined to 1 (available) or 0 (unavailable).
// C++-only macros are guarded and defined to 0 in C mode.

#pragma once

#include <turbo/macros/compiler/compiler.h>
#include <turbo/macros/have/base.h>

// ===========================================================================
// C++ standard library traits
// ===========================================================================
//
// These rely on <type_traits> (C++11+).  In C mode they are unconditionally 0.

// ---------------------------------------------------------------------------
// KUMO_HAVE_STD_IS_TRIVIALLY_DESTRUCTIBLE
// KUMO_HAVE_STD_IS_TRIVIALLY_CONSTRUCTIBLE
// KUMO_HAVE_STD_IS_TRIVIALLY_ASSIGNABLE
// KUMO_HAVE_STD_IS_TRIVIALLY_COPYABLE
//
// 1 when <type_traits> provides std::is_trivially_*<T>.
// Available since C++11.
// ---------------------------------------------------------------------------

#if defined(__cplusplus) && __cplusplus >= 201103L
#define KUMO_HAVE_STD_IS_TRIVIALLY_DESTRUCTIBLE    1
#define KUMO_HAVE_STD_IS_TRIVIALLY_CONSTRUCTIBLE   1
#define KUMO_HAVE_STD_IS_TRIVIALLY_ASSIGNABLE      1
#define KUMO_HAVE_STD_IS_TRIVIALLY_COPYABLE        1
#else
#define KUMO_HAVE_STD_IS_TRIVIALLY_DESTRUCTIBLE    0
#define KUMO_HAVE_STD_IS_TRIVIALLY_CONSTRUCTIBLE   0
#define KUMO_HAVE_STD_IS_TRIVIALLY_ASSIGNABLE      0
#define KUMO_HAVE_STD_IS_TRIVIALLY_COPYABLE        0
#endif

// ---------------------------------------------------------------------------
// KUMO_HAVE_CLASS_TEMPLATE_ARGUMENT_DEDUCTION
//
// 1 when CTAD (class template argument deduction) is supported.
// Available since C++17 (__cpp_deduction_guides >= 201703L).
// ---------------------------------------------------------------------------

#if defined(__cplusplus) && \
    defined(__cpp_deduction_guides) && \
    __cpp_deduction_guides >= 201703L
#define KUMO_HAVE_CLASS_TEMPLATE_ARGUMENT_DEDUCTION  1
#else
#define KUMO_HAVE_CLASS_TEMPLATE_ARGUMENT_DEDUCTION  0
#endif

// ===========================================================================
// Version-comparison helpers
//
// Function-like macros that expand to 1 when the compiler version meets the
// specified minimum.  Always safe to use (no extra side effects).
// ===========================================================================

// ---------------------------------------------------------------------------
// KUMO_HAVE_MIN_GNUC_VERSION(major, minor)
//
//   1 when the compiler is GCC and its version >= (major, minor).
//   0 for all other compilers.
//
//   #if KUMO_HAVE_MIN_GNUC_VERSION(4, 8)
//     // GCC 4.8 or later
//   #endif
// ---------------------------------------------------------------------------

#define KUMO_HAVE_MIN_GNUC_VERSION(major, minor)     \
  (KUMO_COMPILER_GCC &&                              \
   (KUMO_COMPILER_VERSION_MAJOR > (major) ||         \
    (KUMO_COMPILER_VERSION_MAJOR == (major) &&       \
     KUMO_COMPILER_VERSION_MINOR >= (minor))))

// ---------------------------------------------------------------------------
// KUMO_HAVE_MIN_CLANG_VERSION(major, minor)
//
//   1 when the compiler is Clang (including AppleClang) and its version >=
//   (major, minor).  0 for all other compilers.
//
//   #if KUMO_HAVE_MIN_CLANG_VERSION(3, 9)
//     // Clang 3.9 or later
//   #endif
// ---------------------------------------------------------------------------

#define KUMO_HAVE_MIN_CLANG_VERSION(major, minor)    \
  (KUMO_COMPILER_CLANG &&                            \
   (KUMO_COMPILER_VERSION_MAJOR > (major) ||         \
    (KUMO_COMPILER_VERSION_MAJOR == (major) &&       \
     KUMO_COMPILER_VERSION_MINOR >= (minor))))

// ===========================================================================
// POSIX / Linux-specific availability
// ===========================================================================

// KUMO_HAVE_PTHREAD_GETSCHEDPARAM
//
// Checks whether the platform implements the pthread_(get|set)schedparam(3)
// functions as defined in POSIX.1-2001.
#ifdef KUMO_HAVE_PTHREAD_GETSCHEDPARAM
#error KUMO_HAVE_PTHREAD_GETSCHEDPARAM cannot be directly set
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || \
    defined(_AIX) || defined(__ros__) || defined(__OpenBSD__) ||          \
    defined(__NetBSD__) || defined(__VXWORKS__)
#define KUMO_HAVE_PTHREAD_GETSCHEDPARAM 1
#else
#define KUMO_HAVE_PTHREAD_GETSCHEDPARAM 0
#endif

// ---------------------------------------------------------------------------
// KUMO_HAVE_SCHED_GETCPU
//
// 1 when sched_getcpu() is available (Linux with glibc >= 2.6).
// ---------------------------------------------------------------------------

#if defined(__linux__)
#define KUMO_HAVE_SCHED_GETCPU           1
#else
#define KUMO_HAVE_SCHED_GETCPU           0
#endif

// KUMO_HAVE_SCHED_YIELD
//
// Checks whether the platform implements sched_yield(2) as defined in
// POSIX.1-2001.
#ifdef KUMO_HAVE_SCHED_YIELD
#error KUMO_HAVE_SCHED_YIELD cannot be directly set
#elif defined(__linux__) || defined(__ros__) || defined(__native_client__) || \
    defined(__VXWORKS__)
#define KUMO_HAVE_SCHED_YIELD 1
#else
#define KUMO_HAVE_SCHED_YIELD 0
#endif

// KUMO_HAVE_SEMAPHORE_H
//
// Checks whether the platform supports the <semaphore.h> header and sem_init(3)
// family of functions as standardized in POSIX.1-2001.
//
// Note: While Apple provides <semaphore.h> for both iOS and macOS, it is
// explicitly deprecated and will cause build failures if enabled for those
// platforms.  We side-step the issue by not defining it here for Apple
// platforms.
#ifdef KUMO_HAVE_SEMAPHORE_H
#error KUMO_HAVE_SEMAPHORE_H cannot be directly set
#elif defined(__linux__) || defined(__ros__) || defined(__VXWORKS__)
#define KUMO_HAVE_SEMAPHORE_H 1
#else
#define KUMO_HAVE_SEMAPHORE_H 0
#endif



// KUMO_HAVE_ALARM
//
// Checks whether the platform supports the <signal.h> header and alarm(2)
// function as standardized in POSIX.1-2001.
// Always defined to 0 or 1 (use #if, not #ifdef).
#ifdef KUMO_HAVE_ALARM
#error KUMO_HAVE_ALARM cannot be directly set
#elif defined(__GOOGLE_GRTE_VERSION__)
// feature tests for Google's GRTE
#define KUMO_HAVE_ALARM 1
#elif defined(__GLIBC__)
// feature test for glibc
#define KUMO_HAVE_ALARM 1
#elif defined(_MSC_VER)
// feature tests for Microsoft's library
#define KUMO_HAVE_ALARM 0
#elif defined(__MINGW32__)
// mingw32 doesn't provide alarm(2):
// https://osdn.net/projects/mingw/scm/git/mingw-org-wsl/blobs/5.2-trunk/mingwrt/include/unistd.h
// mingw-w64 provides a no-op implementation:
// https://sourceforge.net/p/mingw-w64/mingw-w64/ci/master/tree/mingw-w64-crt/misc/alarm.c
#define KUMO_HAVE_ALARM 0
#elif defined(__EMSCRIPTEN__)
// emscripten doesn't support signals
#define KUMO_HAVE_ALARM 0
#elif defined(__wasi__)
// WASI doesn't support signals
#define KUMO_HAVE_ALARM 0
#elif defined(__Fuchsia__)
// Signals don't exist on fuchsia.
#define KUMO_HAVE_ALARM 0
#elif defined(__hexagon__)
#define KUMO_HAVE_ALARM 0
#else
// other standard libraries
#define KUMO_HAVE_ALARM 1
#endif

// KUMO_HAVE_MMAP
//
// Checks whether the platform has an mmap(2) implementation as defined in
// POSIX.1-2001.
#ifdef KUMO_HAVE_MMAP
#error KUMO_HAVE_MMAP cannot be directly set
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || \
    defined(_AIX) || defined(__ros__) || defined(__asmjs__) ||            \
    defined(__EMSCRIPTEN__) || defined(__Fuchsia__) || defined(__sun) ||  \
    defined(__myriad2__) || defined(__HAIKU__) || defined(__OpenBSD__) || \
    defined(__NetBSD__) || defined(__QNX__) || defined(__VXWORKS__) ||    \
    defined(__hexagon__) || defined(__XTENSA__) ||                        \
    defined(_WASI_EMULATED_MMAN)
#define KUMO_HAVE_MMAP 1
#else
#define KUMO_HAVE_MMAP 0
#endif

