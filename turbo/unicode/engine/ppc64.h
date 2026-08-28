#ifndef UNICODE_PPC64_H
#define UNICODE_PPC64_H

#include <turbo/unicode/engine/portability.h>
#include <turbo/arch/isa.h>

#ifndef UNICODE_IMPLEMENTATION_PPC64
#define UNICODE_IMPLEMENTATION_PPC64 (KUMO_ARCH_PPC64 && KUMO_SIMD_ALTIVEC)
#endif

#include <turbo/arch/cpu_detect.h>

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
