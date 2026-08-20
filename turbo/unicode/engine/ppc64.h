#ifndef UNICODE_PPC64_H
#define UNICODE_PPC64_H

#ifdef UNICODE_FALLBACK_H
#error "ppc64.h must be included before fallback.h"
#endif

#include <turbo/unicode/engine/portability.h>
#include <turbo/arch/isa.h>

#ifndef UNICODE_IMPLEMENTATION_PPC64
#define UNICODE_IMPLEMENTATION_PPC64 (KUMO_ARCH_PPC64 && KUMO_SIMD_ALTIVEC)
#endif
#define UNICODE_CAN_ALWAYS_RUN_PPC64 \
    UNICODE_IMPLEMENTATION_PPC64 && KUMO_ARCH_PPC64 && KUMO_SIMD_ALTIVEC

#include <turbo/arch/isadetection.h>

namespace turbo {
    IsaInfo get_ppc64_info();
} // namespace turbo

#if UNICODE_IMPLEMENTATION_PPC64

namespace turbo {
    /// Implementation for ALTIVEC (PPC64).
    namespace ppc64 { } // namespace ppc64
} // namespace turbo

#include <turbo/unicode/engine/ppc64/implementation.h>

#include <turbo/unicode/engine/ppc64/begin.h>

// Declarations
#include <turbo/unicode/engine/ppc64/intrinsics.h>
#include <turbo/unicode/engine/ppc64/bitmanipulation.h>
#include <turbo/unicode/engine/ppc64/simd.h>

#include <turbo/unicode/engine/ppc64/end.h>

#endif // UNICODE_IMPLEMENTATION_PPC64

#endif // UNICODE_PPC64_H
