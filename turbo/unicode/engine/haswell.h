#ifndef UNICODE_HASWELL_H
#define UNICODE_HASWELL_H

#include <turbo/unicode/engine/portability.h>
#include <turbo/arch/isa.h>

#if defined(UNICODE_IMPLEMENTATION_HASWELL)
#error can not define UNICODE_IMPLEMENTATION_HASWELL
#endif

// Compile Haswell when this TU already has AVX2 (including AVX-512 builds).
#if KUMO_ARCH_X86_64 && defined(__AVX2__)
#define UNICODE_IMPLEMENTATION_HASWELL 1
#else
#define UNICODE_IMPLEMENTATION_HASWELL 0
#endif

namespace turbo {
    IsaInfo get_haswell_info();
} // namespace turbo

#if UNICODE_IMPLEMENTATION_HASWELL

#define UNICODE_TARGET_HASWELL KUMO_TARGET_REGION("avx2,bmi,lzcnt,popcnt")

namespace turbo {
    /// Implementation for Haswell (Intel AVX2).
    namespace haswell { } // namespace haswell
} // namespace turbo

//
// These two need to be included outside KUMO_TARGET_REGION
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
