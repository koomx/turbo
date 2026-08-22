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
// File: strip.h
// -----------------------------------------------------------------------------
//
// This file contains various functions for stripping substrings from a string.
#ifndef TURBO_STRINGS_STRIP_H_
#define TURBO_STRINGS_STRIP_H_

#include <cstddef>
#include <string>

#include <string_view>
#include <turbo/base/nullability.h>
#include <turbo/macros/config.h>
#include <turbo/strings/ascii.h>
#include <turbo/strings/match.h>

namespace turbo {

    /// https://en.wikipedia.org/wiki/Whitespace_character
    /// with some adjustments.

    /// Code points: 0085 00A0 180E 2000..200A 2028..2029 200B..200D 202F 205F 2060 3000 FEFF
    /// The corresponding UTF-8 is: C285 C2A0 E1A08E E28080..E2808A E280A8..E280A9 E2808B..E2808D E280AF E2819F E281A0 E38080 EFBBBF

    /// We check for these bytes directly in UTF8 for simplicity reasons.

    /// C2
    ///    85
    ///    A0
    /// E1 A0 8E
    /// E2
    ///    80
    ///       80..8A
    ///       A8..A9
    ///       8B..8D
    ///       AF
    ///    81
    ///       9F
    ///       A0
    /// E3 80 80
    /// EF BB BF
    ///

    inline const char* trim_left_utf8(const char* pos, const char* end) {
        while (pos < end) {
            if (ascii_isspace(*pos)) {
                ++pos;
            } else {
                const uint8_t* upos = reinterpret_cast<const uint8_t*>(pos);

                if (pos + 1 < end && upos[0] == 0xC2 && (upos[1] == 0x85 || upos[1] == 0xA0)) {
                    pos += 2;
                } else if (pos + 2 < end
                    && ((upos[0] == 0xE1 && upos[1] == 0xA0 && upos[2] == 0x8E)
                        || (upos[0] == 0xE2
                            && ((upos[1] == 0x80
                                    && ((upos[2] >= 0x80 && upos[2] <= 0x8A)
                                        || (upos[2] >= 0xA8 && upos[2] <= 0xA9)
                                        || (upos[2] >= 0x8B && upos[2] <= 0x8D)
                                        || (upos[2] == 0xAF)))
                                || (upos[1] == 0x81 && (upos[2] == 0x9F || upos[2] == 0xA0))))
                        || (upos[0] == 0xE3 && upos[1] == 0x80 && upos[2] == 0x80)
                        || (upos[0] == 0xEF && upos[1] == 0xBB && upos[2] == 0xBF))) {
                    pos += 3;
                } else
                    break;
            }
        }

        return pos;
    }

    // Returns std::string_view with whitespace stripped from the beginning of the
    // given std::string_view.
    [[nodiscard]] inline std::string_view trim_left(
        std::string_view str KUMO_ATTRIBUTE_LIFETIME_BOUND) {
        auto it = std::find_if_not(str.begin(), str.end(), turbo::ascii_isspace);
        return str.substr(static_cast<size_t>(it - str.begin()));
    }

    // Strips in place whitespace from the beginning of the given string.
    inline void trim_left(std::string* turbo_nonnull str) {
        auto it = std::find_if_not(str->begin(), str->end(), turbo::ascii_isspace);
        str->erase(str->begin(), it);
    }

    // Returns std::string_view with whitespace stripped from the end of the given
    // std::string_view.
    [[nodiscard]] inline std::string_view trim_right(
        std::string_view str KUMO_ATTRIBUTE_LIFETIME_BOUND) {
        auto it = std::find_if_not(str.rbegin(), str.rend(), turbo::ascii_isspace);
        return str.substr(0, static_cast<size_t>(str.rend() - it));
    }

    // Strips in place whitespace from the end of the given string
    inline void trim_right(std::string* turbo_nonnull str) {
        auto it = std::find_if_not(str->rbegin(), str->rend(), turbo::ascii_isspace);
        str->erase(static_cast<size_t>(str->rend() - it));
    }

    // Returns std::string_view with whitespace stripped from both ends of the
    // given std::string_view.
    [[nodiscard]] inline std::string_view trim_all(
        std::string_view str KUMO_ATTRIBUTE_LIFETIME_BOUND) {
        return trim_right(trim_left(str));
    }

    // Strips in place whitespace from both ends of the given string
    inline void trim_all(std::string* turbo_nonnull str) {
        trim_right(str);
        trim_left(str);
    }

    [[nodiscard]] inline std::string trim_all_copy(std::string_view str) {
        return std::string(trim_all(str));
    }

    // Removes leading, trailing, and consecutive internal whitespace.
    void trim_in_place(std::string* turbo_nonnull str);

    // consume_prefix()
    //
    // Strips the `expected` prefix, if found, from the start of `str`.
    // If the operation succeeded, `true` is returned.  If not, `false`
    // is returned and `str` is not modified.
    //
    // Example:
    //
    //   std::string_view input("abc");
    //   EXPECT_TRUE(turbo::consume_prefix(&input, "a"));
    //   EXPECT_EQ(input, "bc");
    inline constexpr bool consume_prefix(std::string_view* turbo_nonnull str,
        std::string_view expected) {
        if (!turbo::starts_with(*str, expected))
            return false;
        str->remove_prefix(expected.size());
        return true;
    }
    // consume_suffix()
    //
    // Strips the `expected` suffix, if found, from the end of `str`.
    // If the operation succeeded, `true` is returned.  If not, `false`
    // is returned and `str` is not modified.
    //
    // Example:
    //
    //   std::string_view input("abcdef");
    //   EXPECT_TRUE(turbo::consume_suffix(&input, "def"));
    //   EXPECT_EQ(input, "abc");
    inline constexpr bool consume_suffix(std::string_view* turbo_nonnull str,
        std::string_view expected) {
        if (!turbo::ends_with(*str, expected))
            return false;
        str->remove_suffix(expected.size());
        return true;
    }

    // strip_prefix()
    //
    // Returns a view into the input string `str` with the given `prefix` removed,
    // but leaving the original string intact. If the prefix does not match at the
    // start of the string, returns the original string instead.
    [[nodiscard]] inline constexpr std::string_view strip_prefix(
        std::string_view str KUMO_ATTRIBUTE_LIFETIME_BOUND,
        std::string_view prefix) {
        if (turbo::starts_with(str, prefix))
            str.remove_prefix(prefix.size());
        return str;
    }

    // strip_suffix()
    //
    // Returns a view into the input string `str` with the given `suffix` removed,
    // but leaving the original string intact. If the suffix does not match at the
    // end of the string, returns the original string instead.
    [[nodiscard]] inline constexpr std::string_view strip_suffix(
        std::string_view str KUMO_ATTRIBUTE_LIFETIME_BOUND,
        std::string_view suffix) {
        if (turbo::ends_with(str, suffix))
            str.remove_suffix(suffix.size());
        return str;
    }

} // namespace turbo

#endif // TURBO_STRINGS_STRIP_H_
