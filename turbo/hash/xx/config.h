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

#ifndef XXH_SIZE_OPT
/* default to 1 for -Os or -Oz */
#if (defined(__GNUC__) || defined(__clang__)) && defined(__OPTIMIZE_SIZE__)
#define XXH_SIZE_OPT 1
#else
#define XXH_SIZE_OPT 0
#endif
#endif

#ifndef XXH_FORCE_ALIGN_CHECK
/* don't check on sizeopt, x86, aarch64, or arm when unaligned access is available */
#if XXH_SIZE_OPT >= 1 || defined(__i386) || defined(__x86_64__) || defined(__aarch64__) || defined(__ARM_FEATURE_UNALIGNED) \
    || defined(_M_IX86) || defined(_M_X64) || defined(_M_ARM64) || defined(_M_ARM) /* visual */
#define XXH_FORCE_ALIGN_CHECK 0
#else
#define XXH_FORCE_ALIGN_CHECK 1
#endif
#endif

#define XXH_PRIME32_1 0x9E3779B1U /*!< 0b10011110001101110111100110110001 */
#define XXH_PRIME32_2 0x85EBCA77U /*!< 0b10000101111010111100101001110111 */
#define XXH_PRIME32_3 0xC2B2AE3DU /*!< 0b11000010101100101010111000111101 */
#define XXH_PRIME32_4 0x27D4EB2FU /*!< 0b00100111110101001110101100101111 */
#define XXH_PRIME32_5 0x165667B1U /*!< 0b00010110010101100110011110110001 */

#define XXH_PRIME64_1 0x9E3779B185EBCA87ULL /*!< 0b1001111000110111011110011011000110000101111010111100101010000111 */
#define XXH_PRIME64_2 0xC2B2AE3D27D4EB4FULL /*!< 0b1100001010110010101011100011110100100111110101001110101101001111 */
#define XXH_PRIME64_3 0x165667B19E3779F9ULL /*!< 0b0001011001010110011001111011000110011110001101110111100111111001 */
#define XXH_PRIME64_4 0x85EBCA77C2B2AE63ULL /*!< 0b1000010111101011110010100111011111000010101100101010111001100011 */
#define XXH_PRIME64_5 0x27D4EB2F165667C5ULL /*!< 0b0010011111010100111010110010111100010110010101100110011111000101 */

#ifndef XXH32_ENDJMP
/* generally preferable for performance */
#define XXH32_ENDJMP 0
#endif

#define XXH3_SECRET_SIZE_MIN 136
/// minimum XXH3_SECRET_SIZE_MIN
#define XXH_SECRET_DEFAULT_SIZE 192

#define XXH3_INTERNALBUFFER_SIZE 256

#define XXH3_SECRET_DEFAULT_SIZE 192

#define XXH3_MIDSIZE_MAX 240

#define XXH_SECRET_MERGEACCS_START 11

#if (XXH_SECRET_DEFAULT_SIZE < XXH3_SECRET_SIZE_MIN)
#error "default keyset is not large enough"
#endif

#define XXH_STRIPE_LEN 64
#define XXH_SECRET_CONSUME_RATE 8 /* nb of secret bytes consumed at each accumulation */
#define XXH_ACC_NB (XXH_STRIPE_LEN / sizeof(uint64_t))

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
#endif // XXH_PREFETCH_DIST

/* prefetch
 * can be disabled, by declaring XXH_NO_PREFETCH build macro */
#if defined(XXH_NO_PREFETCH)
#define XXH_PREFETCH(ptr) (void)(ptr) /* disabled */
#else
#if XXH_SIZE_OPT >= 1
#define XXH_PREFETCH(ptr) (void)(ptr)
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86)) /* _mm_prefetch() not defined outside of x86/x64 */
#include <mmintrin.h> /* https://msdn.microsoft.com/fr-fr/library/84szxsww(v=vs.90).aspx */
#define XXH_PREFETCH(ptr) _mm_prefetch((const char*)(ptr), _MM_HINT_T0)
#elif defined(__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))
#define XXH_PREFETCH(ptr) __builtin_prefetch((ptr), 0 /* rw==read */, 3 /* locality */)
#else
#define XXH_PREFETCH(ptr) (void)(ptr) /* disabled */
#endif
#endif /* XXH_NO_PREFETCH */

#if defined(__GNUC__) || defined(__clang__)
#define XXH_ALIASING __attribute__((__may_alias__))
#else
#define XXH_ALIASING /* nothing */
#endif

///////////////////////////////////////////////////////////
/// Controls the alignment of the accumulator,
/// for compatibility with aligned vector loads, which are usually faster.
///
#ifndef XXH_ACC_ALIGN
#if defined(XXH_X86DISPATCH)
#define XXH_ACC_ALIGN 64 /* for compatibility with avx512 */
#elif XXH_VECTOR == XXH_SCALAR /* scalar */
#define XXH_ACC_ALIGN 8
#elif XXH_VECTOR == XXH_SSE2 /* sse2 */
#define XXH_ACC_ALIGN 16
#elif XXH_VECTOR == XXH_AVX2 /* avx2 */
#define XXH_ACC_ALIGN 32
#elif XXH_VECTOR == XXH_NEON /* neon */
#define XXH_ACC_ALIGN 16
#elif XXH_VECTOR == XXH_VSX /* vsx */
#define XXH_ACC_ALIGN 16
#elif XXH_VECTOR == XXH_AVX512 /* avx512 */
#define XXH_ACC_ALIGN 64
#elif XXH_VECTOR == XXH_SVE /* sve */
#define XXH_ACC_ALIGN 64
#elif XXH_VECTOR == XXH_LASX /* lasx */
#define XXH_ACC_ALIGN 64
#elif XXH_VECTOR == XXH_LSX /* lsx */
#define XXH_ACC_ALIGN 64
#elif XXH_VECTOR == XXH_RVV /* rvv */
#define XXH_ACC_ALIGN 64 /* could be 8, but 64 may be faster */
#endif
#endif

#if defined(XXH_X86DISPATCH) || XXH_VECTOR == XXH_SSE2 \
    || XXH_VECTOR == XXH_AVX2 || XXH_VECTOR == XXH_AVX512
#define XXH_SEC_ALIGN XXH_ACC_ALIGN
#elif XXH_VECTOR == XXH_SVE
#define XXH_SEC_ALIGN XXH_ACC_ALIGN
#elif XXH_VECTOR == XXH_RVV
#define XXH_SEC_ALIGN XXH_ACC_ALIGN
#else
#define XXH_SEC_ALIGN 8
#endif

namespace turbo::xxhash {
    static constexpr bool kXxhForceAlignCheck = XXH_FORCE_ALIGN_CHECK == 1;

    static const uint64_t PRIME_MX1 = 0x165667919E3779F9ULL; /*!< 0b0001011001010110011001111001000110011110001101110111100111111001 */
    static const uint64_t PRIME_MX2 = 0x9FB21C651E98DF25ULL; /*!< 0b1001111110110010000111000110010100011110100110001101111100100101 */

} // namespace turbo::xxhash
