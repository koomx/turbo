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

#include <turbo/hash/xx/engine/rvv/interface.h>

#if KUMO_SIMD_RVV
#include <riscv_vector.h>

#if ((defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 13) || (defined(__clang__) && __clang_major__ < 16))
#define XXHASH_RVOP(op) op
#define XXHASH_RVCAST(op) KUMO_CONCAT(vreinterpret_v_, op)
#else
#define XXHASH_RVOP(op) KUMO_CONCAT(__riscv_, op)
#define XXHASH_RVCAST(op) KUMO_CONCAT(__riscv_vreinterpret_v_, op)
#endif

namespace turbo::xxhash {

    KUMO_FORCE_INLINE void xxhash_accumulate_512_rvv(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        KUMO_DASSERT((((size_t)acc) & 63) == 0);
        {
            size_t vl = XXHASH_RVOP(vsetvl_e64m2)(8);

            uint64_t* xacc = (uint64_t*)acc;
            const uint64_t* xinput = (const uint64_t*)input;
            const uint64_t* xsecret = (const uint64_t*)secret;
            static const uint64_t swap_mask[16] = { 1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14 };
            vuint64m2_t xswap_mask = XXHASH_RVOP(vle64_v_u64m2)(swap_mask, vl);

            size_t i;
            for (i = 0; i < kXxhStripeLen / 8; i += vl) {
                vuint64m2_t data_vec = XXHASH_RVCAST(u8m2_u64m2)(XXHASH_RVOP(vle8_v_u8m2)((const uint8_t*)(xinput + i), vl * 8));
                vuint64m2_t key_vec = XXHASH_RVCAST(u8m2_u64m2)(XXHASH_RVOP(vle8_v_u8m2)((const uint8_t*)(xsecret + i), vl * 8));
                vuint64m2_t acc_vec = XXHASH_RVOP(vle64_v_u64m2)(xacc + i, vl);
                vuint64m2_t data_key = XXHASH_RVOP(vxor_vv_u64m2)(data_vec, key_vec, vl);
                vuint64m2_t data_key_hi = XXHASH_RVOP(vsrl_vx_u64m2)(data_key, 32, vl);
                vuint64m2_t data_key_lo = XXHASH_RVOP(vand_vx_u64m2)(data_key, 0xffffffff, vl);
                vuint64m2_t data_swap = XXHASH_RVOP(vrgather_vv_u64m2)(data_vec, xswap_mask, vl);
                acc_vec = XXHASH_RVOP(vmacc_vv_u64m2)(acc_vec, data_key_lo, data_key_hi, vl);
                acc_vec = XXHASH_RVOP(vadd_vv_u64m2)(acc_vec, data_swap, vl);
                XXHASH_RVOP(vse64_v_u64m2)(xacc + i, acc_vec, vl);
            }
        }
    }

    KUMO_FORCE_INLINE void xxhash_accumulate_rvv(uint64_t* KUMO_RESTRICT acc,
        const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret,
        size_t nbStripes) {
        size_t n;
        for (n = 0; n < nbStripes; n++) {
            const uint8_t* const in = input + n * kXxhStripeLen;
            prefetch_to_local_cache(in + kDefaultXxhPrefetchDist);
            xxhash_accumulate_512_rvv(acc, in, secret + n * kXxhSecretConsumeRate);
        }
    }

    KUMO_FORCE_INLINE void xxhash_scramble_acc_rvv(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT secret) {
        KUMO_DASSERT((((size_t)acc) & 15) == 0);
        {
            size_t count = kXxhStripeLen / 8;
            uint64_t* xacc = (uint64_t*)acc;
            const uint8_t* xsecret = (const uint8_t*)secret;
            size_t vl;
            for (; count > 0; count -= vl, xacc += vl, xsecret += vl * 8) {
                vl = XXHASH_RVOP(vsetvl_e64m2)(count);
                {
                    vuint64m2_t key_vec = XXHASH_RVCAST(u8m2_u64m2)(XXHASH_RVOP(vle8_v_u8m2)(xsecret, vl * 8));
                    vuint64m2_t acc_vec = XXHASH_RVOP(vle64_v_u64m2)(xacc, vl);
                    vuint64m2_t vsrl = XXHASH_RVOP(vsrl_vx_u64m2)(acc_vec, 47, vl);
                    acc_vec = XXHASH_RVOP(vxor_vv_u64m2)(acc_vec, vsrl, vl);
                    acc_vec = XXHASH_RVOP(vxor_vv_u64m2)(acc_vec, key_vec, vl);
                    acc_vec = XXHASH_RVOP(vmul_vx_u64m2)(acc_vec, kXxhPrime32_1, vl);
                    XXHASH_RVOP(vse64_v_u64m2)(xacc, acc_vec, vl);
                }
            }
        }
    }

    KUMO_FORCE_INLINE void xxhash_init_custom_secret_rvv(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        static_assert(KUMO_CACHELINE_SIZE >= 8, "KUMO_CACHELINE_SIZE >= 8");
        KUMO_DASSERT(((size_t)customSecret & 7) == 0);
        (void)(&turbo::little_endian::Store64);
        {
            size_t count = kXxhSecretDefaultSize / 8;
            size_t vl;
            size_t VLMAX = XXHASH_RVOP(vsetvlmax_e64m2)();
            int64_t* cSecret = (int64_t*)customSecret;
            const int64_t* kSecret = (const int64_t*)(const void*)kXxhSecret;

#if __riscv_v_intrinsic >= 1000000
            vbool32_t mneg = XXHASH_RVCAST(u8m1_b32)(
                XXHASH_RVOP(vmv_v_x_u8m1)(0xaa, XXHASH_RVOP(vsetvlmax_e8m1)()));
#else
            size_t vlmax = XXHASH_RVOP(vsetvlmax_e8m1)();
            vbool32_t mneg = XXHASH_RVOP(vmseq_vx_u8mf4_b32)(
                XXHASH_RVOP(vand_vx_u8mf4)(
                    XXHASH_RVOP(vid_v_u8mf4)(vlmax), 1, vlmax),
                1, vlmax);
#endif
            vint64m2_t seed = XXHASH_RVOP(vmv_v_x_i64m2)((int64_t)seed64, VLMAX);
            seed = XXHASH_RVOP(vneg_v_i64m2_mu)(mneg, seed, seed, VLMAX);

            for (; count > 0; count -= vl, cSecret += vl, kSecret += vl) {
                vl = XXHASH_RVOP(vsetvl_e64m2)(count < VLMAX ? count : VLMAX);
                {
                    vint64m2_t src = XXHASH_RVOP(vle64_v_i64m2)(kSecret, vl);
                    vint64m2_t res = XXHASH_RVOP(vadd_vv_i64m2)(src, seed, vl);
                    XXHASH_RVOP(vse64_v_i64m2)(cSecret, res, vl);
                }
            }
        }
    }

    void XXHashEngineRvv::init_custom_secret(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        xxhash_init_custom_secret_rvv(customSecret, seed64);
    }

    void XXHashEngineRvv::accumulate(uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret, size_t nbStripes) {
        xxhash_accumulate_rvv(acc, input, secret, nbStripes);
    }

    void XXHashEngineRvv::scramble_acc(void* KUMO_RESTRICT acc, const void* secret) {
        xxhash_scramble_acc_rvv(acc, secret);
    }

    void XXHashEngineRvv::accumulate_512(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        xxhash_accumulate_512_rvv(acc, input, secret);
    }

    static XXHashEngine* get_xxhash_rvv_instance() {
        static XXHashEngineRvv ins;
        return &ins;
    }
} // namespace turbo::xxhash
#else
namespace turbo::xxhash {
    static XXHashEngine* get_xxhash_rvv_instance() {
        return nullptr;
    }
} // namespace turbo::xxhash
#endif

namespace turbo::xxhash {
    IsaInfo get_xxhash_rvv_info() {
        static IsaInfo ins = {
            KUMO_SIMD_RVV == 1,
            false,
            { kRiscvV },
            "rvv",
            get_xxhash_rvv_instance(),
        };
        return ins;
    }
} // namespace turbo::xxhash
