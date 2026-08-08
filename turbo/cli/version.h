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

// project name version
#define XCLI_VERSION_MAJOR 0
#define XCLI_VERSION_MINOR 0
#define XCLI_VERSION_PATCH 5
#define XCLI_VERSION ((PROJECT_VERSION_MAJOR * 1000) + PROJECT_VERSION_MINOR) * 1000 + PROJECT_VERSION_PATCH

#define XCLI_VERSION_STRING "0.0.5"

// build system
#define XCLI_BUILD_SYSTEM "darwin"

// build system version
#define XCLI_BUILD_SYSTEM_VERSION ""


// compiler gnu or clang
#define XCLI_CXX_COMPILER_ID "AppleClang"

// compiler version
#define XCLI_CXX_COMPILER_VERSION "14.0.0.14000029"

// cmake cxx compiler flags
#define XCLI_CMAKE_CXX_COMPILER_FLAGS ""

// user defined cxx compiler flags
#define XCLI_CXX_COMPILER_FLAGS "-Wall;-Wextra;-Wno-cast-qual;-Wno-conversion;-Wno-sign-compare;-Wfloat-overflow-conversion;-Wfloat-zero-conversion;-Wfor-loop-analysis;-Wformat-security;-Wgnu-redeclared-enum;-Winfinite-recursion;-Wliteral-conversion;-Wmissing-declarations;-Woverlength-strings;-Wpointer-arith;-Wself-assign;-Wno-shadow;-Wstring-conversion;-Wtautological-overlap-compare;-Wno-undef;-Wuninitialized;-Wunreachable-code;-Wunused-comparison;-Wunused-local-typedefs;-Wunused-result;-Wno-vla;-Wwrite-strings;-Wno-float-conversion;-Wno-implicit-float-conversion;-Wno-implicit-int-float-conversion;-Wno-implicit-int-conversion;-Wno-shorten-64-to-32;-Wno-sign-conversion;-Wno-unused-parameter;-Wno-unused-function;-DNOMINMAX;-mfpu=neon;-mfpu=neon -mfma;-march=armv8-a+crypto"

// cxx standard
#define XCLI_CXX_STANDARD "17"

// build type
#define XCLI_BUILD_TYPE_STRING "DEBUG"

// build type
#define XCLI_BUILD_DEBUG

// build type
#if defined(XCLI_BUILD_DEBUG)
    #define IS_XCLI_BUILD_TYPE_DEBUG 1
#else
    #define IS_XCLI_BUILD_TYPE_DEBUG 0
#endif

////////////////////////////////////////////////////////////////////////////////
/// simd region

// SIMD target level string (NONE / SSE / SSE2 / ... / AVX512)
#define XCLI_SIMD_LEVEL "AVX2"

// Per-feature SIMD enable flags
#define XCLI_SIMD_ENABLE_SSE     0
#define XCLI_SIMD_ENABLE_SSE2    0
#define XCLI_SIMD_ENABLE_SSE3    0
#define XCLI_SIMD_ENABLE_SSSE3   0
#define XCLI_SIMD_ENABLE_SSE4_1  0
#define XCLI_SIMD_ENABLE_SSE4_2  0
#define XCLI_SIMD_ENABLE_AVX     0
#define XCLI_SIMD_ENABLE_AVX2    0
#define XCLI_SIMD_ENABLE_AVX512F 0
#define XCLI_SIMD_ENABLE_BMI1    0
#define XCLI_SIMD_ENABLE_BMI2     0
#define XCLI_SIMD_ENABLE_FMA     1
#define XCLI_SIMD_ENABLE_F16C    0
#define XCLI_SIMD_ENABLE_POPCNT  0
#define XCLI_SIMD_ENABLE_LZCNT    0
#define XCLI_SIMD_ENABLE_MOVBE    0

///////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// Git Version Information
////////////////////////////////////////////////////////////////////////////////
// Full Git commit hash (e.g., "a1b2c3d4e5f67890abcdef1234567890abcdef12")
#define XCLI_GIT_COMMIT_HASH "2116c2e80e26fe57e06b0f82b17d1af75d580bbb"
// Short Git commit hash (e.g., "a1b2c3d")
#define XCLI_GIT_COMMIT_SHORT_HASH "2116c2e"
// Git dirty flag (0 = clean working tree, 1 = uncommitted changes)
#define XCLI_GIT_IS_DIRTY 0
// Combined Git version string (e.g., "0.6.0-a1b2c3d" or "0.6.0-a1b2c3d-dirty")
#define XCLI_GIT_VERSION_STRING "v0.3.0-2116c2e"
