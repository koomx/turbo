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

#include <turbo/macros/compiler/compiler.h>
#include <turbo/macros/have/base.h>

// `KUMO_HAVE_RTTI` determines whether library is being compiled with
// RTTI support.
#ifdef KUMO_HAVE_RTTI
#error KUMO_HAVE_RTTI cannot be directly set
#elif KUMO_HAVE_FEATURE(cxx_rtti)
#define KUMO_HAVE_RTTI 1
#elif defined(__GNUC__) && defined(__GXX_RTTI)
#define KUMO_HAVE_RTTI 1
#elif defined(_MSC_VER) && defined(_CPPRTTI)
#define KUMO_HAVE_RTTI 1
#elif !defined(__GNUC__) && !defined(_MSC_VER)
// Unknown compiler, default to RTTI
#define KUMO_HAVE_RTTI 1
#else
#define KUMO_HAVE_RTTI 0
#endif

// `KUMO_HAVE_CXA_DEMANGLE` determines whether `abi::__cxa_demangle` is
// available.
#ifdef KUMO_HAVE_CXA_DEMANGLE
#error KUMO_HAVE_CXA_DEMANGLE cannot be directly set
#elif defined(OS_ANDROID) && (defined(__i386__) || defined(__x86_64__))
#define KUMO_HAVE_CXA_DEMANGLE 0
#elif defined(__GNUC__)
#define KUMO_HAVE_CXA_DEMANGLE 1
#elif defined(__clang__) && !defined(_MSC_VER)
#define KUMO_HAVE_CXA_DEMANGLE 1
#endif
