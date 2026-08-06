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

#pragma once

#if defined(__GNUC__) && !defined(__clang__) && !defined(__INTEL_COMPILER)

#define KUMO_COMPILER_GCC       1
#define KUMO_COMPILER_CLANG     0
#define KUMO_COMPILER_MSVC      0
#define KUMO_COMPILER_INTEL     0

#define KUMO_COMPILER_APPLECLANG 0

#define KUMO_COMPILER_VERSION       (__GNUC__ * 100 + __GNUC_MINOR__)
#define KUMO_COMPILER_VERSION_MAJOR __GNUC__
#define KUMO_COMPILER_VERSION_MINOR __GNUC_MINOR__

#if __cplusplus >= 202302L
#define KUMO_CXX_STANDARD       23
#define KUMO_CXX_STANDARD_STRING "C++23"
#elif __cplusplus >= 202002L
#define KUMO_CXX_STANDARD       20
#define KUMO_CXX_STANDARD_STRING "C++20"
#elif __cplusplus >= 201703L
#define KUMO_CXX_STANDARD       17
#define KUMO_CXX_STANDARD_STRING "C++17"
#elif __cplusplus >= 201402L
#define KUMO_CXX_STANDARD       14
#define KUMO_CXX_STANDARD_STRING "C++14"
#elif __cplusplus >= 201103L
#define KUMO_CXX_STANDARD       11
#define KUMO_CXX_STANDARD_STRING "C++11"
#else
#define KUMO_CXX_STANDARD       98
#define KUMO_CXX_STANDARD_STRING "C++98"
#endif

#define KUMO_COMPILER_NAME      "GCC"

#ifdef __has_attribute
#define KUMO_HAVE_ATTRIBUTE(x)  __has_attribute(x)
#else
#define KUMO_HAVE_ATTRIBUTE(x)  0
#endif

// __has_cpp_attribute + vendor::name (e.g. gnu::pure) is C++-only; using it
// from a C TU can fail to parse `::`. Keep the probe as 0 in C so call sites
// fall back to __attribute__((...)) / empty.
#if defined(__cplusplus) && defined(__has_cpp_attribute)
#define KUMO_HAVE_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#define KUMO_HAVE_CPP_ATTRIBUTE(x) 0
#endif

#ifdef __has_builtin
#define KUMO_HAVE_BUILTIN(x)    __has_builtin(x)
#else
#define KUMO_HAVE_BUILTIN(x)    0
#endif

#endif
