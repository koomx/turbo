#ifndef UNICODE_LSX_H
#define UNICODE_LSX_H

#include <turbo/unicode/engine/lasx.h>
#ifdef UNICODE_FALLBACK_H
#error "lsx.h must be included before fallback.h"
#endif

#ifndef UNICODE_CAN_ALWAYS_RUN_LASX
#error "lsx.h must be included after lasx.h"
#endif

#include <turbo/unicode/engine/portability.h>

#ifndef UNICODE_IMPLEMENTATION_LSX
#if UNICODE_CAN_ALWAYS_RUN_LASX
#define UNICODE_IMPLEMENTATION_LSX 0
#else
#define UNICODE_IMPLEMENTATION_LSX (KUMO_SIMD_LSX)
#endif
#endif
#if UNICODE_IMPLEMENTATION_LSX && KUMO_SIMD_LSX
#define UNICODE_CAN_ALWAYS_RUN_LSX 1
#else
#define UNICODE_CAN_ALWAYS_RUN_LSX 0
#endif

#define UNICODE_CAN_ALWAYS_RUN_FALLBACK (UNICODE_IMPLEMENTATION_FALLBACK)
#include <turbo/arch/isadetection.h>

#if UNICODE_IMPLEMENTATION_LSX

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

#endif // UNICODE_IMPLEMENTATION_LSX

#endif // UNICODE_LSX_H
