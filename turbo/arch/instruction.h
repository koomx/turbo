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

#include <cstdint>

namespace turbo {

    enum InstructionSet {
        DEFAULT = 0x0,
        NEON = 0x1,
        AVX2 = 0x4,
        SSE42 = 0x8,
        PCLMULQDQ = 0x10,
        BMI1 = 0x20,
        BMI2 = 0x40,
        ALTIVEC = 0x80,
        AVX512F = 0x100,
        AVX512DQ = 0x200,
        AVX512IFMA = 0x400,
        AVX512PF = 0x800,
        AVX512ER = 0x1000,
        AVX512CD = 0x2000,
        AVX512BW = 0x4000,
        AVX512VL = 0x8000,
        AVX512VBMI2 = 0x10000,
        AVX512VPOPCNTDQ = 0x2000,
        RVV = 0x4000,
        ZVBB = 0x8000,
        LSX = 0x40000,
        LASX = 0x80000,
    };
/*
    struct cpuinfo_x86_isa {
#if CPUINFO_ARCH_X86
        bool rdtsc;
#endif
        bool rdtscp;
        bool rdpid;
        bool sysenter;
#if CPUINFO_ARCH_X86
        bool syscall;
#endif
        bool msr;
        bool clzero;
        bool clflush;
        bool clflushopt;
        bool mwait;
        bool mwaitx;
#if CPUINFO_ARCH_X86
        bool emmx;
#endif
        bool fxsave;
        bool xsave;
#if CPUINFO_ARCH_X86
        bool fpu;
        bool mmx;
        bool mmx_plus;
#endif
        bool three_d_now;
        bool three_d_now_plus;
#if CPUINFO_ARCH_X86
        bool three_d_now_geode;
#endif
        bool prefetch;
        bool prefetchw;
        bool prefetchwt1;
#if CPUINFO_ARCH_X86
        bool daz;
        bool sse;
        bool sse2;
#endif
        bool sse3;
        bool ssse3;
        bool sse4_1;
        bool sse4_2;
        bool sse4a;
        bool misaligned_sse;
        bool avx;
        bool avxvnni;
        bool fma3;
        bool fma4;
        bool xop;
        bool f16c;
        bool avx2;
        bool avx512f;
        bool avx512pf;
        bool avx512er;
        bool avx512cd;
        bool avx512dq;
        bool avx512bw;
        bool avx512vl;
        bool avx512ifma;
        bool avx512vbmi;
        bool avx512vbmi2;
        bool avx512bitalg;
        bool avx512vpopcntdq;
        bool avx512vnni;
        bool avx512bf16;
        bool avx512fp16;
        bool avx512vp2intersect;
        bool avx512_4vnniw;
        bool avx512_4fmaps;
        bool avx10_1;
        bool avx10_2;
        bool amx_bf16;
        bool amx_tile;
        bool amx_int8;
        bool amx_fp16;
        bool amx_fp8;
        bool avx_vnni_int8;
        bool avx_vnni_int16;
        bool avx_ne_convert;
        bool hle;
        bool rtm;
        bool xtest;
        bool mpx;
#if CPUINFO_ARCH_X86
        bool cmov;
        bool cmpxchg8b;
#endif
        bool cmpxchg16b;
        bool clwb;
        bool movbe;
#if CPUINFO_ARCH_X86_64
        bool lahf_sahf;
#endif
        bool fs_gs_base;
        bool lzcnt;
        bool popcnt;
        bool tbm;
        bool bmi;
        bool bmi2;
        bool adx;
        bool aes;
        bool vaes;
        bool pclmulqdq;
        bool vpclmulqdq;
        bool gfni;
        bool rdrand;
        bool rdseed;
        bool sha;
        bool rng;
        bool ace;
        bool ace2;
        bool phe;
        bool pmm;
        bool lwp;
    };
*/
} // namespace turbo
