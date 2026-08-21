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

#include <turbo/arch/isa.h>

namespace turbo {

    // Per-arch ranks are comparable only on that arch. Fallback is always 1
    // (see make_isa_rank). SIMD ranks start at 10 so they beat fallback.

#if KUMO_ARCH_ARM
    /// NEON = 10; anything else on ARM that is not fallback is unusable here.
    static uint32_t make_arm(const IsaInfo& info) {
        if (info.required_isa & InstructionSet::NEON) {
            return 10;
        }
        return 0;
    }
#endif

#if KUMO_ARCH_X86
    /// SSE4.2 = 10, AVX2 = 20, any AVX-512 bit = 50; extra bits add +1 each.
    static uint32_t make_amd(const IsaInfo& info) {
        const uint32_t r = info.required_isa;
        constexpr uint32_t k_avx512 = InstructionSet::AVX512F | InstructionSet::AVX512DQ | InstructionSet::AVX512IFMA | InstructionSet::AVX512PF | InstructionSet::AVX512ER | InstructionSet::AVX512CD | InstructionSet::AVX512BW | InstructionSet::AVX512VL | InstructionSet::AVX512VBMI2 | InstructionSet::AVX512VPOPCNTDQ;

        uint32_t rank = 0;
        if (r & k_avx512) {
            rank = 50;
        } else if (r & InstructionSet::AVX2) {
            rank = 20;
        } else if (r & InstructionSet::SSE42) {
            rank = 10;
        } else {
            return 0;
        }

        if (r & InstructionSet::BMI1) {
            rank += 1;
        }
        if (r & InstructionSet::BMI2) {
            rank += 1;
        }
        if (r & InstructionSet::PCLMULQDQ) {
            rank += 1;
        }
        if (rank >= 50) {
            if (r & InstructionSet::AVX512DQ) {
                rank += 1;
            }
            if (r & InstructionSet::AVX512IFMA) {
                rank += 1;
            }
            if (r & InstructionSet::AVX512PF) {
                rank += 1;
            }
            if (r & InstructionSet::AVX512ER) {
                rank += 1;
            }
            if (r & InstructionSet::AVX512CD) {
                rank += 1;
            }
            if (r & InstructionSet::AVX512BW) {
                rank += 1;
            }
            if (r & InstructionSet::AVX512VL) {
                rank += 1;
            }
            if (r & InstructionSet::AVX512VBMI2) {
                rank += 1;
            }
            if (r & InstructionSet::AVX512VPOPCNTDQ) {
                rank += 1;
            }
        }
        return rank;
    }
#endif

#if KUMO_ARCH_PPC
    /// AltiVec = 10.
    static uint32_t make_ppc(const IsaInfo& info) {
        if (info.required_isa & InstructionSet::ALTIVEC) {
            return 10;
        }
        return 0;
    }
#endif

#if KUMO_ARCH_LOONGARCH
    /// LSX (128-bit) = 10, LASX (256-bit) = 20.
    static uint32_t make_loongarch(const IsaInfo& info) {
        if (info.required_isa & InstructionSet::LASX) {
            return 20;
        }
        if (info.required_isa & InstructionSet::LSX) {
            return 10;
        }
        return 0;
    }
#endif

#if KUMO_ARCH_RISCV
    /// RVV = 10, plus 1 if Zvbb is required.
    static uint32_t make_riscv(const IsaInfo& info) {
        if ((info.required_isa & InstructionSet::RVV) == 0) {
            return 0;
        }
        uint32_t rank = 10;
        if (info.required_isa & InstructionSet::ZVBB) {
            rank += 1;
        }
        return rank;
    }
#endif

    uint32_t make_isa_rank(const IsaInfo& info) {
        // Uncompiled or no engine: never selected.
        // Fallback skips the required_isa check so it always ranks 1.
        if (info.engine == nullptr || !info.compiled) {
            return 0;
        }
        if (info.failback) {
            return 1;
        }
        const uint32_t avail = info.current_compiled & info.current_isa;
        // Need every required bit both compiled in and present on the CPU.
        if ((info.required_isa & avail) != info.required_isa) {
            return 0;
        }

#if KUMO_ARCH_ARM
        return make_arm(info);
#elif KUMO_ARCH_X86
        return make_amd(info);
#elif KUMO_ARCH_PPC
        return make_ppc(info);
#elif KUMO_ARCH_LOONGARCH
        return make_loongarch(info);
#elif KUMO_ARCH_RISCV
        return make_riscv(info);
#else
        return 0;
#endif
    }

} // namespace turbo
