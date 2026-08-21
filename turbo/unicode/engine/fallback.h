#ifndef UNICODE_FALLBACK_H
#define UNICODE_FALLBACK_H

#include <turbo/unicode/engine/portability.h>
#include <turbo/arch/isa.h>

// Default Fallback to on unless a builtin implementation has already been
// selected.
#ifndef UNICODE_IMPLEMENTATION_FALLBACK
#define UNICODE_IMPLEMENTATION_FALLBACK 1
#endif

namespace turbo {
    IsaInfo get_fallback_info();
} // namespace turbo

namespace turbo {
    /// Fallback implementation (runs on any machine).
    namespace fallback { } // namespace fallback
} // namespace turbo

#include <turbo/unicode/engine/fallback/implementation.h>

#include <turbo/unicode/engine/fallback/begin.h>

// Declarations
#include <turbo/unicode/engine/fallback/bitmanipulation.h>

#include <turbo/unicode/engine/fallback/end.h>

#endif // UNICODE_FALLBACK_H
