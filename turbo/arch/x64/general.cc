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

} // namespace turbo
#endif
