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
// File: msvc_env.h
// -----------------------------------------------------------------------------
//
// MSVC toolchain environment.  Includes clang.h / msvc.h so this header
// is usable on its own.  Does not change KUMO_COMPILER_MSVC / CLANG.
//
//   KUMO_COMPILER_MSVC_CLANG  0|1  clang-cl (__clang__ && _MSC_VER)
//   KUMO_COMPILER_MSVC_ENV    0|1  MSVC || MSVC_CLANG  (any _MSC_VER front-end)

#pragma once

#include <turbo/macros/compiler/clang.h>
#include <turbo/macros/compiler/msvc.h>

#if defined(KUMO_COMPILER_MSVC_CLANG) || defined(KUMO_COMPILER_MSVC_ENV)
#error "KUMO_COMPILER_MSVC_CLANG / KUMO_COMPILER_MSVC_ENV cannot be directly set"
#endif

#if KUMO_COMPILER_CLANG && defined(_MSC_VER)
#define KUMO_COMPILER_MSVC_CLANG 1
#else
#define KUMO_COMPILER_MSVC_CLANG 0
#endif

#if KUMO_COMPILER_MSVC || KUMO_COMPILER_MSVC_CLANG
#define KUMO_COMPILER_MSVC_ENV 1
#else
#define KUMO_COMPILER_MSVC_ENV 0
#endif
