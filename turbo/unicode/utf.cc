#include <turbo/unicode/utf.h>

#include <turbo/unicode/encoding_types.cpp>
#include <turbo/unicode/error.cpp>
// The large tables should be included once and they
// should not depend on a kernel.
#include <turbo/unicode/engine/tables/utf8_to_utf16_tables.h>
#include <turbo/unicode/engine/tables/utf16_to_utf8_tables.h>
#include <turbo/unicode/engine/tables/utf32_to_utf16_tables.h>
// End of tables.

// Implementations: they need to be setup before including
// scalar/* code, as the scalar code is sometimes enabled
// only for peculiar build targets.

// The best choice should always come first!
#ifndef SIMDUTF_REGULAR_VISUAL_STUDIO
SIMDUTF_DISABLE_UNUSED_WARNING
#endif
#include <turbo/unicode/engine/arm64.h>
#include <turbo/unicode/engine/icelake.h>
#include <turbo/unicode/engine/haswell.h>
#include <turbo/unicode/engine/westmere.h>
#include <turbo/unicode/engine/ppc64.h>
#include <turbo/unicode/engine/rvv.h>
#include <turbo/unicode/engine/lasx.h>
#include <turbo/unicode/engine/lsx.h>
#include <turbo/unicode/engine/fallback.h> // have it always last.
#ifndef SIMDUTF_REGULAR_VISUAL_STUDIO
SIMDUTF_POP_DISABLE_WARNINGS
#endif

// The scalar routines should be included once.
#include <turbo/unicode/engine/scalar/swap_bytes.h>
#if SIMDUTF_FEATURE_ASCII
  #include <turbo/unicode/engine/scalar/ascii.h>
#endif // SIMDUTF_FEATURE_ASCII
#if SIMDUTF_FEATURE_UTF8 || SIMDUTF_FEATURE_DETECT_ENCODING
  #include <turbo/unicode/engine/scalar/utf8.h>
#endif // SIMDUTF_FEATURE_UTF8 || SIMDUTF_FEATURE_DETECT_ENCODING
#if SIMDUTF_FEATURE_UTF16 || SIMDUTF_FEATURE_DETECT_ENCODING ||                \
    (SIMDUTF_FEATURE_UTF8 && SIMDUTF_FEATURE_LATIN1)
  #include <turbo/unicode/engine/scalar/utf16.h>
#endif // SIMDUTF_FEATURE_UTF16 || SIMDUTF_FEATURE_DETECT_ENCODING ||
       // (SIMDUTF_FEATURE_UTF8 && SIMDUTF_FEATURE_LATIN1)
#if SIMDUTF_FEATURE_UTF32 || SIMDUTF_FEATURE_DETECT_ENCODING
  #include <turbo/unicode/engine/scalar/utf32.h>
#endif // SIMDUTF_FEATURE_UTF32 || SIMDUTF_FEATURE_DETECT_ENCODING
#if SIMDUTF_FEATURE_LATIN1
  #include <turbo/unicode/engine/scalar/latin1.h>
#endif // SIMDUTF_FEATURE_LATIN1
#if SIMDUTF_FEATURE_BASE64
  #include <turbo/unicode/engine/scalar/base64.h>
#endif // SIMDUTF_FEATURE_BASE64

#if SIMDUTF_FEATURE_UTF8 && SIMDUTF_FEATURE_UTF32
  #include <turbo/unicode/engine/scalar/utf32_to_utf8/valid_utf32_to_utf8.h>
  #include <turbo/unicode/engine/scalar/utf32_to_utf8/utf32_to_utf8.h>
#endif // SIMDUTF_FEATURE_UTF8 && SIMDUTF_FEATURE_UTF32

#if SIMDUTF_FEATURE_UTF16 && SIMDUTF_FEATURE_UTF32
  #include <turbo/unicode/engine/scalar/utf32_to_utf16/valid_utf32_to_utf16.h>
  #include <turbo/unicode/engine/scalar/utf32_to_utf16/utf32_to_utf16.h>
#endif // SIMDUTF_FEATURE_UTF16 && SIMDUTF_FEATURE_UTF32

#if SIMDUTF_FEATURE_UTF8 && SIMDUTF_FEATURE_UTF16
  #include <turbo/unicode/engine/scalar/utf16_to_utf8/valid_utf16_to_utf8.h>
  #include <turbo/unicode/engine/scalar/utf16_to_utf8/utf16_to_utf8.h>
#endif // SIMDUTF_FEATURE_UTF8 && SIMDUTF_FEATURE_UTF16

#if SIMDUTF_FEATURE_UTF16 && SIMDUTF_FEATURE_UTF32
  #include <turbo/unicode/engine/scalar/utf16_to_utf32/valid_utf16_to_utf32.h>
  #include <turbo/unicode/engine/scalar/utf16_to_utf32/utf16_to_utf32.h>
#endif // SIMDUTF_FEATURE_UTF16 && SIMDUTF_FEATURE_UTF32

#if SIMDUTF_FEATURE_UTF8 &&                                                    \
    (SIMDUTF_FEATURE_UTF16 || SIMDUTF_FEATURE_UTF32 || SIMDUTF_FEATURE_LATIN1)
  #include <turbo/unicode/engine/scalar/utf8_to_utf16/valid_utf8_to_utf16.h>
  #include <turbo/unicode/engine/scalar/utf8_to_utf16/utf8_to_utf16.h>
#endif // SIMDUTF_FEATURE_UTF8 && (SIMDUTF_FEATURE_UTF16 ||
       // SIMDUTF_FEATURE_UTF32 || SIMDUTF_FEATURE_LATIN1)

#if SIMDUTF_FEATURE_UTF8 || SIMDUTF_FEATURE_UTF32
  #include <turbo/unicode/engine/scalar/utf8_to_utf32/valid_utf8_to_utf32.h>
  #include <turbo/unicode/engine/scalar/utf8_to_utf32/utf8_to_utf32.h>
#endif // SIMDUTF_FEATURE_UTF8 || SIMDUTF_FEATURE_UTF32

#if SIMDUTF_FEATURE_UTF8 && SIMDUTF_FEATURE_LATIN1
  #include <turbo/unicode/engine/scalar/latin1_to_utf8/latin1_to_utf8.h>
#endif // SIMDUTF_FEATURE_UTF8 && SIMDUTF_FEATURE_LATIN1
#if SIMDUTF_FEATURE_UTF16 && SIMDUTF_FEATURE_LATIN1
  #include <turbo/unicode/engine/scalar/latin1_to_utf16/latin1_to_utf16.h>
#endif // SIMDUTF_FEATURE_UTF16 && SIMDUTF_FEATURE_LATIN1
#if SIMDUTF_FEATURE_UTF32 && SIMDUTF_FEATURE_LATIN1
  #include <turbo/unicode/engine/scalar/latin1_to_utf32/latin1_to_utf32.h>
#endif // SIMDUTF_FEATURE_UTF32 && SIMDUTF_FEATURE_LATIN1

#if SIMDUTF_FEATURE_UTF8 && SIMDUTF_FEATURE_LATIN1
  #include <turbo/unicode/engine/scalar/utf8_to_latin1/utf8_to_latin1.h>
#endif // SIMDUTF_FEATURE_UTF8 && SIMDUTF_FEATURE_LATIN1
#if SIMDUTF_FEATURE_UTF16 && SIMDUTF_FEATURE_LATIN1
  #include <turbo/unicode/engine/scalar/utf16_to_latin1/utf16_to_latin1.h>
#endif // SIMDUTF_FEATURE_UTF16 && SIMDUTF_FEATURE_LATIN1
#if SIMDUTF_FEATURE_UTF32 && SIMDUTF_FEATURE_LATIN1
  #include <turbo/unicode/engine/scalar/utf32_to_latin1/utf32_to_latin1.h>
#endif // SIMDUTF_FEATURE_UTF32 && SIMDUTF_FEATURE_LATIN1

#if SIMDUTF_FEATURE_UTF8 && SIMDUTF_FEATURE_LATIN1
  #include <turbo/unicode/engine/scalar/utf8_to_latin1/valid_utf8_to_latin1.h>
#endif // SIMDUTF_FEATURE_UTF8 && SIMDUTF_FEATURE_LATIN1
#if SIMDUTF_FEATURE_UTF16 && SIMDUTF_FEATURE_LATIN1
  #include <turbo/unicode/engine/scalar/utf16_to_latin1/valid_utf16_to_latin1.h>
#endif // SIMDUTF_FEATURE_UTF16 && SIMDUTF_FEATURE_LATIN1
#if SIMDUTF_FEATURE_UTF32 && SIMDUTF_FEATURE_LATIN1
  #include <turbo/unicode/engine/scalar/utf32_to_latin1/valid_utf32_to_latin1.h>
#endif // SIMDUTF_FEATURE_UTF32 && SIMDUTF_FEATURE_LATIN1

#include <turbo/unicode/engine/implementation.cpp>

SIMDUTF_PUSH_DISABLE_WARNINGS
SIMDUTF_DISABLE_UNDESIRED_WARNINGS

#if SIMDUTF_IMPLEMENTATION_ARM64
  #include <turbo/unicode/engine/arm64/implementation.cpp>
#endif
#if SIMDUTF_IMPLEMENTATION_FALLBACK
  #include <turbo/unicode/engine/fallback/implementation.cpp>
#endif
#if SIMDUTF_IMPLEMENTATION_ICELAKE
  #include <turbo/unicode/engine/icelake/implementation.cpp>
#endif
#if SIMDUTF_IMPLEMENTATION_HASWELL
  #include <turbo/unicode/engine/haswell/implementation.cpp>
#endif
#if SIMDUTF_IMPLEMENTATION_PPC64
  #include <turbo/unicode/engine/ppc64/implementation.cpp>
#endif
#if SIMDUTF_IMPLEMENTATION_RVV
  #include <turbo/unicode/engine/rvv/implementation.cpp>
#endif
#if SIMDUTF_IMPLEMENTATION_WESTMERE
  #include <turbo/unicode/engine/westmere/implementation.cpp>
#endif
#if SIMDUTF_IMPLEMENTATION_LASX
  #include <turbo/unicode/engine/lasx/implementation.cpp>
#endif
#if SIMDUTF_IMPLEMENTATION_LSX
  #include <turbo/unicode/engine/lsx/implementation.cpp>
#endif

#include <turbo/unicode/utf_c.cc>
SIMDUTF_POP_DISABLE_WARNINGS
