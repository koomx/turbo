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

#include <turbo/arch/cpu_detect.h>
#if KUMO_ARCH_X86_64
#define TURBO_X64_DETECT
#include <cstdint>
#include <string>

#include <turbo/arch/instruction.h>
#include <turbo/arch/x64/cpuid.h>

namespace turbo {

    namespace {

        enum class Vendor {
            kUnknown,
            kIntel,
            kAmd,
            kHygon,
            kVia,
        };

        Vendor GetVendor() {
            // Get the vendor string (issue CPUID with eax = 0).
            int cpu_info[4];
            __cpuid(cpu_info, 0);

            std::string vendor;
            vendor.append(reinterpret_cast<char*>(&cpu_info[1]), 4);
            vendor.append(reinterpret_cast<char*>(&cpu_info[3]), 4);
            vendor.append(reinterpret_cast<char*>(&cpu_info[2]), 4);
            if (vendor == "GenuineIntel") {
                return Vendor::kIntel;
            } else if (vendor == "AuthenticAMD") {
                return Vendor::kAmd;
            } else if (vendor == "HygonGenuine") {
                return Vendor::kHygon;
            } else if (vendor == "CentaurHauls") {
                return Vendor::kVia;
            } else {
                return Vendor::kUnknown;
            }
        }

        CpuType GetIntelCpuType() {
            // To get general information and extended features we send eax = 1 and
            // ecx = 0 to cpuid.  The response is returned in eax, ebx, ecx and edx.
            // (See Intel 64 and IA-32 Architectures Software Developer's Manual
            // Volume 2A: Instruction Set Reference, A-M CPUID).
            // https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-2a-manual.html
            // https://learn.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex
            int cpu_info[4];
            __cpuid(cpu_info, 1);

            // Response in eax bits as follows:
            // 0-3 (stepping id)
            // 4-7 (model number),
            // 8-11 (family code),
            // 12-13 (processor type),
            // 16-19 (extended model)
            // 20-27 (extended family)

            int family = (cpu_info[0] >> 8) & 0x0f;
            int model_num = (cpu_info[0] >> 4) & 0x0f;
            int ext_family = (cpu_info[0] >> 20) & 0xff;
            int ext_model_num = (cpu_info[0] >> 16) & 0x0f;

            int brand_id = cpu_info[1] & 0xff;

            // Process the extended family and model info if necessary
            if (family == 0x0f) {
                family += ext_family;
            }

            if (family == 0x0f || family == 0x6) {
                model_num += (ext_model_num << 4);
            }

            switch (brand_id) {
            case 0: // no brand ID, so parse CPU family/model
                switch (family) {
                case 6: // Most PentiumIII processors are in this category
                    switch (model_num) {
                    case 0x2c: // Westmere: Gulftown
                        return CpuType::kIntelWestmere;
                    case 0x2d: // Sandybridge
                        return CpuType::kIntelSandybridge;
                    case 0x3e: // Ivybridge
                        return CpuType::kIntelIvybridge;
                    case 0x3c: // Haswell (client)
                    case 0x3f: // Haswell
                        return CpuType::kIntelHaswell;
                    case 0x4f: // Broadwell
                    case 0x56: // BroadwellDE
                        return CpuType::kIntelBroadwell;
                    case 0x55: // Skylake Xeon
                        if ((cpu_info[0] & 0x0f) < 5) { // stepping < 5 is skylake
                            return CpuType::kIntelSkylakeXeon;
                        } else { // stepping >= 5 is cascadelake
                            return CpuType::kIntelCascadelakeXeon;
                        }
                    case 0x5e: // Skylake (client)
                        return CpuType::kIntelSkylake;
                    case 0x6a: // Ice Lake
                        return CpuType::kIntelIcelake;
                    case 0x8f: // Sapphire Rapids
                        return CpuType::kIntelSapphirerapids;
                    case 0xcf: // Emerald Rapids
                        return CpuType::kIntelEmeraldrapids;
                    case 0xad: // Granite Rapids
                        return CpuType::kIntelGraniterapids;
                    default:
                        return CpuType::kUnknown;
                    }
                default:
                    return CpuType::kUnknown;
                }
            default:
                return CpuType::kUnknown;
            }
        }

        CpuType GetAmdCpuType() {
            // To get general information and extended features we send eax = 1 and
            // ecx = 0 to cpuid.  The response is returned in eax, ebx, ecx and edx.
            // (See Intel 64 and IA-32 Architectures Software Developer's Manual
            // Volume 2A: Instruction Set Reference, A-M CPUID).
            // https://learn.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex
            int cpu_info[4];
            __cpuid(cpu_info, 1);

            // Response in eax bits as follows:
            // 0-3 (stepping id)
            // 4-7 (model number),
            // 8-11 (family code),
            // 12-13 (processor type),
            // 16-19 (extended model)
            // 20-27 (extended family)

            int family = (cpu_info[0] >> 8) & 0x0f;
            int model_num = (cpu_info[0] >> 4) & 0x0f;
            int ext_family = (cpu_info[0] >> 20) & 0xff;
            int ext_model_num = (cpu_info[0] >> 16) & 0x0f;

            if (family == 0x0f) {
                family += ext_family;
                model_num += (ext_model_num << 4);
            }

            switch (family) {
            case 0x17:
                switch (model_num) {
                case 0x0: // Stepping Ax
                case 0x1: // Stepping Bx
                    return CpuType::kAmdNaples;
                case 0x30: // Stepping Ax
                case 0x31: // Stepping Bx
                    return CpuType::kAmdRome;
                default:
                    return CpuType::kUnknown;
                }
                break;
            case 0x19:
                switch (model_num) {
                case 0x0: // Stepping Ax
                case 0x1: // Stepping B0
                    return CpuType::kAmdMilan;
                case 0x10: // Stepping A0
                case 0x11: // Stepping B0
                    return CpuType::kAmdGenoa;
                case 0x44: // Stepping A0
                    return CpuType::kAmdRyzenV3000;
                default:
                    return CpuType::kUnknown;
                }
                break;
            case 0x1A:
                switch (model_num) {
                case 0x2:
                    return CpuType::kAmdTurin;
                default:
                    return CpuType::kUnknown;
                }
                break;
            default:
                return CpuType::kUnknown;
            }
        }

    } // namespace

    ////////////////////////////////////////////////////////////////////////////////
    // get_cpu_type
    ////////////////////////////////////////////////////////////////////////////////

    CpuType get_cpu_type() {
        switch (GetVendor()) {
        case Vendor::kIntel:
            return GetIntelCpuType();
        case Vendor::kAmd:
            return GetAmdCpuType();
        default:
            return CpuType::kUnknown;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    // supports_arm_crc32_pmull
    ////////////////////////////////////////////////////////////////////////////////

    bool supports_arm_crc32_pmull() {
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // supports_bmi2
    ////////////////////////////////////////////////////////////////////////////////

    bool supports_bmi2() {
        int cpu_info[4];
        __cpuid(cpu_info, 0);
        if (cpu_info[0] < 7) {
            return false;
        }
        __cpuidex(cpu_info, 7, 0);
        return (cpu_info[1] & (1 << 8)) != 0;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // num_contexts_per_cpu
    ////////////////////////////////////////////////////////////////////////////////

    // Returns how many hardware contexts per CPU exist. Note: AMD CPUs prior to Zen
    // 2 (Rome, 2019) do not support CPUID leaf 0xb. We intentionally avoid falling
    // back to leaf 1 ebx[23:16] because it reports total logical processors per
    // package (not threads per core), which risks false positives on older
    // multi-core non-SMT chips. Pre-Zen 2 AMD safely defaults to 1.
    int num_contexts_per_cpu() {
        int info[4];
        __cpuid(info, 0);
        if (info[0] < 0xb) {
            return 1;
        }

        __cpuid(info, 1);
        bool has_ht = (info[3] & (1 << 28)) != 0;
        if (!has_ht) {
            return 1;
        }

        for (int sub_leaf = 0; sub_leaf < 4; ++sub_leaf) {
            __cpuidex(info, 0xb, sub_leaf);
            int level_type = (info[2] >> 8) & 0xff;
            if (level_type == 0) {
                break;
            }
            if (level_type == 1) {
                int num_threads = info[1] & 0x0ffff;
                if (num_threads >= 1) {
                    return num_threads;
                }
            }
        }

        return 1;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // detect_supported_architectures
    ////////////////////////////////////////////////////////////////////////////////

    namespace {

        namespace cpuid_bit {
            constexpr uint32_t pclmulqdq = uint32_t(1) << 1;
            constexpr uint32_t sse42 = uint32_t(1) << 20;
            constexpr uint32_t osxsave = (uint32_t(1) << 26) | (uint32_t(1) << 27);
            namespace ebx {
                constexpr uint32_t bmi1 = uint32_t(1) << 3;
                constexpr uint32_t avx2 = uint32_t(1) << 5;
                constexpr uint32_t bmi2 = uint32_t(1) << 8;
                constexpr uint32_t avx512f = uint32_t(1) << 16;
                constexpr uint32_t avx512dq = uint32_t(1) << 17;
                constexpr uint32_t avx512ifma = uint32_t(1) << 21;
                constexpr uint32_t avx512cd = uint32_t(1) << 28;
                constexpr uint32_t avx512bw = uint32_t(1) << 30;
                constexpr uint32_t avx512vl = uint32_t(1) << 31;
            } // namespace ebx
            namespace ecx {
                constexpr uint32_t avx512vbmi2 = uint32_t(1) << 6;
                constexpr uint32_t avx512vpopcnt = uint32_t(1) << 14;
            } // namespace ecx
            namespace xcr0_bit {
                constexpr uint64_t avx256_saved = uint64_t(1) << 2;
                constexpr uint64_t avx512_saved = uint64_t(7) << 5;
            } // namespace xcr0_bit
        } // namespace cpuid_bit

        static void isa_cpuid(uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
#if KUMO_COMPILER_MSVC_ENV
            int cpu_info[4];
            __cpuidex(cpu_info, static_cast<int>(*eax), static_cast<int>(*ecx));
            *eax = static_cast<uint32_t>(cpu_info[0]);
            *ebx = static_cast<uint32_t>(cpu_info[1]);
            *ecx = static_cast<uint32_t>(cpu_info[2]);
            *edx = static_cast<uint32_t>(cpu_info[3]);
#elif (defined(HAVE_GCC_GET_CPUID) && defined(USE_GCC_GET_CPUID)) || defined(__FILC__)
            uint32_t level = *eax;
            __get_cpuid(level, eax, ebx, ecx, edx);
#else
            uint32_t a = *eax, b, c = *ecx, d;
            asm volatile("cpuid\n\t" : "+a"(a), "=b"(b), "+c"(c), "=d"(d));
            *eax = a;
            *ebx = b;
            *ecx = c;
            *edx = d;
#endif
        }

        static uint64_t xgetbv() {
#if KUMO_COMPILER_MSVC_ENV
            return _xgetbv(0);
#elif defined(__FILC__)
            return zxgetbv();
#else
            uint32_t xcr0_lo, xcr0_hi;
            asm volatile("xgetbv\n\t" : "=a"(xcr0_lo), "=d"(xcr0_hi) : "c"(0));
            return xcr0_lo | ((uint64_t)xcr0_hi << 32);
#endif
        }

        struct CpuidRegs {
            uint32_t eax{0};
            uint32_t ebx{0};
            uint32_t ecx{0};
            uint32_t edx{0};
        };

        CpuidRegs run_cpuid(uint32_t leaf, uint32_t subleaf) {
            CpuidRegs r;
            r.eax = leaf;
            r.ecx = subleaf;
            isa_cpuid(&r.eax, &r.ebx, &r.ecx, &r.edx);
            return r;
        }

        void fill_x86_isa(CpuIsaX86& isa, Vendor vendor) {
            const CpuidRegs max_info = run_cpuid(0, 0);
            const uint32_t max_base = max_info.eax;
            const CpuidRegs ext_max = run_cpuid(0x80000000u, 0);
            const uint32_t max_extended = ext_max.eax;
            const CpuidRegs basic = run_cpuid(1, 0);
            const CpuidRegs extended =
                (max_extended >= 0x80000001u) ? run_cpuid(0x80000001u, 0) : CpuidRegs{};
            const CpuidRegs feat0 = (max_base >= 7) ? run_cpuid(7, 0) : CpuidRegs{};
            const CpuidRegs feat1 = (max_base >= 7) ? run_cpuid(7, 1) : CpuidRegs{};
            const CpuidRegs feat24 = (max_base >= 0x24) ? run_cpuid(0x24, 0) : CpuidRegs{};
            const CpuidRegs tmm1 = (max_base >= 0x1E) ? run_cpuid(0x1E, 1) : CpuidRegs{};
            const CpuidRegs cap =
                (max_extended >= 0x80000008u) ? run_cpuid(0x80000008u, 0) : CpuidRegs{};

            bool avx_regs = false;
            bool avx512_regs = false;
            bool mpx_regs = false;
            const uint32_t osxsave_mask = 0x0C000000u;
            if ((basic.ecx & osxsave_mask) == osxsave_mask) {
                uint64_t xcr0_valid_bits = 0;
                if (max_base >= 0xD) {
                    const CpuidRegs xsave = run_cpuid(0xD, 0);
                    xcr0_valid_bits = (static_cast<uint64_t>(xsave.edx) << 32) | xsave.eax;
                }
                const uint64_t xcr0 = xgetbv();
                const uint64_t avx_regs_mask = 0x6ull;
                if ((xcr0_valid_bits & avx_regs_mask) == avx_regs_mask) {
                    avx_regs = (xcr0 & avx_regs_mask) == avx_regs_mask;
                }
                const uint64_t avx512_regs_mask = 0xE6ull;
                if ((xcr0_valid_bits & avx512_regs_mask) == avx512_regs_mask) {
                    avx512_regs = (xcr0 & avx512_regs_mask) == avx512_regs_mask;
                }
                const uint64_t mpx_regs_mask = 0x18ull;
                if ((xcr0_valid_bits & mpx_regs_mask) == mpx_regs_mask) {
                    mpx_regs = (xcr0 & mpx_regs_mask) == mpx_regs_mask;
                }
            }

            isa.rdtsc = ((basic.edx | extended.edx) & 0x00000010u) != 0;
            isa.sysenter = (basic.edx & 0x00000800u) != 0;
            isa.syscall = (extended.edx & 0x00000800u) != 0;
            isa.msr = ((basic.edx | extended.edx) & 0x00000020u) != 0;
            isa.clzero = (cap.ebx & 0x00000001u) != 0;
            isa.clflush = (basic.edx & 0x00080000u) != 0;
            isa.clflushopt = (feat0.ebx & 0x00800000u) != 0;
            isa.mwait = (basic.ecx & 0x00000008u) != 0;
            isa.mwaitx = (extended.ecx & 0x20000000u) != 0;
            isa.fxsave = ((basic.edx | extended.edx) & 0x01000000u) != 0;
            isa.xsave = (basic.ecx & 0x04000000u) != 0;
            isa.fpu = ((basic.edx | extended.edx) & 0x00000001u) != 0;
            isa.mmx = ((basic.edx | extended.edx) & 0x00800000u) != 0;
            isa.mmx_plus = ((basic.edx & 0x02000000u) | (extended.edx & 0x00400000u)) != 0;
            isa.three_d_now = (extended.edx & 0x80000000u) != 0;
            isa.three_d_now_plus = (extended.edx & 0x40000000u) != 0;

            if (vendor == Vendor::kAmd || vendor == Vendor::kHygon) {
                isa.prefetch = ((extended.ecx & 0x00000100u) | (extended.edx & 0xE0000000u)) != 0;
                isa.prefetchw = ((extended.ecx & 0x00000100u) | (extended.edx & 0xE0000000u)) != 0;
            } else if (vendor != Vendor::kIntel) {
                isa.prefetch = (extended.edx & 0xC0000000u) != 0;
                isa.prefetchw = ((extended.ecx & 0x00000100u) | (extended.edx & 0xC0000000u)) != 0;
            } else {
                isa.prefetchw = ((extended.ecx & 0x00000100u) | (extended.edx & 0xC0000000u)) != 0;
            }
            isa.prefetchwt1 = (feat0.ecx & 0x00000001u) != 0;

            isa.sse = (basic.edx & 0x02000000u) != 0;
            isa.sse2 = (basic.edx & 0x04000000u) != 0;
            isa.sse3 = (basic.ecx & 0x00000001u) != 0;
            isa.daz = isa.sse3 || isa.sse2;
            isa.ssse3 = (basic.ecx & 0x00000200u) != 0;
            isa.sse4_1 = (basic.ecx & 0x00080000u) != 0;
            isa.sse4_2 = (basic.ecx & 0x00100000u) != 0;
            isa.sse4a = (extended.ecx & 0x00000040u) != 0;
            isa.misaligned_sse = (extended.ecx & 0x00000080u) != 0;

            isa.avx = avx_regs && (basic.ecx & 0x10000000u) != 0;
            isa.fma3 = avx_regs && (basic.ecx & 0x00001000u) != 0;
            isa.fma4 = avx_regs && (extended.ecx & 0x00010000u) != 0;
            isa.xop = avx_regs && (extended.ecx & 0x00000800u) != 0;
            isa.f16c = avx_regs && (basic.ecx & 0x20000000u) != 0;
            isa.avx2 = avx_regs && (feat0.ebx & 0x00000020u) != 0;

            isa.avx512f = avx512_regs && (feat0.ebx & 0x00010000u) != 0;
            isa.avx10_1 = avx512_regs && (feat1.edx & 0x00080000u) != 0;
            isa.avx10_2 = isa.avx10_1 && ((feat24.ebx & 0xFFu) >= 2);
            isa.avx512pf = avx512_regs && (feat0.ebx & 0x04000000u) != 0;
            isa.avx512er = avx512_regs && (feat0.ebx & 0x08000000u) != 0;
            isa.avx512cd = avx512_regs && (feat0.ebx & 0x10000000u) != 0;
            isa.avx512dq = avx512_regs && (feat0.ebx & 0x00020000u) != 0;
            isa.avx512bw = avx512_regs && (feat0.ebx & 0x40000000u) != 0;
            isa.avx512vl = avx512_regs && (feat0.ebx & 0x80000000u) != 0;
            isa.avx512ifma = avx512_regs && (feat0.ebx & 0x00200000u) != 0;
            isa.avx512vbmi = avx512_regs && (feat0.ecx & 0x00000002u) != 0;
            isa.avx512vbmi2 = avx512_regs && (feat0.ecx & 0x00000040u) != 0;
            isa.avx512bitalg = avx512_regs && (feat0.ecx & 0x00001000u) != 0;
            isa.avx512vpopcntdq = avx512_regs && (feat0.ecx & 0x00004000u) != 0;
            isa.avx512vnni = avx512_regs && (feat0.ecx & 0x00000800u) != 0;
            isa.avx512_4vnniw = avx512_regs && (feat0.edx & 0x00000004u) != 0;
            isa.avx512_4fmaps = avx512_regs && (feat0.edx & 0x00000008u) != 0;
            isa.avx512vp2intersect = avx512_regs && (feat0.edx & 0x00000100u) != 0;
            isa.avx512fp16 = avx512_regs && (feat0.edx & 0x00800000u) != 0;
            isa.avxvnni = avx_regs && (feat1.eax & 0x00000010u) != 0;
            isa.avx512bf16 = avx512_regs && (feat1.eax & 0x00000020u) != 0;
            isa.amx_bf16 = avx512_regs && (feat0.edx & 0x00400000u) != 0;
            isa.amx_tile = avx512_regs && (feat0.edx & 0x01000000u) != 0;
            isa.amx_int8 = avx512_regs && (feat0.edx & 0x02000000u) != 0;
            isa.amx_fp16 = avx512_regs && (feat1.eax & 0x00200000u) != 0;
            isa.amx_fp8 = avx512_regs && (tmm1.eax & 0x00000010u) != 0;
            isa.avx_vnni_int8 = avx_regs && (feat1.edx & 0x00000010u) != 0;
            isa.avx_vnni_int16 = avx_regs && (feat1.edx & 0x00000400u) != 0;
            isa.avx_ne_convert = avx_regs && (feat1.edx & 0x00000020u) != 0;

            isa.hle = (feat0.ebx & 0x00000010u) != 0;
            isa.rtm = (feat0.ebx & 0x00000800u) != 0;
            isa.xtest = isa.hle || isa.rtm;
            isa.mpx = mpx_regs && (feat0.ebx & 0x00004000u) != 0;
            isa.cmov = ((basic.edx | extended.edx) & 0x00008000u) != 0;
            isa.cmpxchg8b = ((basic.edx | extended.edx) & 0x00000100u) != 0;
            isa.cmpxchg16b = (basic.ecx & 0x00002000u) != 0;
            isa.clwb = (feat0.ebx & 0x01000000u) != 0;
            isa.movbe = (basic.ecx & 0x00400000u) != 0;
            isa.lahf_sahf = (extended.ecx & 0x00000001u) != 0;
            isa.fs_gs_base = (feat0.ebx & 0x00000001u) != 0;
            isa.lzcnt = (extended.ecx & 0x00000020u) != 0;
            isa.popcnt = (basic.ecx & 0x00800000u) != 0;
            isa.tbm = (extended.ecx & 0x00200000u) != 0;
            isa.bmi = (feat0.ebx & 0x00000008u) != 0;
            isa.bmi2 = (feat0.ebx & 0x00000100u) != 0;
            isa.adx = (feat0.ebx & 0x00080000u) != 0;
            isa.aes = (basic.ecx & 0x02000000u) != 0;
            isa.vaes = (feat0.ecx & 0x00000200u) != 0;
            isa.pclmulqdq = (basic.ecx & 0x00000002u) != 0;
            isa.vpclmulqdq = (feat0.ecx & 0x00000400u) != 0;
            isa.gfni = (feat0.ecx & 0x00000100u) != 0;
            isa.rdrand = (basic.ecx & 0x40000000u) != 0;
            isa.rdseed = (feat0.ebx & 0x00040000u) != 0;
            isa.sha = (feat0.ebx & 0x20000000u) != 0;

            if (vendor == Vendor::kVia) {
                const CpuidRegs padlock_meta = run_cpuid(0xC0000000u, 0);
                if (padlock_meta.eax >= 0xC0000001u) {
                    const CpuidRegs padlock = run_cpuid(0xC0000001u, 0);
                    isa.rng = (padlock.edx & 0x0000000Cu) == 0x0000000Cu;
                    isa.ace = (padlock.edx & 0x000000C0u) == 0x000000C0u;
                    isa.ace2 = (padlock.edx & 0x00000300u) == 0x00000300u;
                    isa.phe = (padlock.edx & 0x00000C00u) == 0x00000C00u;
                    isa.pmm = (padlock.edx & 0x00003000u) == 0x00003000u;
                }
            }

            isa.lwp = (extended.ecx & 0x00008000u) != 0;
            isa.rdtscp = (extended.edx & 0x08000000u) != 0;
            isa.rdpid = (feat0.ecx & 0x00400000u) != 0;
        }

    } // namespace

    uint32_t detect_supported_architectures() {
        uint32_t eax;
        uint32_t ebx = 0;
        uint32_t ecx = 0;
        uint32_t edx = 0;
        uint32_t host_isa = 0x0;

        eax = 0x1;
        isa_cpuid(&eax, &ebx, &ecx, &edx);

        if (ecx & cpuid_bit::sse42) {
            host_isa |= InstructionSet::SSE42;
        }

        if (ecx & cpuid_bit::pclmulqdq) {
            host_isa |= InstructionSet::PCLMULQDQ;
        }

        if ((ecx & cpuid_bit::osxsave) != cpuid_bit::osxsave) {
            return host_isa;
        }

        uint64_t xcr0 = xgetbv();

        if ((xcr0 & cpuid_bit::xcr0_bit::avx256_saved) == 0) {
            return host_isa;
        }
        eax = 0x7;
        ecx = 0x0;
        isa_cpuid(&eax, &ebx, &ecx, &edx);
        if (ebx & cpuid_bit::ebx::avx2) {
            host_isa |= InstructionSet::AVX2;
        }
        if (ebx & cpuid_bit::ebx::bmi1) {
            host_isa |= InstructionSet::BMI1;
        }
        if (ebx & cpuid_bit::ebx::bmi2) {
            host_isa |= InstructionSet::BMI2;
        }
        if (!((xcr0 & cpuid_bit::xcr0_bit::avx512_saved) == cpuid_bit::xcr0_bit::avx512_saved)) {
            return host_isa;
        }
        if (ebx & cpuid_bit::ebx::avx512f) {
            host_isa |= InstructionSet::AVX512F;
        }
        if (ebx & cpuid_bit::ebx::avx512bw) {
            host_isa |= InstructionSet::AVX512BW;
        }
        if (ebx & cpuid_bit::ebx::avx512cd) {
            host_isa |= InstructionSet::AVX512CD;
        }
        if (ebx & cpuid_bit::ebx::avx512dq) {
            host_isa |= InstructionSet::AVX512DQ;
        }
        if (ebx & cpuid_bit::ebx::avx512vl) {
            host_isa |= InstructionSet::AVX512VL;
        }
        if (ecx & cpuid_bit::ecx::avx512vbmi2) {
            host_isa |= InstructionSet::AVX512VBMI2;
        }
        if (ecx & cpuid_bit::ecx::avx512vpopcnt) {
            host_isa |= InstructionSet::AVX512VPOPCNTDQ;
        }
        return host_isa;
    }

    CpuIsaInfo detect_cpu_isa_info_internal() {
        CpuIsaInfo info{};
        info.x86_isa.is_this_arch = true;
        fill_x86_isa(info.x86_isa, GetVendor());
        return info;
    }

} // namespace turbo
#endif
