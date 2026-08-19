#ifndef SIMDUTF_IMPLEMENTATION_H
#define SIMDUTF_IMPLEMENTATION_H
#if !defined(SIMDUTF_NO_THREADS)
#include <atomic>
#endif
#ifdef SIMDUTF_INTERNAL_TESTS
#include <vector>
#endif
#include <turbo/unicode/engine/common_defs.h>
#include <turbo/unicode/engine/compiler_check.h>
#include <turbo/unicode/text_encoding.h>
#include <turbo/unicode/error.h>
#include <turbo/unicode/internal/isadetection.h>

#include <string_view>


#ifndef SIMDUTF_FEATURE_DETECT_ENCODING
#define SIMDUTF_FEATURE_DETECT_ENCODING 1
#endif
#ifndef SIMDUTF_FEATURE_ASCII
#define SIMDUTF_FEATURE_ASCII 1
#endif
#ifndef SIMDUTF_FEATURE_LATIN1
#define SIMDUTF_FEATURE_LATIN1 1
#endif
#ifndef SIMDUTF_FEATURE_UTF8
#define SIMDUTF_FEATURE_UTF8 1
#endif
#ifndef SIMDUTF_FEATURE_UTF16
#define SIMDUTF_FEATURE_UTF16 1
#endif
#ifndef SIMDUTF_FEATURE_UTF32
#define SIMDUTF_FEATURE_UTF32 1
#endif
#ifndef SIMDUTF_FEATURE_BASE64
#define SIMDUTF_FEATURE_BASE64 1
#endif

// these includes are needed for constexpr support. they are
// not part of the public api.
#include <turbo/unicode/scalar/swap_bytes.h>
#include <turbo/unicode/scalar/ascii.h>
#include <turbo/unicode/scalar/atomic_util.h>
#include <turbo/unicode/scalar/latin1.h>
#include <turbo/unicode/scalar/latin1_to_utf16/latin1_to_utf16.h>
#include <turbo/unicode/scalar/latin1_to_utf32/latin1_to_utf32.h>
#include <turbo/unicode/scalar/latin1_to_utf8/latin1_to_utf8.h>
#include <turbo/unicode/scalar/utf16.h>
#include <turbo/unicode/scalar/utf16_to_latin1/utf16_to_latin1.h>
#include <turbo/unicode/scalar/utf16_to_latin1/valid_utf16_to_latin1.h>
#include <turbo/unicode/scalar/utf16_to_utf32/utf16_to_utf32.h>
#include <turbo/unicode/scalar/utf16_to_utf32/valid_utf16_to_utf32.h>
#include <turbo/unicode/scalar/utf16_to_utf8/utf16_to_utf8.h>
#include <turbo/unicode/scalar/utf16_to_utf8/valid_utf16_to_utf8.h>
#include <turbo/unicode/scalar/utf32.h>
#include <turbo/unicode/scalar/utf32_to_latin1/utf32_to_latin1.h>
#include <turbo/unicode/scalar/utf32_to_latin1/valid_utf32_to_latin1.h>
#include <turbo/unicode/scalar/utf32_to_utf16/utf32_to_utf16.h>
#include <turbo/unicode/scalar/utf32_to_utf16/valid_utf32_to_utf16.h>
#include <turbo/unicode/scalar/utf32_to_utf8/utf32_to_utf8.h>
#include <turbo/unicode/scalar/utf32_to_utf8/valid_utf32_to_utf8.h>
#include <turbo/unicode/scalar/utf8.h>
#include <turbo/unicode/scalar/utf8_to_latin1/utf8_to_latin1.h>
#include <turbo/unicode/scalar/utf8_to_latin1/valid_utf8_to_latin1.h>
#include <turbo/unicode/scalar/utf8_to_utf16/utf8_to_utf16.h>
#include <turbo/unicode/scalar/utf8_to_utf16/valid_utf8_to_utf16.h>
#include <turbo/unicode/scalar/utf8_to_utf32/utf8_to_utf32.h>
#include <turbo/unicode/scalar/utf8_to_utf32/valid_utf8_to_utf32.h>

#include <turbo/unicode/api/detect.h>
#include <turbo/unicode/api/utf8.h>
#include <turbo/unicode/api/ascii.h>
#include <turbo/unicode/api/utf16.h>
#include <turbo/unicode/api/utf32.h>
#include <turbo/unicode/api/latin1.h>
#include <turbo/unicode/api/base64.h>
#include <turbo/unicode/engine/interface.h>
#include <turbo/unicode/engine/backend_select.h>

namespace turbo {

    /// @private
    namespace internal {


    } // namespace internal

} // namespace turbo

#endif // SIMDUTF_IMPLEMENTATION_H
