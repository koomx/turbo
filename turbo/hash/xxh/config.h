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

#include <turbo/macros/macros.h>
#include <cassert>

#if defined(__GNUC__) || defined(__clang__)
#if defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
#endif
#if defined(__ARM_NEON__) || defined(__ARM_NEON) \
    || (defined(_M_ARM) && _M_ARM >= 7)          \
    || defined(_M_ARM64) || defined(_M_ARM64EC)  \
    || (defined(__wasm_simd128__) && KUMO_HAS_INCLUDE(<arm_neon.h>)) /* WASM SIMD128 via SIMDe */
#define inline __inline__ /* circumvent a clang bug */
#include <arm_neon.h>
#undef inline
#elif defined(__AVX2__)
#include <immintrin.h>
#elif defined(__SSE2__)
#include <emmintrin.h>
#elif defined(__loongarch_asx)
#include <lasxintrin.h>
#include <lsxintrin.h>
#elif defined(__loongarch_sx)
#include <lsxintrin.h>
#elif defined(__riscv_vector)
#include <riscv_vector.h>
#endif
#endif

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#if KUMO_ARCH_PPC64
#if defined(__s390x__)
#include <s390intrin.h>
#else
#include <altivec.h>
#endif
#endif


/// @def XXH3_SECRET_DEFAULT_SIZE
/// @brief Default Secret's size
///
/// This is the size of internal XXH3_kSecret
/// and is needed by XXH3_generateSecret_fromSeed().
///
/// Not to be confused with @ref XXH3_SECRET_SIZE_MIN.
#define XXH3_SECRET_DEFAULT_SIZE 192

/// @internal
/// @brief The size of the internal XXH3 buffer.
///
/// This is the optimal update size for incremental hashing.
///
/// @see XXH3_64b_update(), XXH3_128b_update().
#define XXH3_INTERNALBUFFER_SIZE 256

/// @brief Maximum size of "short" key in bytes.
#define XXH3_MIDSIZE_MAX 240

/// Default secret size. This is the minimum size required by XXH3_SECRET_SIZE_MIN.
#define XXH_SECRET_DEFAULT_SIZE 192

/// The bare minimum size for a custom secret.
///
/// @see
///   XXH3_64bits_withSecret(), XXH3_64bits_reset_withSecret(),
///   XXH3_128bits_withSecret(), XXH3_128bits_reset_withSecret().
#define XXH3_SECRET_SIZE_MIN 136

/// Ensure that the default secret meets the minimum size requirement.
#if (XXH_SECRET_DEFAULT_SIZE < XXH3_SECRET_SIZE_MIN)
#error "default keyset is not large enough"
#endif

///////////////////////////////////////////////////////
/// @internal
/// @def XXH3_kSecret
/// @brief Pseudorandom secret taken directly from FARSH.
///
KUMO_ALIGN(64)
static const uint8_t XXH3_kSecret[XXH_SECRET_DEFAULT_SIZE] = {
    0xb8,
    0xfe,
    0x6c,
    0x39,
    0x23,
    0xa4,
    0x4b,
    0xbe,
    0x7c,
    0x01,
    0x81,
    0x2c,
    0xf7,
    0x21,
    0xad,
    0x1c,
    0xde,
    0xd4,
    0x6d,
    0xe9,
    0x83,
    0x90,
    0x97,
    0xdb,
    0x72,
    0x40,
    0xa4,
    0xa4,
    0xb7,
    0xb3,
    0x67,
    0x1f,
    0xcb,
    0x79,
    0xe6,
    0x4e,
    0xcc,
    0xc0,
    0xe5,
    0x78,
    0x82,
    0x5a,
    0xd0,
    0x7d,
    0xcc,
    0xff,
    0x72,
    0x21,
    0xb8,
    0x08,
    0x46,
    0x74,
    0xf7,
    0x43,
    0x24,
    0x8e,
    0xe0,
    0x35,
    0x90,
    0xe6,
    0x81,
    0x3a,
    0x26,
    0x4c,
    0x3c,
    0x28,
    0x52,
    0xbb,
    0x91,
    0xc3,
    0x00,
    0xcb,
    0x88,
    0xd0,
    0x65,
    0x8b,
    0x1b,
    0x53,
    0x2e,
    0xa3,
    0x71,
    0x64,
    0x48,
    0x97,
    0xa2,
    0x0d,
    0xf9,
    0x4e,
    0x38,
    0x19,
    0xef,
    0x46,
    0xa9,
    0xde,
    0xac,
    0xd8,
    0xa8,
    0xfa,
    0x76,
    0x3f,
    0xe3,
    0x9c,
    0x34,
    0x3f,
    0xf9,
    0xdc,
    0xbb,
    0xc7,
    0xc7,
    0x0b,
    0x4f,
    0x1d,
    0x8a,
    0x51,
    0xe0,
    0x4b,
    0xcd,
    0xb4,
    0x59,
    0x31,
    0xc8,
    0x9f,
    0x7e,
    0xc9,
    0xd9,
    0x78,
    0x73,
    0x64,
    0xea,
    0xc5,
    0xac,
    0x83,
    0x34,
    0xd3,
    0xeb,
    0xc3,
    0xc5,
    0x81,
    0xa0,
    0xff,
    0xfa,
    0x13,
    0x63,
    0xeb,
    0x17,
    0x0d,
    0xdd,
    0x51,
    0xb7,
    0xf0,
    0xda,
    0x49,
    0xd3,
    0x16,
    0x55,
    0x26,
    0x29,
    0xd4,
    0x68,
    0x9e,
    0x2b,
    0x16,
    0xbe,
    0x58,
    0x7d,
    0x47,
    0xa1,
    0xfc,
    0x8f,
    0xf8,
    0xb8,
    0xd1,
    0x7a,
    0xd0,
    0x31,
    0xce,
    0x45,
    0xcb,
    0x3a,
    0x8f,
    0x95,
    0x16,
    0x04,
    0x28,
    0xaf,
    0xd7,
    0xfb,
    0xca,
    0xbb,
    0x4b,
    0x40,
    0x7e,
};


///////////////////////////////////////////////////////
/// 0b0001011001010110011001111001000110011110001101110111100111111001
static const uint64_t PRIME_MX1 = 0x165667919E3779F9ULL;
/// 0b1001111110110010000111000110010100011110100110001101111100100101
static const uint64_t PRIME_MX2 = 0x9FB21C651E98DF25ULL;


/// @ingroup tuning
/// @brief Possible values for @ref XXH_VECTOR.
///
/// Unless set explicitly, determined automatically.
#define XXH_SCALAR 0 /*!< Portable scalar version */
#define XXH_SSE2 1 /*!< SSE2 for Pentium 4, Opteron, all x86_64. */
#define XXH_AVX2 2 /*!< AVX2 for Haswell and Bulldozer */
#define XXH_AVX512 3 /*!< AVX512 for Skylake and Icelake */
#define XXH_NEON 4 /*!< NEON for most ARMv7-A, all AArch64, and WASM SIMD128 */
#define XXH_VSX 5 /*!< VSX and ZVector for POWER8/z13 (64-bit) */
#define XXH_SVE 6 /*!< SVE for some ARMv8-A and ARMv9-A */
#define XXH_LSX 7 /*!< LSX (128-bit SIMD) for LoongArch64 */
#define XXH_LASX 8 /*!< LASX (256-bit SIMD) for LoongArch64 */
#define XXH_RVV 9 /*!< RVV (RISC-V Vector) for RISC-V */

#ifndef XXH32_ENDJMP
/* generally preferable for performance */
#define XXH32_ENDJMP 0
#endif

/// #define instead of static const, to be used as initializers
/// 0b10011110001101110111100110110001
#define XXH_PRIME32_1 0x9E3779B1U
/// 0b10000101111010111100101001110111
#define XXH_PRIME32_2 0x85EBCA77U
/// 0b11000010101100101010111000111101
#define XXH_PRIME32_3 0xC2B2AE3DU
/// 0b00100111110101001110101100101111
#define XXH_PRIME32_4 0x27D4EB2FU
/// 0b00010110010101100110011110110001
#define XXH_PRIME32_5 0x165667B1U



/// #define instead of static const, to be used as initializers
/// 0b1001111000110111011110011011000110000101111010111100101010000111
#define XXH_PRIME64_1 0x9E3779B185EBCA87ULL
/// 0b1100001010110010101011100011110100100111110101001110101101001111
#define XXH_PRIME64_2 0xC2B2AE3D27D4EB4FULL
/// 0b0001011001010110011001111011000110011110001101110111100111111001
#define XXH_PRIME64_3 0x165667B19E3779F9ULL
/// 0b1000010111101011110010100111011111000010101100101010111001100011
#define XXH_PRIME64_4 0x85EBCA77C2B2AE63ULL
/// 0b0010011111010100111010110010111100010110010101100110011111000101
#define XXH_PRIME64_5 0x27D4EB2F165667C5ULL


#define XXH_STRIPE_LEN 64
#define XXH_SECRET_CONSUME_RATE 8 /* nb of secret bytes consumed at each accumulation */
#define XXH_ACC_NB (XXH_STRIPE_LEN / sizeof(uint64_t))
#define XXH_SECRET_MERGEACCS_START 11

#define XXH3_MIDSIZE_STARTOFFSET 3
#define XXH3_MIDSIZE_LASTOFFSET 17

#ifndef XXH_PREFETCH_DIST
#ifdef __clang__
#define XXH_PREFETCH_DIST 320
#else
#if (XXH_VECTOR == XXH_AVX512)
#define XXH_PREFETCH_DIST 512
#else
#define XXH_PREFETCH_DIST 384
#endif
#endif /* __clang__ */
#endif /* XXH_PREFETCH_DIST */


//////////////////////////////////////////////////////////////////////////
/// Debug
/// KUMO_DEBUG_*
//////////////////////////////////////////////////////////////////////////
/// @ingroup tuning
/// @def XXH_DEBUGLEVEL
/// @brief Sets the debugging level.
///
/// XXH_DEBUGLEVEL is expected to be defined externally, typically via the
/// compiler's command line options. The value must be a number.
///
#ifndef XXH_DEBUGLEVEL
#ifdef DEBUGLEVEL /* backwards compat */
#define XXH_DEBUGLEVEL DEBUGLEVEL
#else
#define XXH_DEBUGLEVEL 0
#endif
#endif

/* prefetch
  * can be disabled, by declaring XXH_NO_PREFETCH build macro */
#if defined(XXH_NO_PREFETCH)
#define XXH_PREFETCH(ptr) (void)(ptr) /* disabled */
#else
#if XXH_SIZE_OPT >= 1
#define XXH_PREFETCH(ptr) (void)(ptr)
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
/// _mm_prefetch() not defined outside of x86/x64
/// https://msdn.microsoft.com/fr-fr/library/84szxsww(v=vs.90).aspx
#include <mmintrin.h>
#define XXH_PREFETCH(ptr) _mm_prefetch((const char*)(ptr), _MM_HINT_T0)
#elif defined(__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))
#define XXH_PREFETCH(ptr) __builtin_prefetch((ptr), 0 /* rw==read */, 3 /* locality */)
#else
#define XXH_PREFETCH(ptr) (void)(ptr) /* disabled */
#endif
#endif /* XXH_NO_PREFETCH */
