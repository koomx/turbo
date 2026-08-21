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
#if KUMO_ARCH_ARM64 && (KUMO_OS_LINUX || KUMO_OS_ANDROID)
#include <cstdint>

#include <asm/hwcap.h>
#include <sys/auxv.h>

namespace turbo {

#ifndef HWCAP_CPUID
#define HWCAP_CPUID (1 << 11)
#endif

#define TURBO_INTERNAL_AARCH64_ID_REG_READ(id, val) \
    asm("mrs %0, " #id : "=r"(val))

    ////////////////////////////////////////////////////////////////////////////////
    // get_cpu_type
    ////////////////////////////////////////////////////////////////////////////////

    CpuType get_cpu_type() {
        // MIDR_EL1 is not visible to EL0, however the access will be emulated by
        // linux if AT_HWCAP has HWCAP_CPUID set.
        //
        // This method will be unreliable on heterogeneous computing systems (ex:
        // big.LITTLE) since the value of MIDR_EL1 will change based on the calling
        // thread.
        uint64_t hwcaps = getauxval(AT_HWCAP);
        if (hwcaps & HWCAP_CPUID) {
            uint64_t midr = 0;
            TURBO_INTERNAL_AARCH64_ID_REG_READ(MIDR_EL1, midr);
            uint32_t implementer = (midr >> 24) & 0xff;
            uint32_t part_number = (midr >> 4) & 0xfff;
            switch (implementer) {
            case 0x41:
                switch (part_number) {
                case 0xd0c:
                    return CpuType::kArmNeoverseN1;
                case 0xd40:
                    return CpuType::kArmNeoverseV1;
                case 0xd49:
                    return CpuType::kArmNeoverseN2;
                case 0xd4f: {
                    uint64_t isar0 = 0;
                    TURBO_INTERNAL_AARCH64_ID_REG_READ(ID_AA64ISAR0_EL1, isar0);
                    if (((isar0 >> 60) & 0xf) == 0x0) {
                        return CpuType::kNvidiaGrace;
                    }
                    return CpuType::kArmNeoverseV2;
                }
                case 0xd8e:
                    return CpuType::kArmNeoverseN3;
                default:
                    return CpuType::kUnknown;
                }
                break;
            case 0xc0:
                switch (part_number) {
                case 0xac3:
                    return CpuType::kAmpereSiryn;
                default:
                    return CpuType::kUnknown;
                }
                break;
            default:
                return CpuType::kUnknown;
            }
        }
        return CpuType::kUnknown;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // supports_arm_crc32_pmull
    ////////////////////////////////////////////////////////////////////////////////

    bool supports_arm_crc32_pmull() {
#if defined(HWCAP_CRC32) && defined(HWCAP_PMULL)
        uint64_t hwcaps = getauxval(AT_HWCAP);
        return (hwcaps & HWCAP_CRC32) && (hwcaps & HWCAP_PMULL);
#else
        return false;
#endif
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

} // namespace turbo
#endif
