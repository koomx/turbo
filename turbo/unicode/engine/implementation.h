#ifndef UNICODE_IMPLEMENTATION_H
#define UNICODE_IMPLEMENTATION_H

#include <atomic>
#include <vector>
#include <turbo/unicode/engine/portability.h>
#include <turbo/unicode/text_encoding.h>
#include <turbo/unicode/error.h>
#include <turbo/arch/cpu_detect.h>
#include <turbo/bits/bits.h>
#include <string_view>

// these includes are needed for constexpr support. they are
// not part of the public api.
#include <turbo/bits/bits.h>
#include <turbo/unicode/scalar/ascii.h>
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
#include <turbo/unicode/api/wchar.h>
#include <turbo/unicode/engine/interface.h>


#endif // UNICODE_IMPLEMENTATION_H
