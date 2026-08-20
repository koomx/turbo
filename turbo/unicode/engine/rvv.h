#ifndef UNICODE_RVV_H
#define UNICODE_RVV_H

#ifdef UNICODE_FALLBACK_H
#error "rvv.h must be included before fallback.h"
#endif

#include <turbo/unicode/engine/portability.h>

#define UNICODE_CAN_ALWAYS_RUN_RVV UNICODE_IS_RVV

#ifndef UNICODE_IMPLEMENTATION_RVV
#define UNICODE_IMPLEMENTATION_RVV \
    (UNICODE_CAN_ALWAYS_RUN_RVV || (KUMO_ARCH_RISCV64 && UNICODE_HAS_RVV_INTRINSICS && UNICODE_HAS_RVV_TARGET_REGION))
#endif

#if UNICODE_IMPLEMENTATION_RVV

#if UNICODE_CAN_ALWAYS_RUN_RVV
#define UNICODE_TARGET_RVV
#else
#define UNICODE_TARGET_RVV UNICODE_TARGET_REGION("arch=+v")
#endif
#if !UNICODE_IS_ZVBB && UNICODE_HAS_ZVBB_INTRINSICS
#define UNICODE_TARGET_ZVBB UNICODE_TARGET_REGION("arch=+v,+zvbb")
#endif

namespace turbo {
    namespace rvv { } // namespace rvv
} // namespace turbo

#include <turbo/unicode/engine/rvv/implementation.h>
#include <turbo/unicode/engine/rvv/begin.h>
#include <turbo/unicode/engine/rvv/intrinsics.h>
#include <turbo/unicode/engine/rvv/end.h>

#endif // UNICODE_IMPLEMENTATION_RVV

#endif // UNICODE_RVV_H
