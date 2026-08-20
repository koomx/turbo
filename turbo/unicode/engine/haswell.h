#ifndef UNICODE_HASWELL_H
#define UNICODE_HASWELL_H

#ifdef UNICODE_WESTMERE_H
#error "haswell.h must be included before westmere.h"
#endif
#ifdef UNICODE_FALLBACK_H
#error "haswell.h must be included before fallback.h"
#endif

#include <turbo/unicode/engine/portability.h>

// Default Haswell to on if this is x86-64. Even if we are not compiled for it,
// it could be selected at runtime.
#ifndef UNICODE_IMPLEMENTATION_HASWELL
//
// You do not want to restrict it like so: KUMO_ARCH_X86_64 && __AVX2__
// because we want to rely on *runtime dispatch*.
//
#if UNICODE_CAN_ALWAYS_RUN_ICELAKE
#define UNICODE_IMPLEMENTATION_HASWELL 0
#else
#define UNICODE_IMPLEMENTATION_HASWELL (KUMO_ARCH_X86_64)
#endif

#endif
// To see why  (__BMI__) && (__LZCNT__) are not part of this next line, see
// https://github.com/simdutf/simdutf/issues/1247
#if ((UNICODE_IMPLEMENTATION_HASWELL) && (KUMO_ARCH_X86_64) && (__AVX2__))
#define UNICODE_CAN_ALWAYS_RUN_HASWELL 1
#else
#define UNICODE_CAN_ALWAYS_RUN_HASWELL 0
#endif

#if UNICODE_IMPLEMENTATION_HASWELL

#define UNICODE_TARGET_HASWELL UNICODE_TARGET_REGION("avx2,bmi,lzcnt,popcnt")

namespace turbo {
    /// Implementation for Haswell (Intel AVX2).
    namespace haswell { } // namespace haswell
} // namespace turbo

//
// These two need to be included outside UNICODE_TARGET_REGION
//
#include <turbo/unicode/engine/haswell/implementation.h>
#include <turbo/unicode/engine/haswell/intrinsics.h>

//
// The rest need to be inside the region
//
#include <turbo/unicode/engine/haswell/begin.h>
  // Declarations
#include <turbo/unicode/engine/haswell/bitmanipulation.h>
#include <turbo/unicode/engine/haswell/simd.h>

#include <turbo/unicode/engine/haswell/end.h>

#endif // UNICODE_IMPLEMENTATION_HASWELL
#endif // UNICODE_HASWELL_COMMON_H
