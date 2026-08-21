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

#ifndef TURBO_X64_DETECT
#error internal header only can be included internally
#endif

#if KUMO_OS_WINDOWS
#include <intrin.h>
#endif

#if KUMO_ARCH_X86_64
#if KUMO_HAVE_BUILTIN(__cpuid)
// MSVC-equivalent __cpuid intrinsic declaration for clang-like compilers
// for non-Windows build environments.
extern void __cpuid(int[4], int);
extern void __cpuidex(int[4], int, int);
#elif !KUMO_OS_WINDOWS
// MSVC defines this function for us.
// https://learn.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex
static void __cpuid(int cpu_info[4], int info_type) {
    __asm__ volatile("cpuid \n\t"
        : "=a"(cpu_info[0]), "=b"(cpu_info[1]), "=c"(cpu_info[2]),
        "=d"(cpu_info[3])
        : "a"(info_type), "c"(0));
}
static void __cpuidex(int cpu_info[4], int info_type, int ecx) {
    __asm__ volatile("cpuid \n\t"
        : "=a"(cpu_info[0]), "=b"(cpu_info[1]), "=c"(cpu_info[2]),
        "=d"(cpu_info[3])
        : "a"(info_type), "c"(ecx));
}
#endif
#endif
