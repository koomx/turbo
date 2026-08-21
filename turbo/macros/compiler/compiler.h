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
// File: compiler.h
// -----------------------------------------------------------------------------
//
// Compiler detection umbrella header.
//
// Unified macro set:
//
//   // Compiler identity (exactly one is 1)
//   KUMO_COMPILER_GCC        0|1
//   KUMO_COMPILER_CLANG      0|1
//   KUMO_COMPILER_MSVC       0|1
//   KUMO_COMPILER_INTEL      0|1
//
//   // Compiler variant
//   KUMO_COMPILER_APPLECLANG 0|1   (Apple's Clang fork)
//   KUMO_COMPILER_MSVC_CLANG 0|1   (clang-cl)
//   KUMO_COMPILER_MSVC_ENV   0|1   (MSVC || MSVC_CLANG)
//
//   // Version
//   KUMO_COMPILER_VERSION           integer  (GCC: 1402 / Clang: 1800 / MSVC: 1939)
//   KUMO_COMPILER_VERSION_MAJOR     integer
//   KUMO_COMPILER_VERSION_MINOR     integer
//
//   // C++ standard
//   KUMO_CXX_STANDARD               integer  98|11|14|17|20|23
//   KUMO_CXX_STANDARD_STRING        string
//
//   // Compiler name
//   KUMO_COMPILER_NAME              string
//
//   // Feature-test helpers (function-like, always safe to use)
//   KUMO_HAVE_ATTRIBUTE(x)          0|1
//   KUMO_HAVE_CPP_ATTRIBUTE(x)      0|1
//   KUMO_HAVE_BUILTIN(x)            0|1

#pragma once

#include <turbo/macros/compiler/intel.h>
#include <turbo/macros/compiler/clang.h>
#include <turbo/macros/compiler/gnu.h>
#include <turbo/macros/compiler/msvc.h>
#include <turbo/macros/compiler/msvc_env.h>

#include <turbo/macros/compiler/cuda.h>
#include <turbo/macros/compiler/lang.h>

// ---------------------------------------------------------------------------
// Completeness check
// ---------------------------------------------------------------------------

#ifndef KUMO_COMPILER_GCC
#error "KUMO_COMPILER_GCC is not defined — no compiler header matched this target"
#endif

#ifndef KUMO_COMPILER_CLANG
#error "KUMO_COMPILER_CLANG is not defined"
#endif

#ifndef KUMO_COMPILER_MSVC
#error "KUMO_COMPILER_MSVC is not defined"
#endif

#ifndef KUMO_COMPILER_MSVC_CLANG
#error "KUMO_COMPILER_MSVC_CLANG is not defined"
#endif

#ifndef KUMO_COMPILER_MSVC_ENV
#error "KUMO_COMPILER_MSVC_ENV is not defined"
#endif

#ifndef KUMO_COMPILER_INTEL
#error "KUMO_COMPILER_INTEL is not defined"
#endif

#ifndef KUMO_COMPILER_APPLECLANG
#error "KUMO_COMPILER_APPLECLANG is not defined"
#endif

#ifndef KUMO_COMPILER_VERSION
#error "KUMO_COMPILER_VERSION is not defined"
#endif

#ifndef KUMO_COMPILER_VERSION_MAJOR
#error "KUMO_COMPILER_VERSION_MAJOR is not defined"
#endif

#ifndef KUMO_COMPILER_VERSION_MINOR
#error "KUMO_COMPILER_VERSION_MINOR is not defined"
#endif

#ifndef KUMO_CXX_STANDARD
#error "KUMO_CXX_STANDARD is not defined"
#endif

#ifndef KUMO_CXX_STANDARD_STRING
#error "KUMO_CXX_STANDARD_STRING is not defined"
#endif

#ifndef KUMO_COMPILER_NAME
#error "KUMO_COMPILER_NAME is not defined"
#endif

#ifndef KUMO_HAVE_ATTRIBUTE
#error "KUMO_HAVE_ATTRIBUTE is not defined"
#endif

#ifndef KUMO_HAVE_CPP_ATTRIBUTE
#error "KUMO_HAVE_CPP_ATTRIBUTE is not defined"
#endif

#ifndef KUMO_HAVE_BUILTIN
#error "KUMO_HAVE_BUILTIN is not defined"
#endif

#ifndef KUMO_COMPILER_CUDA
#error "KUMO_COMPILER_CUDA is not defined"
#endif

#ifndef KUMO_COMPILER_HIP
#error "KUMO_COMPILER_HIP is not defined"
#endif
