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
// File: have.h
// -----------------------------------------------------------------------------
//
// Compile-time feature detection macros.  Every macro in this file is defined
// to either 1 (feature available) or 0 (feature unavailable).

#pragma once

#include <turbo/macros/compiler/compiler.h>

// ---------------------------------------------------------------------------
// Thread-local storage
//
// KUMO_HAVE_THREAD_LOCAL: C++11 thread_local (C++ only).
// KUMO_HAVE_TLS:          C99 __thread (C and C++).
// ---------------------------------------------------------------------------

#if defined(__clang__)
#if __has_feature(cxx_thread_local) && defined(__cplusplus)
#define KUMO_HAVE_THREAD_LOCAL          1
#else
#define KUMO_HAVE_THREAD_LOCAL          0
#endif
#elif defined(__GNUC__)
#if defined(__cpp_threadsafe_static_init) || defined(__cplusplus)
#define KUMO_HAVE_THREAD_LOCAL          1
#else
#define KUMO_HAVE_THREAD_LOCAL          0
#endif
#elif defined(_MSC_VER)
#if defined(__cplusplus)
#define KUMO_HAVE_THREAD_LOCAL          1
#else
#define KUMO_HAVE_THREAD_LOCAL          0
#endif
#else
#define KUMO_HAVE_THREAD_LOCAL          0
#endif

// KUMO_HAVE_TLS is defined to 1 when __thread should be supported.
// We assume __thread is supported on Linux when compiled with Clang or
// compiled against libstdc++ with _GLIBCXX_HAVE_TLS defined.
#ifdef KUMO_HAVE_TLS
#error KUMO_HAVE_TLS cannot be directly set
#elif (defined(__linux__)) && (defined(__clang__) || defined(_GLIBCXX_HAVE_TLS))
#define KUMO_HAVE_TLS 1
#elif defined(__INTEL_LLVM_COMPILER)
#define KUMO_HAVE_TLS 1
#else
#define KUMO_HAVE_TLS 0
#endif


#if defined(KUMO_PER_THREAD_TLS)
#error KUMO_PER_THREAD_TLS cannot be directly set
#elif defined(KUMO_PER_THREAD_TLS_KEYWORD)
#error KUMO_PER_THREAD_TLS_KEYWORD cannot be directly set
#elif KUMO_HAVE_TLS
#define KUMO_PER_THREAD_TLS_KEYWORD __thread
#define KUMO_PER_THREAD_TLS 1
#elif defined(_MSC_VER)
#define KUMO_PER_THREAD_TLS_KEYWORD __declspec(thread)
#define KUMO_PER_THREAD_TLS 1
#else
#define KUMO_PER_THREAD_TLS_KEYWORD
#define KUMO_PER_THREAD_TLS 0
#endif


// ---------------------------------------------------------------------------
// KUMO_HAVE_UNISTD_H
//
// 1 when the POSIX <unistd.h> header is available.
// ---------------------------------------------------------------------------

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || \
    defined(__OpenBSD__) || defined(__NetBSD__) || defined(__CYGWIN__)
#define KUMO_HAVE_UNISTD_H              1
#else
#define KUMO_HAVE_UNISTD_H              0
#endif

// ---------------------------------------------------------------------------
// KUMO_HAVE_FEATURE
//
// Wraps __has_feature for Clang; always 0 on other compilers.
// ---------------------------------------------------------------------------

#if defined(__has_feature)
#define KUMO_HAVE_FEATURE(x)            __has_feature(x)
#else
#define KUMO_HAVE_FEATURE(x)            0
#endif

// ---------------------------------------------------------------------------
// KUMO_HAVE_DLADDR
//
// 1 when the POSIX dladdr() function is available.
// ---------------------------------------------------------------------------

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || \
    defined(__OpenBSD__) || defined(__NetBSD__) || defined(__sun)
#define KUMO_HAVE_DLADDR                1
#else
#define KUMO_HAVE_DLADDR                0
#endif

#if KUMO_HAVE_BUILTIN(__builtin_LINE) && KUMO_HAVE_BUILTIN(__builtin_FILE)
#define KUMO_HAVE_BUILTIN_LINE_FILE 1
#elif defined(__GNUC__) && !defined(__clang__) && 5 <= __GNUC__ && __GNUC__ < 10
#define KUMO_HAVE_BUILTIN_LINE_FILE 1
#elif defined(_MSC_VER) && _MSC_VER >= 1926
#define KUMO_HAVE_BUILTIN_LINE_FILE 1
#else
#define KUMO_HAVE_BUILTIN_LINE_FILE 0
#endif
