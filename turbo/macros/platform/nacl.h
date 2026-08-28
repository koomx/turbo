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

#if defined(__native_client__)

#define KUMO_OS_NACL             1

#if defined(__wasm64__) || defined(_WIN64) || defined(__LP64__) || \
    defined(_LP64) || defined(__x86_64__) || defined(_M_X64) || \
    defined(_M_AMD64) || defined(__aarch64__) || defined(_M_ARM64) || \
    defined(__powerpc64__) || defined(__PPC64__) || defined(__s390x__) || \
    defined(__mips64) || defined(__loongarch64) || \
    (defined(__riscv) && (__riscv_xlen == 64))
#define KUMO_PTR_SIZE          8
#else
#define KUMO_PTR_SIZE          4
#endif

#define KUMO_PLATFORM_NAME     "NaCl"

#if defined(__native_client_nonsfi__)
#undef KUMO_OS_NACL_NONSFI
#undef KUMO_OS_NACL_SFI
#define KUMO_OS_NACL_NONSFI    1
#define KUMO_OS_NACL_SFI       0
#else
#undef KUMO_OS_NACL_NONSFI
#undef KUMO_OS_NACL_SFI
#define KUMO_OS_NACL_NONSFI    0
#define KUMO_OS_NACL_SFI       1
#endif

#else
#define KUMO_OS_NACL             0
#define KUMO_OS_NACL_NONSFI    0
#define KUMO_OS_NACL_SFI       0
#endif
