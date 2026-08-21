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
#include <sys/sysctl.h>
#include <sys/types.h>

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

} // namespace turbo
#endif
