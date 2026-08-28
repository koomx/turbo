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
#if KUMO_ARCH_ARM64 && (KUMO_OS_MACOSX || KUMO_OS_IOS || KUMO_OS_TVOS || \
    KUMO_OS_WATCHOS || KUMO_OS_VISIONOS)
#include <cstdint>
#include <optional>

#if defined(__has_include) && __has_include(<arm/cpu_capabilities_public.h>)
#include <arm/cpu_capabilities_public.h>
#endif
#include <mach/machine.h>
#include <sys/sysctl.h>
#include <sys/types.h>

#include <turbo/arch/instruction.h>

#ifndef CPUFAMILY_ARM_MONSOON_MISTRAL
#define CPUFAMILY_ARM_MONSOON_MISTRAL 0xE81E7EF6
#endif
#ifndef CPUFAMILY_ARM_VORTEX_TEMPEST
#define CPUFAMILY_ARM_VORTEX_TEMPEST 0x07D34B9F
#endif
#ifndef CPUFAMILY_ARM_LIGHTNING_THUNDER
#define CPUFAMILY_ARM_LIGHTNING_THUNDER 0x462504D2
#endif
#ifndef CPUFAMILY_ARM_FIRESTORM_ICESTORM
#define CPUFAMILY_ARM_FIRESTORM_ICESTORM 0x1B588BB3
#endif

namespace turbo {

    ////////////////////////////////////////////////////////////////////////////////
    // get_cpu_type
    ////////////////////////////////////////////////////////////////////////////////

    CpuType get_cpu_type() {
        return CpuType::kUnknown;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // supports_arm_crc32_pmull
    ////////////////////////////////////////////////////////////////////////////////

    template <typename T>
    static std::optional<T> ReadSysctlByName(const char* name) {
        T val;
        size_t val_size = sizeof(T);
        int ret = sysctlbyname(name, &val, &val_size, nullptr, 0);
        if (ret == -1) {
            return std::nullopt;
        }
        return val;
    }

    bool supports_arm_crc32_pmull() {
        // Newer XNU kernels support querying all capabilities in a single
        // sysctlbyname.
#if defined(CAP_BIT_CRC32) && defined(CAP_BIT_FEAT_PMULL)
        static const std::optional<uint64_t> caps = ReadSysctlByName<uint64_t>("hw.optional.arm.caps");
        if (caps.has_value()) {
            constexpr uint64_t kCrc32AndPmullCaps = (uint64_t { 1 } << CAP_BIT_CRC32) | (uint64_t { 1 } << CAP_BIT_FEAT_PMULL);
            return (*caps & kCrc32AndPmullCaps) == kCrc32AndPmullCaps;
        }
#endif

        // https://developer.apple.com/documentation/kernel/1387446-sysctlbyname/determining_instruction_set_characteristics#3915619
        static const std::optional<int> armv8_crc32 = ReadSysctlByName<int>("hw.optional.armv8_crc32");
        if (armv8_crc32.value_or(0) == 0) {
            return false;
        }
        // https://developer.apple.com/documentation/kernel/1387446-sysctlbyname/determining_instruction_set_characteristics#3918855
        static const std::optional<int> feat_pmull = ReadSysctlByName<int>("hw.optional.arm.FEAT_PMULL");
        if (feat_pmull.value_or(0) == 0) {
            return false;
        }
        return true;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // supports_bmi2
    ////////////////////////////////////////////////////////////////////////////////

    bool supports_bmi2() {
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // num_contexts_per_cpu
    ////////////////////////////////////////////////////////////////////////////////

    int num_contexts_per_cpu() {
        return 1;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // detect_cpu_isa_info_internal  (cpuinfo arm/mach/init.c ISA fill)
    ////////////////////////////////////////////////////////////////////////////////

    namespace {

    uint32_t apple_sysctl_u32(const char* name) {
        size_t size = 0;
        uint32_t result = 0;
        if (sysctlbyname(name, nullptr, &size, nullptr, 0) != 0) {
            return 0;
        }
        if (size != sizeof(uint32_t)) {
            return 0;
        }
        sysctlbyname(name, &result, &size, nullptr, 0);
        return result;
    }

    } // namespace

    CpuIsaInfo detect_cpu_isa_info_internal() {
        CpuIsaInfo info{};
        CpuIsaArm& isa = info.arm_isa;
        isa.is_this_arch = true;
        isa.armv8 = true;
        isa.neon = true;
        isa.idiv = true;
        isa.aes = true;
        isa.sha1 = true;
        isa.sha2 = true;
        isa.pmull = true;
        isa.crc32 = true;

        const uint32_t cpu_family = apple_sysctl_u32("hw.cpufamily");

        isa.atomics = apple_sysctl_u32("hw.optional.arm.FEAT_LSE") != 0;
        if (!isa.atomics) {
            switch (cpu_family) {
                case CPUFAMILY_ARM_MONSOON_MISTRAL:
                case CPUFAMILY_ARM_VORTEX_TEMPEST:
                case CPUFAMILY_ARM_LIGHTNING_THUNDER:
                case CPUFAMILY_ARM_FIRESTORM_ICESTORM:
                    isa.atomics = true;
                    break;
            }
        }

        isa.rdm = apple_sysctl_u32("hw.optional.arm.FEAT_RDM") != 0;
        if (!isa.rdm) {
            switch (cpu_family) {
                case CPUFAMILY_ARM_MONSOON_MISTRAL:
                case CPUFAMILY_ARM_VORTEX_TEMPEST:
                case CPUFAMILY_ARM_LIGHTNING_THUNDER:
                case CPUFAMILY_ARM_FIRESTORM_ICESTORM:
                    isa.rdm = true;
                    break;
            }
        }

        isa.fp16arith = apple_sysctl_u32("hw.optional.arm.FEAT_FP16") != 0;
        if (!isa.fp16arith) {
            switch (cpu_family) {
                case CPUFAMILY_ARM_MONSOON_MISTRAL:
                case CPUFAMILY_ARM_VORTEX_TEMPEST:
                case CPUFAMILY_ARM_LIGHTNING_THUNDER:
                case CPUFAMILY_ARM_FIRESTORM_ICESTORM:
                    isa.fp16arith = true;
                    break;
            }
        }

        isa.fhm = apple_sysctl_u32("hw.optional.arm.FEAT_FHM") != 0;
        if (!isa.fhm) {
            isa.fhm = apple_sysctl_u32("hw.optional.armv8_2_fhm") != 0;
            if (!isa.fhm) {
                switch (cpu_family) {
                    case CPUFAMILY_ARM_LIGHTNING_THUNDER:
                    case CPUFAMILY_ARM_FIRESTORM_ICESTORM:
                        isa.fhm = true;
                        break;
                }
            }
        }

        isa.bf16 = apple_sysctl_u32("hw.optional.arm.FEAT_BF16") != 0;
        isa.fcma = apple_sysctl_u32("hw.optional.arm.FEAT_FCMA") != 0;
        if (!isa.fcma) {
            switch (cpu_family) {
                case CPUFAMILY_ARM_LIGHTNING_THUNDER:
                case CPUFAMILY_ARM_FIRESTORM_ICESTORM:
                    isa.fcma = true;
                    break;
            }
        }

        isa.jscvt = apple_sysctl_u32("hw.optional.arm.FEAT_JSCVT") != 0;
        if (!isa.jscvt) {
            switch (cpu_family) {
                case CPUFAMILY_ARM_LIGHTNING_THUNDER:
                case CPUFAMILY_ARM_FIRESTORM_ICESTORM:
                    isa.jscvt = true;
                    break;
            }
        }

        isa.dot = apple_sysctl_u32("hw.optional.arm.FEAT_DotProd") != 0;
        if (!isa.dot) {
            switch (cpu_family) {
                case CPUFAMILY_ARM_LIGHTNING_THUNDER:
                case CPUFAMILY_ARM_FIRESTORM_ICESTORM:
                    isa.dot = true;
                    break;
            }
        }

        isa.i8mm = apple_sysctl_u32("hw.optional.arm.FEAT_I8MM") != 0;
        isa.sme = apple_sysctl_u32("hw.optional.arm.FEAT_SME") != 0;
        isa.sme2 = apple_sysctl_u32("hw.optional.arm.FEAT_SME2") != 0;
        isa.sme2p1 = apple_sysctl_u32("hw.optional.arm.FEAT_SME2p1") != 0;
        isa.sme_i16i32 = apple_sysctl_u32("hw.optional.arm.SME_I16I32") != 0;
        isa.sme_bi32i32 = apple_sysctl_u32("hw.optional.arm.SME_BI32I32") != 0;
        isa.sme_b16b16 = apple_sysctl_u32("hw.optional.arm.FEAT_SME_B16B16") != 0;
        isa.sme_f16f16 = apple_sysctl_u32("hw.optional.arm.FEAT_SME_F16F16") != 0;
        isa.fp8 = apple_sysctl_u32("hw.optional.arm.FEAT_FP8") != 0;
        isa.f8dot = apple_sysctl_u32("hw.optional.arm.FEAT_FP8DOT4") != 0;
        isa.f8mm = apple_sysctl_u32("hw.optional.arm.FEAT_F8F32MM") != 0;
        isa.smelen = apple_sysctl_u32("hw.optional.arm.sme_max_svl_b");

        isa.sve = apple_sysctl_u32("hw.optional.arm.FEAT_SVE") != 0;
        isa.sve2 = apple_sysctl_u32("hw.optional.arm.FEAT_SVE2") != 0;
        isa.svelen = apple_sysctl_u32("hw.optional.arm.sve_max_svl_b");

        return info;
    }

} // namespace turbo
#endif
