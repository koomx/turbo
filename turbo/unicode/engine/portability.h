#ifndef UNICODE_PORTABILITY_H
#define UNICODE_PORTABILITY_H

#include <turbo/macros/macros.h>

#include <cfloat>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#ifndef _WIN32
// strcasecmp, strncasecmp
#include <strings.h>
#endif

#if KUMO_COMPILER_MSVC
// https://en.wikipedia.org/wiki/C_alternative_tokens
// This header should have no effect, except maybe
// under Visual Studio.
#include <iso646.h>
#endif

// RVV / ZVBB compiler capability — unicode kernels, not kumo arch.
#ifndef UNICODE_HAS_RVV_INTRINSICS
#if defined(__riscv_v_intrinsic) && __riscv_v_intrinsic >= 11000
#define UNICODE_HAS_RVV_INTRINSICS 1
#else
#define UNICODE_HAS_RVV_INTRINSICS 0
#endif
#endif

#ifndef UNICODE_HAS_ZVBB_INTRINSICS
#define UNICODE_HAS_ZVBB_INTRINSICS 0
#endif

#ifndef UNICODE_IS_RVV
#if UNICODE_HAS_RVV_INTRINSICS && defined(__riscv_vector) && \
    defined(__riscv_v_min_vlen) && __riscv_v_min_vlen >= 128 && \
    defined(__riscv_v_elen) && __riscv_v_elen >= 64
#define UNICODE_IS_RVV 1
#else
#define UNICODE_IS_RVV 0
#endif
#endif

#ifndef UNICODE_IS_ZVBB
#if UNICODE_IS_RVV && UNICODE_HAS_ZVBB_INTRINSICS && defined(__riscv_zvbb) && \
    __riscv_zvbb >= 1000000
#define UNICODE_IS_ZVBB 1
#else
#define UNICODE_IS_ZVBB 0
#endif
#endif

#if KUMO_COMPILER_MSVC_ENV
// This is one case where we do not distinguish between
// regular visual studio and clang under visual studio.
// clang under Windows has _stricmp (like visual studio) but not strcasecmp
// (as clang normally has)
#define unicode_strcasecmp _stricmp
#define unicode_strncasecmp _strnicmp
#else
// The strcasecmp, strncasecmp, and strcasestr functions do not work with
// multibyte strings (e.g. UTF-8). So they are only useful for ASCII in our
// context.
// https://www.gnu.org/software/libunistring/manual/libunistring.html#char-_002a-strings
#define unicode_strcasecmp strcasecmp
#define unicode_strncasecmp strncasecmp
#endif

#if defined(__GNUC__) && !defined(__clang__)
#if __GNUC__ >= 11
#define UNICODE_GCC11ORMORE 1
#endif //  __GNUC__ >= 11
#if __GNUC__ == 10
#define UNICODE_GCC10 1
#endif //  __GNUC__ == 10
#if __GNUC__ < 10
#define UNICODE_GCC9OROLDER 1
#endif //  __GNUC__ == 10
#endif // defined(__GNUC__) && !defined(__clang__)

#endif // UNICODE_PORTABILITY_H
