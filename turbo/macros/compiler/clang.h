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

#if defined(__clang__)

#define KUMO_COMPILER_GCC       0
#define KUMO_COMPILER_CLANG     1
#define KUMO_COMPILER_MSVC      0
#define KUMO_COMPILER_INTEL     0

#if defined(__apple_build_version__)
#define KUMO_COMPILER_APPLECLANG 1
#else
#define KUMO_COMPILER_APPLECLANG 0
#endif

#define KUMO_COMPILER_VERSION       (__clang_major__ * 100 + __clang_minor__)
#define KUMO_COMPILER_VERSION_MAJOR __clang_major__
#define KUMO_COMPILER_VERSION_MINOR __clang_minor__

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

#if defined(__apple_build_version__)
#define KUMO_COMPILER_NAME      "AppleClang"
#else
#define KUMO_COMPILER_NAME      "Clang"
#endif

#ifdef __has_attribute
#define KUMO_HAVE_ATTRIBUTE(x)  __has_attribute(x)
#else
#define KUMO_HAVE_ATTRIBUTE(x)  0
#endif

// C++-only: scoped args like clang::fallthrough are not valid in C TUs.
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
