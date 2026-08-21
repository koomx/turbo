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
// File: visibility.h
// -----------------------------------------------------------------------------
//
// Symbol visibility / DLL export-import macros.
//
// Usage example — per-library API macro:
//
//   // mylib.h
//   #if COMPILING_MY_LIB
//   #define MY_LIB_API KUMO_EXPORT
//   #else
//   #define MY_LIB_API KUMO_IMPORT
//   #endif
//
// Control macros (define before including this header):
//   KUMO_BUILD_DLL   — building the library as a shared library / DLL
//   KUMO_USE_DLL     — consuming the library as a shared library / DLL
//   (neither         — static build; all macros expand to nothing)
//
// Unified macro set:
//
//   KUMO_EXPORT      default visibility (dllexport on Windows, default on GCC/Clang)
//   KUMO_IMPORT      imported symbol (dllimport on Windows, default on GCC/Clang)
//   KUMO_LOCAL       hidden visibility (hidden on GCC/Clang, nothing on Windows)

#pragma once

#include <turbo/macros/option.h>

// ---------------------------------------------------------------------------
// Windows (MSVC, MinGW, Cygwin)
// ---------------------------------------------------------------------------

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)

#if defined(__GNUC__) && !defined(__clang__)
// GCC on Windows (MinGW): use C++ attribute syntax to avoid a GCC parser bug
// where __declspec(dllexport) breaks [[nodiscard]] and similar attributes.
#define KUMO_DLL_EXPORT   [[gnu::dllexport]]
#define KUMO_DLL_IMPORT   [[gnu::dllimport]]
#else
#define KUMO_DLL_EXPORT   __declspec(dllexport)
#define KUMO_DLL_IMPORT   __declspec(dllimport)
#endif

#if defined(KUMO_BUILD_DLL)
#define KUMO_EXPORT       KUMO_DLL_EXPORT
#define KUMO_IMPORT       KUMO_DLL_IMPORT
#elif defined(KUMO_USE_DLL)
#define KUMO_EXPORT       KUMO_DLL_IMPORT
#define KUMO_IMPORT       KUMO_DLL_IMPORT
#else
// Static build — no declspec needed
#define KUMO_EXPORT
#define KUMO_IMPORT
#endif

#define KUMO_LOCAL

// ---------------------------------------------------------------------------
// Non-Windows (GCC, Clang)
// ---------------------------------------------------------------------------

#elif defined(__GNUC__) || defined(__clang__)

#define KUMO_EXPORT       __attribute__((visibility("default")))
#define KUMO_IMPORT       __attribute__((visibility("default")))
#define KUMO_LOCAL        __attribute__((visibility("hidden")))

// ---------------------------------------------------------------------------
// Unknown compiler
// ---------------------------------------------------------------------------

#else

#define KUMO_EXPORT
#define KUMO_IMPORT
#define KUMO_LOCAL

#endif


// KUMO_DLL
//
// When building library as a DLL, this macro expands to `__declspec(dllexport)`
// so we can annotate symbols appropriately as being exported. When used in
// headers consuming a DLL, this macro expands to `__declspec(dllimport)` so
// that consumers know the symbol is defined inside the DLL. In all other cases,
// the macro expands to nothing.
#if defined(_MSC_VER)
#if KUMO_BUILD_DLL
#define KUMO_DLL __declspec(dllexport)
#elif KUMO_CONSUME_DLL
#define KUMO_DLL __declspec(dllimport)
#else
#define KUMO_DLL
#endif
#else
#define KUMO_DLL
#endif  // defined(_MSC_VER)


#if defined(_MSC_VER)
#if KUMO_BUILD_TEST_DLL
#define KUMO_TEST_DLL __declspec(dllexport)
#elif KUMO_CONSUME_TEST_DLL
#define KUMO_TEST_DLL __declspec(dllimport)
#else
#define KUMO_TEST_DLL
#endif
#else
#define KUMO_TEST_DLL
#endif  // defined(_MSC_VER)
