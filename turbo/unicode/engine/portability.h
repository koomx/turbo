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

// We are going to use runtime dispatch.
#if KUMO_ARCH_X86_64 || KUMO_SIMD_LSX
#ifdef __clang__
// clang does not have GCC push pop
// warning: clang attribute push can't be used within a namespace in clang
// up til 8.0 so UNICODE_TARGET_REGION and UNICODE_UNTARGET_REGION must be
// *outside* of a namespace.
#define UNICODE_TARGET_REGION(T)                    \
    _Pragma(KUMO_STRINGIFY(clang attribute push( \
        __attribute__((target(T))), apply_to = function)))
#define UNICODE_UNTARGET_REGION _Pragma("clang attribute pop")
#elif defined(__GNUC__)
// GCC is easier
#define UNICODE_TARGET_REGION(T) \
    _Pragma("GCC push_options") _Pragma(KUMO_STRINGIFY(GCC target(T)))
#define UNICODE_UNTARGET_REGION _Pragma("GCC pop_options")
#endif // clang then gcc

#endif // KUMO_ARCH_X86_64 || KUMO_SIMD_LSX

// Default target region macros don't do anything.
#ifndef UNICODE_TARGET_REGION
#define UNICODE_TARGET_REGION(T)
#define UNICODE_UNTARGET_REGION
#endif

// Is threading enabled?
#if defined(_REENTRANT) || defined(_MT)
#ifndef UNICODE_THREADS_ENABLED
#define UNICODE_THREADS_ENABLED
#endif
#endif

// workaround for large stack sizes under -O0.
// https://github.com/simdutf/simdutf/issues/691
#ifdef __APPLE__
#ifndef __OPTIMIZE__
// Apple systems have small stack sizes in secondary threads.
// Lack of compiler optimization may generate high stack usage.
// Users may want to disable threads for safety, but only when
// in debug mode which we detect by the fact that the __OPTIMIZE__
// macro is not defined.
#undef UNICODE_THREADS_ENABLED
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

#ifndef UNICODE_NO_THREADS
#define UNICODE_NO_THREADS 0
#endif

#endif // UNICODE_PORTABILITY_H
