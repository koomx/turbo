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
#include <sys/auxv.h>
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
#ifndef UNICODE_RISCV_HWPROBE_EXT_ZFH
#define UNICODE_RISCV_HWPROBE_EXT_ZFH (1ULL << 27)
#endif
#ifndef UNICODE_RISCV_HWPROBE_EXT_ZVFH
#define UNICODE_RISCV_HWPROBE_EXT_ZVFH (1ULL << 29)
#endif

#define COMPAT_HWCAP_ISA_I (1 << ('I' - 'A'))
#define COMPAT_HWCAP_ISA_E (1 << ('E' - 'A'))
#define COMPAT_HWCAP_ISA_M (1 << ('M' - 'A'))
#define COMPAT_HWCAP_ISA_A (1 << ('A' - 'A'))
#define COMPAT_HWCAP_ISA_F (1 << ('F' - 'A'))
#define COMPAT_HWCAP_ISA_D (1 << ('D' - 'A'))
#define COMPAT_HWCAP_ISA_C (1 << ('C' - 'A'))
#define COMPAT_HWCAP_ISA_V (1 << ('V' - 'A'))

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

    CpuIsaInfo detect_cpu_isa_info_internal() {
        CpuIsaInfo info{};
        CpuIsaRiscv& isa = info.riscv_isa;
        isa.is_this_arch = true;

        const unsigned long hwcap = getauxval(AT_HWCAP);
        isa.i = (hwcap & COMPAT_HWCAP_ISA_I) != 0;
        isa.e = (hwcap & COMPAT_HWCAP_ISA_E) != 0;
        isa.m = (hwcap & COMPAT_HWCAP_ISA_M) != 0;
        isa.a = (hwcap & COMPAT_HWCAP_ISA_A) != 0;
        isa.f = (hwcap & COMPAT_HWCAP_ISA_F) != 0;
        isa.d = (hwcap & COMPAT_HWCAP_ISA_D) != 0;
        isa.c = (hwcap & COMPAT_HWCAP_ISA_C) != 0;
        isa.v = (hwcap & COMPAT_HWCAP_ISA_V) != 0;

        unicode_riscv_hwprobe probes[] = { { UNICODE_RISCV_HWPROBE_KEY_IMA_EXT_0, 0 } };
        const long ret = unicode_riscv_hwprobe(&probes, sizeof probes / sizeof *probes, 0, nullptr, 0);
        if (ret == 0) {
            const uint64_t extensions = probes[0].value;
            if (extensions & UNICODE_RISCV_HWPROBE_IMA_V) {
                isa.v = true;
            }
            if (extensions & UNICODE_RISCV_HWPROBE_EXT_ZFH) {
                isa.zfh = true;
            }
            if (extensions & UNICODE_RISCV_HWPROBE_EXT_ZVFH) {
                isa.zvfh = true;
            }
        }
#if defined(RUN_IN_SPIKE_SIMULATOR)
        isa.v = true;
#endif
        return info;
    }

} // namespace turbo
#endif
