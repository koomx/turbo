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
#if KUMO_ARCH_RISCV64 && (KUMO_OS_LINUX || KUMO_OS_ANDROID)
#include <cstdint>
#include <unistd.h>

#include <turbo/arch/instruction.h>

struct unicode_riscv_hwprobe {
    int64_t key;
    uint64_t value;
};
#define unicode_riscv_hwprobe(...) syscall(258, __VA_ARGS__)
#define UNICODE_RISCV_HWPROBE_KEY_IMA_EXT_0 4
#define UNICODE_RISCV_HWPROBE_IMA_V (1 << 2)
#define UNICODE_RISCV_HWPROBE_EXT_ZVBB (1 << 17)

namespace turbo {

    ////////////////////////////////////////////////////////////////////////////////
    // detect_supported_architectures
    ////////////////////////////////////////////////////////////////////////////////

    uint32_t detect_supported_architectures() {
        uint32_t host_isa = InstructionSet::DEFAULT;
#if UNICODE_IS_RVV
        host_isa |= InstructionSet::RVV;
#endif
#if UNICODE_IS_ZVBB
        host_isa |= InstructionSet::ZVBB;
#endif
        unicode_riscv_hwprobe probes[] = { { UNICODE_RISCV_HWPROBE_KEY_IMA_EXT_0, 0 } };
        long ret = unicode_riscv_hwprobe(&probes, sizeof probes / sizeof *probes, 0,
            nullptr, 0);
        if (ret == 0) {
            uint64_t extensions = probes[0].value;
            if (extensions & UNICODE_RISCV_HWPROBE_IMA_V)
                host_isa |= InstructionSet::RVV;
            if (extensions & UNICODE_RISCV_HWPROBE_EXT_ZVBB)
                host_isa |= InstructionSet::ZVBB;
        }
#if defined(RUN_IN_SPIKE_SIMULATOR)
        host_isa |= InstructionSet::RVV;
#endif
        return host_isa;
    }

} // namespace turbo
#endif
