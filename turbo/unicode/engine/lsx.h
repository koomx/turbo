#ifndef UNICODE_LSX_H
#define UNICODE_LSX_H

#include <turbo/unicode/engine/lasx.h>
#ifdef UNICODE_FALLBACK_H
#error "lsx.h must be included before fallback.h"
#endif

#include <turbo/unicode/engine/portability.h>
#include <turbo/arch/isa.h>

#ifndef UNICODE_IMPLEMENTATION_LSX
#define UNICODE_IMPLEMENTATION_LSX (KUMO_SIMD_LSX)
#endif

#include <turbo/arch/isadetection.h>

namespace turbo {
    IsaInfo get_lsx_info();
} // namespace turbo

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
