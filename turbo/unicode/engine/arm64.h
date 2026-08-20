#ifndef UNICODE_ARM64_H
#define UNICODE_ARM64_H

#ifdef UNICODE_FALLBACK_H
#error "arm64.h must be included before fallback.h"
#endif

#include <turbo/unicode/engine/portability.h>
#include <turbo/arch/isa.h>

#ifndef UNICODE_IMPLEMENTATION_ARM64
#define UNICODE_IMPLEMENTATION_ARM64 (KUMO_ARCH_ARM64)
#endif
#if UNICODE_IMPLEMENTATION_ARM64 && KUMO_ARCH_ARM64
#define UNICODE_CAN_ALWAYS_RUN_ARM64 1
#else
#define UNICODE_CAN_ALWAYS_RUN_ARM64 0
#endif

#include <turbo/arch/isadetection.h>

namespace turbo {
    IsaInfo get_arm64_info();
} // namespace turbo


#if UNICODE_IMPLEMENTATION_ARM64

namespace turbo {
    /// Implementation for NEON (ARMv8).
    namespace arm64 { } // namespace arm64
} // namespace turbo

#include <turbo/unicode/engine/arm64/implementation.h>

#include <turbo/unicode/engine/arm64/begin.h>

// Declarations
#include <turbo/unicode/engine/arm64/intrinsics.h>
#include <turbo/unicode/engine/arm64/bitmanipulation.h>
#include <turbo/unicode/engine/arm64/simd.h>

#include <turbo/unicode/engine/arm64/end.h>



#endif // UNICODE_IMPLEMENTATION_ARM64

#endif // UNICODE_ARM64_H
