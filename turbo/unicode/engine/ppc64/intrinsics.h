#ifndef UNICODE_PPC64_INTRINSICS_H
#define UNICODE_PPC64_INTRINSICS_H

#include <turbo/unicode/utf.h>

// This should be the correct header whether
// you use visual studio or other compilers.
#include <altivec.h>

// These are defined by altivec.h in GCC toolchain, it is safe to undef them.
#ifdef bool
#undef bool
#endif

#ifdef vector
#undef vector
#endif

#endif //  UNICODE_PPC64_INTRINSICS_H
