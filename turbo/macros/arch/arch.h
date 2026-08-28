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
// File: arch.h
// -----------------------------------------------------------------------------
//
// Architecture / CPU / SIMD detection umbrella.
//
// These macros reflect the **compile target** (compiler predefined macros /
// -m flags).  Runtime CPUID belongs in turbo/base/internal/cpu_detect.
//
// Family (exactly one is 1):
//   KUMO_ARCH_X86 / ARM / RISCV / LOONGARCH / PPC / S390 / MIPS / E2K / WASM
//
// Variants (examples):
//   KUMO_ARCH_X86_64 / X86_32 / ARM64 / ARM32 / ARM64EC / AARCH64(=ARM64)
//   KUMO_ARCH_PPC64 / PPC32 / PPC64LE / S390X / MIPS64 / WASM32 / ...
//
// SIMD (always 0|1 on every target):
//   KUMO_SIMD_SSE..AVX512* / NEON / SVE / AES / PCLMUL / x86 extras / RVV / LSX / LASX /
//   ALTIVEC / VSX / CRYPTO
//
// Also: KUMO_CACHELINE_SIZE, KUMO_SIMD_LEVEL, KUMO_ARCH_NAME
//       KUMO_HAVE_ACCELERATED_AES — compat with TURBO_HAVE_ACCELERATED_AES

#pragma once

#include <turbo/macros/arch/riscv.h>
#include <turbo/macros/arch/x86.h>
#include <turbo/macros/arch/arm.h>
#include <turbo/macros/arch/loongarch.h>
#include <turbo/macros/arch/ppc.h>
#include <turbo/macros/arch/s390.h>
#include <turbo/macros/arch/mips.h>
#include <turbo/macros/arch/e2k.h>
#include <turbo/macros/arch/wasm.h>


#if KUMO_ARCH_ARM64 || KUMO_ARCH_E2K || KUMO_ARCH_LOONGARCH64 \
    || KUMO_ARCH_MIPS64 \
    || KUMO_ARCH_PPC64 \
    || KUMO_ARCH_RISCV64 \
    || KUMO_ARCH_S390X || KUMO_ARCH_WASM64 || KUMO_ARCH_X86_64
#define KUMO_ARCH_128_BIT    0
#define KUMO_ARCH_64_BIT     1
#define KUMO_ARCH_32_BIT     0
#elif KUMO_ARCH_ARM32 || KUMO_ARCH_LOONGARCH32 \
    || KUMO_ARCH_MIPS32 \
    || KUMO_ARCH_PPC32 \
    || KUMO_ARCH_RISCV32 \
    || KUMO_ARCH_S390_31 || KUMO_ARCH_WASM32 || KUMO_ARCH_X86_32
#define KUMO_ARCH_64_BIT     0
#define KUMO_ARCH_32_BIT     1
#define KUMO_ARCH_128_BIT    0
#endif


// Alias used by some internal modules (TURBO_ARCH_AARCH64).
#ifndef KUMO_ARCH_AARCH64
#define KUMO_ARCH_AARCH64 KUMO_ARCH_ARM64
#endif

// Accelerated AES detection (x86_64 treats AVX as a stand-in; keep that OR with AES-NI).
#if KUMO_SIMD_X86_AES || (KUMO_ARCH_X86_64 && KUMO_SIMD_AVX)
#define KUMO_HAVE_ACCELERATED_AES 1
#else
#define KUMO_HAVE_ACCELERATED_AES 0
#endif

// ---------------------------------------------------------------------------
// Completeness check
// ---------------------------------------------------------------------------

#ifndef KUMO_ARCH_X86
#error "KUMO_ARCH_X86 is not defined — no architecture header matched"
#endif
#ifndef KUMO_ARCH_ARM
#error "KUMO_ARCH_ARM is not defined"
#endif
#ifndef KUMO_ARCH_RISCV
#error "KUMO_ARCH_RISCV is not defined"
#endif
#ifndef KUMO_ARCH_LOONGARCH
#error "KUMO_ARCH_LOONGARCH is not defined"
#endif
#ifndef KUMO_ARCH_PPC
#error "KUMO_ARCH_PPC is not defined"
#endif
#ifndef KUMO_ARCH_S390
#error "KUMO_ARCH_S390 is not defined"
#endif
#ifndef KUMO_ARCH_MIPS
#error "KUMO_ARCH_MIPS is not defined"
#endif
#ifndef KUMO_ARCH_E2K
#error "KUMO_ARCH_E2K is not defined"
#endif
#ifndef KUMO_ARCH_WASM
#error "KUMO_ARCH_WASM is not defined"
#endif

#ifndef KUMO_ARCH_X86_64
#error "KUMO_ARCH_X86_64 is not defined"
#endif
#ifndef KUMO_ARCH_X86_32
#error "KUMO_ARCH_X86_32 is not defined"
#endif
#ifndef KUMO_ARCH_ARM64
#error "KUMO_ARCH_ARM64 is not defined"
#endif
#ifndef KUMO_ARCH_ARM32
#error "KUMO_ARCH_ARM32 is not defined"
#endif
#ifndef KUMO_ARCH_ARM64EC
#error "KUMO_ARCH_ARM64EC is not defined"
#endif
#ifndef KUMO_ARCH_RISCV64
#error "KUMO_ARCH_RISCV64 is not defined"
#endif
#ifndef KUMO_ARCH_RISCV32
#error "KUMO_ARCH_RISCV32 is not defined"
#endif
#ifndef KUMO_ARCH_LOONGARCH64
#error "KUMO_ARCH_LOONGARCH64 is not defined"
#endif
#ifndef KUMO_ARCH_LOONGARCH32
#error "KUMO_ARCH_LOONGARCH32 is not defined"
#endif
#ifndef KUMO_ARCH_PPC64
#error "KUMO_ARCH_PPC64 is not defined"
#endif
#ifndef KUMO_ARCH_PPC32
#error "KUMO_ARCH_PPC32 is not defined"
#endif
#ifndef KUMO_ARCH_PPC64LE
#error "KUMO_ARCH_PPC64LE is not defined"
#endif
#ifndef KUMO_ARCH_S390X
#error "KUMO_ARCH_S390X is not defined"
#endif
#ifndef KUMO_ARCH_S390_31
#error "KUMO_ARCH_S390_31 is not defined"
#endif
#ifndef KUMO_ARCH_MIPS64
#error "KUMO_ARCH_MIPS64 is not defined"
#endif
#ifndef KUMO_ARCH_MIPS32
#error "KUMO_ARCH_MIPS32 is not defined"
#endif
#ifndef KUMO_ARCH_WASM64
#error "KUMO_ARCH_WASM64 is not defined"
#endif
#ifndef KUMO_ARCH_WASM32
#error "KUMO_ARCH_WASM32 is not defined"
#endif

#ifndef KUMO_ARCH_32_BIT
#error "KUMO_ARCH_32_BIT is not defined"
#endif
#ifndef KUMO_ARCH_64_BIT
#error "KUMO_ARCH_64_BIT is not defined"
#endif

#ifndef KUMO_SIMD_SSE
#error "KUMO_SIMD_SSE is not defined"
#endif
#ifndef KUMO_SIMD_SSE2
#error "KUMO_SIMD_SSE2 is not defined"
#endif
#ifndef KUMO_SIMD_SSE3
#error "KUMO_SIMD_SSE3 is not defined"
#endif
#ifndef KUMO_SIMD_SSSE3
#error "KUMO_SIMD_SSSE3 is not defined"
#endif
#ifndef KUMO_SIMD_SSE4_1
#error "KUMO_SIMD_SSE4_1 is not defined"
#endif
#ifndef KUMO_SIMD_SSE4_2
#error "KUMO_SIMD_SSE4_2 is not defined"
#endif
#ifndef KUMO_SIMD_AVX
#error "KUMO_SIMD_AVX is not defined"
#endif
#ifndef KUMO_SIMD_AVX2
#error "KUMO_SIMD_AVX2 is not defined"
#endif
#ifndef KUMO_SIMD_AVX512F
#error "KUMO_SIMD_AVX512F is not defined"
#endif
#ifndef KUMO_SIMD_AVX512BW
#error "KUMO_SIMD_AVX512BW is not defined"
#endif
#ifndef KUMO_SIMD_AVX512VL
#error "KUMO_SIMD_AVX512VL is not defined"
#endif
#ifndef KUMO_SIMD_AVX512DQ
#error "KUMO_SIMD_AVX512DQ is not defined"
#endif
#ifndef KUMO_SIMD_AVX512IFMA
#error "KUMO_SIMD_AVX512IFMA is not defined"
#endif
#ifndef KUMO_SIMD_AVX512CD
#error "KUMO_SIMD_AVX512CD is not defined"
#endif
#ifndef KUMO_SIMD_AVX512VBMI
#error "KUMO_SIMD_AVX512VBMI is not defined"
#endif
#ifndef KUMO_SIMD_AVX512VBMI2
#error "KUMO_SIMD_AVX512VBMI2 is not defined"
#endif
#ifndef KUMO_SIMD_AVX512VNNI
#error "KUMO_SIMD_AVX512VNNI is not defined"
#endif
#ifndef KUMO_SIMD_AVX512BITALG
#error "KUMO_SIMD_AVX512BITALG is not defined"
#endif
#ifndef KUMO_SIMD_AVX512VPOPCNTDQ
#error "KUMO_SIMD_AVX512VPOPCNTDQ is not defined"
#endif
#ifndef KUMO_SIMD_FMA
#error "KUMO_SIMD_FMA is not defined"
#endif
#ifndef KUMO_SIMD_BMI1
#error "KUMO_SIMD_BMI1 is not defined"
#endif
#ifndef KUMO_SIMD_BMI2
#error "KUMO_SIMD_BMI2 is not defined"
#endif
#ifndef KUMO_SIMD_POPCNT
#error "KUMO_SIMD_POPCNT is not defined"
#endif
#ifndef KUMO_SIMD_LZCNT
#error "KUMO_SIMD_LZCNT is not defined"
#endif
#ifndef KUMO_SIMD_NEON
#error "KUMO_SIMD_NEON is not defined"
#endif
#ifndef KUMO_SIMD_SVE
#error "KUMO_SIMD_SVE is not defined"
#endif
#ifndef KUMO_SIMD_SVE2
#error "KUMO_SIMD_SVE2 is not defined"
#endif
#ifndef KUMO_SIMD_X86_AES
#error "KUMO_SIMD_X86_AES is not defined"
#endif

#ifndef KUMO_SIMD_ARM_AES
#error "KUMO_SIMD_ARM_AES is not defined"
#endif

#ifndef KUMO_SIMD_PPC_AES
#error "KUMO_SIMD_PPC_AES is not defined"
#endif
#ifndef KUMO_SIMD_PCLMUL
#error "KUMO_SIMD_PCLMUL is not defined"
#endif
#ifndef KUMO_SIMD_PRFCHW
#error "KUMO_SIMD_PRFCHW is not defined"
#endif
#ifndef KUMO_SIMD_PREFETCH
#error "KUMO_SIMD_PREFETCH is not defined"
#endif
#ifndef KUMO_SIMD_PREFETCHWT1
#error "KUMO_SIMD_PREFETCHWT1 is not defined"
#endif
#ifndef KUMO_SIMD_CX16
#error "KUMO_SIMD_CX16 is not defined"
#endif
#ifndef KUMO_SIMD_MOVBE
#error "KUMO_SIMD_MOVBE is not defined"
#endif
#ifndef KUMO_SIMD_XSAVE
#error "KUMO_SIMD_XSAVE is not defined"
#endif
#ifndef KUMO_SIMD_F16C
#error "KUMO_SIMD_F16C is not defined"
#endif
#ifndef KUMO_SIMD_RDRND
#error "KUMO_SIMD_RDRND is not defined"
#endif
#ifndef KUMO_SIMD_RDSEED
#error "KUMO_SIMD_RDSEED is not defined"
#endif
#ifndef KUMO_SIMD_FSGSBASE
#error "KUMO_SIMD_FSGSBASE is not defined"
#endif
#ifndef KUMO_SIMD_SHA
#error "KUMO_SIMD_SHA is not defined"
#endif
#ifndef KUMO_SIMD_ADX
#error "KUMO_SIMD_ADX is not defined"
#endif
#ifndef KUMO_SIMD_CLFLUSHOPT
#error "KUMO_SIMD_CLFLUSHOPT is not defined"
#endif
#ifndef KUMO_SIMD_CLWB
#error "KUMO_SIMD_CLWB is not defined"
#endif
#ifndef KUMO_SIMD_CLZERO
#error "KUMO_SIMD_CLZERO is not defined"
#endif
#ifndef KUMO_SIMD_RDTSCP
#error "KUMO_SIMD_RDTSCP is not defined"
#endif
#ifndef KUMO_SIMD_RDPID
#error "KUMO_SIMD_RDPID is not defined"
#endif
#ifndef KUMO_SIMD_HLE
#error "KUMO_SIMD_HLE is not defined"
#endif
#ifndef KUMO_SIMD_RTM
#error "KUMO_SIMD_RTM is not defined"
#endif
#ifndef KUMO_SIMD_MPX
#error "KUMO_SIMD_MPX is not defined"
#endif
#ifndef KUMO_SIMD_SSE4A
#error "KUMO_SIMD_SSE4A is not defined"
#endif
#ifndef KUMO_SIMD_FMA4
#error "KUMO_SIMD_FMA4 is not defined"
#endif
#ifndef KUMO_SIMD_XOP
#error "KUMO_SIMD_XOP is not defined"
#endif
#ifndef KUMO_SIMD_TBM
#error "KUMO_SIMD_TBM is not defined"
#endif
#ifndef KUMO_SIMD_LWP
#error "KUMO_SIMD_LWP is not defined"
#endif
#ifndef KUMO_SIMD_VAES
#error "KUMO_SIMD_VAES is not defined"
#endif
#ifndef KUMO_SIMD_VPCLMUL
#error "KUMO_SIMD_VPCLMUL is not defined"
#endif
#ifndef KUMO_SIMD_GFNI
#error "KUMO_SIMD_GFNI is not defined"
#endif
#ifndef KUMO_SIMD_AVXVNNI
#error "KUMO_SIMD_AVXVNNI is not defined"
#endif
#ifndef KUMO_SIMD_AVX512PF
#error "KUMO_SIMD_AVX512PF is not defined"
#endif
#ifndef KUMO_SIMD_AVX512ER
#error "KUMO_SIMD_AVX512ER is not defined"
#endif
#ifndef KUMO_SIMD_AVX512BF16
#error "KUMO_SIMD_AVX512BF16 is not defined"
#endif
#ifndef KUMO_SIMD_AVX512FP16
#error "KUMO_SIMD_AVX512FP16 is not defined"
#endif
#ifndef KUMO_SIMD_AVX512VP2
#error "KUMO_SIMD_AVX512VP2 is not defined"
#endif
#ifndef KUMO_SIMD_AMX_TILE
#error "KUMO_SIMD_AMX_TILE is not defined"
#endif
#ifndef KUMO_SIMD_AMX_INT8
#error "KUMO_SIMD_AMX_INT8 is not defined"
#endif
#ifndef KUMO_SIMD_AMX_BF16
#error "KUMO_SIMD_AMX_BF16 is not defined"
#endif
#ifndef KUMO_SIMD_AMX_FP16
#error "KUMO_SIMD_AMX_FP16 is not defined"
#endif
#ifndef KUMO_SIMD_RVV
#error "KUMO_SIMD_RVV is not defined"
#endif
#ifndef KUMO_SIMD_LSX
#error "KUMO_SIMD_LSX is not defined"
#endif
#ifndef KUMO_SIMD_LASX
#error "KUMO_SIMD_LASX is not defined"
#endif
#ifndef KUMO_SIMD_ALTIVEC
#error "KUMO_SIMD_ALTIVEC is not defined"
#endif
#ifndef KUMO_SIMD_VSX
#error "KUMO_SIMD_VSX is not defined"
#endif
#ifndef KUMO_SIMD_CRYPTO
#error "KUMO_SIMD_CRYPTO is not defined"
#endif

#ifndef KUMO_CACHELINE_SIZE
#error "KUMO_CACHELINE_SIZE is not defined"
#endif
#ifndef KUMO_SIMD_LEVEL
#error "KUMO_SIMD_LEVEL is not defined"
#endif
#ifndef KUMO_ARCH_NAME
#error "KUMO_ARCH_NAME is not defined"
#endif
