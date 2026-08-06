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
// File: gpu.h
// -----------------------------------------------------------------------------
//
// GPU / accelerator architecture detection umbrella header.
//
// Every GPU architecture header defines the SAME set of macros with the
// KUMO_ prefix, each to either 0 or 1.  Only the header matching the current
// device target fires.
//
// Unified macro set:
//
//   // GPU vendor (exactly one is 1)
//   KUMO_GPU_NVIDIA         0|1
//   KUMO_GPU_AMD            0|1
//   KUMO_GPU_INTEL          0|1
//
//   // Compute capability (integer, e.g. 75 for sm_75)
//   KUMO_GPU_CUDA_ARCH      0|integer   __CUDA_ARCH__

#pragma once

// ---------------------------------------------------------------------------
// Skeleton  —  all macros are 0 until a concrete GPU header is written.
// ---------------------------------------------------------------------------

#define KUMO_GPU_NVIDIA     0
#define KUMO_GPU_AMD        0
#define KUMO_GPU_INTEL      0
#define KUMO_GPU_CUDA_ARCH  0
