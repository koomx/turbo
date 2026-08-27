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

#include <turbo/arch/instruction.h>
#include <turbo/macros/config.h>

#include <cstring>
#include <sstream>
#include <utility>

namespace turbo {
    namespace {

        CpuIsaMeta make_meta(uint32_t fno, bool bits32, bool bits64, const char* name, const char* arch,
            const char* level, std::vector<const char*> gnu, std::vector<const char*> clang,
            std::vector<const char*> msvc, std::vector<const char*> detect) {
            return CpuIsaMeta { fno,
                bits32,
                bits64,
                name,
                arch,
                level,
                std::move(gnu),
                std::move(clang),
                std::move(msvc),
                std::move(detect) };
        }

        CpuIsaMeta x86(uint32_t fno, const char* name, const char* level, std::vector<const char*> flags = { },
            std::vector<const char*> msvc = { }, std::vector<const char*> detect = { }) {
            auto clang = flags;
            return make_meta(fno, true, true, name, "x86", level, std::move(flags), std::move(clang), std::move(msvc),
                std::move(detect));
        }

        CpuIsaMeta arm(uint32_t fno, const char* name, const char* level, std::vector<const char*> flags = { },
            std::vector<const char*> detect = { }) {
            auto clang = flags;
            return make_meta(fno, true, true, name, "arm", level, std::move(flags), std::move(clang), { }, std::move(detect));
        }

        CpuIsaMeta loong(uint32_t fno, const char* name, const char* level, std::vector<const char*> flags,
            std::vector<const char*> detect) {
            auto clang = flags;
            return make_meta(fno, false, true, name, "loongarch", level, std::move(flags), std::move(clang), { },
                std::move(detect));
        }

        CpuIsaMeta riscv(uint32_t fno, const char* name, const char* level, std::vector<const char*> flags = { },
            std::vector<const char*> detect = { }) {
            auto clang = flags;
            return make_meta(fno, true, true, name, "riscv", level, std::move(flags), std::move(clang), { }, std::move(detect));
        }

        CpuIsaMeta ppc(uint32_t fno, const char* name, const char* level, std::vector<const char*> flags,
            std::vector<const char*> detect) {
            auto clang = flags;
            return make_meta(fno, false, true, name, "ppc", level, std::move(flags), std::move(clang), { }, std::move(detect));
        }

        void insert(std::map<uint32_t, CpuIsaMeta>& catalog, CpuIsaMeta row) {
            const uint32_t fno = row.fno;
            catalog.emplace(fno, std::move(row));
        }

        std::map<uint32_t, CpuIsaMeta> build_catalog() {
            std::map<uint32_t, CpuIsaMeta> catalog;

            insert(catalog, x86(kX86Rdtsc, "rdtsc", "baseline"));
            insert(catalog, x86(kX86Rdtscp, "rdtscp", "baseline"));
            insert(catalog, x86(kX86Rdpid, "rdpid", "baseline", { "-mrdpid" }));
            insert(catalog, x86(kX86Sysenter, "sysenter", "baseline"));
            insert(catalog, x86(kX86Syscall, "syscall", "baseline"));
            insert(catalog, x86(kX86Msr, "msr", "baseline"));
            insert(catalog, x86(kX86Clzero, "clzero", "baseline", { "-mclzero" }));
            insert(catalog, x86(kX86Clflush, "clflush", "baseline"));
            insert(catalog, x86(kX86Clflushopt, "clflushopt", "baseline", { "-mclflushopt" }));
            insert(catalog, x86(kX86Mwait, "mwait", "baseline"));
            insert(catalog, x86(kX86Mwaitx, "mwaitx", "baseline", { "-mmwaitx" }));
            insert(catalog, x86(kX86Emmx, "emmx", "baseline"));
            insert(catalog, x86(kX86Fxsave, "fxsave", "baseline"));
            insert(catalog, x86(kX86Xsave, "xsave", "baseline", { "-mxsave" }));
            insert(catalog, x86(kX86Fpu, "fpu", "baseline"));
            insert(catalog, x86(kX86Mmx, "mmx", "baseline", { "-mmmx" }));
            insert(catalog, x86(kX86MmxPlus, "mmx_plus", "baseline"));
            insert(catalog, x86(kX86ThreeDNow, "three_d_now", "baseline", { "-m3dnow" }));
            insert(catalog, x86(kX86ThreeDNowPlus, "three_d_now_plus", "baseline", { "-m3dnowa" }));
            insert(catalog, x86(kX86ThreeDNowGeode, "three_d_now_geode", "baseline"));
            insert(catalog, x86(kX86Prefetch, "prefetch", "baseline"));
            insert(catalog, x86(kX86Prefetchw, "prefetchw", "baseline", { "-mprfchw" }));
            insert(catalog, x86(kX86Prefetchwt1, "prefetchwt1", "baseline", { "-mprefetchwt1" }));
            insert(catalog, x86(kX86Daz, "daz", "baseline"));
            insert(catalog, x86(kX86Sse, "sse", "baseline", { "-msse" }, { }, { "KUMO_SIMD_SSE" }));
            insert(catalog, x86(kX86Cmov, "cmov", "baseline"));
            insert(catalog, x86(kX86Cmpxchg8b, "cmpxchg8b", "baseline"));
            insert(catalog, x86(kX86Cmpxchg16b, "cmpxchg16b", "baseline", { "-mcx16" }));
            insert(catalog, x86(kX86Clwb, "clwb", "baseline", { "-mclwb" }));
            insert(catalog, x86(kX86Movbe, "movbe", "baseline", { "-mmovbe" }));
            insert(catalog, x86(kX86LahfSahf, "lahf_sahf", "baseline", { "-msahf" }));
            insert(catalog, x86(kX86FsGsBase, "fs_gs_base", "baseline", { "-mfsgsbase" }));
            insert(catalog, x86(kX86Hle, "hle", "baseline", { "-mhle" }));
            insert(catalog, x86(kX86Rtm, "rtm", "baseline", { "-mrtm" }));
            insert(catalog, x86(kX86Xtest, "xtest", "baseline"));
            insert(catalog, x86(kX86Mpx, "mpx", "baseline", { "-mmpx" }));
            insert(catalog, x86(kX86Rdrand, "rdrand", "baseline", { "-mrdrnd" }));
            insert(catalog, x86(kX86Rdseed, "rdseed", "baseline", { "-mrdseed" }));
            insert(catalog, x86(kX86Rng, "rng", "baseline"));
            insert(catalog, x86(kX86Ace, "ace", "baseline"));
            insert(catalog, x86(kX86Ace2, "ace2", "baseline"));
            insert(catalog, x86(kX86Phe, "phe", "baseline"));
            insert(catalog, x86(kX86Pmm, "pmm", "baseline"));
            insert(catalog, x86(kX86Lwp, "lwp", "baseline", { "-mlwp" }));

            insert(catalog, x86(kX86Sse2, "sse2", "sse2", { "-msse2" }, { "/arch:SSE2" }, { "KUMO_SIMD_SSE2" }));
            insert(catalog, x86(kX86Sse3, "sse3", "sse2", { "-msse3" }, { }, { "KUMO_SIMD_SSE3" }));
            insert(catalog, x86(kX86Ssse3, "ssse3", "sse2", { "-mssse3" }, { }, { "KUMO_SIMD_SSSE3" }));
            insert(catalog, x86(kX86MisalignedSse, "misaligned_sse", "sse2"));

            insert(catalog, x86(kX86Sse4_1, "sse4_1", "sse4", { "-msse4.1" }, { }, { "KUMO_SIMD_SSE4_1" }));
            insert(catalog, x86(kX86Sse4_2, "sse4_2", "sse4", { "-msse4.2" }, { }, { "KUMO_SIMD_SSE4_2" }));
            insert(catalog, x86(kX86Sse4a, "sse4a", "sse4", { "-msse4a" }));
            insert(catalog, x86(kX86Popcnt, "popcnt", "sse4", { "-mpopcnt" }, { }, { "KUMO_SIMD_POPCNT" }));
            insert(catalog, x86(kX86Lzcnt, "lzcnt", "sse4", { "-mlzcnt" }, { }, { "KUMO_SIMD_LZCNT" }));
            insert(catalog, x86(kX86Pclmulqdq, "pclmulqdq", "sse4", { "-mpclmul" }, { }, { "KUMO_SIMD_PCLMUL" }));
            insert(catalog, x86(kX86Aes, "aes", "sse4", { "-maes" }, { }, { "KUMO_SIMD_X86_AES" }));

            insert(catalog, x86(kX86Avx, "avx", "avx", { "-mavx" }, { "/arch:AVX" }, { "KUMO_SIMD_AVX" }));
            insert(catalog, x86(kX86Fma3, "fma3", "avx", { "-mfma" }, { }, { "KUMO_SIMD_FMA" }));
            insert(catalog, x86(kX86Fma4, "fma4", "avx", { "-mfma4" }));
            insert(catalog, x86(kX86Xop, "xop", "avx", { "-mxop" }));
            insert(catalog, x86(kX86F16c, "f16c", "avx", { "-mf16c" }));

            insert(catalog, x86(kX86Avx2, "avx2", "avx2", { "-mavx2" }, { "/arch:AVX2" }, { "KUMO_SIMD_AVX2" }));
            insert(catalog, x86(kX86Bmi, "bmi", "avx2", { "-mbmi" }, { }, { "KUMO_SIMD_BMI1" }));
            insert(catalog, x86(kX86Bmi2, "bmi2", "avx2", { "-mbmi2" }, { }, { "KUMO_SIMD_BMI2" }));
            insert(catalog, x86(kX86Adx, "adx", "avx2", { "-madx" }));
            insert(catalog, x86(kX86Tbm, "tbm", "avx2", { "-mtbm" }));
            insert(catalog, x86(kX86Sha, "sha", "avx2", { "-msha" }));
            insert(catalog, x86(kX86Gfni, "gfni", "avx2", { "-mgfni" }));
            insert(catalog, x86(kX86Vaes, "vaes", "avx2", { "-mvaes" }));
            insert(catalog, x86(kX86Vpclmulqdq, "vpclmulqdq", "avx2", { "-mvpclmulqdq" }));
            insert(catalog, x86(kX86AvxVnni, "avxvnni", "avx2", { "-mavxvnni" }));
            insert(catalog, x86(kX86AvxVnniInt8, "avx_vnni_int8", "avx2", { "-mavxvnniint8" }));
            insert(catalog, x86(kX86AvxVnniInt16, "avx_vnni_int16", "avx2", { "-mavxvnniint16" }));
            insert(catalog, x86(kX86AvxNeConvert, "avx_ne_convert", "avx2", { "-mavxneconvert" }));

            insert(catalog, x86(kX86Avx512F, "avx512f", "avx512", { "-mavx512f" }, { "/arch:AVX512" }, { "KUMO_SIMD_AVX512F" }));
            insert(catalog, x86(kX86Avx512Pf, "avx512pf", "avx512", { "-mavx512pf" }));
            insert(catalog, x86(kX86Avx512Er, "avx512er", "avx512", { "-mavx512er" }));
            insert(catalog, x86(kX86Avx512Cd, "avx512cd", "avx512", { "-mavx512cd" }, { }, { "KUMO_SIMD_AVX512CD" }));
            insert(catalog, x86(kX86Avx512Dq, "avx512dq", "avx512", { "-mavx512dq" }, { }, { "KUMO_SIMD_AVX512DQ" }));
            insert(catalog, x86(kX86Avx512Bw, "avx512bw", "avx512", { "-mavx512bw" }, { }, { "KUMO_SIMD_AVX512BW" }));
            insert(catalog, x86(kX86Avx512Vl, "avx512vl", "avx512", { "-mavx512vl" }, { }, { "KUMO_SIMD_AVX512VL" }));
            insert(catalog, x86(kX86Avx512Ifma, "avx512ifma", "avx512", { "-mavx512ifma" }, { }, { "KUMO_SIMD_AVX512IFMA" }));
            insert(catalog, x86(kX86Avx512Vbmi, "avx512vbmi", "avx512", { "-mavx512vbmi" }, { }, { "KUMO_SIMD_AVX512VBMI" }));
            insert(catalog, x86(kX86Avx512Vbmi2, "avx512vbmi2", "avx512", { "-mavx512vbmi2" }, { }, { "KUMO_SIMD_AVX512VBMI2" }));
            insert(catalog, x86(kX86Avx512Bitalg, "avx512bitalg", "avx512", { "-mavx512bitalg" }, { }, { "KUMO_SIMD_AVX512BITALG" }));
            insert(catalog, x86(kX86Avx512Vpopcntdq, "avx512vpopcntdq", "avx512", { "-mavx512vpopcntdq" }, { }, { "KUMO_SIMD_AVX512VPOPCNTDQ" }));
            insert(catalog, x86(kX86Avx512Vnni, "avx512vnni", "avx512", { "-mavx512vnni" }, { }, { "KUMO_SIMD_AVX512VNNI" }));
            insert(catalog, x86(kX86Avx512Bf16, "avx512bf16", "avx512", { "-mavx512bf16" }));
            insert(catalog, x86(kX86Avx512Fp16, "avx512fp16", "avx512", { "-mavx512fp16" }));
            insert(catalog, x86(kX86Avx512Vp2intersect, "avx512vp2intersect", "avx512", { "-mavx512vp2intersect" }));
            insert(catalog, x86(kX86Avx512_4vnniw, "avx512_4vnniw", "avx512", { "-mavx5124vnniw" }));
            insert(catalog, x86(kX86Avx512_4fmaps, "avx512_4fmaps", "avx512", { "-mavx5124fmaps" }));

            insert(catalog, x86(kX86AmxTile, "amx_tile", "amx", { "-mamx-tile" }));
            insert(catalog, x86(kX86AmxInt8, "amx_int8", "amx", { "-mamx-int8" }));
            insert(catalog, x86(kX86AmxBf16, "amx_bf16", "amx", { "-mamx-bf16" }));
            insert(catalog, x86(kX86AmxFp16, "amx_fp16", "amx", { "-mamx-fp16" }));
            insert(catalog, x86(kX86AmxFp8, "amx_fp8", "amx", { "-mamx-fp8" }));

            insert(catalog, x86(kX86Avx10_1, "avx10_1", "avx10", { "-mavx10.1" }));
            insert(catalog, x86(kX86Avx10_2, "avx10_2", "avx10", { "-mavx10.2" }));

            insert(catalog, arm(kArmThumb, "thumb", "baseline", { "-mthumb" }));
            insert(catalog, arm(kArmThumb2, "thumb2", "baseline"));
            insert(catalog, arm(kArmThumbee, "thumbee", "baseline"));
            insert(catalog, arm(kArmJazelle, "jazelle", "baseline"));
            insert(catalog, arm(kArmArmv5e, "armv5e", "baseline", { "-march=armv5te" }));
            insert(catalog, arm(kArmArmv6, "armv6", "baseline", { "-march=armv6" }));
            insert(catalog, arm(kArmArmv6k, "armv6k", "baseline", { "-march=armv6k" }));
            insert(catalog, arm(kArmArmv7, "armv7", "baseline", { "-march=armv7-a" }));
            insert(catalog, arm(kArmArmv7mp, "armv7mp", "baseline", { "-march=armv7-a" }));
            insert(catalog, arm(kArmArmv8, "armv8", "baseline", { "-march=armv8-a" }));
            insert(catalog, arm(kArmIdiv, "idiv", "baseline"));
            insert(catalog, arm(kArmVfpv2, "vfpv2", "baseline", { "-mfpu=vfp" }));
            insert(catalog, arm(kArmVfpv3, "vfpv3", "baseline", { "-mfpu=vfpv3" }));
            insert(catalog, arm(kArmD32, "d32", "baseline"));
            insert(catalog, arm(kArmFp16, "fp16", "baseline"));
            insert(catalog, arm(kArmFma, "fma", "baseline"));
            insert(catalog, arm(kArmWmmx, "wmmx", "baseline"));
            insert(catalog, arm(kArmWmmx2, "wmmx2", "baseline"));

            insert(catalog, arm(kArmNeon, "neon", "neon", { "-mfpu=neon" }, { "KUMO_SIMD_NEON" }));
            insert(catalog, arm(kArmAtomics, "atomics", "neon", { "-moutline-atomics" }));
            insert(catalog, arm(kArmRdm, "rdm", "neon", { "-march=armv8.1-a" }));
            insert(catalog, arm(kArmFp16arith, "fp16arith", "neon"));
            insert(catalog, arm(kArmDot, "dot", "neon", { "-march=armv8.2-a+dotprod" }));
            insert(catalog, arm(kArmJscvt, "jscvt", "neon"));
            insert(catalog, arm(kArmFcma, "fcma", "neon"));
            insert(catalog, arm(kArmFhm, "fhm", "neon"));
            insert(catalog, arm(kArmAes, "aes", "neon", { "-march=armv8-a+crypto" }, { "KUMO_SIMD_ARM_AES" }));
            insert(catalog, arm(kArmSha1, "sha1", "neon", { "-march=armv8-a+crypto" }));
            insert(catalog, arm(kArmSha2, "sha2", "neon", { "-march=armv8-a+crypto" }));
            insert(catalog, arm(kArmPmull, "pmull", "neon", { "-march=armv8-a+crypto" }));
            insert(catalog, arm(kArmCrc32, "crc32", "neon", { "-march=armv8-a+crc" }));

            insert(catalog, arm(kArmSve, "sve", "sve", { "-march=armv8-a+sve" }, { "KUMO_SIMD_SVE" }));
            insert(catalog, arm(kArmBf16, "bf16", "sve", { "-march=armv8.2-a+bf16" }));
            insert(catalog, arm(kArmI8mm, "i8mm", "sve", { "-march=armv8.2-a+i8mm" }));

            insert(catalog, arm(kArmSve2, "sve2", "sve2", { "-march=armv8-a+sve2" }, { "KUMO_SIMD_SVE2" }));

            insert(catalog, arm(kArmSme, "sme", "sme", { "-march=armv9-a+sme" }));
            insert(catalog, arm(kArmSme2, "sme2", "sme", { "-march=armv9-a+sme2" }));
            insert(catalog, arm(kArmSme2p1, "sme2p1", "sme"));
            insert(catalog, arm(kArmSmeI16i32, "sme_i16i32", "sme"));
            insert(catalog, arm(kArmSmeBi32i32, "sme_bi32i32", "sme"));
            insert(catalog, arm(kArmSmeB16b16, "sme_b16b16", "sme"));
            insert(catalog, arm(kArmSmeF16f16, "sme_f16f16", "sme"));
            insert(catalog, arm(kArmFp8, "fp8", "sme"));
            insert(catalog, arm(kArmF8dot, "f8dot", "sme"));
            insert(catalog, arm(kArmF8mm, "f8mm", "sme"));

            insert(catalog, loong(kLoongLsx, "lsx", "lsx", { "-mlsx" }, { "KUMO_SIMD_LSX" }));
            insert(catalog, loong(kLoongLasx, "lasx", "lasx", { "-mlasx" }, { "KUMO_SIMD_LASX" }));

            insert(catalog, riscv(kRiscvI, "i", "baseline", { "-march=rv64i" }));
            insert(catalog, riscv(kRiscvE, "e", "baseline", { "-march=rv32e" }));
            insert(catalog, riscv(kRiscvM, "m", "baseline"));
            insert(catalog, riscv(kRiscvA, "a", "baseline"));
            insert(catalog, riscv(kRiscvF, "f", "baseline"));
            insert(catalog, riscv(kRiscvD, "d", "baseline"));
            insert(catalog, riscv(kRiscvC, "c", "baseline"));
            insert(catalog, riscv(kRiscvZfh, "zfh", "baseline", { "-march=rv64gc_zfh" }));
            insert(catalog, riscv(kRiscvV, "v", "rvv", { "-march=rv64gcv" }, { "KUMO_SIMD_RVV" }));
            insert(catalog, riscv(kRiscvZvfh, "zvfh", "rvv"));

            insert(catalog, ppc(kPpcAltivec, "altivec", "altivec", { "-maltivec" }, { "KUMO_SIMD_ALTIVEC" }));
            insert(catalog, ppc(kPpcVsx, "vsx", "vsx", { "-mvsx" }, { "KUMO_SIMD_VSX" }));

            return catalog;
        }

    } // namespace

    const std::map<uint32_t, CpuIsaMeta>& cpu_isa_catalog() {
        static const std::map<uint32_t, CpuIsaMeta> catalog = build_catalog();
        return catalog;
    }

    const CpuIsaMeta* cpu_isa_meta(uint32_t fno) {
        const auto& catalog = cpu_isa_catalog();
        const auto it = catalog.find(fno);
        if (it == catalog.end()) {
            return nullptr;
        }
        return &it->second;
    }

    namespace {

        enum class CpuKind { kNone,
            kX86,
            kArm,
            kLoong,
            kRiscv,
            kPpc };

        CpuKind cpu_kind_from_arch_name(const char* arch_name) {
            if (std::strcmp(arch_name, "x86") == 0) {
                return CpuKind::kX86;
            }
            if (std::strcmp(arch_name, "arm") == 0) {
                return CpuKind::kArm;
            }
            if (std::strcmp(arch_name, "loongarch") == 0) {
                return CpuKind::kLoong;
            }
            if (std::strcmp(arch_name, "riscv") == 0) {
                return CpuKind::kRiscv;
            }
            if (std::strcmp(arch_name, "ppc") == 0) {
                return CpuKind::kPpc;
            }
            return CpuKind::kNone;
        }

        const char* cpu_kind_name(CpuKind kind) {
            switch (kind) {
            case CpuKind::kX86:
                return "x86";
            case CpuKind::kArm:
                return "arm";
            case CpuKind::kLoong:
                return "loongarch";
            case CpuKind::kRiscv:
                return "riscv";
            case CpuKind::kPpc:
                return "ppc";
            case CpuKind::kNone:
                return "none";
            }
            return "none";
        }

        bool apply_x86(CpuIsaX86& isa, uint32_t fno) {
            switch (fno) {
            case kX86Rdtsc:
                isa.rdtsc = true;
                return true;
            case kX86Rdtscp:
                isa.rdtscp = true;
                return true;
            case kX86Rdpid:
                isa.rdpid = true;
                return true;
            case kX86Sysenter:
                isa.sysenter = true;
                return true;
            case kX86Syscall:
                isa.syscall = true;
                return true;
            case kX86Msr:
                isa.msr = true;
                return true;
            case kX86Clzero:
                isa.clzero = true;
                return true;
            case kX86Clflush:
                isa.clflush = true;
                return true;
            case kX86Clflushopt:
                isa.clflushopt = true;
                return true;
            case kX86Mwait:
                isa.mwait = true;
                return true;
            case kX86Mwaitx:
                isa.mwaitx = true;
                return true;
            case kX86Emmx:
                isa.emmx = true;
                return true;
            case kX86Fxsave:
                isa.fxsave = true;
                return true;
            case kX86Xsave:
                isa.xsave = true;
                return true;
            case kX86Fpu:
                isa.fpu = true;
                return true;
            case kX86Mmx:
                isa.mmx = true;
                return true;
            case kX86MmxPlus:
                isa.mmx_plus = true;
                return true;
            case kX86ThreeDNow:
                isa.three_d_now = true;
                return true;
            case kX86ThreeDNowPlus:
                isa.three_d_now_plus = true;
                return true;
            case kX86ThreeDNowGeode:
                isa.three_d_now_geode = true;
                return true;
            case kX86Prefetch:
                isa.prefetch = true;
                return true;
            case kX86Prefetchw:
                isa.prefetchw = true;
                return true;
            case kX86Prefetchwt1:
                isa.prefetchwt1 = true;
                return true;
            case kX86Daz:
                isa.daz = true;
                return true;
            case kX86Sse:
                isa.sse = true;
                return true;
            case kX86Sse2:
                isa.sse2 = true;
                return true;
            case kX86Sse3:
                isa.sse3 = true;
                return true;
            case kX86Ssse3:
                isa.ssse3 = true;
                return true;
            case kX86Sse4_1:
                isa.sse4_1 = true;
                return true;
            case kX86Sse4_2:
                isa.sse4_2 = true;
                return true;
            case kX86Sse4a:
                isa.sse4a = true;
                return true;
            case kX86MisalignedSse:
                isa.misaligned_sse = true;
                return true;
            case kX86Avx:
                isa.avx = true;
                return true;
            case kX86AvxVnni:
                isa.avxvnni = true;
                return true;
            case kX86Fma3:
                isa.fma3 = true;
                return true;
            case kX86Fma4:
                isa.fma4 = true;
                return true;
            case kX86Xop:
                isa.xop = true;
                return true;
            case kX86F16c:
                isa.f16c = true;
                return true;
            case kX86Avx2:
                isa.avx2 = true;
                return true;
            case kX86Avx512F:
                isa.avx512f = true;
                return true;
            case kX86Avx512Pf:
                isa.avx512pf = true;
                return true;
            case kX86Avx512Er:
                isa.avx512er = true;
                return true;
            case kX86Avx512Cd:
                isa.avx512cd = true;
                return true;
            case kX86Avx512Dq:
                isa.avx512dq = true;
                return true;
            case kX86Avx512Bw:
                isa.avx512bw = true;
                return true;
            case kX86Avx512Vl:
                isa.avx512vl = true;
                return true;
            case kX86Avx512Ifma:
                isa.avx512ifma = true;
                return true;
            case kX86Avx512Vbmi:
                isa.avx512vbmi = true;
                return true;
            case kX86Avx512Vbmi2:
                isa.avx512vbmi2 = true;
                return true;
            case kX86Avx512Bitalg:
                isa.avx512bitalg = true;
                return true;
            case kX86Avx512Vpopcntdq:
                isa.avx512vpopcntdq = true;
                return true;
            case kX86Avx512Vnni:
                isa.avx512vnni = true;
                return true;
            case kX86Avx512Bf16:
                isa.avx512bf16 = true;
                return true;
            case kX86Avx512Fp16:
                isa.avx512fp16 = true;
                return true;
            case kX86Avx512Vp2intersect:
                isa.avx512vp2intersect = true;
                return true;
            case kX86Avx512_4vnniw:
                isa.avx512_4vnniw = true;
                return true;
            case kX86Avx512_4fmaps:
                isa.avx512_4fmaps = true;
                return true;
            case kX86Avx10_1:
                isa.avx10_1 = true;
                return true;
            case kX86Avx10_2:
                isa.avx10_2 = true;
                return true;
            case kX86AmxBf16:
                isa.amx_bf16 = true;
                return true;
            case kX86AmxTile:
                isa.amx_tile = true;
                return true;
            case kX86AmxInt8:
                isa.amx_int8 = true;
                return true;
            case kX86AmxFp16:
                isa.amx_fp16 = true;
                return true;
            case kX86AmxFp8:
                isa.amx_fp8 = true;
                return true;
            case kX86AvxVnniInt8:
                isa.avx_vnni_int8 = true;
                return true;
            case kX86AvxVnniInt16:
                isa.avx_vnni_int16 = true;
                return true;
            case kX86AvxNeConvert:
                isa.avx_ne_convert = true;
                return true;
            case kX86Hle:
                isa.hle = true;
                return true;
            case kX86Rtm:
                isa.rtm = true;
                return true;
            case kX86Xtest:
                isa.xtest = true;
                return true;
            case kX86Mpx:
                isa.mpx = true;
                return true;
            case kX86Cmov:
                isa.cmov = true;
                return true;
            case kX86Cmpxchg8b:
                isa.cmpxchg8b = true;
                return true;
            case kX86Cmpxchg16b:
                isa.cmpxchg16b = true;
                return true;
            case kX86Clwb:
                isa.clwb = true;
                return true;
            case kX86Movbe:
                isa.movbe = true;
                return true;
            case kX86LahfSahf:
                isa.lahf_sahf = true;
                return true;
            case kX86FsGsBase:
                isa.fs_gs_base = true;
                return true;
            case kX86Lzcnt:
                isa.lzcnt = true;
                return true;
            case kX86Popcnt:
                isa.popcnt = true;
                return true;
            case kX86Tbm:
                isa.tbm = true;
                return true;
            case kX86Bmi:
                isa.bmi = true;
                return true;
            case kX86Bmi2:
                isa.bmi2 = true;
                return true;
            case kX86Adx:
                isa.adx = true;
                return true;
            case kX86Aes:
                isa.aes = true;
                return true;
            case kX86Vaes:
                isa.vaes = true;
                return true;
            case kX86Pclmulqdq:
                isa.pclmulqdq = true;
                return true;
            case kX86Vpclmulqdq:
                isa.vpclmulqdq = true;
                return true;
            case kX86Gfni:
                isa.gfni = true;
                return true;
            case kX86Rdrand:
                isa.rdrand = true;
                return true;
            case kX86Rdseed:
                isa.rdseed = true;
                return true;
            case kX86Sha:
                isa.sha = true;
                return true;
            case kX86Rng:
                isa.rng = true;
                return true;
            case kX86Ace:
                isa.ace = true;
                return true;
            case kX86Ace2:
                isa.ace2 = true;
                return true;
            case kX86Phe:
                isa.phe = true;
                return true;
            case kX86Pmm:
                isa.pmm = true;
                return true;
            case kX86Lwp:
                isa.lwp = true;
                return true;
            default:
                return false;
            }
        }

        bool apply_arm(CpuIsaArm& isa, uint32_t fno) {
            switch (fno) {
            case kArmThumb:
                isa.thumb = true;
                return true;
            case kArmThumb2:
                isa.thumb2 = true;
                return true;
            case kArmThumbee:
                isa.thumbee = true;
                return true;
            case kArmJazelle:
                isa.jazelle = true;
                return true;
            case kArmArmv5e:
                isa.armv5e = true;
                return true;
            case kArmArmv6:
                isa.armv6 = true;
                return true;
            case kArmArmv6k:
                isa.armv6k = true;
                return true;
            case kArmArmv7:
                isa.armv7 = true;
                return true;
            case kArmArmv7mp:
                isa.armv7mp = true;
                return true;
            case kArmArmv8:
                isa.armv8 = true;
                return true;
            case kArmIdiv:
                isa.idiv = true;
                return true;
            case kArmVfpv2:
                isa.vfpv2 = true;
                return true;
            case kArmVfpv3:
                isa.vfpv3 = true;
                return true;
            case kArmD32:
                isa.d32 = true;
                return true;
            case kArmFp16:
                isa.fp16 = true;
                return true;
            case kArmFma:
                isa.fma = true;
                return true;
            case kArmWmmx:
                isa.wmmx = true;
                return true;
            case kArmWmmx2:
                isa.wmmx2 = true;
                return true;
            case kArmNeon:
                isa.neon = true;
                return true;
            case kArmAtomics:
                isa.atomics = true;
                return true;
            case kArmBf16:
                isa.bf16 = true;
                return true;
            case kArmSve:
                isa.sve = true;
                return true;
            case kArmSve2:
                isa.sve2 = true;
                return true;
            case kArmI8mm:
                isa.i8mm = true;
                return true;
            case kArmSme:
                isa.sme = true;
                return true;
            case kArmSme2:
                isa.sme2 = true;
                return true;
            case kArmSme2p1:
                isa.sme2p1 = true;
                return true;
            case kArmSmeI16i32:
                isa.sme_i16i32 = true;
                return true;
            case kArmSmeBi32i32:
                isa.sme_bi32i32 = true;
                return true;
            case kArmSmeB16b16:
                isa.sme_b16b16 = true;
                return true;
            case kArmSmeF16f16:
                isa.sme_f16f16 = true;
                return true;
            case kArmFp8:
                isa.fp8 = true;
                return true;
            case kArmF8dot:
                isa.f8dot = true;
                return true;
            case kArmF8mm:
                isa.f8mm = true;
                return true;
            case kArmRdm:
                isa.rdm = true;
                return true;
            case kArmFp16arith:
                isa.fp16arith = true;
                return true;
            case kArmDot:
                isa.dot = true;
                return true;
            case kArmJscvt:
                isa.jscvt = true;
                return true;
            case kArmFcma:
                isa.fcma = true;
                return true;
            case kArmFhm:
                isa.fhm = true;
                return true;
            case kArmAes:
                isa.aes = true;
                return true;
            case kArmSha1:
                isa.sha1 = true;
                return true;
            case kArmSha2:
                isa.sha2 = true;
                return true;
            case kArmPmull:
                isa.pmull = true;
                return true;
            case kArmCrc32:
                isa.crc32 = true;
                return true;
            default:
                return false;
            }
        }

        bool apply_loong(CpuIsaLoong& isa, uint32_t fno) {
            switch (fno) {
            case kLoongLsx:
                isa.lsx = true;
                return true;
            case kLoongLasx:
                isa.lasx = true;
                return true;
            default:
                return false;
            }
        }

        bool apply_riscv(CpuIsaRiscv& isa, uint32_t fno) {
            switch (fno) {
            case kRiscvI:
                isa.i = true;
                return true;
            case kRiscvE:
                isa.e = true;
                return true;
            case kRiscvM:
                isa.m = true;
                return true;
            case kRiscvA:
                isa.a = true;
                return true;
            case kRiscvF:
                isa.f = true;
                return true;
            case kRiscvD:
                isa.d = true;
                return true;
            case kRiscvC:
                isa.c = true;
                return true;
            case kRiscvZfh:
                isa.zfh = true;
                return true;
            case kRiscvV:
                isa.v = true;
                return true;
            case kRiscvZvfh:
                isa.zvfh = true;
                return true;
            default:
                return false;
            }
        }

        bool apply_ppc(CpuIsaPpc& isa, uint32_t fno) {
            switch (fno) {
            case kPpcAltivec:
                isa.altivec = true;
                return true;
            case kPpcVsx:
                isa.vsx = true;
                return true;
            default:
                return false;
            }
        }

        bool apply_fno(CpuIsaInfo& out, CpuKind kind, uint32_t fno) {
            switch (kind) {
            case CpuKind::kX86:
                return apply_x86(out.x86_isa, fno);
            case CpuKind::kArm:
                return apply_arm(out.arm_isa, fno);
            case CpuKind::kLoong:
                return apply_loong(out.loong_isa, fno);
            case CpuKind::kRiscv:
                return apply_riscv(out.riscv_isa, fno);
            case CpuKind::kPpc:
                return apply_ppc(out.ppc_isa, fno);
            case CpuKind::kNone:
                return false;
            }
            return false;
        }

        int this_arch_count(const CpuIsaInfo& info) {
            return static_cast<int>(info.x86_isa.is_this_arch) + static_cast<int>(info.arm_isa.is_this_arch) + static_cast<int>(info.riscv_isa.is_this_arch) + static_cast<int>(info.loong_isa.is_this_arch) + static_cast<int>(info.ppc_isa.is_this_arch);
        }

        void mark_this_arch(CpuIsaInfo& out, CpuKind kind) {
            switch (kind) {
            case CpuKind::kX86:
                out.x86_isa.is_this_arch = true;
                break;
            case CpuKind::kArm:
                out.arm_isa.is_this_arch = true;
                break;
            case CpuKind::kLoong:
                out.loong_isa.is_this_arch = true;
                break;
            case CpuKind::kRiscv:
                out.riscv_isa.is_this_arch = true;
                break;
            case CpuKind::kPpc:
                out.ppc_isa.is_this_arch = true;
                break;
            case CpuKind::kNone:
                break;
            }
        }

        void collect_true_fnos(const CpuIsaX86& isa, std::vector<uint32_t>& fnos) {
            if (isa.rdtsc) {
                fnos.push_back(kX86Rdtsc);
            }
            if (isa.rdtscp) {
                fnos.push_back(kX86Rdtscp);
            }
            if (isa.rdpid) {
                fnos.push_back(kX86Rdpid);
            }
            if (isa.sysenter) {
                fnos.push_back(kX86Sysenter);
            }
            if (isa.syscall) {
                fnos.push_back(kX86Syscall);
            }
            if (isa.msr) {
                fnos.push_back(kX86Msr);
            }
            if (isa.clzero) {
                fnos.push_back(kX86Clzero);
            }
            if (isa.clflush) {
                fnos.push_back(kX86Clflush);
            }
            if (isa.clflushopt) {
                fnos.push_back(kX86Clflushopt);
            }
            if (isa.mwait) {
                fnos.push_back(kX86Mwait);
            }
            if (isa.mwaitx) {
                fnos.push_back(kX86Mwaitx);
            }
            if (isa.emmx) {
                fnos.push_back(kX86Emmx);
            }
            if (isa.fxsave) {
                fnos.push_back(kX86Fxsave);
            }
            if (isa.xsave) {
                fnos.push_back(kX86Xsave);
            }
            if (isa.fpu) {
                fnos.push_back(kX86Fpu);
            }
            if (isa.mmx) {
                fnos.push_back(kX86Mmx);
            }
            if (isa.mmx_plus) {
                fnos.push_back(kX86MmxPlus);
            }
            if (isa.three_d_now) {
                fnos.push_back(kX86ThreeDNow);
            }
            if (isa.three_d_now_plus) {
                fnos.push_back(kX86ThreeDNowPlus);
            }
            if (isa.three_d_now_geode) {
                fnos.push_back(kX86ThreeDNowGeode);
            }
            if (isa.prefetch) {
                fnos.push_back(kX86Prefetch);
            }
            if (isa.prefetchw) {
                fnos.push_back(kX86Prefetchw);
            }
            if (isa.prefetchwt1) {
                fnos.push_back(kX86Prefetchwt1);
            }
            if (isa.daz) {
                fnos.push_back(kX86Daz);
            }
            if (isa.sse) {
                fnos.push_back(kX86Sse);
            }
            if (isa.sse2) {
                fnos.push_back(kX86Sse2);
            }
            if (isa.sse3) {
                fnos.push_back(kX86Sse3);
            }
            if (isa.ssse3) {
                fnos.push_back(kX86Ssse3);
            }
            if (isa.sse4_1) {
                fnos.push_back(kX86Sse4_1);
            }
            if (isa.sse4_2) {
                fnos.push_back(kX86Sse4_2);
            }
            if (isa.sse4a) {
                fnos.push_back(kX86Sse4a);
            }
            if (isa.misaligned_sse) {
                fnos.push_back(kX86MisalignedSse);
            }
            if (isa.avx) {
                fnos.push_back(kX86Avx);
            }
            if (isa.avxvnni) {
                fnos.push_back(kX86AvxVnni);
            }
            if (isa.fma3) {
                fnos.push_back(kX86Fma3);
            }
            if (isa.fma4) {
                fnos.push_back(kX86Fma4);
            }
            if (isa.xop) {
                fnos.push_back(kX86Xop);
            }
            if (isa.f16c) {
                fnos.push_back(kX86F16c);
            }
            if (isa.avx2) {
                fnos.push_back(kX86Avx2);
            }
            if (isa.avx512f) {
                fnos.push_back(kX86Avx512F);
            }
            if (isa.avx512pf) {
                fnos.push_back(kX86Avx512Pf);
            }
            if (isa.avx512er) {
                fnos.push_back(kX86Avx512Er);
            }
            if (isa.avx512cd) {
                fnos.push_back(kX86Avx512Cd);
            }
            if (isa.avx512dq) {
                fnos.push_back(kX86Avx512Dq);
            }
            if (isa.avx512bw) {
                fnos.push_back(kX86Avx512Bw);
            }
            if (isa.avx512vl) {
                fnos.push_back(kX86Avx512Vl);
            }
            if (isa.avx512ifma) {
                fnos.push_back(kX86Avx512Ifma);
            }
            if (isa.avx512vbmi) {
                fnos.push_back(kX86Avx512Vbmi);
            }
            if (isa.avx512vbmi2) {
                fnos.push_back(kX86Avx512Vbmi2);
            }
            if (isa.avx512bitalg) {
                fnos.push_back(kX86Avx512Bitalg);
            }
            if (isa.avx512vpopcntdq) {
                fnos.push_back(kX86Avx512Vpopcntdq);
            }
            if (isa.avx512vnni) {
                fnos.push_back(kX86Avx512Vnni);
            }
            if (isa.avx512bf16) {
                fnos.push_back(kX86Avx512Bf16);
            }
            if (isa.avx512fp16) {
                fnos.push_back(kX86Avx512Fp16);
            }
            if (isa.avx512vp2intersect) {
                fnos.push_back(kX86Avx512Vp2intersect);
            }
            if (isa.avx512_4vnniw) {
                fnos.push_back(kX86Avx512_4vnniw);
            }
            if (isa.avx512_4fmaps) {
                fnos.push_back(kX86Avx512_4fmaps);
            }
            if (isa.avx10_1) {
                fnos.push_back(kX86Avx10_1);
            }
            if (isa.avx10_2) {
                fnos.push_back(kX86Avx10_2);
            }
            if (isa.amx_bf16) {
                fnos.push_back(kX86AmxBf16);
            }
            if (isa.amx_tile) {
                fnos.push_back(kX86AmxTile);
            }
            if (isa.amx_int8) {
                fnos.push_back(kX86AmxInt8);
            }
            if (isa.amx_fp16) {
                fnos.push_back(kX86AmxFp16);
            }
            if (isa.amx_fp8) {
                fnos.push_back(kX86AmxFp8);
            }
            if (isa.avx_vnni_int8) {
                fnos.push_back(kX86AvxVnniInt8);
            }
            if (isa.avx_vnni_int16) {
                fnos.push_back(kX86AvxVnniInt16);
            }
            if (isa.avx_ne_convert) {
                fnos.push_back(kX86AvxNeConvert);
            }
            if (isa.hle) {
                fnos.push_back(kX86Hle);
            }
            if (isa.rtm) {
                fnos.push_back(kX86Rtm);
            }
            if (isa.xtest) {
                fnos.push_back(kX86Xtest);
            }
            if (isa.mpx) {
                fnos.push_back(kX86Mpx);
            }
            if (isa.cmov) {
                fnos.push_back(kX86Cmov);
            }
            if (isa.cmpxchg8b) {
                fnos.push_back(kX86Cmpxchg8b);
            }
            if (isa.cmpxchg16b) {
                fnos.push_back(kX86Cmpxchg16b);
            }
            if (isa.clwb) {
                fnos.push_back(kX86Clwb);
            }
            if (isa.movbe) {
                fnos.push_back(kX86Movbe);
            }
            if (isa.lahf_sahf) {
                fnos.push_back(kX86LahfSahf);
            }
            if (isa.fs_gs_base) {
                fnos.push_back(kX86FsGsBase);
            }
            if (isa.lzcnt) {
                fnos.push_back(kX86Lzcnt);
            }
            if (isa.popcnt) {
                fnos.push_back(kX86Popcnt);
            }
            if (isa.tbm) {
                fnos.push_back(kX86Tbm);
            }
            if (isa.bmi) {
                fnos.push_back(kX86Bmi);
            }
            if (isa.bmi2) {
                fnos.push_back(kX86Bmi2);
            }
            if (isa.adx) {
                fnos.push_back(kX86Adx);
            }
            if (isa.aes) {
                fnos.push_back(kX86Aes);
            }
            if (isa.vaes) {
                fnos.push_back(kX86Vaes);
            }
            if (isa.pclmulqdq) {
                fnos.push_back(kX86Pclmulqdq);
            }
            if (isa.vpclmulqdq) {
                fnos.push_back(kX86Vpclmulqdq);
            }
            if (isa.gfni) {
                fnos.push_back(kX86Gfni);
            }
            if (isa.rdrand) {
                fnos.push_back(kX86Rdrand);
            }
            if (isa.rdseed) {
                fnos.push_back(kX86Rdseed);
            }
            if (isa.sha) {
                fnos.push_back(kX86Sha);
            }
            if (isa.rng) {
                fnos.push_back(kX86Rng);
            }
            if (isa.ace) {
                fnos.push_back(kX86Ace);
            }
            if (isa.ace2) {
                fnos.push_back(kX86Ace2);
            }
            if (isa.phe) {
                fnos.push_back(kX86Phe);
            }
            if (isa.pmm) {
                fnos.push_back(kX86Pmm);
            }
            if (isa.lwp) {
                fnos.push_back(kX86Lwp);
            }
        }

        void collect_true_fnos(const CpuIsaArm& isa, std::vector<uint32_t>& fnos) {
            if (isa.thumb) {
                fnos.push_back(kArmThumb);
            }
            if (isa.thumb2) {
                fnos.push_back(kArmThumb2);
            }
            if (isa.thumbee) {
                fnos.push_back(kArmThumbee);
            }
            if (isa.jazelle) {
                fnos.push_back(kArmJazelle);
            }
            if (isa.armv5e) {
                fnos.push_back(kArmArmv5e);
            }
            if (isa.armv6) {
                fnos.push_back(kArmArmv6);
            }
            if (isa.armv6k) {
                fnos.push_back(kArmArmv6k);
            }
            if (isa.armv7) {
                fnos.push_back(kArmArmv7);
            }
            if (isa.armv7mp) {
                fnos.push_back(kArmArmv7mp);
            }
            if (isa.armv8) {
                fnos.push_back(kArmArmv8);
            }
            if (isa.idiv) {
                fnos.push_back(kArmIdiv);
            }
            if (isa.vfpv2) {
                fnos.push_back(kArmVfpv2);
            }
            if (isa.vfpv3) {
                fnos.push_back(kArmVfpv3);
            }
            if (isa.d32) {
                fnos.push_back(kArmD32);
            }
            if (isa.fp16) {
                fnos.push_back(kArmFp16);
            }
            if (isa.fma) {
                fnos.push_back(kArmFma);
            }
            if (isa.wmmx) {
                fnos.push_back(kArmWmmx);
            }
            if (isa.wmmx2) {
                fnos.push_back(kArmWmmx2);
            }
            if (isa.neon) {
                fnos.push_back(kArmNeon);
            }
            if (isa.atomics) {
                fnos.push_back(kArmAtomics);
            }
            if (isa.bf16) {
                fnos.push_back(kArmBf16);
            }
            if (isa.sve) {
                fnos.push_back(kArmSve);
            }
            if (isa.sve2) {
                fnos.push_back(kArmSve2);
            }
            if (isa.i8mm) {
                fnos.push_back(kArmI8mm);
            }
            if (isa.sme) {
                fnos.push_back(kArmSme);
            }
            if (isa.sme2) {
                fnos.push_back(kArmSme2);
            }
            if (isa.sme2p1) {
                fnos.push_back(kArmSme2p1);
            }
            if (isa.sme_i16i32) {
                fnos.push_back(kArmSmeI16i32);
            }
            if (isa.sme_bi32i32) {
                fnos.push_back(kArmSmeBi32i32);
            }
            if (isa.sme_b16b16) {
                fnos.push_back(kArmSmeB16b16);
            }
            if (isa.sme_f16f16) {
                fnos.push_back(kArmSmeF16f16);
            }
            if (isa.fp8) {
                fnos.push_back(kArmFp8);
            }
            if (isa.f8dot) {
                fnos.push_back(kArmF8dot);
            }
            if (isa.f8mm) {
                fnos.push_back(kArmF8mm);
            }
            if (isa.rdm) {
                fnos.push_back(kArmRdm);
            }
            if (isa.fp16arith) {
                fnos.push_back(kArmFp16arith);
            }
            if (isa.dot) {
                fnos.push_back(kArmDot);
            }
            if (isa.jscvt) {
                fnos.push_back(kArmJscvt);
            }
            if (isa.fcma) {
                fnos.push_back(kArmFcma);
            }
            if (isa.fhm) {
                fnos.push_back(kArmFhm);
            }
            if (isa.aes) {
                fnos.push_back(kArmAes);
            }
            if (isa.sha1) {
                fnos.push_back(kArmSha1);
            }
            if (isa.sha2) {
                fnos.push_back(kArmSha2);
            }
            if (isa.pmull) {
                fnos.push_back(kArmPmull);
            }
            if (isa.crc32) {
                fnos.push_back(kArmCrc32);
            }
        }

    } // namespace

    CpuIsaInfo convert_feature_to_isa_info(const std::vector<uint32_t>& info, std::string& err) {
        err.clear();
        CpuIsaInfo out { };
        CpuKind selected = CpuKind::kNone;
        for (uint32_t fno : info) {
            const CpuIsaMeta* meta = cpu_isa_meta(fno);
            if (meta == nullptr) {
                std::ostringstream oss;
                oss << "unknown isa fno " << fno;
                err = oss.str();
                return CpuIsaInfo { };
            }
            const CpuKind kind = cpu_kind_from_arch_name(meta->arch_name);
            if (kind == CpuKind::kNone) {
                std::ostringstream oss;
                oss << "unknown arch_name for fno " << fno;
                err = oss.str();
                return CpuIsaInfo { };
            }
            if (selected == CpuKind::kNone) {
                selected = kind;
            } else if (kind != selected) {
                std::ostringstream oss;
                oss << "mixed cpu isa: " << cpu_kind_name(selected) << " and " << cpu_kind_name(kind);
                err = oss.str();
                return CpuIsaInfo { };
            }
            if (!apply_fno(out, kind, fno)) {
                std::ostringstream oss;
                oss << "unmapped isa fno " << fno;
                err = oss.str();
                return CpuIsaInfo { };
            }
        }
        if (selected != CpuKind::kNone) {
            mark_this_arch(out, selected);
        }
        if (this_arch_count(out) > 1) {
            err = "mixed cpu isa: more than one is_this_arch";
            return CpuIsaInfo { };
        }
        return out;
    }

    CpuIsaInfo convert_feature_to_isa_info(const std::map<uint32_t, CpuIsaMeta*>& infos, std::string& err) {
        std::vector<uint32_t> fnos;
        fnos.reserve(infos.size());
        for (const auto& item : infos) {
            if (item.second == nullptr) {
                std::ostringstream oss;
                oss << "null CpuIsaMeta for fno " << item.first;
                err = oss.str();
                return CpuIsaInfo { };
            }
            fnos.push_back(item.first);
        }
        return convert_feature_to_isa_info(fnos, err);
    }

    std::map<uint32_t, CpuIsaMeta*> convert_isa_info_to_feature(CpuIsaInfo info, std::string& err) {
        err.clear();
        if (this_arch_count(info) > 1) {
            err = "mixed cpu isa: more than one is_this_arch";
            return { };
        }
        std::vector<uint32_t> fnos;
        if (info.x86_isa.is_this_arch) {
            collect_true_fnos(info.x86_isa, fnos);
        } else if (info.arm_isa.is_this_arch) {
            collect_true_fnos(info.arm_isa, fnos);
        } else if (info.loong_isa.is_this_arch) {
            if (info.loong_isa.lsx) {
                fnos.push_back(kLoongLsx);
            }
            if (info.loong_isa.lasx) {
                fnos.push_back(kLoongLasx);
            }
        } else if (info.riscv_isa.is_this_arch) {
            if (info.riscv_isa.i) {
                fnos.push_back(kRiscvI);
            }
            if (info.riscv_isa.e) {
                fnos.push_back(kRiscvE);
            }
            if (info.riscv_isa.m) {
                fnos.push_back(kRiscvM);
            }
            if (info.riscv_isa.a) {
                fnos.push_back(kRiscvA);
            }
            if (info.riscv_isa.f) {
                fnos.push_back(kRiscvF);
            }
            if (info.riscv_isa.d) {
                fnos.push_back(kRiscvD);
            }
            if (info.riscv_isa.c) {
                fnos.push_back(kRiscvC);
            }
            if (info.riscv_isa.zfh) {
                fnos.push_back(kRiscvZfh);
            }
            if (info.riscv_isa.v) {
                fnos.push_back(kRiscvV);
            }
            if (info.riscv_isa.zvfh) {
                fnos.push_back(kRiscvZvfh);
            }
        } else if (info.ppc_isa.is_this_arch) {
            if (info.ppc_isa.altivec) {
                fnos.push_back(kPpcAltivec);
            }
            if (info.ppc_isa.vsx) {
                fnos.push_back(kPpcVsx);
            }
        }
        std::map<uint32_t, CpuIsaMeta*> out;
        for (uint32_t fno : fnos) {
            CpuIsaMeta* meta = const_cast<CpuIsaMeta*>(cpu_isa_meta(fno));
            if (meta == nullptr) {
                std::ostringstream oss;
                oss << "unknown isa fno " << fno;
                err = oss.str();
                return { };
            }
            out.emplace(fno, meta);
        }
        return out;
    }

    extern CpuIsaInfo detect_cpu_isa_info_internal();

    CpuIsaInfo detect_cpu_isa_info() {
        static CpuIsaInfo ins = detect_cpu_isa_info_internal();
        return ins;
    }

    CpuIsaInfo detect_current_enabled_isa_info() {
        CpuIsaInfo info{};
#if KUMO_ARCH_X86_64
        CpuIsaX86& isa = info.x86_isa;
        isa.is_this_arch = true;
        isa.fpu = true;
        isa.mmx = true;
        isa.mmx_plus = true;
        isa.cmov = true;
        isa.cmpxchg8b = true;
        isa.fxsave = true;
        isa.rdtsc = true;
        isa.syscall = true;
        isa.clflush = true;
        isa.sse = true;
        isa.sse2 = true;
        isa.daz = true;
        isa.sse3 = KUMO_SIMD_SSE3;
        isa.ssse3 = KUMO_SIMD_SSSE3;
        isa.sse4_1 = KUMO_SIMD_SSE4_1;
        isa.sse4_2 = KUMO_SIMD_SSE4_2;
        isa.avx = KUMO_SIMD_AVX;
        isa.avx2 = KUMO_SIMD_AVX2;
        isa.avx512f = KUMO_SIMD_AVX512F;
        isa.avx512bw = KUMO_SIMD_AVX512BW;
        isa.avx512vl = KUMO_SIMD_AVX512VL;
        isa.avx512dq = KUMO_SIMD_AVX512DQ;
        isa.avx512ifma = KUMO_SIMD_AVX512IFMA;
        isa.avx512cd = KUMO_SIMD_AVX512CD;
        isa.avx512vbmi = KUMO_SIMD_AVX512VBMI;
        isa.avx512vbmi2 = KUMO_SIMD_AVX512VBMI2;
        isa.avx512vnni = KUMO_SIMD_AVX512VNNI;
        isa.avx512bitalg = KUMO_SIMD_AVX512BITALG;
        isa.avx512vpopcntdq = KUMO_SIMD_AVX512VPOPCNTDQ;
        isa.fma3 = KUMO_SIMD_FMA;
        isa.bmi = KUMO_SIMD_BMI1;
        isa.bmi2 = KUMO_SIMD_BMI2;
        isa.popcnt = KUMO_SIMD_POPCNT;
        isa.lzcnt = KUMO_SIMD_LZCNT;
        isa.aes = KUMO_SIMD_X86_AES;
        isa.pclmulqdq = KUMO_SIMD_PCLMUL;
        isa.prefetch = KUMO_SIMD_PREFETCH;
        isa.prefetchw = KUMO_SIMD_PRFCHW;
        isa.prefetchwt1 = KUMO_SIMD_PREFETCHWT1;
        isa.cmpxchg16b = KUMO_SIMD_CX16;
        isa.lahf_sahf = true;
        isa.movbe = KUMO_SIMD_MOVBE;
        isa.xsave = KUMO_SIMD_XSAVE || KUMO_SIMD_AVX;
        isa.f16c = KUMO_SIMD_F16C;
        isa.rdrand = KUMO_SIMD_RDRND;
        isa.rdseed = KUMO_SIMD_RDSEED;
        isa.fs_gs_base = KUMO_SIMD_FSGSBASE;
        isa.sha = KUMO_SIMD_SHA;
        isa.adx = KUMO_SIMD_ADX;
        isa.clflushopt = KUMO_SIMD_CLFLUSHOPT;
        isa.clwb = KUMO_SIMD_CLWB;
        isa.clzero = KUMO_SIMD_CLZERO;
        isa.rdtscp = KUMO_SIMD_RDTSCP;
        isa.rdpid = KUMO_SIMD_RDPID;
        isa.hle = KUMO_SIMD_HLE;
        isa.rtm = KUMO_SIMD_RTM;
        isa.xtest = isa.hle || isa.rtm;
        isa.mpx = KUMO_SIMD_MPX;
        isa.sse4a = KUMO_SIMD_SSE4A;
        isa.fma4 = KUMO_SIMD_FMA4;
        isa.xop = KUMO_SIMD_XOP;
        isa.tbm = KUMO_SIMD_TBM;
        isa.lwp = KUMO_SIMD_LWP;
        isa.vaes = KUMO_SIMD_VAES;
        isa.vpclmulqdq = KUMO_SIMD_VPCLMUL;
        isa.gfni = KUMO_SIMD_GFNI;
        isa.avxvnni = KUMO_SIMD_AVXVNNI;
        isa.avx512pf = KUMO_SIMD_AVX512PF;
        isa.avx512er = KUMO_SIMD_AVX512ER;
        isa.avx512bf16 = KUMO_SIMD_AVX512BF16;
        isa.avx512fp16 = KUMO_SIMD_AVX512FP16;
        isa.avx512vp2intersect = KUMO_SIMD_AVX512VP2;
        isa.amx_tile = KUMO_SIMD_AMX_TILE;
        isa.amx_int8 = KUMO_SIMD_AMX_INT8;
        isa.amx_bf16 = KUMO_SIMD_AMX_BF16;
        isa.amx_fp16 = KUMO_SIMD_AMX_FP16;
#elif KUMO_ARCH_ARM64
        CpuIsaArm& isa = info.arm_isa;
        isa.is_this_arch = true;
        isa.armv8 = true;
        isa.idiv = true;
        isa.d32 = true;
        isa.fma = true;
        isa.neon = KUMO_SIMD_NEON;
        isa.sve = KUMO_SIMD_SVE;
        isa.sve2 = KUMO_SIMD_SVE2;
        isa.aes = KUMO_SIMD_ARM_AES;
#elif KUMO_ARCH_LOONGARCH64
        info.loong_isa.is_this_arch = true;
        info.loong_isa.lsx = KUMO_SIMD_LSX;
        info.loong_isa.lasx = KUMO_SIMD_LASX;
#elif KUMO_ARCH_RISCV64
        info.riscv_isa.is_this_arch = true;
        info.riscv_isa.i = true;
        info.riscv_isa.m = true;
        info.riscv_isa.a = true;
        info.riscv_isa.f = true;
        info.riscv_isa.d = true;
        info.riscv_isa.c = true;
        info.riscv_isa.v = KUMO_SIMD_RVV;
#elif KUMO_ARCH_PPC64
        info.ppc_isa.is_this_arch = true;
        info.ppc_isa.altivec = KUMO_SIMD_ALTIVEC;
        info.ppc_isa.vsx = KUMO_SIMD_VSX;
#endif
        return info;
    }
} // namespace turbo
