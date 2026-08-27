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
#include <map>
#include <string>
#include <string_view>
#include <vector>

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

    /// Generation base for FNO = LevelRank + sub (0–99).
    /// Arch families use disjoint 10000-wide ranges so IDs never collide.
    enum class LevelRank : uint32_t {
        kDefault = 0,

        // x86 / x86-64  (0)
        kSse2 = 100,
        kSse4 = 200,
        kAvx = 300,
        kAvx2 = 400,
        kAvx512 = 500,
        kAmx = 600,
        kAvx10 = 700,

        // ARM / ARM64  (10000)
        kNeon = 10100,
        kSve = 10200,
        kSve2 = 10300,
        kSme = 10400,

        // LoongArch  (20000)
        kLsx = 20100,
        kLasx = 20200,

        // RISC-V  (30000)
        kRvv = 30100,

        // PowerPC  (40000)
        kAltivec = 40100,
        kVsx = 40200,
    };
    ////////////////////////////////////////////////////////////////
    /// x86 / x64 FNO = LevelRank + sub (0–99).
    /// baseline 1–99 | SSE2 100 | SSE4 200 | AVX 300 | AVX2 400 | AVX512 500 | AMX 600 | AVX10 700
    static constexpr uint32_t kX86Rdtsc = 1;
    static constexpr uint32_t kX86Rdtscp = 2;
    static constexpr uint32_t kX86Rdpid = 3;
    static constexpr uint32_t kX86Sysenter = 4;
    static constexpr uint32_t kX86Syscall = 5;
    static constexpr uint32_t kX86Msr = 6;
    static constexpr uint32_t kX86Clzero = 7;
    static constexpr uint32_t kX86Clflush = 8;
    static constexpr uint32_t kX86Clflushopt = 9;
    static constexpr uint32_t kX86Mwait = 10;
    static constexpr uint32_t kX86Mwaitx = 11;
    static constexpr uint32_t kX86Emmx = 12;
    static constexpr uint32_t kX86Fxsave = 13;
    static constexpr uint32_t kX86Xsave = 14;
    static constexpr uint32_t kX86Fpu = 15;
    static constexpr uint32_t kX86Mmx = 16;
    static constexpr uint32_t kX86MmxPlus = 17;
    static constexpr uint32_t kX86ThreeDNow = 18;
    static constexpr uint32_t kX86ThreeDNowPlus = 19;
    static constexpr uint32_t kX86ThreeDNowGeode = 20;
    static constexpr uint32_t kX86Prefetch = 21;
    static constexpr uint32_t kX86Prefetchw = 22;
    static constexpr uint32_t kX86Prefetchwt1 = 23;
    static constexpr uint32_t kX86Daz = 24;
    static constexpr uint32_t kX86Sse = 25;
    static constexpr uint32_t kX86Cmov = 26;
    static constexpr uint32_t kX86Cmpxchg8b = 27;
    static constexpr uint32_t kX86Cmpxchg16b = 28;
    static constexpr uint32_t kX86Clwb = 29;
    static constexpr uint32_t kX86Movbe = 30;
    static constexpr uint32_t kX86LahfSahf = 31;
    static constexpr uint32_t kX86FsGsBase = 32;
    static constexpr uint32_t kX86Hle = 33;
    static constexpr uint32_t kX86Rtm = 34;
    static constexpr uint32_t kX86Xtest = 35;
    static constexpr uint32_t kX86Mpx = 36;
    static constexpr uint32_t kX86Rdrand = 37;
    static constexpr uint32_t kX86Rdseed = 38;
    static constexpr uint32_t kX86Rng = 39;
    static constexpr uint32_t kX86Ace = 40;
    static constexpr uint32_t kX86Ace2 = 41;
    static constexpr uint32_t kX86Phe = 42;
    static constexpr uint32_t kX86Pmm = 43;
    static constexpr uint32_t kX86Lwp = 44;

    static constexpr uint32_t kX86Sse2 = 100;
    static constexpr uint32_t kX86Sse3 = 101;
    static constexpr uint32_t kX86Ssse3 = 102;
    static constexpr uint32_t kX86MisalignedSse = 103;

    static constexpr uint32_t kX86Sse4_1 = 200;
    static constexpr uint32_t kX86Sse4_2 = 201;
    static constexpr uint32_t kX86Sse4a = 202;
    static constexpr uint32_t kX86Popcnt = 203;
    static constexpr uint32_t kX86Lzcnt = 204;
    static constexpr uint32_t kX86Pclmulqdq = 205;
    static constexpr uint32_t kX86Aes = 206;

    static constexpr uint32_t kX86Avx = 300;
    static constexpr uint32_t kX86Fma3 = 301;
    static constexpr uint32_t kX86Fma4 = 302;
    static constexpr uint32_t kX86Xop = 303;
    static constexpr uint32_t kX86F16c = 304;

    static constexpr uint32_t kX86Avx2 = 400;
    static constexpr uint32_t kX86Bmi = 401;
    static constexpr uint32_t kX86Bmi2 = 402;
    static constexpr uint32_t kX86Adx = 403;
    static constexpr uint32_t kX86Tbm = 404;
    static constexpr uint32_t kX86Sha = 405;
    static constexpr uint32_t kX86Gfni = 406;
    static constexpr uint32_t kX86Vaes = 407;
    static constexpr uint32_t kX86Vpclmulqdq = 408;
    static constexpr uint32_t kX86AvxVnni = 409;
    static constexpr uint32_t kX86AvxVnniInt8 = 410;
    static constexpr uint32_t kX86AvxVnniInt16 = 411;
    static constexpr uint32_t kX86AvxNeConvert = 412;

    static constexpr uint32_t kX86Avx512F = 500;
    static constexpr uint32_t kX86Avx512Pf = 501;
    static constexpr uint32_t kX86Avx512Er = 502;
    static constexpr uint32_t kX86Avx512Cd = 503;
    static constexpr uint32_t kX86Avx512Dq = 504;
    static constexpr uint32_t kX86Avx512Bw = 505;
    static constexpr uint32_t kX86Avx512Vl = 506;
    static constexpr uint32_t kX86Avx512Ifma = 507;
    static constexpr uint32_t kX86Avx512Vbmi = 508;
    static constexpr uint32_t kX86Avx512Vbmi2 = 509;
    static constexpr uint32_t kX86Avx512Bitalg = 510;
    static constexpr uint32_t kX86Avx512Vpopcntdq = 511;
    static constexpr uint32_t kX86Avx512Vnni = 512;
    static constexpr uint32_t kX86Avx512Bf16 = 513;
    static constexpr uint32_t kX86Avx512Fp16 = 514;
    static constexpr uint32_t kX86Avx512Vp2intersect = 515;
    static constexpr uint32_t kX86Avx512_4vnniw = 516;
    static constexpr uint32_t kX86Avx512_4fmaps = 517;

    static constexpr uint32_t kX86AmxTile = 600;
    static constexpr uint32_t kX86AmxInt8 = 601;
    static constexpr uint32_t kX86AmxBf16 = 602;
    static constexpr uint32_t kX86AmxFp16 = 603;
    static constexpr uint32_t kX86AmxFp8 = 604;

    static constexpr uint32_t kX86Avx10_1 = 700;
    static constexpr uint32_t kX86Avx10_2 = 701;

    ////////////////////////////////////////////////////////////////
    /// ARM / ARM64 FNO = LevelRank + sub (0–99).
    /// baseline 10001–10099 | NEON 10100 | SVE 10200 | SVE2 10300 | SME 10400
    static constexpr uint32_t kArmThumb = 10001;
    static constexpr uint32_t kArmThumb2 = 10002;
    static constexpr uint32_t kArmThumbee = 10003;
    static constexpr uint32_t kArmJazelle = 10004;
    static constexpr uint32_t kArmArmv5e = 10005;
    static constexpr uint32_t kArmArmv6 = 10006;
    static constexpr uint32_t kArmArmv6k = 10007;
    static constexpr uint32_t kArmArmv7 = 10008;
    static constexpr uint32_t kArmArmv7mp = 10009;
    static constexpr uint32_t kArmArmv8 = 10010;
    static constexpr uint32_t kArmIdiv = 10011;
    static constexpr uint32_t kArmVfpv2 = 10012;
    static constexpr uint32_t kArmVfpv3 = 10013;
    static constexpr uint32_t kArmD32 = 10014;
    static constexpr uint32_t kArmFp16 = 10015;
    static constexpr uint32_t kArmFma = 10016;
    static constexpr uint32_t kArmWmmx = 10017;
    static constexpr uint32_t kArmWmmx2 = 10018;

    static constexpr uint32_t kArmNeon = 10100;
    static constexpr uint32_t kArmAtomics = 10101;
    static constexpr uint32_t kArmRdm = 10102;
    static constexpr uint32_t kArmFp16arith = 10103;
    static constexpr uint32_t kArmDot = 10104;
    static constexpr uint32_t kArmJscvt = 10105;
    static constexpr uint32_t kArmFcma = 10106;
    static constexpr uint32_t kArmFhm = 10107;
    static constexpr uint32_t kArmAes = 10108;
    static constexpr uint32_t kArmSha1 = 10109;
    static constexpr uint32_t kArmSha2 = 10110;
    static constexpr uint32_t kArmPmull = 10111;
    static constexpr uint32_t kArmCrc32 = 10112;

    static constexpr uint32_t kArmSve = 10200;
    static constexpr uint32_t kArmBf16 = 10201;
    static constexpr uint32_t kArmI8mm = 10202;

    static constexpr uint32_t kArmSve2 = 10300;

    static constexpr uint32_t kArmSme = 10400;
    static constexpr uint32_t kArmSme2 = 10401;
    static constexpr uint32_t kArmSme2p1 = 10402;
    static constexpr uint32_t kArmSmeI16i32 = 10403;
    static constexpr uint32_t kArmSmeBi32i32 = 10404;
    static constexpr uint32_t kArmSmeB16b16 = 10405;
    static constexpr uint32_t kArmSmeF16f16 = 10406;
    static constexpr uint32_t kArmFp8 = 10407;
    static constexpr uint32_t kArmF8dot = 10408;
    static constexpr uint32_t kArmF8mm = 10409;

    ////////////////////////////////////////////////////////////////
    /// RISC-V FNO = LevelRank + sub (0–99).
    /// baseline 30001–30099 | RVV 30100
    static constexpr uint32_t kRiscvI = 30001;
    static constexpr uint32_t kRiscvE = 30002;
    static constexpr uint32_t kRiscvM = 30003;
    static constexpr uint32_t kRiscvA = 30004;
    static constexpr uint32_t kRiscvF = 30005;
    static constexpr uint32_t kRiscvD = 30006;
    static constexpr uint32_t kRiscvC = 30007;
    static constexpr uint32_t kRiscvZfh = 30008;
    static constexpr uint32_t kRiscvV = 30100;
    static constexpr uint32_t kRiscvZvfh = 30101;

    ////////////////////////////////////////////////////////////////
    /// LoongArch FNO = LevelRank + sub (0–99).
    /// LSX 20100 | LASX 20200
    static constexpr uint32_t kLoongLsx = 20100;
    static constexpr uint32_t kLoongLasx = 20200;

    ////////////////////////////////////////////////////////////////
    /// PowerPC FNO = LevelRank + sub (0–99).
    /// AltiVec 40100 | VSX 40200
    static constexpr uint32_t kPpcAltivec = 40100;
    static constexpr uint32_t kPpcVsx = 40200;

    struct CpuIsaX86 {
        bool is_this_arch{false};
        /// kX86Rdtsc - 1
        bool rdtsc;
        /// kX86Rdtscp - 2
        bool rdtscp;
        /// kX86Rdpid - 3
        bool rdpid;
        /// kX86Sysenter - 4
        bool sysenter;
        /// kX86Syscall - 5
        bool syscall;
        /// kX86Msr - 6
        bool msr;
        /// kX86Clzero - 7
        bool clzero;
        /// kX86Clflush - 8
        bool clflush;
        /// kX86Clflushopt - 9
        bool clflushopt;
        /// kX86Mwait - 10
        bool mwait;
        /// kX86Mwaitx - 11
        bool mwaitx;
        /// kX86Emmx - 12
        bool emmx;
        /// kX86Fxsave - 13
        bool fxsave;
        /// kX86Xsave - 14
        bool xsave;
        /// kX86Fpu - 15
        bool fpu;
        /// kX86Mmx - 16
        bool mmx;
        /// kX86MmxPlus - 17
        bool mmx_plus;
        /// kX86ThreeDNow - 18
        bool three_d_now;
        /// kX86ThreeDNowPlus - 19
        bool three_d_now_plus;
        /// kX86ThreeDNowGeode - 20
        bool three_d_now_geode;
        /// kX86Prefetch - 21
        bool prefetch;
        /// kX86Prefetchw - 22
        bool prefetchw;
        /// kX86Prefetchwt1 - 23
        bool prefetchwt1;
        /// kX86Daz - 24
        bool daz;
        /// kX86Sse - 25
        bool sse;
        /// kX86Sse2 - 100
        bool sse2;
        /// kX86Sse3 - 101
        bool sse3;
        /// kX86Ssse3 - 102
        bool ssse3;
        /// kX86Sse4_1 - 200
        bool sse4_1;
        /// kX86Sse4_2 - 201
        bool sse4_2;
        /// kX86Sse4a - 202
        bool sse4a;
        /// kX86MisalignedSse - 103
        bool misaligned_sse;
        /// kX86Avx - 300
        bool avx;
        /// kX86AvxVnni - 409
        bool avxvnni;
        /// kX86Fma3 - 301
        bool fma3;
        /// kX86Fma4 - 302
        bool fma4;
        /// kX86Xop - 303
        bool xop;
        /// kX86F16c - 304
        bool f16c;
        /// kX86Avx2 - 400
        bool avx2;
        /// kX86Avx512F - 500
        bool avx512f;
        /// kX86Avx512Pf - 501
        bool avx512pf;
        /// kX86Avx512Er - 502
        bool avx512er;
        /// kX86Avx512Cd - 503
        bool avx512cd;
        /// kX86Avx512Dq - 504
        bool avx512dq;
        /// kX86Avx512Bw - 505
        bool avx512bw;
        /// kX86Avx512Vl - 506
        bool avx512vl;
        /// kX86Avx512Ifma - 507
        bool avx512ifma;
        /// kX86Avx512Vbmi - 508
        bool avx512vbmi;
        /// kX86Avx512Vbmi2 - 509
        bool avx512vbmi2;
        /// kX86Avx512Bitalg - 510
        bool avx512bitalg;
        /// kX86Avx512Vpopcntdq - 511
        bool avx512vpopcntdq;
        /// kX86Avx512Vnni - 512
        bool avx512vnni;
        /// kX86Avx512Bf16 - 513
        bool avx512bf16;
        /// kX86Avx512Fp16 - 514
        bool avx512fp16;
        /// kX86Avx512Vp2intersect - 515
        bool avx512vp2intersect;
        /// kX86Avx512_4vnniw - 516
        bool avx512_4vnniw;
        /// kX86Avx512_4fmaps - 517
        bool avx512_4fmaps;
        /// kX86Avx10_1 - 700
        bool avx10_1;
        /// kX86Avx10_2 - 701
        bool avx10_2;
        /// kX86AmxBf16 - 602
        bool amx_bf16;
        /// kX86AmxTile - 600
        bool amx_tile;
        /// kX86AmxInt8 - 601
        bool amx_int8;
        /// kX86AmxFp16 - 603
        bool amx_fp16;
        /// kX86AmxFp8 - 604
        bool amx_fp8;
        /// kX86AvxVnniInt8 - 410
        bool avx_vnni_int8;
        /// kX86AvxVnniInt16 - 411
        bool avx_vnni_int16;
        /// kX86AvxNeConvert - 412
        bool avx_ne_convert;
        /// kX86Hle - 33
        bool hle;
        /// kX86Rtm - 34
        bool rtm;
        /// kX86Xtest - 35
        bool xtest;
        /// kX86Mpx - 36
        bool mpx;
        /// kX86Cmov - 26
        bool cmov;
        /// kX86Cmpxchg8b - 27
        bool cmpxchg8b;
        /// kX86Cmpxchg16b - 28
        bool cmpxchg16b;
        /// kX86Clwb - 29
        bool clwb;
        /// kX86Movbe - 30
        bool movbe;
        /// kX86LahfSahf - 31
        bool lahf_sahf;
        /// kX86FsGsBase - 32
        bool fs_gs_base;
        /// kX86Lzcnt - 204
        bool lzcnt;
        /// kX86Popcnt - 203
        bool popcnt;
        /// kX86Tbm - 404
        bool tbm;
        /// kX86Bmi - 401
        bool bmi;
        /// kX86Bmi2 - 402
        bool bmi2;
        /// kX86Adx - 403
        bool adx;
        /// kX86Aes - 206
        bool aes;
        /// kX86Vaes - 407
        bool vaes;
        /// kX86Pclmulqdq - 205
        bool pclmulqdq;
        /// kX86Vpclmulqdq - 408
        bool vpclmulqdq;
        /// kX86Gfni - 406
        bool gfni;
        /// kX86Rdrand - 37
        bool rdrand;
        /// kX86Rdseed - 38
        bool rdseed;
        /// kX86Sha - 405
        bool sha;
        /// kX86Rng - 39
        bool rng;
        /// kX86Ace - 40
        bool ace;
        /// kX86Ace2 - 41
        bool ace2;
        /// kX86Phe - 42
        bool phe;
        /// kX86Pmm - 43
        bool pmm;
        /// kX86Lwp - 44
        bool lwp;
    };

    struct CpuIsaArm {
        bool is_this_arch{false};
        /// kArmThumb - 10001
        bool thumb;
        /// kArmThumb2 - 10002
        bool thumb2;
        /// kArmThumbee - 10003
        bool thumbee;
        /// kArmJazelle - 10004
        bool jazelle;
        /// kArmArmv5e - 10005
        bool armv5e;
        /// kArmArmv6 - 10006
        bool armv6;
        /// kArmArmv6k - 10007
        bool armv6k;
        /// kArmArmv7 - 10008
        bool armv7;
        /// kArmArmv7mp - 10009
        bool armv7mp;
        /// kArmArmv8 - 10010
        bool armv8;
        /// kArmIdiv - 10011
        bool idiv;

        /// kArmVfpv2 - 10012
        bool vfpv2;
        /// kArmVfpv3 - 10013
        bool vfpv3;
        /// kArmD32 - 10014
        bool d32;
        /// kArmFp16 - 10015
        bool fp16;
        /// kArmFma - 10016
        bool fma;

        /// kArmWmmx - 10017
        bool wmmx;
        /// kArmWmmx2 - 10018
        bool wmmx2;
        /// kArmNeon - 10100
        bool neon;
        /// kArmAtomics - 10101
        bool atomics;
        /// kArmBf16 - 10201
        bool bf16;
        /// kArmSve - 10200
        bool sve;
        /// kArmSve2 - 10300
        bool sve2;
        /// kArmI8mm - 10202
        bool i8mm;
        /// kArmSme - 10400
        bool sme;
        /// kArmSme2 - 10401
        bool sme2;
        /// kArmSme2p1 - 10402
        bool sme2p1;
        /// kArmSmeI16i32 - 10403
        bool sme_i16i32;
        /// kArmSmeBi32i32 - 10404
        bool sme_bi32i32;
        /// kArmSmeB16b16 - 10405
        bool sme_b16b16;
        /// kArmSmeF16f16 - 10406
        bool sme_f16f16;
        /// kArmFp8 - 10407
        bool fp8;
        /// kArmF8dot - 10408
        bool f8dot;
        /// kArmF8mm - 10409
        bool f8mm;
        uint32_t svelen;
        uint32_t smelen;
        /// kArmRdm - 10102
        bool rdm;
        /// kArmFp16arith - 10103
        bool fp16arith;
        /// kArmDot - 10104
        bool dot;
        /// kArmJscvt - 10105
        bool jscvt;
        /// kArmFcma - 10106
        bool fcma;
        /// kArmFhm - 10107
        bool fhm;

        /// kArmAes - 10108
        bool aes;
        /// kArmSha1 - 10109
        bool sha1;
        /// kArmSha2 - 10110
        bool sha2;
        /// kArmPmull - 10111
        bool pmull;
        /// kArmCrc32 - 10112
        bool crc32;
    };

    struct CpuIsaRiscv {
        bool is_this_arch{false};
        /**
         * Keep fields in line with the canonical order as defined by
         * Section 27.11 Subset Naming Convention.
         */
        /// kRiscvI - 30001
        bool i;
        /// kRiscvE - 30002
        bool e;
        /// kRiscvM - 30003
        bool m;
        /// kRiscvA - 30004
        bool a;
        /// kRiscvF - 30005
        bool f;
        /// kRiscvD - 30006
        bool d;
        /// kRiscvC - 30007
        bool c;
        /// kRiscvV - 30100
        bool v;

        /// kRiscvZfh - 30008
        bool zfh;
        /// kRiscvZvfh - 30101
        bool zvfh;
    };

    struct CpuIsaLoong {
        bool is_this_arch{false};
        /// kLoongLsx - 20100
        bool lsx;
        /// kLoongLasx - 20200
        bool lasx;
    };

    struct CpuIsaPpc {
        bool is_this_arch{false};
        /// kPpcAltivec - 40100
        bool altivec;
        /// kPpcVsx - 40200
        bool vsx;
    };

    struct CpuIsaInfo {
        CpuIsaX86 x86_isa;
        CpuIsaArm arm_isa;
        CpuIsaRiscv riscv_isa;
        CpuIsaLoong loong_isa;
        CpuIsaPpc ppc_isa;
    };

    /// ISA metadata DB keyed by FNO. `cpu_isa_meta(fno) == nullptr` if unknown.
    /// Rank / arch band are derived from `fno` (see `cpu_isa_level_rank`).
    struct CpuIsaMeta {
        uint32_t fno{0};
        bool is_bits_32{false};
        bool is_bits_64{false};
        /// eg. "avx512f"
        const char* name{""};
        /// eg. "x64", "arm64"
        const char* arch_name{""};
        /// eg. "avx512"
        const char* level_name{""};
        std::vector<const char*> gnu_flags;
        std::vector<const char*> clang_flags;
        std::vector<const char*> msvc_flags;
        /// Hint only, e.g. "KUMO_SIMD_AVX512F". Not used for detection.
        std::vector<const char*> detect_macros;
    };

    constexpr uint32_t cpu_isa_level_rank(uint32_t fno) {
        return (fno / 100u) * 100u;
    }

    constexpr uint32_t cpu_isa_sub_level_rank(uint32_t fno) {
        return fno % 100u;
    }

    const std::map<uint32_t, CpuIsaMeta>& cpu_isa_catalog();
    const CpuIsaMeta* cpu_isa_meta(uint32_t fno);

    CpuIsaInfo convert_feature_to_isa_info(const std::vector<uint32_t> &info, std::string &err);

    CpuIsaInfo convert_feature_to_isa_info(const std::map<uint32_t,CpuIsaMeta*> &infos,std::string &err);

    std::map<uint32_t,CpuIsaMeta*> convert_isa_info_to_feature(CpuIsaInfo info, std::string &err);

    CpuIsaInfo detect_cpu_isa_info();

    CpuIsaInfo detect_current_enabled_isa_info();

} // namespace turbo
