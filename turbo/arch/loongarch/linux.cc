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
#if KUMO_ARCH_LOONGARCH && (KUMO_OS_LINUX || KUMO_OS_ANDROID)
#include <cstdint>
#include <sys/auxv.h>

#include <turbo/arch/instruction.h>

#ifndef HWCAP_LOONGARCH_LSX
#define HWCAP_LOONGARCH_LSX (1 << 4)
#endif
#ifndef HWCAP_LOONGARCH_LASX
#define HWCAP_LOONGARCH_LASX (1 << 5)
#endif

namespace turbo {

    ////////////////////////////////////////////////////////////////////////////////
    // detect_supported_architectures
    ////////////////////////////////////////////////////////////////////////////////

    uint32_t detect_supported_architectures() {
        uint32_t host_isa = InstructionSet::DEFAULT;
        uint64_t hwcap = getauxval(AT_HWCAP);
        if (hwcap & HWCAP_LOONGARCH_LSX) {
            host_isa |= InstructionSet::LSX;
        }
        if (hwcap & HWCAP_LOONGARCH_LASX) {
            host_isa |= InstructionSet::LASX;
        }
        return host_isa;
    }

} // namespace turbo
#endif
