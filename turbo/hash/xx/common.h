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

#include <cstdint>
#include <turbo/hash/xx/config.h>
#include <turbo/bits/bits.h>

namespace turbo::xxhash {
    typedef enum {
        XXH_aligned,  /*!< Aligned */
        XXH_unaligned /*!< Possibly unaligned */
    } XXH_alignment;


    KUMO_FORCE_INLINE uint64_t xxhash_scalar_avalanche64(uint64_t hash) {
        hash ^= hash >> 33;
        hash *= XXH_PRIME64_2;
        hash ^= hash >> 29;
        hash *= XXH_PRIME64_3;
        hash ^= hash >> 32;
        return hash;
    }

    /*! Seems to produce slightly better code on GCC for some reason. */
    KUMO_FORCE_INLINE uint64_t xxhash_scalar_xorshift64(uint64_t v64, int shift)
    {
        KUMO_DASSERT(0 <= shift && shift < 64);
        return v64 ^ (v64 >> shift);
    }

    KUMO_FORCE_INLINE uint64_t XXH3_avalanche(uint64_t h64)
    {
        h64 = xxhash_scalar_xorshift64(h64, 37);
        h64 *= turbo::xxhash::PRIME_MX1;
        h64 = xxhash_scalar_xorshift64(h64, 32);
        return h64;
    }

    KUMO_FORCE_INLINE uint64_t xxhash_scalar_rrmxmx(uint64_t h64, uint64_t len)
    {
        /* this mix is inspired by Pelle Evensen's rrmxmx */
        h64 ^= turbo::rotl(h64, 49) ^ turbo::rotl(h64, 24);
        h64 *= turbo::xxhash::PRIME_MX2;
        h64 ^= (h64 >> 35) + len ;
        h64 *= turbo::xxhash::PRIME_MX2;
        return xxhash_scalar_xorshift64(h64, 28);
    }

    typedef struct {
        uint64_t low64;   /*!< `value & 0xFFFFFFFFFFFFFFFF` */
        uint64_t high64;  /*!< `value >> 64` */
    } XXH128_hash_t;


    ///////////////////////////////////////////////////////////////////////
    /// @internal
    /// @def XXH3_kSecret
    /// @brief Pseudorandom secret taken directly from FARSH.
    ///
    alignas(64) static constexpr uint8_t XXH3_kSecret[XXH_SECRET_DEFAULT_SIZE] = {
        0xb8, 0xfe, 0x6c, 0x39, 0x23, 0xa4, 0x4b, 0xbe, 0x7c, 0x01, 0x81, 0x2c, 0xf7, 0x21, 0xad, 0x1c,
        0xde, 0xd4, 0x6d, 0xe9, 0x83, 0x90, 0x97, 0xdb, 0x72, 0x40, 0xa4, 0xa4, 0xb7, 0xb3, 0x67, 0x1f,
        0xcb, 0x79, 0xe6, 0x4e, 0xcc, 0xc0, 0xe5, 0x78, 0x82, 0x5a, 0xd0, 0x7d, 0xcc, 0xff, 0x72, 0x21,
        0xb8, 0x08, 0x46, 0x74, 0xf7, 0x43, 0x24, 0x8e, 0xe0, 0x35, 0x90, 0xe6, 0x81, 0x3a, 0x26, 0x4c,
        0x3c, 0x28, 0x52, 0xbb, 0x91, 0xc3, 0x00, 0xcb, 0x88, 0xd0, 0x65, 0x8b, 0x1b, 0x53, 0x2e, 0xa3,
        0x71, 0x64, 0x48, 0x97, 0xa2, 0x0d, 0xf9, 0x4e, 0x38, 0x19, 0xef, 0x46, 0xa9, 0xde, 0xac, 0xd8,
        0xa8, 0xfa, 0x76, 0x3f, 0xe3, 0x9c, 0x34, 0x3f, 0xf9, 0xdc, 0xbb, 0xc7, 0xc7, 0x0b, 0x4f, 0x1d,
        0x8a, 0x51, 0xe0, 0x4b, 0xcd, 0xb4, 0x59, 0x31, 0xc8, 0x9f, 0x7e, 0xc9, 0xd9, 0x78, 0x73, 0x64,
        0xea, 0xc5, 0xac, 0x83, 0x34, 0xd3, 0xeb, 0xc3, 0xc5, 0x81, 0xa0, 0xff, 0xfa, 0x13, 0x63, 0xeb,
        0x17, 0x0d, 0xdd, 0x51, 0xb7, 0xf0, 0xda, 0x49, 0xd3, 0x16, 0x55, 0x26, 0x29, 0xd4, 0x68, 0x9e,
        0x2b, 0x16, 0xbe, 0x58, 0x7d, 0x47, 0xa1, 0xfc, 0x8f, 0xf8, 0xb8, 0xd1, 0x7a, 0xd0, 0x31, 0xce,
        0x45, 0xcb, 0x3a, 0x8f, 0x95, 0x16, 0x04, 0x28, 0xaf, 0xd7, 0xfb, 0xca, 0xbb, 0x4b, 0x40, 0x7e,
    };

#if defined(_MSC_VER) && defined(_M_IX86)
#    define XXH_mult32to64(x, y) __emulu((unsigned)(x), (unsigned)(y))
#else

#    define XXH_mult32to64(x, y) ((uint64_t)(uint32_t)(x) * (uint64_t)(uint32_t)(y))
#endif


    KUMO_FORCE_INLINE uint64_t
    XXH_mult32to64_add64(uint64_t lhs, uint64_t rhs, uint64_t acc) {
#if defined(__aarch64__) && (defined(__GNUC__) || defined(__clang__))

        uint64_t ret;
        /// note: %x = 64-bit register, %w = 32-bit register
        __asm__("umaddl %x0, %w1, %w2, %x3" : "=r" (ret) : "r" (lhs), "r" (rhs), "r" (acc));
        return ret;
#else
        return XXH_mult32to64((uint32_t)lhs, (uint32_t)rhs) + acc;
#endif
    }

    KUMO_FORCE_INLINE void XXH3_combine16(void* dst, turbo::xxhash::XXH128_hash_t h128) {
        turbo::little_endian::Store64( dst, turbo::little_endian::Load64(dst) ^ h128.low64 );
        turbo::little_endian::Store64( (char*)dst+8, turbo::little_endian::Load64((char*)dst+8) ^ h128.high64 );
    }

    typedef enum {
        XXH_OK = 0, /*!< OK */
        XXH_ERROR   /*!< Error */
    } XXH_errorcode;

    struct XXH128_canonical_t{ unsigned char digest[sizeof(turbo::xxhash::XXH128_hash_t)]; };

}  // namespace turbo::xxhash


#define XXH3_INIT_ACC { XXH_PRIME32_3, XXH_PRIME64_1, XXH_PRIME64_2, XXH_PRIME64_3, \
    XXH_PRIME64_4, XXH_PRIME32_2, XXH_PRIME64_5, XXH_PRIME32_1 }
