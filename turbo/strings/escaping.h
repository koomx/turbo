//
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
//
// -----------------------------------------------------------------------------
// File: escaping.h
// -----------------------------------------------------------------------------
//
// This header file contains string utilities involved in escaping and
// unescaping strings in various ways.

#ifndef TURBO_STRINGS_ESCAPING_H_
#define TURBO_STRINGS_ESCAPING_H_

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include <string_view>
#include <turbo/base/nullability.h>
#include <turbo/macros/config.h>
#include <turbo/strings/ascii.h>
#include <turbo/strings/str_join.h>

namespace turbo {
    // c_unescape()
    //
    // Unescapes a `source` string and copies it into `dest`, rewriting C-style
    // escape sequences (https://en.cppreference.com/w/cpp/language/escape) into
    // their proper code point equivalents, returning `true` if successful.
    //
    // The following unescape sequences can be handled:
    //
    //   * ASCII escape sequences ('\n','\r','\\', etc.) to their ASCII equivalents
    //   * Octal escape sequences ('\nnn') to byte nnn. The unescaped value must
    //     resolve to a single byte or an error will occur. E.g. values greater than
    //     0xff will produce an error.
    //   * Hexadecimal escape sequences ('\xnn') to byte nn. While an arbitrary
    //     number of following digits are allowed, the unescaped value must resolve
    //     to a single byte or an error will occur. E.g. '\x0045' is equivalent to
    //     '\x45', but '\x1234' will produce an error.
    //   * Unicode escape sequences ('\unnnn' for exactly four hex digits or
    //     '\Unnnnnnnn' for exactly eight hex digits, which will be encoded in
    //     UTF-8. (E.g., `\u2019` unescapes to the three bytes 0xE2, 0x80, and
    //     0x99).
    //
    // If any errors are encountered, this function returns `false`, leaving the
    // `dest` output parameter in an unspecified state, and stores the first
    // encountered error in `error`. To disable error reporting, set `error` to
    // `nullptr` or use the overload with no error reporting below.
    //
    // Example:
    //
    //   std::string s = "foo\\rbar\\nbaz\\t";
    //   std::string unescaped_s;
    //   if (!turbo::c_unescape(s, &unescaped_s)) {
    //     ...
    //   }
    //   EXPECT_EQ(unescaped_s, "foo\rbar\nbaz\t");
    bool c_unescape(std::string_view source, std::string* turbo_nonnull dest,
        std::string* turbo_nullable error);

    // Overload of `c_unescape()` with no error reporting.
    inline bool c_unescape(std::string_view source,
        std::string* turbo_nonnull dest) {
        return c_unescape(source, dest, nullptr);
    }

    // c_escape()
    //
    // Escapes a `src` string using C-style escapes sequences
    // (https://en.cppreference.com/w/cpp/language/escape), escaping other
    // non-printable/non-whitespace bytes as octal sequences (e.g. "\377").
    //
    // Example:
    //
    //   std::string s = "foo\rbar\tbaz\010\011\012\013\014\x0d\n";
    //   std::string escaped_s = turbo::c_escape(s);
    //   EXPECT_EQ(escaped_s, "foo\\rbar\\tbaz\\010\\t\\n\\013\\014\\r\\n");
    std::string c_escape(std::string_view src);

    // c_hex_escape()
    //
    // Escapes a `src` string using C-style escape sequences, escaping
    // other non-printable/non-whitespace bytes as hexadecimal sequences (e.g.
    // "\xFF").
    //
    // Example:
    //
    //   std::string s = "foo\rbar\tbaz\010\011\012\013\014\x0d\n";
    //   std::string escaped_s = turbo::c_hex_escape(s);
    //   EXPECT_EQ(escaped_s, "foo\\rbar\\tbaz\\x08\\t\\n\\x0b\\x0c\\r\\n");
    std::string c_hex_escape(std::string_view src);

    // utf8_safe_c_escape()
    //
    // Escapes a `src` string using C-style escape sequences, escaping bytes as
    // octal sequences, and passing through UTF-8 characters without conversion.
    // I.e., when encountering any bytes with their high bit set, this function
    // will not escape those values, whether or not they are valid UTF-8.
    std::string utf8_safe_c_escape(std::string_view src);

    // utf8_safe_chex_escape()
    //
    // Escapes a `src` string using C-style escape sequences, escaping bytes as
    // hexadecimal sequences, and passing through UTF-8 characters without
    // conversion.
    std::string utf8_safe_chex_escape(std::string_view src);

    // hex_string_to_bytes()
    //
    // Converts the hexadecimal encoded data in `hex` into raw bytes in the `bytes`
    // output string.  If `hex` does not consist of valid hexadecimal data, this
    // function returns false and leaves `bytes` in an unspecified state. Returns
    // true on success.
    [[nodiscard]] bool hex_string_to_bytes(std::string_view hex,
        std::string* turbo_nonnull bytes);

    // bytes_to_hex_string()
    //
    // Converts binary data into an ASCII text string, returning a string of size
    // `2*from.size()`.
    std::string bytes_to_hex_string(std::string_view from);

} // namespace turbo

#endif // TURBO_STRINGS_ESCAPING_H_
