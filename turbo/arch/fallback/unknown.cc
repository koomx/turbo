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
#include <turbo/arch/instruction.h>
#if !KUMO_ARCH_X86_64 && \
    !(KUMO_ARCH_ARM64 && (KUMO_OS_LINUX || KUMO_OS_ANDROID)) && \
    !(KUMO_ARCH_ARM64 && (KUMO_OS_MACOSX || KUMO_OS_IOS || KUMO_OS_TVOS || \
        KUMO_OS_WATCHOS || KUMO_OS_VISIONOS))

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

    bool supports_arm_crc32_pmull() {
        return false;
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

#if !KUMO_ARCH_X86_64 && !KUMO_ARCH_ARM64 && !KUMO_ARCH_PPC64 && \
    !KUMO_ARCH_RISCV64 && !KUMO_ARCH_LOONGARCH

namespace turbo {

    ////////////////////////////////////////////////////////////////////////////////
    // detect_supported_architectures
    ////////////////////////////////////////////////////////////////////////////////

    uint32_t detect_supported_architectures() {
        return InstructionSet::DEFAULT;
    }

} // namespace turbo
#endif

#if !(KUMO_ARCH_ARM64 && (KUMO_OS_MACOSX || KUMO_OS_IOS || KUMO_OS_TVOS || \
        KUMO_OS_WATCHOS || KUMO_OS_VISIONOS)) && \
    !(KUMO_ARCH_ARM64 && (KUMO_OS_LINUX || KUMO_OS_ANDROID)) && \
    !KUMO_ARCH_LOONGARCH && !KUMO_ARCH_PPC64 && !KUMO_ARCH_RISCV64 && \
    !KUMO_ARCH_X86_64
namespace turbo {

    CpuIsaInfo detect_cpu_isa_info_internal() {
        return CpuIsaInfo{};
    }

} // namespace turbo
#endif
