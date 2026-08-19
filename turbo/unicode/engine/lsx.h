#ifndef SIMDUTF_LSX_H
#define SIMDUTF_LSX_H

#include <turbo/unicode/engine/lasx.h>
#ifdef SIMDUTF_FALLBACK_H
#error "lsx.h must be included before fallback.h"
#endif

#ifndef SIMDUTF_CAN_ALWAYS_RUN_LASX
#error "lsx.h must be included after lasx.h"
#endif

#include <turbo/unicode/engine/portability.h>

#ifndef SIMDUTF_IMPLEMENTATION_LSX
#if SIMDUTF_CAN_ALWAYS_RUN_LASX
#define SIMDUTF_IMPLEMENTATION_LSX 0
#else
#define SIMDUTF_IMPLEMENTATION_LSX (SIMDUTF_IS_LSX)
#endif
#endif
#if SIMDUTF_IMPLEMENTATION_LSX && SIMDUTF_IS_LSX
#define SIMDUTF_CAN_ALWAYS_RUN_LSX 1
#else
#define SIMDUTF_CAN_ALWAYS_RUN_LSX 0
#endif

#define SIMDUTF_CAN_ALWAYS_RUN_FALLBACK (SIMDUTF_IMPLEMENTATION_FALLBACK)
#include <turbo/unicode/internal/isadetection.h>

#if SIMDUTF_IMPLEMENTATION_LSX

namespace turbo {
    /// Implementation for LoongArch SX.
    namespace lsx { } // namespace lsx
} // namespace turbo

#include <turbo/unicode/engine/lsx/implementation.h>

#include <turbo/unicode/engine/lsx/begin.h>

// Declarations
#include <turbo/unicode/engine/lsx/intrinsics.h>
#include <turbo/unicode/engine/lsx/bitmanipulation.h>
#include <turbo/unicode/engine/lsx/simd.h>

#include <turbo/unicode/engine/lsx/end.h>

#endif // SIMDUTF_IMPLEMENTATION_LSX

#endif // SIMDUTF_LSX_H
