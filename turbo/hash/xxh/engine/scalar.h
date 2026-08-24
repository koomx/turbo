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

#pragma once

#include <turbo/hash/xxh/config.h>
#include <turbo/hash/xxh/scalar.h>
#include <turbo/macros/macros.h>

namespace turbo {

    /// @internal
    /// @brief Scalar round for @ref XXH3_accumulate_512_scalar().
    ///
    /// This is extracted to its own function because the NEON path uses a combination
    /// of NEON and scalar.
    KUMO_FORCE_INLINE void
    XXH3_scalarRound(void* KUMO_RESTRICT acc,
        void const* KUMO_RESTRICT input,
        void const* KUMO_RESTRICT secret,
        size_t lane) {
        uint64_t* xacc = (uint64_t*)acc;
        uint8_t const* xinput = (uint8_t const*)input;
        uint8_t const* xsecret = (uint8_t const*)secret;
        KUMO_DASSERT(lane < XXH_ACC_NB);
        KUMO_DASSERT(((size_t)acc & (XXH_ACC_ALIGN - 1)) == 0);
        {
            uint64_t const data_val = turbo::little_endian::Load64(xinput + lane * 8);
            uint64_t const data_key = data_val ^ turbo::little_endian::Load64(xsecret + lane * 8);
            xacc[lane ^ 1] += data_val;
            xacc[lane] = XXH_mult32to64_add64(data_key, data_key >> 32, xacc[lane]);
        }
    }

    KUMO_FORCE_INLINE void XXH3_accumulate_512_scalar(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        size_t i;
        /// ARM GCC refuses to unroll this loop, resulting in a 24% slowdown on ARMv6.
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 8 \
    && (defined(__arm__) || defined(__thumb2__))              \
    && defined(__ARM_FEATURE_UNALIGNED)                       \
    && XXH_SIZE_OPT <= 0
#pragma GCC unroll 8
#endif
        for (i = 0; i < XXH_ACC_NB; i++) {
            XXH3_scalarRound(acc, input, secret, i);
        }
    }

    /// @internal
    /// @brief Scalar scramble step for @ref XXH3_scrambleAcc_scalar().
    ///
    /// This is extracted to its own function because the NEON path uses a combination
    /// of NEON and scalar.
    KUMO_FORCE_INLINE void XXH3_scalarScrambleRound(void* KUMO_RESTRICT acc,
        void const* KUMO_RESTRICT secret,
        size_t lane) {
        uint64_t* const xacc = (uint64_t*)acc; /* presumed aligned */
        const uint8_t* const xsecret = (const uint8_t*)secret; /* no alignment restriction */
        KUMO_DASSERT((((size_t)acc) & (XXH_ACC_ALIGN - 1)) == 0);
        KUMO_DASSERT(lane < XXH_ACC_NB);
        {
            uint64_t const key64 = turbo::little_endian::Load64(xsecret + lane * 8);
            uint64_t acc64 = xacc[lane];
            acc64 = xxh_xorshift64(acc64, 47);
            acc64 ^= key64;
            acc64 *= XXH_PRIME32_1;
            xacc[lane] = acc64;
        }
    }
    /// @internal
    /// @brief Scrambles the accumulators after a large chunk has been read
    KUMO_FORCE_INLINE void
    XXH3_scrambleAcc_scalar(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT secret) {
        size_t i;
        for (i = 0; i < XXH_ACC_NB; i++) {
            XXH3_scalarScrambleRound(acc, secret, i);
        }
    }

    KUMO_FORCE_INLINE void
    XXH3_initCustomSecret_scalar(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        /// We need a separate pointer for the hack below,
        /// which requires a non-const pointer.
        /// Any decent compiler will optimize this out otherwise.
        const uint8_t* kSecretPtr = XXH3_kSecret;
        static_assert((XXH_SECRET_DEFAULT_SIZE & 15) == 0, "(XXH_SECRET_DEFAULT_SIZE & 15) == 0");

#if defined(__GNUC__) && defined(__aarch64__)
        /// UGLY HACK:
        /// GCC and Clang generate a bunch of MOV/MOVK pairs for aarch64, and they are
        /// placed sequentially, in order, at the top of the unrolled loop.
        ///
        /// While MOVK is great for generating constants (2 cycles for a 64-bit
        /// constant compared to 4 cycles for LDR), it fights for bandwidth with
        /// the arithmetic instructions.
        ///
        ///   I   L   S
        /// MOVK
        /// MOVK
        /// MOVK
        /// MOVK
        /// ADD
        /// SUB      STR
        ///          STR
        /// By forcing loads from memory (as the asm line causes the compiler to assume
        /// that XXH3_kSecretPtr has been changed), the pipelines are used more
        /// efficiently:
        ///   I   L   S
        ///      LDR
        ///  ADD LDR
        ///  SUB     STR
        ///          STR
        ///
        /// See XXH3_NEON_LANES for details on the pipeline.
        ///
        /// XXH3_64bits_withSeed, len == 256, Snapdragon 835
        ///   without hack: 2654.4 MB/s
        ///   with hack:    3202.9 MB/s
        KUMO_CCO_BARRIER(kSecretPtr);
#endif
        {
            int const nbRounds = XXH_SECRET_DEFAULT_SIZE / 16;
            int i;
            for (i = 0; i < nbRounds; i++) {
                /*
                 * The asm hack causes the compiler to assume that kSecretPtr aliases with
                 * customSecret, and on aarch64, this prevented LDP from merging two
                 * loads together for free. Putting the loads together before the stores
                 * properly generates LDP.
                 */
                uint64_t lo = turbo::little_endian::Load64(kSecretPtr + 16 * i) + seed64;
                uint64_t hi = turbo::little_endian::Load64(kSecretPtr + 16 * i + 8) - seed64;
                turbo::little_endian::Store64((uint8_t*)customSecret + 16 * i, lo);
                turbo::little_endian::Store64((uint8_t*)customSecret + 16 * i + 8, hi);
            }
        }
    }

} // namespace turbo
