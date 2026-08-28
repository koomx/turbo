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
#if KUMO_ARCH_PPC64
#include <turbo/arch/instruction.h>
#if KUMO_OS_LINUX || KUMO_OS_ANDROID
#include <cstdint>
#include <sys/auxv.h>
#ifndef PPC_FEATURE_HAS_ALTIVEC
#define PPC_FEATURE_HAS_ALTIVEC 0x10000000
#endif
#ifndef PPC_FEATURE_HAS_VSX
#define PPC_FEATURE_HAS_VSX 0x00000080
#endif
#endif

namespace turbo {

    ////////////////////////////////////////////////////////////////////////////////
    // detect_supported_architectures
    ////////////////////////////////////////////////////////////////////////////////

    uint32_t detect_supported_architectures() {
        return InstructionSet::ALTIVEC;
    }

    CpuIsaInfo detect_cpu_isa_info_internal() {
        CpuIsaInfo info{};
        info.ppc_isa.is_this_arch = true;
#if KUMO_OS_LINUX || KUMO_OS_ANDROID
        const uint64_t hwcap = getauxval(AT_HWCAP);
        info.ppc_isa.altivec = (hwcap & PPC_FEATURE_HAS_ALTIVEC) != 0;
        info.ppc_isa.vsx = (hwcap & PPC_FEATURE_HAS_VSX) != 0;
#else
        info.ppc_isa.altivec = true;
#endif
        return info;
    }

} // namespace turbo
#endif
