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
// File: cuda.h
// -----------------------------------------------------------------------------
//
// GPU compiler detection — NVIDIA CUDA (NVCC) and AMD HIP (HIPCC).
//
// Unlike host compilers (GCC, Clang, MSVC), CUDA/HIP compilers wrap the host
// compiler and do not replace its pre-defined macros.  This file only defines
// additional KUMO_COMPILER_CUDA and KUMO_COMPILER_HIP flags; it does NOT
// participate in the exclusive host-compiler identity chain.
//
// This header must be included AFTER the host-compiler headers.

#pragma once

#if defined(__CUDACC__)
#define KUMO_COMPILER_CUDA  1
#else
#define KUMO_COMPILER_CUDA  0
#endif

#if defined(__HIPCC__)
#define KUMO_COMPILER_HIP   1
#else
#define KUMO_COMPILER_HIP   0
#endif
