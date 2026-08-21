#ifndef UNICODE_WESTMERE_INTRINSICS_H
#define UNICODE_WESTMERE_INTRINSICS_H

#include <turbo/macros/macros/pragma/pragma.h>

#if KUMO_COMPILER_MSVC_ENV
// under clang within visual studio, this will include <x86intrin.h>
#include <intrin.h> // visual studio or clang
#else

#if UNICODE_GCC11ORMORE
// We should not get warnings while including <x86intrin.h> yet we do
// under some versions of GCC.
// If the x86intrin.h header has uninitialized values that are problematic,
// it is a GCC issue, we want to ignore these warnings.
KUMO_PRAGMA_DIAG_PUSH
KUMO_PRAGMA_DIAG_IGNORED("-Wuninitialized")
#endif

#include <x86intrin.h> // elsewhere

#if UNICODE_GCC11ORMORE
// cancels the suppression of the -Wuninitialized
KUMO_PRAGMA_DIAG_POP
#endif

#endif // KUMO_COMPILER_MSVC_ENV

#if KUMO_COMPILER_MSVC_CLANG
/// You are not supposed, normally, to include these
/// headers directly. Instead you should either include intrin.h
/// or x86intrin.h. However, when compiling with clang
/// under Windows (i.e., when _MSC_VER is set), these headers
/// only get included *if* the corresponding features are detected
/// from macros:
#include <smmintrin.h> // for _mm_alignr_epi8
#endif

#endif // UNICODE_WESTMERE_INTRINSICS_H
