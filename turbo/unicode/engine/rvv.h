#ifndef UNICODE_RVV_H
#define UNICODE_RVV_H

#ifdef UNICODE_FALLBACK_H
#error "rvv.h must be included before fallback.h"
#endif

#include <turbo/unicode/engine/portability.h>
#include <turbo/arch/isa.h>

#ifndef UNICODE_IMPLEMENTATION_RVV
#define UNICODE_IMPLEMENTATION_RVV \
    (UNICODE_IS_RVV || (KUMO_ARCH_RISCV64 && UNICODE_HAS_RVV_INTRINSICS))
#endif

namespace turbo {
    IsaInfo get_rvv_info();
} // namespace turbo

#if UNICODE_IMPLEMENTATION_RVV

#if UNICODE_IS_RVV
#define UNICODE_TARGET_RVV
#else
#define UNICODE_TARGET_RVV KUMO_TARGET_REGION("arch=+v")
#endif
#if !UNICODE_IS_ZVBB && UNICODE_HAS_ZVBB_INTRINSICS
#define UNICODE_TARGET_ZVBB KUMO_TARGET_REGION("arch=+v,+zvbb")
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
