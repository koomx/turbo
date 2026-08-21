#ifndef UNICODE_WESTMERE_H
#define UNICODE_WESTMERE_H

#include <turbo/unicode/engine/portability.h>
#include <turbo/arch/isa.h>

#ifdef UNICODE_IMPLEMENTATION_WESTMERE
#error UNICODE_IMPLEMENTATION_WESTMERE xxx
#endif

// Default Westmere to on if this is x86-64, unless we'll always select Haswell.
//
// You do not want to set it to (KUMO_ARCH_X86_64 &&
// !UNICODE_REQUIRES_HASWELL) because you want to rely on runtime dispatch!
//
#if KUMO_ARCH_X86_64 && __SSE4_2__
#define UNICODE_IMPLEMENTATION_WESTMERE 1
#else
#define UNICODE_IMPLEMENTATION_WESTMERE 0
#endif

namespace turbo {
    IsaInfo get_westmere_info();
} // namespace turbo

#if UNICODE_IMPLEMENTATION_WESTMERE

#define UNICODE_TARGET_WESTMERE KUMO_TARGET_REGION("sse4.2,popcnt")

namespace turbo {
    /// Implementation for Westmere (Intel SSE4.2).
    namespace westmere { } // namespace westmere
} // namespace turbo

//
// These two need to be included outside KUMO_TARGET_REGION
//
#include <turbo/unicode/engine/westmere/implementation.h>
#include <turbo/unicode/engine/westmere/intrinsics.h>

//
// The rest need to be inside the region
//
#include <turbo/unicode/engine/westmere/begin.h>

// Declarations
#include <turbo/unicode/engine/westmere/bitmanipulation.h>
#include <turbo/unicode/engine/westmere/simd.h>

#include <turbo/unicode/engine/westmere/end.h>

#endif // UNICODE_IMPLEMENTATION_WESTMERE
#endif // UNICODE_WESTMERE_COMMON_H
