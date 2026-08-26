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

#include <turbo/hash/xx/engine/sve/interface.h>

#if KUMO_SIMD_SVE
namespace turbo::xxhash {

#define ACCRND(acc, offset)                                              \
    do {                                                                 \
        svuint64_t input_vec = svld1_u64(mask, xinput + offset);         \
        svuint64_t secret_vec = svld1_u64(mask, xsecret + offset);       \
        svuint64_t mixed = sveor_u64_x(mask, secret_vec, input_vec);     \
        svuint64_t swapped = svtbl_u64(input_vec, kSwap);                \
        svuint64_t mixed_lo = svextw_u64_x(mask, mixed);                 \
        svuint64_t mixed_hi = svlsr_n_u64_x(mask, mixed, 32);            \
        svuint64_t mul = svmad_u64_x(mask, mixed_lo, mixed_hi, swapped); \
        acc = svadd_u64_x(mask, acc, mul);                               \
    } while (0)

    KUMO_FORCE_INLINE void XXH3_accumulate_512_sve(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        uint64_t* xacc = (uint64_t*)acc;
        const uint64_t* xinput = (const uint64_t*)(const void*)input;
        const uint64_t* xsecret = (const uint64_t*)(const void*)secret;
        svuint64_t kSwap = sveor_n_u64_z(svptrue_b64(), svindex_u64(0, 1), 1);
        uint64_t element_count = svcntd();
        if (element_count >= 8) {
            svbool_t mask = svptrue_pat_b64(SV_VL8);
            svuint64_t vacc = svld1_u64(mask, xacc);
            ACCRND(vacc, 0);
            svst1_u64(mask, xacc, vacc);
        } else if (element_count == 2) {
            svbool_t mask = svptrue_pat_b64(SV_VL2);
            svuint64_t acc0 = svld1_u64(mask, xacc + 0);
            svuint64_t acc1 = svld1_u64(mask, xacc + 2);
            svuint64_t acc2 = svld1_u64(mask, xacc + 4);
            svuint64_t acc3 = svld1_u64(mask, xacc + 6);
            ACCRND(acc0, 0);
            ACCRND(acc1, 2);
            ACCRND(acc2, 4);
            ACCRND(acc3, 6);
            svst1_u64(mask, xacc + 0, acc0);
            svst1_u64(mask, xacc + 2, acc1);
            svst1_u64(mask, xacc + 4, acc2);
            svst1_u64(mask, xacc + 6, acc3);
        } else {
            svbool_t mask = svptrue_pat_b64(SV_VL4);
            svuint64_t acc0 = svld1_u64(mask, xacc + 0);
            svuint64_t acc1 = svld1_u64(mask, xacc + 4);
            ACCRND(acc0, 0);
            ACCRND(acc1, 4);
            svst1_u64(mask, xacc + 0, acc0);
            svst1_u64(mask, xacc + 4, acc1);
        }
    }

    KUMO_FORCE_INLINE void XXH3_accumulate_sve(uint64_t* KUMO_RESTRICT acc,
        const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret,
        size_t nbStripes) {
        if (nbStripes != 0) {
            uint64_t* xacc = (uint64_t*)acc;
            const uint64_t* xinput = (const uint64_t*)(const void*)input;
            const uint64_t* xsecret = (const uint64_t*)(const void*)secret;
            svuint64_t kSwap = sveor_n_u64_z(svptrue_b64(), svindex_u64(0, 1), 1);
            uint64_t element_count = svcntd();
            if (element_count >= 8) {
                svbool_t mask = svptrue_pat_b64(SV_VL8);
                svuint64_t vacc = svld1_u64(mask, xacc + 0);
                do {
                    svprfd(mask, xinput + 128, SV_PLDL1STRM);
                    ACCRND(vacc, 0);
                    xinput += 8;
                    xsecret += 1;
                    nbStripes--;
                } while (nbStripes != 0);

                svst1_u64(mask, xacc + 0, vacc);
            } else if (element_count == 2) {
                svbool_t mask = svptrue_pat_b64(SV_VL2);
                svuint64_t acc0 = svld1_u64(mask, xacc + 0);
                svuint64_t acc1 = svld1_u64(mask, xacc + 2);
                svuint64_t acc2 = svld1_u64(mask, xacc + 4);
                svuint64_t acc3 = svld1_u64(mask, xacc + 6);
                do {
                    svprfd(mask, xinput + 128, SV_PLDL1STRM);
                    ACCRND(acc0, 0);
                    ACCRND(acc1, 2);
                    ACCRND(acc2, 4);
                    ACCRND(acc3, 6);
                    xinput += 8;
                    xsecret += 1;
                    nbStripes--;
                } while (nbStripes != 0);

                svst1_u64(mask, xacc + 0, acc0);
                svst1_u64(mask, xacc + 2, acc1);
                svst1_u64(mask, xacc + 4, acc2);
                svst1_u64(mask, xacc + 6, acc3);
            } else {
                svbool_t mask = svptrue_pat_b64(SV_VL4);
                svuint64_t acc0 = svld1_u64(mask, xacc + 0);
                svuint64_t acc1 = svld1_u64(mask, xacc + 4);
                do {
                    svprfd(mask, xinput + 128, SV_PLDL1STRM);
                    ACCRND(acc0, 0);
                    ACCRND(acc1, 4);
                    xinput += 8;
                    xsecret += 1;
                    nbStripes--;
                } while (nbStripes != 0);

                svst1_u64(mask, xacc + 0, acc0);
                svst1_u64(mask, xacc + 4, acc1);
            }
        }
    }

    void XXHashEngineSve::init_custom_secret(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        XXH3_initCustomSecret_scalar(customSecret, seed64);
    }

    void XXHashEngineSve::accumulate(uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret, size_t nbStripes) {
        XXH3_accumulate_sve(acc, input, secret, nbStripes);
    }

    void XXHashEngineSve::scramble_acc(void* KUMO_RESTRICT acc, const void* secret) {
        XXH3_scrambleAcc_scalar(acc, secret);
    }

    void XXHashEngineSve::accumulate_512(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        XXH3_accumulate_512_sve(acc, input, secret);
    }

    static XXHashEngine* get_xxhash_sve_instance() {
        static XXHashEngineSve ins;
        return &ins;
    }
} // namespace turbo::xxhash
#else
namespace turbo::xxhash {
    static XXHashEngine* get_xxhash_sve_instance() {
        return nullptr;
    }
} // namespace turbo::xxhash
#endif

namespace turbo::xxhash {
    IsaInfo get_xxhash_sve_info() {
        static IsaInfo ins = {
            KUMO_SIMD_SVE == 1,
            false,
            static_cast<uint32_t>(InstructionSet::NEON),
            "sve",
            get_xxhash_sve_instance(),
        };
        return ins;
    }
} // namespace turbo::xxhash
