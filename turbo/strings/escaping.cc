// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <turbo/strings/escaping.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
#include <optional>
#include <string>
#include <utility>

#include <string_view>
#include <turbo/base/internal/raw_logging.h>
#include <turbo/base/nullability.h>
#include <turbo/bits/endian.h>
#include <turbo/bits/unaligned_access.h>
#include <turbo/unicode/api/utf32.h>
#include <turbo/unicode/api/wchar.h>
#include <turbo/macros/config.h>
#include <turbo/strings/ascii.h>
#include <turbo/strings/charset.h>
#include <turbo/strings/internal/append_and_overwrite.h>
#include <turbo/strings/internal/escaping.h>
#include <turbo/strings/numbers.h>
#include <turbo/base/resize_and_overwrite.h>
#include <turbo/strings/str_cat.h>

namespace turbo {

    namespace {

        // These are used for the leave_nulls_escaped argument to CUnescapeInternal().
        constexpr bool kUnescapeNulls = false;

        inline bool IsSurrogate(char32_t c, std::string_view src,
            std::string* turbo_nullable error) {
            if (c >= 0xD800 && c <= 0xDFFF) {
                if (error) {
                    *error = turbo::str_cat("invalid surrogate character (0xD800-DFFF): \\",
                        src);
                }
                return true;
            }
            return false;
        }

        // ----------------------------------------------------------------------
        // CUnescapeInternal()
        //    Implements both c_unescape() and CUnescapeForNullTerminatedString().
        //
        //    Unescapes C escape sequences and is the reverse of c_escape().
        //
        //    If `src` is valid, stores the unescaped string in `dst` and the length of
        //    unescaped string in `dst_size`, and returns true. Otherwise returns false
        //    and optionally stores the error description in `error`. Set `error` to
        //    nullptr to disable error reporting.
        //
        //    `src` and `dst` may use the same underlying buffer (but keep in mind
        //    that if this returns an error, it will leave both `src` and `dst` in
        //    an unspecified state because they are using the same underlying buffer.)
        //    `dst` must have at least as much space as `src`.
        // ----------------------------------------------------------------------

        bool CUnescapeInternal(std::string_view src, bool leave_nulls_escaped,
            char* turbo_nonnull dst, size_t* turbo_nonnull dst_size,
            std::string* turbo_nullable error) {
            std::string_view::size_type p = 0; // Current src position.
            size_t d = 0; // Current dst position.

            // When unescaping in-place, skip any prefix that does not have escaping.
            if (src.data() == dst) {
                while (p < src.size() && src[p] != '\\')
                    p++, d++;
            }

            while (p < src.size()) {
                if (src[p] != '\\') {
                    dst[d++] = src[p++];
                } else {
                    if (++p >= src.size()) { // skip past the '\\'
                        if (error != nullptr) {
                            *error = "String cannot end with \\";
                        }
                        return false;
                    }
                    switch (src[p]) {
                        // clang-format off
        case 'a':  dst[d++] = '\a';  break;
        case 'b':  dst[d++] = '\b';  break;
        case 'f':  dst[d++] = '\f';  break;
        case 'n':  dst[d++] = '\n';  break;
        case 'r':  dst[d++] = '\r';  break;
        case 't':  dst[d++] = '\t';  break;
        case 'v':  dst[d++] = '\v';  break;
        case '\\': dst[d++] = '\\';  break;
        case '?':  dst[d++] = '\?';  break;
        case '\'': dst[d++] = '\'';  break;
        case '"':  dst[d++] = '\"';  break;
                    // clang-format on
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7': {
                        // octal digit: 1 to 3 digits
                        auto octal_start = p;
                        unsigned int ch = static_cast<unsigned int>(src[p] - '0'); // digit 1
                        if (p + 1 < src.size() && is_octal_digit(src[p + 1]))
                            ch = ch * 8 + static_cast<unsigned int>(src[++p] - '0'); // digit 2
                        if (p + 1 < src.size() && is_octal_digit(src[p + 1]))
                            ch = ch * 8 + static_cast<unsigned int>(src[++p] - '0'); // digit 3
                        if (ch > 0xff) {
                            if (error != nullptr) {
                                *error = "Value of \\" + std::string(src.substr(octal_start, p + 1 - octal_start)) + " exceeds 0xff";
                            }
                            return false;
                        }
                        if ((ch == 0) && leave_nulls_escaped) {
                            // Copy the escape sequence for the null character
                            dst[d++] = '\\';
                            while (octal_start <= p) {
                                dst[d++] = src[octal_start++];
                            }
                            break;
                        }
                        dst[d++] = static_cast<char>(ch);
                        break;
                    }
                    case 'x':
                    case 'X': {
                        if (p + 1 >= src.size()) {
                            if (error != nullptr) {
                                *error = "String cannot end with \\x";
                            }
                            return false;
                        } else if (!turbo::ascii_isxdigit(
                                       static_cast<unsigned char>(src[p + 1]))) {
                            if (error != nullptr) {
                                *error = "\\x cannot be followed by a non-hex digit";
                            }
                            return false;
                        }
                        unsigned int ch = 0;
                        auto hex_start = p;
                        while (p + 1 < src.size() && turbo::ascii_isxdigit(static_cast<unsigned char>(src[p + 1]))) {
                            // Arbitrarily many hex digits
                            ch = (ch << 4) + hex_digit_to_int(src[++p]);
                            // If ch was 0xFF at the start of this loop, the most can it can be
                            // here is (0xFF << 4) + 0xF, which is 4095, thus ch cannot overflow
                            // 32-bits here. The check below is sufficient.
                            if (ch > 0xFF) {
                                if (error != nullptr) {
                                    *error = "Value of \\" + std::string(src.substr(hex_start, p + 1 - hex_start)) + " exceeds 0xff";
                                }
                                return false;
                            }
                        }
                        if ((ch == 0) && leave_nulls_escaped) {
                            // Copy the escape sequence for the null character
                            dst[d++] = '\\';
                            while (hex_start <= p) {
                                dst[d++] = src[hex_start++];
                            }
                            break;
                        }
                        dst[d++] = static_cast<char>(ch);
                        break;
                    }
                    case 'u': {
                        // \uhhhh => convert 4 hex digits to UTF-8
                        char32_t rune = 0;
                        auto hex_start = p;
                        if (p + 4 >= src.size()) {
                            if (error != nullptr) {
                                *error = "\\u must be followed by 4 hex digits";
                            }
                            return false;
                        }
                        for (int i = 0; i < 4; ++i) {
                            // Look one char ahead.
                            if (turbo::ascii_isxdigit(static_cast<unsigned char>(src[p + 1]))) {
                                rune = (rune << 4) + hex_digit_to_int(src[++p]);
                            } else {
                                if (error != nullptr) {
                                    *error = "\\u must be followed by 4 hex digits: \\" + std::string(src.substr(hex_start, p + 1 - hex_start));
                                }
                                return false;
                            }
                        }
                        if ((rune == 0) && leave_nulls_escaped) {
                            // Copy the escape sequence for the null character
                            dst[d++] = '\\';
                            while (hex_start <= p) {
                                dst[d++] = src[hex_start++];
                            }
                            break;
                        }
                        if (IsSurrogate(rune, src.substr(hex_start, 5), error)) {
                            return false;
                        }
                        d += encode_utf32_to_utf8(dst + d, rune);
                        break;
                    }
                    case 'U': {
                        // \Uhhhhhhhh => convert 8 hex digits to UTF-8
                        char32_t rune = 0;
                        auto hex_start = p;
                        if (p + 8 >= src.size()) {
                            if (error != nullptr) {
                                *error = "\\U must be followed by 8 hex digits";
                            }
                            return false;
                        }
                        for (int i = 0; i < 8; ++i) {
                            // Look one char ahead.
                            if (turbo::ascii_isxdigit(static_cast<unsigned char>(src[p + 1]))) {
                                // Don't change rune until we're sure this
                                // is within the Unicode limit, but do advance p.
                                uint32_t newrune = (rune << 4) + hex_digit_to_int(src[++p]);
                                if (newrune > 0x10FFFF) {
                                    if (error != nullptr) {
                                        *error = "Value of \\" + std::string(src.substr(hex_start, p + 1 - hex_start)) + " exceeds Unicode limit (0x10FFFF)";
                                    }
                                    return false;
                                } else {
                                    rune = newrune;
                                }
                            } else {
                                if (error != nullptr) {
                                    *error = "\\U must be followed by 8 hex digits: \\" + std::string(src.substr(hex_start, p + 1 - hex_start));
                                }
                                return false;
                            }
                        }
                        if ((rune == 0) && leave_nulls_escaped) {
                            // Copy the escape sequence for the null character
                            dst[d++] = '\\';
                            // U00000000
                            while (hex_start <= p) {
                                dst[d++] = src[hex_start++];
                            }
                            break;
                        }
                        if (IsSurrogate(rune, src.substr(hex_start, 9), error)) {
                            return false;
                        }
                        d += encode_utf32_to_utf8(dst + d, rune);
                        break;
                    }
                    default: {
                        if (error != nullptr) {
                            *error = std::string("Unknown escape sequence: \\") + src[p];
                        }
                        return false;
                    }
                    }
                    p++; // Read past letter we escaped.
                }
            }

            *dst_size = d;
            return true;
        }

        // ----------------------------------------------------------------------
        // c_escape()
        // c_hex_escape()
        // utf8_safe_c_escape()
        // utf8_safe_chex_escape()
        //    Escapes 'src' using C-style escape sequences.  This is useful for
        //    preparing query flags.  The 'Hex' version uses hexadecimal rather than
        //    octal sequences.  The 'Utf8Safe' version does not touch UTF-8 bytes.
        //
        //    Escaped chars: \n, \r, \t, ", ', \, and !turbo::ascii_isprint().
        // ----------------------------------------------------------------------
        std::string CEscapeInternal(std::string_view src, bool use_hex,
            bool utf8_safe) {
            std::string dest;
            bool last_hex_escape = false; // true if last output char was \xNN.

            for (char c : src) {
                bool is_hex_escape = false;
                switch (c) {
                case '\n':
                    dest.append("\\"
                                "n");
                    break;
                case '\r':
                    dest.append("\\"
                                "r");
                    break;
                case '\t':
                    dest.append("\\"
                                "t");
                    break;
                case '\"':
                    dest.append("\\"
                                "\"");
                    break;
                case '\'':
                    dest.append("\\"
                                "'");
                    break;
                case '\\':
                    dest.append("\\"
                                "\\");
                    break;
                default: {
                    // Note that if we emit \xNN and the src character after that is a hex
                    // digit then that digit must be escaped too to prevent it being
                    // interpreted as part of the character code by C.
                    const unsigned char uc = static_cast<unsigned char>(c);
                    if ((!utf8_safe || uc < 0x80) && (!turbo::ascii_isprint(uc) || (last_hex_escape && turbo::ascii_isxdigit(uc)))) {
                        if (use_hex) {
                            dest.append("\\"
                                        "x");
                            dest.push_back(format_internal::kHexChar[uc / 16]);
                            dest.push_back(format_internal::kHexChar[uc % 16]);
                            is_hex_escape = true;
                        } else {
                            dest.append("\\");
                            dest.push_back(format_internal::kHexChar[uc / 64]);
                            dest.push_back(format_internal::kHexChar[(uc % 64) / 8]);
                            dest.push_back(format_internal::kHexChar[uc % 8]);
                        }
                    } else {
                        dest.push_back(c);
                        break;
                    }
                }
                }
                last_hex_escape = is_hex_escape;
            }

            return dest;
        }

        /* clang-format off */
constexpr std::array<unsigned char, 256> kCEscapedLen = {
    4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 2, 4, 4, 2, 4, 4,  // \t, \n, \r
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1,  // ", '
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // '0'..'9'
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 'A'..'O'
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1,  // 'P'..'Z', '\'
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 'a'..'o'
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4,  // 'p'..'z', DEL
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
};
        /* clang-format on */

        constexpr std::array<std::array<char, 4>, 256> kCEscapedSequence = []() {
            std::array<std::array<char, 4>, 256> a { };
            for (size_t c = 0; c < 256; ++c) {
                size_t char_len = kCEscapedLen[c];
                if (char_len == 1) {
                    a[c][0] = static_cast<char>(c);
                } else if (char_len == 2) {
                    a[c][0] = '\\';
                    // clang-format off
      switch (c) {
        case '\n': a[c][1] = 'n'; break;
        case '\r': a[c][1] = 'r'; break;
        case '\t': a[c][1] = 't'; break;
        case '\"': a[c][1] = '\"'; break;
        case '\'': a[c][1] = '\''; break;
        case '\\': a[c][1] = '\\'; break;
      }
                    // clang-format on
                } else {
                    assert(char_len == 4);
                    // A backslash followed by the octal value of the byte.
                    a[c][0] = '\\';
                    a[c][1] = static_cast<char>('0' + (c / 64));
                    a[c][2] = static_cast<char>('0' + ((c % 64) / 8));
                    a[c][3] = static_cast<char>('0' + (c % 8));
                }
            }
            return a;
        }();

        // Calculates the length of the C-style escaped version of 'src'.
        // Assumes that non-printable characters are escaped using octal sequences, and
        // that UTF-8 bytes are not handled specially.
        inline size_t CEscapedLength(std::string_view src) {
            size_t escaped_len = 0;
            // The maximum value of kCEscapedLen[x] is 4, so we can escape any string of
            // length size_t_max/4 without checking for overflow.
            size_t unchecked_limit = std::min<size_t>(src.size(), std::numeric_limits<size_t>::max() / 4);
            size_t i = 0;
            while (i < unchecked_limit) {
                // Common case: No need to check for overflow.
                escaped_len += kCEscapedLen[static_cast<unsigned char>(src[i++])];
            }
            while (i < src.size()) {
                // Beyond unchecked_limit we need to check for overflow before adding.
                size_t char_len = kCEscapedLen[static_cast<unsigned char>(src[i++])];
                TURBO_INTERNAL_CHECK(
                    escaped_len <= std::numeric_limits<size_t>::max() - char_len,
                    "escaped_len overflow");
                escaped_len += char_len;
            }
            return escaped_len;
        }

        void CEscapeAndAppendInternal(std::string_view src,
            std::string* turbo_nonnull dest) {
            size_t escaped_len = CEscapedLength(src);
            if (escaped_len == src.size()) {
                dest->append(src.data(), src.size());
                return;
            }

            // The small `memcpy` is faster when the size is a compile-time constant, so
            // keep 3 slop bytes so that we can call memcpy with size=4.
            constexpr size_t kSlopBytes = 3;
            TURBO_INTERNAL_CHECK(
                escaped_len <= std::numeric_limits<size_t>::max() - kSlopBytes,
                "c_escape length overflow");
            size_t append_buf_len = escaped_len + kSlopBytes;
            strings_internal::StringAppendAndOverwrite(
                *dest, append_buf_len, [src, escaped_len](char* append_ptr, size_t) {
                    for (char c : src) {
                        unsigned char uc = static_cast<unsigned char>(c);
                        memcpy(append_ptr, kCEscapedSequence[uc].data(), 4);
                        append_ptr += kCEscapedLen[uc];
                    }
                    return escaped_len;
                });
        }



        // The arrays below map base64-escaped characters back to their original values.
        // For the inverse case, see k(WebSafe)Base64Chars in the internal
        // escaping.cc.
        // These arrays were generated by the following inversion code:
        // #include <sys/time.h>
        // #include <stdlib.h>
        // #include <string.h>
        // main()
        // {
        //   static const char Base64[] =
        //     "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        //   char* pos;
        //   int idx, i, j;
        //   printf("    ");
        //   for (i = 0; i < 255; i += 8) {
        //     for (j = i; j < i + 8; j++) {
        //       pos = strchr(Base64, j);
        //       if ((pos == nullptr) || (j == 0))
        //         idx = -1;
        //       else
        //         idx = pos - Base64;
        //       if (idx == -1)
        //         printf(" %2d,     ", idx);
        //       else
        //         printf(" %2d/*%c*/,", idx, j);
        //     }
        //     printf("\n    ");
        //   }
        // }
        //
        // where the value of "Base64[]" was replaced by one of k(WebSafe)Base64Chars
        // in the internal escaping.cc.

        /* clang-format off */
constexpr std::array<uint8_t, 256> kHexValueLenient = {
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,  1,  2,  3,  4,  5,  6, 7, 8, 9, 0, 0, 0, 0, 0, 0,  // '0'..'9'
    0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 'A'..'F'
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 'a'..'f'
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

constexpr std::array<int8_t, 256> kHexValueStrict = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,  // '0'..'9'
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 'A'..'F'
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 'a'..'f'
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};
        /* clang-format on */

        // This is a templated function so that T can be either a char*
        // or a string.  This works because we use the [] operator to access
        // individual characters at a time.
        template <typename T>
        void HexStringToBytesInternal(const char* turbo_nullable from, T to,
            size_t num) {
            for (size_t i = 0; i < num; i++) {
                to[i] = static_cast<char>(kHexValueLenient[from[i * 2] & 0xFF] << 4) + static_cast<char>(kHexValueLenient[from[i * 2 + 1] & 0xFF]);
            }
        }

        void BytesToHexStringInternal(const unsigned char* turbo_nullable src,
            char* dest, size_t num) {
            for (auto src_ptr = src; src_ptr != (src + num); ++src_ptr, dest += 2) {
                const char* hex_p = &format_internal::kHexTable[*src_ptr * 2];
                std::copy(hex_p, hex_p + 2, dest);
            }
        }

    } // namespace

    // ----------------------------------------------------------------------
    // c_unescape()
    //
    // See CUnescapeInternal() for implementation details.
    // ----------------------------------------------------------------------

    bool c_unescape(std::string_view source, std::string* turbo_nonnull dest,
        std::string* turbo_nullable error) {
        bool success;

        // `c_unescape()` allows for in-place unescaping, which means `source` may
        // alias `*dest`.  However, turbo::string_resize_and_overwrite() invalidates all
        // iterators, pointers, and references into the string, regardless whether
        // reallocation occurs. Therefore we need to avoid calling
        // turbo::string_resize_and_overwrite() when `source.data() ==
        // dest->data()`. Comparing the sizes is sufficient to cover this case.
        if (dest->size() >= source.size()) {
            size_t dest_size = 0;
            success = CUnescapeInternal(source, kUnescapeNulls, dest->data(),
                &dest_size, error);
            KUMO_ASSERT(dest_size <= dest->size());
            dest->erase(dest_size);
        } else {
            string_resize_and_overwrite(
                *dest, source.size(),
                [source, error, &success](char* buf, size_t buf_size) {
                    size_t dest_size = 0;
                    success = CUnescapeInternal(source, kUnescapeNulls, buf, &dest_size, error);
                    KUMO_ASSERT(dest_size <= buf_size);
                    return dest_size;
                });
        }
        return success;
    }

    std::string c_escape(std::string_view src) {
        std::string dest;
        CEscapeAndAppendInternal(src, &dest);
        return dest;
    }

    std::string c_hex_escape(std::string_view src) {
        return CEscapeInternal(src, true, false);
    }

    std::string utf8_safe_c_escape(std::string_view src) {
        return CEscapeInternal(src, false, true);
    }

    std::string utf8_safe_chex_escape(std::string_view src) {
        return CEscapeInternal(src, true, true);
    }

    bool hex_string_to_bytes(std::string_view hex, std::string* turbo_nonnull bytes) {
        std::string output;

        size_t num_bytes = hex.size() / 2;
        if (hex.size() != num_bytes * 2) {
            return false;
        }

        string_resize_and_overwrite(
            output, num_bytes, [hex](char* buf, size_t buf_size) {
                auto hex_p = hex.cbegin();
                for (size_t i = 0; i < buf_size; ++i) {
                    int h1 = turbo::kHexValueStrict[static_cast<size_t>(
                        static_cast<uint8_t>(*hex_p++))];
                    int h2 = turbo::kHexValueStrict[static_cast<size_t>(
                        static_cast<uint8_t>(*hex_p++))];
                    if (h1 == -1 || h2 == -1) {
                        return size_t { 0 };
                    }
                    buf[i] = static_cast<char>((h1 << 4) + h2);
                }
                return buf_size;
            });

        if (output.size() != num_bytes) {
            return false;
        }
        *bytes = std::move(output);
        return true;
    }

    std::string hex_string_to_bytes(std::string_view from) {
        std::string result;
        const auto num = from.size() / 2;
        string_resize_and_overwrite(result, num, [from](char* buf, size_t buf_size) {
            turbo::HexStringToBytesInternal<char*>(from.data(), buf, buf_size);
            return buf_size;
        });
        return result;
    }

    std::string bytes_to_hex_string(std::string_view from) {
        std::string result;
        TURBO_INTERNAL_CHECK(from.size() <= std::numeric_limits<size_t>::max() / 2,
            "bytes_to_hex_string() overflow");
        string_resize_and_overwrite(
            result, 2 * from.size(), [from](char* buf, size_t buf_size) {
                turbo::BytesToHexStringInternal(
                    reinterpret_cast<const unsigned char*>(from.data()), buf,
                    from.size());
                return buf_size;
            });
        return result;
    }


} // namespace turbo
