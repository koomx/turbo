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

#include <turbo/hash/xx/engine/ppc64/interface.h>

#if KUMO_SIMD_VSX
#include <cstring>

#ifdef __has_builtin
#define XXH_HAS_BUILTIN(x) __has_builtin(x)
#else
#define XXH_HAS_BUILTIN(x) 0
#endif

#pragma push_macro("bool")
#pragma push_macro("vector")
#pragma push_macro("pixel")
#undef bool
#undef vector
#undef pixel

#if defined(__s390x__)
#include <s390intrin.h>
#else
#include <altivec.h>
#endif

#pragma pop_macro("pixel")
#pragma pop_macro("vector")
#pragma pop_macro("bool")

namespace turbo::xxhash {

    typedef __vector unsigned long long xxh_u64x2;
    typedef __vector unsigned char xxh_u8x16;
    typedef __vector unsigned xxh_u32x4;
    typedef xxh_u64x2 xxh_aliasing_u64x2 XXH_ALIASING;

#ifndef XXH_VSX_BE
#if defined(__BIG_ENDIAN__) \
    || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define XXH_VSX_BE 1
#elif defined(__VEC_ELEMENT_REG_ORDER__) && __VEC_ELEMENT_REG_ORDER__ == __ORDER_BIG_ENDIAN__
#define XXH_VSX_BE 1
#else
#define XXH_VSX_BE 0
#endif
#endif

#if XXH_VSX_BE
#if defined(__POWER9_VECTOR__) || (defined(__clang__) && defined(__s390x__))
#define XXH_vec_revb vec_revb
#else
    KUMO_FORCE_INLINE xxh_u64x2 XXH_vec_revb(xxh_u64x2 val) {
        xxh_u8x16 const vByteSwap = { 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
            0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08 };
        return vec_perm(val, val, vByteSwap);
    }
#endif
#endif

    KUMO_FORCE_INLINE xxh_u64x2 XXH_vec_loadu(const void* ptr) {
        xxh_u64x2 ret;
        memcpy(&ret, ptr, sizeof(xxh_u64x2));
#if XXH_VSX_BE
        ret = XXH_vec_revb(ret);
#endif
        return ret;
    }

#if defined(__s390x__)
#define XXH_vec_mulo vec_mulo
#define XXH_vec_mule vec_mule
#elif defined(__clang__) && XXH_HAS_BUILTIN(__builtin_altivec_vmuleuw) && !defined(__ibmxl__)
#define XXH_vec_mulo __builtin_altivec_vmulouw
#define XXH_vec_mule __builtin_altivec_vmuleuw
#else
    KUMO_FORCE_INLINE xxh_u64x2 XXH_vec_mulo(xxh_u32x4 a, xxh_u32x4 b) {
        xxh_u64x2 result;
        __asm__("vmulouw %0, %1, %2" : "=v"(result) : "v"(a), "v"(b));
        return result;
    }
    KUMO_FORCE_INLINE xxh_u64x2 XXH_vec_mule(xxh_u32x4 a, xxh_u32x4 b) {
        xxh_u64x2 result;
        __asm__("vmuleuw %0, %1, %2" : "=v"(result) : "v"(a), "v"(b));
        return result;
    }
#endif

    KUMO_FORCE_INLINE void XXH3_accumulate_512_vsx(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        xxh_aliasing_u64x2* const xacc = (xxh_aliasing_u64x2*)acc;
        uint8_t const* const xinput = (uint8_t const*)input;
        uint8_t const* const xsecret = (uint8_t const*)secret;
        xxh_u64x2 const v32 = { 32, 32 };
        size_t i;
        for (i = 0; i < XXH_STRIPE_LEN / sizeof(xxh_u64x2); i++) {
            xxh_u64x2 const data_vec = XXH_vec_loadu(xinput + 16 * i);
            xxh_u64x2 const key_vec = XXH_vec_loadu(xsecret + 16 * i);
            xxh_u64x2 const data_key = data_vec ^ key_vec;
            xxh_u32x4 const shuffled = (xxh_u32x4)vec_rl(data_key, v32);
            xxh_u64x2 const product = XXH_vec_mulo((xxh_u32x4)data_key, shuffled);
            xxh_u64x2 acc_vec = xacc[i];
            acc_vec += product;
#ifdef __s390x__
            acc_vec += vec_permi(data_vec, data_vec, 2);
#else
            acc_vec += vec_xxpermdi(data_vec, data_vec, 2);
#endif
            xacc[i] = acc_vec;
        }
    }

    KUMO_FORCE_INLINE void XXH3_accumulate_vsx(uint64_t* KUMO_RESTRICT acc,
        const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret,
        size_t nbStripes) {
        size_t n;
        for (n = 0; n < nbStripes; n++) {
            const uint8_t* const in = input + n * XXH_STRIPE_LEN;
            XXH_PREFETCH(in + XXH_PREFETCH_DIST);
            XXH3_accumulate_512_vsx(acc, in, secret + n * XXH_SECRET_CONSUME_RATE);
        }
    }

    KUMO_FORCE_INLINE void XXH3_scrambleAcc_vsx(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT secret) {
        KUMO_DASSERT((((size_t)acc) & 15) == 0);
        {
            xxh_aliasing_u64x2* const xacc = (xxh_aliasing_u64x2*)acc;
            const uint8_t* const xsecret = (const uint8_t*)secret;
            xxh_u64x2 const v32 = { 32, 32 };
            xxh_u64x2 const v47 = { 47, 47 };
            xxh_u32x4 const prime = { XXH_PRIME32_1, XXH_PRIME32_1, XXH_PRIME32_1, XXH_PRIME32_1 };
            size_t i;
            for (i = 0; i < XXH_STRIPE_LEN / sizeof(xxh_u64x2); i++) {
                xxh_u64x2 const acc_vec = xacc[i];
                xxh_u64x2 const data_vec = acc_vec ^ (acc_vec >> v47);
                xxh_u64x2 const key_vec = XXH_vec_loadu(xsecret + 16 * i);
                xxh_u64x2 const data_key = data_vec ^ key_vec;
                xxh_u64x2 const prod_even = XXH_vec_mule((xxh_u32x4)data_key, prime);
                xxh_u64x2 const prod_odd = XXH_vec_mulo((xxh_u32x4)data_key, prime);
                xacc[i] = prod_odd + (prod_even << v32);
            }
        }
    }

    void XXHashEnginePpc64::init_custom_secret(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        XXH3_initCustomSecret_scalar(customSecret, seed64);
    }

    void XXHashEnginePpc64::accumulate(uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret, size_t nbStripes) {
        XXH3_accumulate_vsx(acc, input, secret, nbStripes);
    }

    void XXHashEnginePpc64::scramble_acc(void* KUMO_RESTRICT acc, const void* secret) {
        XXH3_scrambleAcc_vsx(acc, secret);
    }

    void XXHashEnginePpc64::accumulate_512(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        XXH3_accumulate_512_vsx(acc, input, secret);
    }

    static XXHashEngine* get_xxhash_ppc64_instance() {
        static XXHashEnginePpc64 ins;
        return &ins;
    }
} // namespace turbo::xxhash
#else
namespace turbo::xxhash {
    static XXHashEngine* get_xxhash_ppc64_instance() {
        return nullptr;
    }
} // namespace turbo::xxhash
#endif

namespace turbo::xxhash {
    IsaInfo get_xxhash_ppc64_info() {
        static IsaInfo ins = {
            KUMO_SIMD_VSX == 1,
            false,
            static_cast<uint32_t>(InstructionSet::ALTIVEC),
            "ppc64",
            get_xxhash_ppc64_instance(),
        };
        return ins;
    }
} // namespace turbo::xxhash
