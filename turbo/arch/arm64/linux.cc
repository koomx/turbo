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
#include <sys/prctl.h>

#include <turbo/arch/instruction.h>

#ifndef HWCAP_CPUID
#define HWCAP_CPUID (1 << 11)
#endif
#ifndef HWCAP_AES
#define HWCAP_AES (1 << 3)
#endif
#ifndef HWCAP_PMULL
#define HWCAP_PMULL (1 << 4)
#endif
#ifndef HWCAP_SHA1
#define HWCAP_SHA1 (1 << 5)
#endif
#ifndef HWCAP_SHA2
#define HWCAP_SHA2 (1 << 6)
#endif
#ifndef HWCAP_CRC32
#define HWCAP_CRC32 (1 << 7)
#endif
#ifndef HWCAP_ATOMICS
#define HWCAP_ATOMICS (1 << 8)
#endif
#ifndef HWCAP_FPHP
#define HWCAP_FPHP (1 << 9)
#endif
#ifndef HWCAP_ASIMDHP
#define HWCAP_ASIMDHP (1 << 10)
#endif
#ifndef HWCAP_ASIMDRDM
#define HWCAP_ASIMDRDM (1 << 12)
#endif
#ifndef HWCAP_JSCVT
#define HWCAP_JSCVT (1 << 13)
#endif
#ifndef HWCAP_FCMA
#define HWCAP_FCMA (1 << 14)
#endif
#ifndef HWCAP_ASIMDDP
#define HWCAP_ASIMDDP (1 << 20)
#endif
#ifndef HWCAP_SVE
#define HWCAP_SVE (1 << 22)
#endif
#ifndef HWCAP_ASIMDFHM
#define HWCAP_ASIMDFHM (1 << 23)
#endif
#ifndef HWCAP_ASIMD
#define HWCAP_ASIMD (1 << 1)
#endif
#ifndef HWCAP_F8MM
#define HWCAP_F8MM (1ULL << 35)
#endif

#ifndef HWCAP2_SVE2
#define HWCAP2_SVE2 (1UL << 1)
#endif
#ifndef HWCAP2_SVEBF16
#define HWCAP2_SVEBF16 (1UL << 12)
#endif
#ifndef HWCAP2_I8MM
#define HWCAP2_I8MM (1UL << 13)
#endif
#ifndef HWCAP2_BF16
#define HWCAP2_BF16 (1UL << 14)
#endif
#ifndef HWCAP2_SME
#define HWCAP2_SME (1UL << 23)
#endif
#ifndef HWCAP2_SME2
#define HWCAP2_SME2 (1ULL << 37)
#endif
#ifndef HWCAP2_SME2P1
#define HWCAP2_SME2P1 (1ULL << 38)
#endif
#ifndef HWCAP2_SME_I16I32
#define HWCAP2_SME_I16I32 (1ULL << 39)
#endif
#ifndef HWCAP2_SME_BI32I32
#define HWCAP2_SME_BI32I32 (1ULL << 40)
#endif
#ifndef HWCAP2_SME_B16B16
#define HWCAP2_SME_B16B16 (1ULL << 41)
#endif
#ifndef HWCAP2_SME_F16F16
#define HWCAP2_SME_F16F16 (1ULL << 42)
#endif
#ifndef HWCAP2_FP8
#define HWCAP2_FP8 (1ULL << 51)
#endif
#ifndef HWCAP2_F8DOT
#define HWCAP2_F8DOT (1ULL << 53)
#endif

#ifndef AT_HWCAP2
#define AT_HWCAP2 26
#endif

#ifndef PR_SVE_GET_VL
#define PR_SVE_GET_VL 51
#endif
#ifndef PR_SVE_VL_LEN_MASK
#define PR_SVE_VL_LEN_MASK 0xffff
#endif
#ifndef PR_SME_GET_VL
#define PR_SME_GET_VL 64
#endif
#ifndef PR_SME_VL_LEN_MASK
#define PR_SME_VL_LEN_MASK 0xffff
#endif

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

    ////////////////////////////////////////////////////////////////////////////////
    // detect_cpu_isa_info_internal  (cpuinfo arm/linux/aarch64-isa.c)
    ////////////////////////////////////////////////////////////////////////////////

    namespace {

    constexpr uint32_t kMidrImplementerMask = 0xFF000000u;
    constexpr uint32_t kMidrPartMask = 0x0000FFF0u;
    constexpr uint32_t kMidrImplPartMask = kMidrImplementerMask | kMidrPartMask;

    uint32_t midr_variant(uint32_t midr) {
        return (midr >> 20) & 0xfu;
    }

    uint32_t read_midr(uint64_t hwcap) {
        if ((hwcap & HWCAP_CPUID) == 0) {
            return 0;
        }
        uint64_t midr = 0;
        TURBO_INTERNAL_AARCH64_ID_REG_READ(MIDR_EL1, midr);
        return static_cast<uint32_t>(midr);
    }

    void decode_arm64_linux_isa(CpuIsaArm& isa, uint64_t features, uint64_t features2, uint32_t midr) {
        isa.aes = (features & HWCAP_AES) != 0;
        isa.pmull = (features & HWCAP_PMULL) != 0;
        isa.sha1 = (features & HWCAP_SHA1) != 0;
        isa.sha2 = (features & HWCAP_SHA2) != 0;
        isa.crc32 = (features & HWCAP_CRC32) != 0;
        isa.atomics = (features & HWCAP_ATOMICS) != 0;

        const uint32_t fp16arith_mask = HWCAP_FPHP | HWCAP_ASIMDHP;
        switch (midr & kMidrImplPartMask) {
            case 0x4100D050u: /* Cortex-A55 */
            case 0x4100D060u: /* Cortex-A65 */
            case 0x4100D0A0u: /* Cortex-A75 */
            case 0x4100D0B0u: /* Cortex-A76 */
            case 0x4100D0C0u: /* Neoverse N1 */
            case 0x4100D0D0u: /* Cortex-A77 */
            case 0x4100D0E0u: /* Cortex-A76AE */
            case 0x4100D400u: /* Neoverse V1 */
            case 0x4100D490u: /* Neoverse N2 */
            case 0x4100D4F0u: /* Neoverse V2 */
            case 0x4800D400u: /* Cortex-A76 (HiSilicon) */
            case 0x51008020u: /* Kryo 385 Gold */
            case 0x51008030u: /* Kryo 385 Silver */
            case 0x51008040u: /* Kryo 485 Gold */
            case 0x51008050u: /* Kryo 485 Silver */
            case 0x53000030u: /* Exynos M4 */
            case 0x53000040u: /* Exynos M5 */
                isa.fp16arith = true;
                isa.rdm = true;
                break;
            default:
                if ((features & fp16arith_mask) == fp16arith_mask) {
                    isa.fp16arith = true;
                }
                if (features & HWCAP_ASIMDRDM) {
                    isa.rdm = true;
                }
                break;
        }

        isa.i8mm = (features2 & HWCAP2_I8MM) != 0;

        switch (midr & kMidrImplPartMask) {
            case 0x4100D060u:
            case 0x4100D0B0u:
            case 0x4100D0C0u:
            case 0x4100D0D0u:
            case 0x4100D0E0u:
            case 0x4100D400u:
            case 0x4100D490u:
            case 0x4100D4A0u: /* Neoverse E1 */
            case 0x4100D4F0u:
            case 0x4800D400u:
            case 0x51008040u:
            case 0x51008050u:
            case 0x53000030u:
            case 0x53000040u:
                isa.dot = true;
                break;
            case 0x4100D050u: /* Cortex-A55 r1+ */
                isa.dot = midr_variant(midr) >= 1;
                break;
            case 0x4100D0A0u: /* Cortex-A75 r2+ */
                isa.dot = midr_variant(midr) >= 2;
                break;
            default:
                isa.dot = (features & HWCAP_ASIMDDP) != 0;
                break;
        }

        isa.jscvt = (features & HWCAP_JSCVT) != 0;
        isa.fcma = (features & HWCAP_FCMA) != 0;
        isa.sve = (features & HWCAP_SVE) != 0;
        isa.sve2 = (features2 & HWCAP2_SVE2) != 0;
        isa.sme = (features2 & HWCAP2_SME) != 0;
        isa.sme2 = (features2 & HWCAP2_SME2) != 0;
        isa.sme2p1 = (features2 & HWCAP2_SME2P1) != 0;
        isa.sme_i16i32 = (features2 & HWCAP2_SME_I16I32) != 0;
        isa.sme_bi32i32 = (features2 & HWCAP2_SME_BI32I32) != 0;
        isa.sme_b16b16 = (features2 & HWCAP2_SME_B16B16) != 0;
        isa.sme_f16f16 = (features2 & HWCAP2_SME_F16F16) != 0;
        isa.bf16 = (features2 & (HWCAP2_BF16 | HWCAP2_SVEBF16)) != 0;
        isa.fhm = (features & HWCAP_ASIMDFHM) != 0;
        isa.fp8 = (features2 & HWCAP2_FP8) != 0;
        isa.f8dot = (features2 & HWCAP2_F8DOT) != 0;
        isa.f8mm = (features & HWCAP_F8MM) != 0;

        const int sve_vl = prctl(PR_SVE_GET_VL);
        isa.svelen = sve_vl < 0 ? 0 : static_cast<uint32_t>(sve_vl) & PR_SVE_VL_LEN_MASK;
        const int sme_vl = prctl(PR_SME_GET_VL);
        isa.smelen = sme_vl < 0 ? 0 : static_cast<uint32_t>(sme_vl) & PR_SME_VL_LEN_MASK;
    }

    } // namespace

    CpuIsaInfo detect_cpu_isa_info_internal() {
        CpuIsaInfo info{};
        CpuIsaArm& isa = info.arm_isa;
        isa.is_this_arch = true;
        isa.armv8 = true;
        isa.idiv = true;

        const uint64_t features = getauxval(AT_HWCAP);
        const uint64_t features2 = getauxval(AT_HWCAP2);
        if (features & HWCAP_ASIMD) {
            isa.neon = true;
            isa.d32 = true;
            isa.fma = true;
        }
        if (features & HWCAP_FPHP) {
            isa.fp16 = true;
        }

        decode_arm64_linux_isa(isa, features, features2, read_midr(features));
        return info;
    }

} // namespace turbo
#endif
