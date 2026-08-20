#ifndef UNICODE_ICELAKE_H
#define UNICODE_ICELAKE_H

#include <turbo/unicode/engine/portability.h>

#ifdef __has_include
// How do we detect that a compiler supports vbmi2?
// For sure if the following header is found, we are ok?
#if __has_include(<avx512vbmi2intrin.h>)
#define UNICODE_COMPILER_SUPPORTS_VBMI2 1
#endif
#endif

#ifdef _MSC_VER
#if _MSC_VER >= 1930
// Visual Studio 2022 and up support VBMI2 under x64 even if the header
// avx512vbmi2intrin.h is not found.
// Visual Studio 2019 technically supports VBMI2, but the implementation
// might be unreliable. Search for visualstudio2019icelakeissue in our
// tests.
#ifndef UNICODE_COMPILER_SUPPORTS_VBMI2
#define UNICODE_COMPILER_SUPPORTS_VBMI2 1
#endif
#endif
#endif

#if UNICODE_GCC9OROLDER && KUMO_ARCH_X86_64
#define UNICODE_IMPLEMENTATION_ICELAKE 0
#warning \
    "You are using a legacy GCC compiler, we are disabling AVX-512 support"
#endif

// We allow icelake on x64 as long as the compiler is known to support VBMI2.
#ifndef UNICODE_IMPLEMENTATION_ICELAKE
#define UNICODE_IMPLEMENTATION_ICELAKE \
    ((KUMO_ARCH_X86_64) && (UNICODE_COMPILER_SUPPORTS_VBMI2))
#endif

// To see why  (__BMI__) && (__LZCNT__) are not part of this next line, see
// https://github.com/simdutf/simdutf/issues/1247
#if ((UNICODE_IMPLEMENTATION_ICELAKE) && (KUMO_ARCH_X86_64) && (__AVX2__) && (KUMO_SIMD_AVX512F && KUMO_SIMD_AVX512DQ && KUMO_SIMD_AVX512VL && KUMO_SIMD_AVX512VBMI2) && (!KUMO_ARCH_32_BIT))
#define UNICODE_CAN_ALWAYS_RUN_ICELAKE 1
#else
#define UNICODE_CAN_ALWAYS_RUN_ICELAKE 0
#endif

#if UNICODE_IMPLEMENTATION_ICELAKE
#if UNICODE_CAN_ALWAYS_RUN_ICELAKE
#define UNICODE_TARGET_ICELAKE
#else
#define UNICODE_TARGET_ICELAKE                                       \
    UNICODE_TARGET_REGION(                                           \
        "avx512f,avx512dq,avx512cd,avx512bw,avx512vbmi,avx512vbmi2," \
        "avx512vl,avx2,bmi,bmi2,pclmul,lzcnt,popcnt,avx512vpopcntdq")
#endif

namespace turbo {
    namespace icelake { } // namespace icelake
} // namespace turbo

//
// These two need to be included outside UNICODE_TARGET_REGION
//
#include <turbo/unicode/engine/icelake/intrinsics.h>
#include <turbo/unicode/engine/icelake/implementation.h>

//
// The rest need to be inside the region
//
#include <turbo/unicode/engine/icelake/begin.h>
  // Declarations
#include <turbo/unicode/engine/icelake/bitmanipulation.h>
#include <turbo/unicode/engine/icelake/simd.h>

#include <turbo/unicode/engine/icelake/end.h>

#endif // UNICODE_IMPLEMENTATION_ICELAKE
#endif // UNICODE_ICELAKE_H
