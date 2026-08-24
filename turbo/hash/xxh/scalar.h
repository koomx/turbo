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

#include <turbo/bits/bits.h>
#include <turbo/hash/xxh/config.h>

#if defined(_MSC_VER) && defined(_M_IX86)
#define XXH_mult32to64(x, y) __emulu((unsigned)(x), (unsigned)(y))
#else
/// Downcast + upcast is usually better than masking on older compilers like
/// GCC 4.2 (especially 32-bit ones), all without affecting newer compilers.
///
/// The other method, (x & 0xFFFFFFFF) * (y & 0xFFFFFFFF), will AND both operands
/// and perform a full 64x64 multiply -- entirely redundant on 32-bit.
#define XXH_mult32to64(x, y) ((uint64_t)(uint32_t)(x) * (uint64_t)(uint32_t)(y))
#endif

namespace turbo {

    ////////////////////////////////////////////////////////////////////////////
    /// 64-bit variant
    ////////////////////////////////////////////////////////////////////////////
    inline uint64_t xxh64_avalanche(uint64_t hash) {
        hash ^= hash >> 33;
        hash *= XXH_PRIME64_2;
        hash ^= hash >> 29;
        hash *= XXH_PRIME64_3;
        hash ^= hash >> 32;
        return hash;
    }

    /// @brief Calculates a 64->128-bit long multiply.
    ///
    /// Uses `__uint128_t` and `_umul128` if available, otherwise uses a scalar
    /// version.
    ///
    /// @param lhs , rhs The 64-bit integers to be multiplied
    /// @return The 128-bit result represented in an @ref XXH128_hash_t.
    static XXH128_hash_t XXH_mult64to128(uint64_t lhs, uint64_t rhs) {
        /// GCC/Clang __uint128_t method.
        ///
        /// On most 64-bit targets, GCC and Clang define a __uint128_t type.
        /// This is usually the best way as it usually uses a native long 64-bit
        /// multiply, such as MULQ on x86_64 or MUL + UMULH on aarch64.
        ///
        /// Usually.
        ///
        /// Despite being a 32-bit platform, Clang (and emscripten) define this type
        /// despite not having the arithmetic for it. This results in a laggy
        /// compiler builtin call which calculates a full 128-bit multiply.
        /// In that case it is best to use the portable one.
        /// https://github.com/Cyan4973/xxHash/issues/211#issuecomment-515575677
#if (defined(__GNUC__) || defined(__clang__)) && !defined(__wasm__) \
        && defined(__SIZEOF_INT128__)                               \
    || (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128)

        __uint128_t const product = (__uint128_t)lhs * (__uint128_t)rhs;
        XXH128_hash_t r128;
        r128.low64 = (uint64_t)(product);
        r128.high64 = (uint64_t)(product >> 64);
        return r128;

        /// MSVC for x64's _umul128 method.
        ///
        /// uint64_t _umul128(uint64_t Multiplier, uint64_t Multiplicand, uint64_t *HighProduct);
        ///
        /// This compiles to single operand MUL on x64.
#elif (defined(_M_X64) || defined(_M_IA64)) && !defined(_M_ARM64EC)

#ifndef _MSC_VER
#pragma intrinsic(_umul128)
#endif
        uint64_t product_high;
        uint64_t const product_low = _umul128(lhs, rhs, &product_high);
        XXH128_hash_t r128;
        r128.low64 = product_low;
        r128.high64 = product_high;
        return r128;

        /// MSVC for ARM64's __umulh method.
        ///
        /// This compiles to the same MUL + UMULH as GCC/Clang's __uint128_t method.
#elif defined(_M_ARM64) || defined(_M_ARM64EC)

#ifndef _MSC_VER
#pragma intrinsic(__umulh)
#endif
        XXH128_hash_t r128;
        r128.low64 = lhs * rhs;
        r128.high64 = __umulh(lhs, rhs);
        return r128;

#else
        /// Portable scalar method. Optimized for 32-bit and 64-bit ALUs.
        ///
        /// This is a fast and simple grade school multiply, which is shown below
        /// with base 10 arithmetic instead of base 0x100000000.
        ///
        ///           9 3 // D2 lhs = 93
        ///         x 7 5 // D2 rhs = 75
        ///     ----------
        ///           1 5 // D2 lo_lo = (93 % 10) * (75 % 10) = 15
        ///         4 5 | // D2 hi_lo = (93 / 10) * (75 % 10) = 45
        ///         2 1 | // D2 lo_hi = (93 % 10) * (75 / 10) = 21
        ///     + 6 3 | | // D2 hi_hi = (93 / 10) * (75 / 10) = 63
        ///     ---------
        ///         2 7 | // D2 cross = (15 / 10) + (45 % 10) + 21 = 27
        ///     + 6 7 | | // D2 upper = (27 / 10) + (45 / 10) + 63 = 67
        ///     ---------
        ///       6 9 7 5 // D4 res = (27 * 10) + (15 % 10) + (67 * 100) = 6975
        ///
        /// The reasons for adding the products like this are:
        ///  1. It avoids manual carry tracking. Just like how
        ///     (9 * 9) + 9 + 9 = 99, the same applies with this for UINT64_MAX.
        ///     This avoids a lot of complexity.
        ///
        ///  2. It hints for, and on Clang, compiles to, the powerful UMAAL
        ///     instruction available in ARM's Digital Signal Processing extension
        ///     in 32-bit ARMv6 and later, which is shown below:
        ///
        ///         void UMAAL(uint32_t *RdLo, uint32_t *RdHi, uint32_t Rn, uint32_t Rm)
        ///         {
        ///             uint64_t product = (uint64_t)*RdLo * (uint64_t)*RdHi + Rn + Rm;
        ///             *RdLo = (uint32_t)(product & 0xFFFFFFFF);
        ///             *RdHi = (uint32_t)(product >> 32);
        ///         }
        ///
        ///     This instruction was designed for efficient long multiplication, and
        ///     allows this to be calculated in only 4 instructions at speeds
        ///     comparable to some 64-bit ALUs.
        ///
        ///  3. It isn't terrible on other platforms. Usually this will be a couple
        ///     of 32-bit ADD/ADCs.
        /// First calculate all of the cross products.
        uint64_t const lo_lo = XXH_mult32to64(lhs & 0xFFFFFFFF, rhs & 0xFFFFFFFF);
        uint64_t const hi_lo = XXH_mult32to64(lhs >> 32, rhs & 0xFFFFFFFF);
        uint64_t const lo_hi = XXH_mult32to64(lhs & 0xFFFFFFFF, rhs >> 32);
        uint64_t const hi_hi = XXH_mult32to64(lhs >> 32, rhs >> 32);

        /// Now add the products together. These will never overflow.
        uint64_t const cross = (lo_lo >> 32) + (hi_lo & 0xFFFFFFFF) + lo_hi;
        uint64_t const upper = (hi_lo >> 32) + (cross >> 32) + hi_hi;
        uint64_t const lower = (cross << 32) | (lo_lo & 0xFFFFFFFF);

        XXH128_hash_t r128;
        r128.low64 = lower;
        r128.high64 = upper;
        return r128;
#endif
    }

    /// @brief Calculates a 64-bit to 128-bit multiply, then XOR folds it.
    ///
    /// The reason for the separate function is to prevent passing too many structs
    /// around by value. This will hopefully inline the multiply, but we don't force it.
    ///
    /// @param lhs , rhs The 64-bit integers to multiply
    /// @return The low 64 bits of the product XOR'd by the high 64 bits.
    /// @see XXH_mult64to128()
    static uint64_t
    xxh3_mul128_fold64(uint64_t lhs, uint64_t rhs) {
        XXH128_hash_t product = XXH_mult64to128(lhs, rhs);
        return product.low64 ^ product.high64;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// xxh3
    ////////////////////////////////////////////////////////////////////////////
    /// Seems to produce slightly better code on GCC for some reason.
    KUMO_ATTRIBUTE_CONST_FUNCTION KUMO_FORCE_INLINE uint64_t xxh_xorshift64(uint64_t v64, int shift) {
        KUMO_DASSERT(0 <= shift && shift < 64);
        return v64 ^ (v64 >> shift);
    }

    /// This is a fast avalanche stage,
    /// suitable when input bits are already partially mixed
    inline uint64_t xxh3_avalanche(uint64_t h64) {
        h64 = xxh_xorshift64(h64, 37);
        h64 *= PRIME_MX1;
        h64 = xxh_xorshift64(h64, 32);
        return h64;
    }

    /// ==========================================
    /// Short keys
    /// ==========================================
    /// One of the shortcomings of XXH32 and XXH64 was that their performance was
    /// sub-optimal on short lengths. It used an iterative algorithm which strongly
    /// favored lengths that were a multiple of 4 or 8.
    ///
    /// Instead of iterating over individual inputs, we use a set of single shot
    /// functions which piece together a range of lengths and operate in constant time.
    ///
    /// Additionally, the number of multiplies has been significantly reduced. This
    /// reduces latency, especially when emulating 64-bit multiplies on 32-bit.
    ///
    /// Depending on the platform, this may or may not be faster than XXH32, but it
    /// is almost guaranteed to be faster than XXH64.

    /// At very short lengths, there isn't enough input to fully hide secrets, or use
    /// the entire secret.
    ///
    /// There is also only a limited amount of mixing we can do before significantly
    /// impacting performance.
    ///
    /// Therefore, we use different sections of the secret and always mix two secret
    /// samples with an XOR. This should have no effect on performance on the
    /// seedless or withSeed variants because everything _should_ be constant folded
    /// by modern compilers.
    ///
    /// The XOR mixing hides individual parts of the secret and increases entropy.
    ///
    /// This adds an extra layer of strength for custom secrets.

    KUMO_ATTRIBUTE_PURE_FUNCTION KUMO_FORCE_INLINE uint64_t
    xxh3_len_1to3_64b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed) {
        KUMO_DASSERT(input != NULL);
        KUMO_DASSERT(1 <= len && len <= 3);
        KUMO_DASSERT(secret != NULL);
        /// len = 1: combined = { input[0], 0x01, input[0], input[0] }
        /// len = 2: combined = { input[1], 0x02, input[0], input[1] }
        /// len = 3: combined = { input[2], 0x03, input[0], input[1] }
        {
            uint8_t const c1 = input[0];
            uint8_t const c2 = input[len >> 1];
            uint8_t const c3 = input[len - 1];
            uint32_t const combined = ((uint32_t)c1 << 16) | ((uint32_t)c2 << 24)
                | ((uint32_t)c3 << 0) | ((uint32_t)len << 8);
            uint64_t const bitflip = (turbo::little_endian::Load32(secret) ^ turbo::little_endian::Load32(secret + 4)) + seed;
            uint64_t const keyed = (uint64_t)combined ^ bitflip;
            return xxh64_avalanche(keyed);
        }
    }

    /// This is a stronger avalanche,
    /// inspired by Pelle Evensen's rrmxmx
    /// preferable when input has not been previously mixed
    inline uint64_t xxh3_rrmxmx(uint64_t h64, uint64_t len) {
        /// this mix is inspired by Pelle Evensen's rrmxmx
        h64 ^= turbo::rotl(h64, 49) ^ turbo::rotl(h64, 24);
        h64 *= PRIME_MX2;
        h64 ^= (h64 >> 35) + len;
        h64 *= PRIME_MX2;
        return xxh_xorshift64(h64, 28);
    }

    /// DISCLAIMER: There are known *seed-dependent* multicollisions here due to
    /// multiplication by zero, affecting hashes of lengths 17 to 240.
    ///
    /// However, they are very unlikely.
    ///
    /// Keep this in mind when using the unseeded XXH3_64bits() variant: As with all
    /// unseeded non-cryptographic hashes, it does not attempt to defend itself
    /// against specially crafted inputs, only random inputs.
    ///
    /// Compared to classic UMAC where a 1 in 2^31 chance of 4 consecutive bytes
    /// cancelling out the secret is taken an arbitrary number of times (addressed
    /// in XXH3_accumulate_512), this collision is very unlikely with random inputs
    /// and/or proper seeding:
    ///
    /// This only has a 1 in 2^63 chance of 8 consecutive bytes cancelling out, in a
    /// function that is only called up to 16 times per hash with up to 240 bytes of
    /// input.
    ///
    /// This is not too bad for a non-cryptographic hash function, especially with
    /// only 64 bit outputs.
    ///
    /// The 128-bit variant (which trades some speed for strength) is NOT affected
    /// by this, although it is always a good idea to use a proper seed if you care
    /// about strength.
    KUMO_FORCE_INLINE uint64_t xxh3_mix16B(const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret, uint64_t seed64) {
#if defined(__GNUC__) && !defined(__clang__)  \
    && defined(__i386__) && defined(__SSE2__) \
    && !defined(XXH_ENABLE_AUTOVECTORIZE) /// Define to disable like XXH32 hack
        /// UGLY HACK:
        /// GCC for x86 tends to autovectorize the 128-bit multiply, resulting in
        /// slower code.
        ///
        /// By forcing seed64 into a register, we disrupt the cost model and
        /// cause it to scalarize. See `XXH32_round()`
        ///
        /// FIXME: Clang's output is still _much_ faster -- On an AMD Ryzen 3600,
        /// XXH3_64bits @ len=240 runs at 4.6 GB/s with Clang 9, but 3.3 GB/s on
        /// GCC 9.2, despite both emitting scalar code.
        ///
        /// GCC generates much better scalar code than Clang for the rest of XXH3,
        /// which is why finding a more optimal codepath is an interest.
        XXH_COMPILER_GUARD(seed64);
#endif
        {
            uint64_t const input_lo = turbo::little_endian::Load64(input);
            uint64_t const input_hi = turbo::little_endian::Load64(input + 8);
            return xxh3_mul128_fold64(
                input_lo ^ (turbo::little_endian::Load64(secret) + seed64),
                input_hi ^ (turbo::little_endian::Load64(secret + 8) - seed64));
        }
    }

    /// ==========================================
    /// XXH3 128 bits (a.k.a XXH128)
    /// ==========================================
    /// XXH3's 128-bit variant has better mixing and strength than the 64-bit variant,
    /// even without counting the significantly larger output size.
    ///
    /// For example, extra steps are taken to avoid the seed-dependent collisions
    /// in 17-240 byte inputs (See xxh3_mix16B and xxh128_mix32B).
    ///
    /// This strength naturally comes at the cost of some speed, especially on short
    /// lengths. Note that longer hashes are about as fast as the 64-bit version
    /// due to it using only a slight modification of the 64-bit loop.
    ///
    /// XXH128 is also more oriented towards 64-bit machines. It is still extremely
    /// fast for a _128-bit_ hash on 32-bit (it usually clears XXH64).
    KUMO_ATTRIBUTE_PURE_FUNCTION KUMO_FORCE_INLINE uint64_t
    xxh3_len_4to8_64b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed) {
        KUMO_DASSERT(input != NULL);
        KUMO_DASSERT(secret != NULL);
        KUMO_DASSERT(4 <= len && len <= 8);
        seed ^= (uint64_t)turbo::byteswap((uint32_t)seed) << 32;
        {
            uint32_t const input1 = turbo::little_endian::Load32(input);
            uint32_t const input2 = turbo::little_endian::Load32(input + len - 4);
            uint64_t const bitflip = (turbo::little_endian::Load64(secret + 8) ^ turbo::little_endian::Load64(secret + 16)) - seed;
            uint64_t const input64 = input2 + (((uint64_t)input1) << 32);
            uint64_t const keyed = input64 ^ bitflip;
            return xxh3_rrmxmx(keyed, len);
        }
    }

    KUMO_ATTRIBUTE_PURE_FUNCTION KUMO_ATTRIBUTE_NOINLINE uint64_t
    xxh3_len_129to240_64b(const uint8_t* KUMO_RESTRICT input, size_t len,
        const uint8_t* KUMO_RESTRICT secret, size_t secretSize,
        uint64_t seed) {
        KUMO_DASSERT(secretSize >= XXH3_SECRET_SIZE_MIN);
        (void)secretSize;
        KUMO_DASSERT(128 < len && len <= XXH3_MIDSIZE_MAX);

        {
            uint64_t acc = len * XXH_PRIME64_1;
            uint64_t acc_end;
            unsigned int const nbRounds = (unsigned int)len / 16;
            unsigned int i;
            KUMO_DASSERT(128 < len && len <= XXH3_MIDSIZE_MAX);
            for (i = 0; i < 8; i++) {
                acc += xxh3_mix16B(input + (16 * i), secret + (16 * i), seed);
            }
            /// last bytes
            acc_end = xxh3_mix16B(input + len - 16, secret + XXH3_SECRET_SIZE_MIN - XXH3_MIDSIZE_LASTOFFSET, seed);
            KUMO_DASSERT(nbRounds >= 8);
            acc = xxh3_avalanche(acc);
#if defined(__clang__)                                \
    && (defined(__ARM_NEON) || defined(__ARM_NEON__)) \
    && !defined(XXH_ENABLE_AUTOVECTORIZE)
            /// UGLY HACK:
            /// Clang for ARMv7-A tries to vectorize this loop, similar to GCC x86.
            /// In everywhere else, it uses scalar code.
            ///
            /// For 64->128-bit multiplies, even if the NEON was 100% optimal, it
            /// would still be slower than UMAAL (see XXH_mult64to128).
            ///
            /// Unfortunately, Clang doesn't handle the long multiplies properly and
            /// converts them to the nonexistent "vmulq_u64" intrinsic, which is then
            /// scalarized into an ugly mess of VMOV.32 instructions.
            ///
            /// This mess is difficult to avoid without turning autovectorization
            /// off completely, but they are usually relatively minor and/or not
            /// worth it to fix.
            ///
            /// This loop is the easiest to fix, as unlike XXH32, this pragma
            /// _actually works_ because it is a loop vectorization instead of an
            /// SLP vectorization.
#pragma clang loop vectorize(disable)
#endif
            for (i = 8; i < nbRounds; i++) {
                /// Prevents clang for unrolling the acc loop and interleaving with this one.
                KUMO_CCO_BARRIER(acc);
                acc_end += xxh3_mix16B(input + (16 * i), secret + (16 * (i - 8)) + XXH3_MIDSIZE_STARTOFFSET, seed);
            }
            return xxh3_avalanche(acc + acc_end);
        }
    }

    KUMO_ATTRIBUTE_PURE_FUNCTION KUMO_FORCE_INLINE uint64_t
    xxh3_len_9to16_64b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed) {
        KUMO_DASSERT(input != NULL);
        KUMO_DASSERT(secret != NULL);
        KUMO_DASSERT(9 <= len && len <= 16);
        {
            uint64_t const bitflip1 = (turbo::little_endian::Load64(secret + 24) ^ turbo::little_endian::Load64(secret + 32)) + seed;
            uint64_t const bitflip2 = (turbo::little_endian::Load64(secret + 40) ^ turbo::little_endian::Load64(secret + 48)) - seed;
            uint64_t const input_lo = turbo::little_endian::Load64(input) ^ bitflip1;
            uint64_t const input_hi = turbo::little_endian::Load64(input + len - 8) ^ bitflip2;
            uint64_t const acc = len
                + turbo::byteswap(input_lo) + input_hi
                + xxh3_mul128_fold64(input_lo, input_hi);
            return xxh3_avalanche(acc);
        }
    }

    KUMO_ATTRIBUTE_PURE_FUNCTION KUMO_FORCE_INLINE uint64_t xxh3_len_0to16_64b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed) {
        KUMO_DASSERT(len <= 16);
        {
            if (KUMO_LIKELY(len > 8))
                return xxh3_len_9to16_64b(input, len, secret, seed);
            if (KUMO_LIKELY(len >= 4))
                return xxh3_len_4to8_64b(input, len, secret, seed);
            if (len)
                return xxh3_len_1to3_64b(input, len, secret, seed);
            return xxh64_avalanche(seed ^ (turbo::little_endian::Load64(secret + 56) ^ turbo::little_endian::Load64(secret + 64)));
        }
    }

    /// For mid range keys, XXH3 uses a Mum-hash variant.
    KUMO_ATTRIBUTE_PURE_FUNCTION KUMO_FORCE_INLINE uint64_t xxh3_len_17to128_64b(const uint8_t* KUMO_RESTRICT input, size_t len,
        const uint8_t* KUMO_RESTRICT secret, size_t secretSize,
        uint64_t seed) {
        KUMO_DASSERT(secretSize >= XXH3_SECRET_SIZE_MIN);
        (void)secretSize;
        KUMO_DASSERT(16 < len && len <= 128);

        {
            uint64_t acc = len * XXH_PRIME64_1;
#if XXH_SIZE_OPT >= 1
            /// Smaller and cleaner, but slightly slower.
            unsigned int i = (unsigned int)(len - 1) / 32;
            do {
                acc += xxh3_mix16B(input + 16 * i, secret + 32 * i, seed);
                acc += xxh3_mix16B(input + len - 16 * (i + 1), secret + 32 * i + 16, seed);
            } while (i-- != 0);
#else
            if (len > 32) {
                if (len > 64) {
                    if (len > 96) {
                        acc += xxh3_mix16B(input + 48, secret + 96, seed);
                        acc += xxh3_mix16B(input + len - 64, secret + 112, seed);
                    }
                    acc += xxh3_mix16B(input + 32, secret + 64, seed);
                    acc += xxh3_mix16B(input + len - 48, secret + 80, seed);
                }
                acc += xxh3_mix16B(input + 16, secret + 32, seed);
                acc += xxh3_mix16B(input + len - 32, secret + 48, seed);
            }
            acc += xxh3_mix16B(input + 0, secret + 0, seed);
            acc += xxh3_mix16B(input + len - 16, secret + 16, seed);
#endif
            return xxh3_avalanche(acc);
        }
    }

    KUMO_FORCE_INLINE XXH128_hash_t xxh128_mix32B(XXH128_hash_t acc, const uint8_t* input_1, const uint8_t* input_2,
        const uint8_t* secret, uint64_t seed) {
        acc.low64 += xxh3_mix16B(input_1, secret + 0, seed);
        acc.low64 ^= turbo::little_endian::Load64(input_2) + turbo::little_endian::Load64(input_2 + 8);
        acc.high64 += xxh3_mix16B(input_2, secret + 16, seed);
        acc.high64 ^= turbo::little_endian::Load64(input_1) + turbo::little_endian::Load64(input_1 + 8);
        return acc;
    }

    KUMO_ATTRIBUTE_PURE_FUNCTION KUMO_FORCE_INLINE XXH128_hash_t
    xxh3_len_1to3_128b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed) {
        /// A doubled version of 1to3_64b with different constants.
        KUMO_DASSERT(input != NULL);
        KUMO_DASSERT(1 <= len && len <= 3);
        KUMO_DASSERT(secret != NULL);
        /// len = 1: combinedl = { input[0], 0x01, input[0], input[0] }
        /// len = 2: combinedl = { input[1], 0x02, input[0], input[1] }
        /// len = 3: combinedl = { input[2], 0x03, input[0], input[1] }
        {
            uint8_t const c1 = input[0];
            uint8_t const c2 = input[len >> 1];
            uint8_t const c3 = input[len - 1];
            uint32_t const combinedl = ((uint32_t)c1 << 16) | ((uint32_t)c2 << 24)
                | ((uint32_t)c3 << 0) | ((uint32_t)len << 8);
            uint32_t const combinedh = turbo::rotl(turbo::byteswap(combinedl), 13);
            uint64_t const bitflipl = (turbo::little_endian::Load32(secret) ^ turbo::little_endian::Load32(secret + 4)) + seed;
            uint64_t const bitfliph = (turbo::little_endian::Load32(secret + 8) ^ turbo::little_endian::Load32(secret + 12)) - seed;
            uint64_t const keyed_lo = (uint64_t)combinedl ^ bitflipl;
            uint64_t const keyed_hi = (uint64_t)combinedh ^ bitfliph;
            XXH128_hash_t h128;
            h128.low64 = xxh64_avalanche(keyed_lo);
            h128.high64 = xxh64_avalanche(keyed_hi);
            return h128;
        }
    }

    KUMO_ATTRIBUTE_PURE_FUNCTION KUMO_FORCE_INLINE XXH128_hash_t xxh3_len_17to128_128b(const uint8_t* KUMO_RESTRICT input, size_t len,
        const uint8_t* KUMO_RESTRICT secret, size_t secretSize,
        uint64_t seed) {
        KUMO_DASSERT(secretSize >= XXH3_SECRET_SIZE_MIN);
        (void)secretSize;
        KUMO_DASSERT(16 < len && len <= 128);

        {
            XXH128_hash_t acc;
            acc.low64 = len * XXH_PRIME64_1;
            acc.high64 = 0;

#if XXH_SIZE_OPT >= 1
            {
                /// Smaller, but slightly slower.
                unsigned int i = (unsigned int)(len - 1) / 32;
                do {
                    acc = xxh128_mix32B(acc, input + 16 * i, input + len - 16 * (i + 1), secret + 32 * i, seed);
                } while (i-- != 0);
            }
#else
            if (len > 32) {
                if (len > 64) {
                    if (len > 96) {
                        acc = xxh128_mix32B(acc, input + 48, input + len - 64, secret + 96, seed);
                    }
                    acc = xxh128_mix32B(acc, input + 32, input + len - 48, secret + 64, seed);
                }
                acc = xxh128_mix32B(acc, input + 16, input + len - 32, secret + 32, seed);
            }
            acc = xxh128_mix32B(acc, input, input + len - 16, secret, seed);
#endif
            {
                XXH128_hash_t h128;
                h128.low64 = acc.low64 + acc.high64;
                h128.high64 = (acc.low64 * XXH_PRIME64_1)
                    + (acc.high64 * XXH_PRIME64_4)
                    + ((len - seed) * XXH_PRIME64_2);
                h128.low64 = xxh3_avalanche(h128.low64);
                h128.high64 = (uint64_t)0 - xxh3_avalanche(h128.high64);
                return h128;
            }
        }
    }

    KUMO_ATTRIBUTE_PURE_FUNCTION KUMO_FORCE_INLINE XXH128_hash_t
    xxh3_len_4to8_128b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed) {
        KUMO_DASSERT(input != NULL);
        KUMO_DASSERT(secret != NULL);
        KUMO_DASSERT(4 <= len && len <= 8);
        seed ^= (uint64_t)turbo::byteswap((uint32_t)seed) << 32;
        {
            uint32_t const input_lo = turbo::little_endian::Load32(input);
            uint32_t const input_hi = turbo::little_endian::Load32(input + len - 4);
            uint64_t const input_64 = input_lo + ((uint64_t)input_hi << 32);
            uint64_t const bitflip = (turbo::little_endian::Load64(secret + 16) ^ turbo::little_endian::Load64(secret + 24)) + seed;
            uint64_t const keyed = input_64 ^ bitflip;

            /// Shift len to the left to ensure it is even, this avoids even multiplies.
            XXH128_hash_t m128 = XXH_mult64to128(keyed, XXH_PRIME64_1 + (len << 2));

            m128.high64 += (m128.low64 << 1);
            m128.low64 ^= (m128.high64 >> 3);

            m128.low64 = xxh_xorshift64(m128.low64, 35);
            m128.low64 *= PRIME_MX2;
            m128.low64 = xxh_xorshift64(m128.low64, 28);
            m128.high64 = xxh3_avalanche(m128.high64);
            return m128;
        }
    }

    KUMO_ATTRIBUTE_PURE_FUNCTION KUMO_FORCE_INLINE XXH128_hash_t
    xxh3_len_9to16_128b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed) {
        KUMO_DASSERT(input != NULL);
        KUMO_DASSERT(secret != NULL);
        KUMO_DASSERT(9 <= len && len <= 16);
        {
            uint64_t const bitflipl = (turbo::little_endian::Load64(secret + 32) ^ turbo::little_endian::Load64(secret + 40)) - seed;
            uint64_t const bitfliph = (turbo::little_endian::Load64(secret + 48) ^ turbo::little_endian::Load64(secret + 56)) + seed;
            uint64_t const input_lo = turbo::little_endian::Load64(input);
            uint64_t input_hi = turbo::little_endian::Load64(input + len - 8);
            XXH128_hash_t m128 = XXH_mult64to128(input_lo ^ input_hi ^ bitflipl, XXH_PRIME64_1);
            /// Put len in the middle of m128 to ensure that the length gets mixed to
            /// both the low and high bits in the 128x64 multiply below.
            m128.low64 += (uint64_t)(len - 1) << 54;
            input_hi ^= bitfliph;
            /// Add the high 32 bits of input_hi to the high 32 bits of m128, then
            /// add the long product of the low 32 bits of input_hi and XXH_PRIME32_2 to
            /// the high 64 bits of m128.
            ///
            /// The best approach to this operation is different on 32-bit and 64-bit.
            if (sizeof(void*) < sizeof(uint64_t)) {
                /// 32-bit optimized version, which is more readable.
                ///
                /// On 32-bit, it removes an ADC and delays a dependency between the two
                /// halves of m128.high64, but it generates an extra mask on 64-bit.
                m128.high64 += (input_hi & 0xFFFFFFFF00000000ULL) + XXH_mult32to64((uint32_t)input_hi, XXH_PRIME32_2);
            } else {
                /// 64-bit optimized (albeit more confusing) version.
                ///
                /// Uses some properties of addition and multiplication to remove the mask:
                ///
                /// Let:
                ///    a = input_hi.lo = (input_hi & 0x00000000FFFFFFFF)
                ///    b = input_hi.hi = (input_hi & 0xFFFFFFFF00000000)
                ///    c = XXH_PRIME32_2
                ///
                ///    a + (b * c)
                /// Inverse Property: x + y - x == y
                ///    a + (b * (1 + c - 1))
                /// Distributive Property: x * (y + z) == (x * y) + (x * z)
                ///    a + (b * 1) + (b * (c - 1))
                /// Identity Property: x * 1 == x
                ///    a + b + (b * (c - 1))
                ///
                /// Substitute a, b, and c:
                ///    input_hi.hi + input_hi.lo + ((uint64_t)input_hi.lo * (XXH_PRIME32_2 - 1))
                ///
                /// Since input_hi.hi + input_hi.lo == input_hi, we get this:
                ///    input_hi + ((uint64_t)input_hi.lo * (XXH_PRIME32_2 - 1))
                m128.high64 += input_hi + XXH_mult32to64((uint32_t)input_hi, XXH_PRIME32_2 - 1);
            }
            /// m128 ^= turbo::byteswap(m128 >> 64);
            m128.low64 ^= turbo::byteswap(m128.high64);

            {
                /// 128x64 multiply: h128 = m128 * XXH_PRIME64_2;
                XXH128_hash_t h128 = XXH_mult64to128(m128.low64, XXH_PRIME64_2);
                h128.high64 += m128.high64 * XXH_PRIME64_2;

                h128.low64 = xxh3_avalanche(h128.low64);
                h128.high64 = xxh3_avalanche(h128.high64);
                return h128;
            }
        }
    }

    KUMO_ATTRIBUTE_PURE_FUNCTION KUMO_FORCE_INLINE XXH128_hash_t xxh3_len_0to16_128b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed) {
        KUMO_DASSERT(len <= 16);
        {
            if (len > 8)
                return xxh3_len_9to16_128b(input, len, secret, seed);
            if (len >= 4)
                return xxh3_len_4to8_128b(input, len, secret, seed);
            if (len)
                return xxh3_len_1to3_128b(input, len, secret, seed);
            {
                XXH128_hash_t h128;
                uint64_t const bitflipl = turbo::little_endian::Load64(secret + 64) ^ turbo::little_endian::Load64(secret + 72);
                uint64_t const bitfliph = turbo::little_endian::Load64(secret + 80) ^ turbo::little_endian::Load64(secret + 88);
                h128.low64 = xxh64_avalanche(seed ^ bitflipl);
                h128.high64 = xxh64_avalanche(seed ^ bitfliph);
                return h128;
            }
        }
    }

    KUMO_ATTRIBUTE_PURE_FUNCTION KUMO_ATTRIBUTE_NOINLINE XXH128_hash_t
    xxh3_len_129to240_128b(const uint8_t* KUMO_RESTRICT input, size_t len,
        const uint8_t* KUMO_RESTRICT secret, size_t secretSize,
        uint64_t seed) {
        KUMO_DASSERT(secretSize >= XXH3_SECRET_SIZE_MIN);
        (void)secretSize;
        KUMO_DASSERT(128 < len && len <= XXH3_MIDSIZE_MAX);

        {
            XXH128_hash_t acc;
            unsigned i;
            acc.low64 = len * XXH_PRIME64_1;
            acc.high64 = 0;
            /// We set as `i` as offset + 32. We do this so that unchanged
            /// `len` can be used as upper bound. This reaches a sweet spot
            /// where both x86 and aarch64 get simple agen and good codegen
            /// for the loop.
            for (i = 32; i < 160; i += 32) {
                acc = xxh128_mix32B(acc,
                    input + i - 32,
                    input + i - 16,
                    secret + i - 32,
                    seed);
            }
            acc.low64 = xxh3_avalanche(acc.low64);
            acc.high64 = xxh3_avalanche(acc.high64);
            /// NB: `i <= len` will duplicate the last 32-bytes if
            /// len % 32 was zero. This is an unfortunate necessity to keep
            /// the hash result stable.
            for (i = 160; i <= len; i += 32) {
                acc = xxh128_mix32B(acc,
                    input + i - 32,
                    input + i - 16,
                    secret + XXH3_MIDSIZE_STARTOFFSET + i - 160,
                    seed);
            }
            /// last bytes
            acc = xxh128_mix32B(acc,
                input + len - 16,
                input + len - 32,
                secret + XXH3_SECRET_SIZE_MIN - XXH3_MIDSIZE_LASTOFFSET - 16,
                (uint64_t)0 - seed);

            {
                XXH128_hash_t h128;
                h128.low64 = acc.low64 + acc.high64;
                h128.high64 = (acc.low64 * XXH_PRIME64_1)
                    + (acc.high64 * XXH_PRIME64_4)
                    + ((len - seed) * XXH_PRIME64_2);
                h128.low64 = xxh3_avalanche(h128.low64);
                h128.high64 = (uint64_t)0 - xxh3_avalanche(h128.high64);
                return h128;
            }
        }
    }

#if defined(__aarch64__) && (defined(__GNUC__) || defined(__clang__))
    /// In XXH3_scalarRound(), GCC and Clang have a similar codegen issue, where they
    /// emit an excess mask and a full 64-bit multiply-add (MADD X-form).
    ///
    /// While this might not seem like much, as AArch64 is a 64-bit architecture, only
    /// big Cortex designs have a full 64-bit multiplier.
    ///
    /// On the little cores, the smaller 32-bit multiplier is used, and full 64-bit
    /// multiplies expand to 2-3 multiplies in microcode. This has a major penalty
    /// of up to 4 latency cycles and 2 stall cycles in the multiply pipeline.
    ///
    /// Thankfully, AArch64 still provides the 32-bit long multiply-add (UMADDL) which does
    /// not have this penalty and does the mask automatically.
    KUMO_FORCE_INLINE uint64_t
    XXH_mult32to64_add64(uint64_t lhs, uint64_t rhs, uint64_t acc) {
        uint64_t ret;
        /* note: %x = 64-bit register, %w = 32-bit register */
        __asm__("umaddl %x0, %w1, %w2, %x3" : "=r"(ret) : "r"(lhs), "r"(rhs), "r"(acc));
        return ret;
    }
#else
    KUMO_FORCE_INLINE uint64_t
    XXH_mult32to64_add64(uint64_t lhs, uint64_t rhs, uint64_t acc) {
        return XXH_mult32to64((uint32_t)lhs, (uint32_t)rhs) + acc;
    }
#endif

} // namespace turbo
