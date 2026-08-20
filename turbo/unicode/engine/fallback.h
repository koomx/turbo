#ifndef UNICODE_FALLBACK_H
#define UNICODE_FALLBACK_H

#include <turbo/unicode/engine/portability.h>
#include <turbo/arch/isa.h>

// Note that fallback.h is always imported last.

// Default Fallback to on unless a builtin implementation has already been
// selected.
#ifndef UNICODE_IMPLEMENTATION_FALLBACK
#if UNICODE_CAN_ALWAYS_RUN_ARM64 || UNICODE_CAN_ALWAYS_RUN_ICELAKE || UNICODE_CAN_ALWAYS_RUN_HASWELL || UNICODE_CAN_ALWAYS_RUN_WESTMERE || UNICODE_CAN_ALWAYS_RUN_PPC64 || UNICODE_CAN_ALWAYS_RUN_RVV || UNICODE_CAN_ALWAYS_RUN_LSX || UNICODE_CAN_ALWAYS_RUN_LASX
#define UNICODE_IMPLEMENTATION_FALLBACK 0
#else
#define UNICODE_IMPLEMENTATION_FALLBACK 1
#endif
#endif

#define UNICODE_CAN_ALWAYS_RUN_FALLBACK (UNICODE_IMPLEMENTATION_FALLBACK)

namespace turbo {
    IsaInfo get_fallback_info();
} // namespace turbo

#if UNICODE_IMPLEMENTATION_FALLBACK

namespace turbo {
    /// Fallback implementation (runs on any machine).
    namespace fallback { } // namespace fallback
} // namespace turbo

#include <turbo/unicode/engine/fallback/implementation.h>

#include <turbo/unicode/engine/fallback/begin.h>

// Declarations
#include <turbo/unicode/engine/fallback/bitmanipulation.h>

#include <turbo/unicode/engine/fallback/end.h>

#endif // UNICODE_IMPLEMENTATION_FALLBACK
#endif // UNICODE_FALLBACK_H
