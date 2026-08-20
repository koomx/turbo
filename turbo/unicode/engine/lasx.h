#ifndef UNICODE_LASX_H
#define UNICODE_LASX_H

#ifdef UNICODE_FALLBACK_H
#error "lasx.h must be included before fallback.h"
#endif

#include <turbo/unicode/engine/portability.h>

#ifndef UNICODE_IMPLEMENTATION_LASX
#if KUMO_SIMD_LASX
#define UNICODE_IMPLEMENTATION_LASX 1
#elif KUMO_SIMD_LSX
#if defined(__GNUC__) && !defined(__clang__) && !defined(__INTEL_COMPILER) && \
    !defined(__NVCOMPILER) && \
    (__GNUC__ > 15 || (__GNUC__ == 15 && __GNUC_MINOR__ >= 0))
#define UNICODE_IMPLEMENTATION_LASX 1
#else
#define UNICODE_IMPLEMENTATION_LASX 0
#endif
#else
#define UNICODE_IMPLEMENTATION_LASX 0
#endif
#endif
#if UNICODE_IMPLEMENTATION_LASX && KUMO_SIMD_LASX
#define UNICODE_CAN_ALWAYS_RUN_LASX 1
#else
#define UNICODE_CAN_ALWAYS_RUN_LASX 0
#endif

#define UNICODE_CAN_ALWAYS_RUN_FALLBACK (UNICODE_IMPLEMENTATION_FALLBACK)
#include <turbo/arch/isadetection.h>

#if UNICODE_IMPLEMENTATION_LASX
#define UNICODE_TARGET_LASX UNICODE_TARGET_REGION("lasx,lsx")

// For runtime dispatching to work, we need the lsxintrin to appear
// before we call UNICODE_TARGET_LASX. It is unclear why.
#include <lsxintrin.h>

namespace turbo {
    /// Implementation for LoongArch ASX.
    namespace lasx { } // namespace lasx
} // namespace turbo

#include <turbo/unicode/engine/lasx/implementation.h>

#include <turbo/unicode/engine/lasx/begin.h>

// Declarations
#include <turbo/unicode/engine/lasx/intrinsics.h>
#include <turbo/unicode/engine/lasx/bitmanipulation.h>
#include <turbo/unicode/engine/lasx/simd.h>

#include <turbo/unicode/engine/lasx/end.h>

#endif // UNICODE_IMPLEMENTATION_LASX

#endif // UNICODE_LASX_H
