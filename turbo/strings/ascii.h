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
// File: ascii.h
// -----------------------------------------------------------------------------
//
// This package contains functions operating on characters and strings
// restricted to standard ASCII. These include character classification
// functions analogous to those found in the ANSI C Standard Library <ctype.h>
// header file.
//
// C++ implementations provide <ctype.h> functionality based on their
// C environment locale. In general, reliance on such a locale is not ideal, as
// the locale standard is problematic (and may not return invariant information
// for the same character set, for example). These `ascii_*()` functions are
// hard-wired for standard ASCII, much faster, and guaranteed to behave
// consistently.  They will never be overloaded, nor will their function
// signature change.
//
// `ascii_isalnum()`, `ascii_isalpha()`, `ascii_isascii()`, `ascii_isblank()`,
// `ascii_iscntrl()`, `ascii_isdigit()`, `ascii_isgraph()`, `ascii_islower()`,
// `ascii_isprint()`, `ascii_ispunct()`, `ascii_isspace()`, `ascii_isupper()`,
// `ascii_isxdigit()`
//   Analogous to the <ctype.h> functions with similar names, these
//   functions take an unsigned char and return a bool, based on whether the
//   character matches the condition specified.
//
//   If the input character has a numerical value greater than 127, these
//   functions return `false`.
//
// `ascii_tolower()`, `ascii_toupper()`
//   Analogous to the <ctype.h> functions with similar names, these functions
//   take an unsigned char and return a char.
//
//   If the input character is not an ASCII {lower,upper}-case letter (including
//   numerical values greater than 127) then the functions return the same value
//   as the input character.

#ifndef TURBO_STRINGS_ASCII_H_
#define TURBO_STRINGS_ASCII_H_

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>

#include <string_view>
#include <turbo/base/nullability.h>
#include <turbo/macros/config.h>
#include <turbo/base/resize_and_overwrite.h>

namespace turbo {

    namespace ascii_internal {

        // Declaration for an array of bitfields holding character information.
        KUMO_DLL extern const unsigned char kPropertyBits[256];

        // Declaration for the array of characters to upper-case characters.
        KUMO_DLL extern const char kToUpper[256];

        // Declaration for the array of characters to lower-case characters.
        KUMO_DLL extern const char kToLower[256];

        void str_to_lower(char* turbo_nonnull dst, const char* turbo_nullable src,
            size_t n);

        void str_to_upper(char* turbo_nonnull dst, const char* turbo_nullable src,
            size_t n);

    } // namespace ascii_internal

    // ascii_isalpha()
    //
    // Determines whether the given character is an alphabetic character.
    inline bool ascii_isalpha(unsigned char c) {
        return (ascii_internal::kPropertyBits[c] & 0x01) != 0;
    }

    // ascii_isalnum()
    //
    // Determines whether the given character is an alphanumeric character.
    inline bool ascii_isalnum(unsigned char c) {
        return (ascii_internal::kPropertyBits[c] & 0x04) != 0;
    }

    // ascii_isspace()
    //
    // Determines whether the given character is a whitespace character (space,
    // tab, vertical tab, formfeed, linefeed, or carriage return).
    inline bool ascii_isspace(unsigned char c) {
        return (ascii_internal::kPropertyBits[c] & 0x08) != 0;
    }

    // ascii_ispunct()
    //
    // Determines whether the given character is a punctuation character.
    inline bool ascii_ispunct(unsigned char c) {
        return (ascii_internal::kPropertyBits[c] & 0x10) != 0;
    }

    // ascii_isblank()
    //
    // Determines whether the given character is a blank character (tab or space).
    inline bool ascii_isblank(unsigned char c) {
        return (ascii_internal::kPropertyBits[c] & 0x20) != 0;
    }

    // ascii_iscntrl()
    //
    // Determines whether the given character is a control character.
    inline bool ascii_iscntrl(unsigned char c) {
        return (ascii_internal::kPropertyBits[c] & 0x40) != 0;
    }

    // ascii_isxdigit()
    //
    // Determines whether the given character can be represented as a hexadecimal
    // digit character (i.e. {0-9} or {A-F} or {a-f}).
    inline bool ascii_isxdigit(unsigned char c) {
        return (ascii_internal::kPropertyBits[c] & 0x80) != 0;
    }

    // ascii_isdigit()
    //
    // Determines whether the given character can be represented as a decimal
    // digit character (i.e. {0-9}).
    inline constexpr bool ascii_isdigit(unsigned char c) {
        return c >= '0' && c <= '9';
    }

    // ascii_isprint()
    //
    // Determines whether the given character is printable, including spaces.
    inline constexpr bool ascii_isprint(unsigned char c) {
        return c >= 32 && c < 127;
    }

    // ascii_isgraph()
    //
    // Determines whether the given character has a graphical representation.
    inline constexpr bool ascii_isgraph(unsigned char c) {
        return c > 32 && c < 127;
    }

    // ascii_isupper()
    //
    // Determines whether the given character is uppercase.
    inline constexpr bool ascii_isupper(unsigned char c) {
        return c >= 'A' && c <= 'Z';
    }

    // ascii_islower()
    //
    // Determines whether the given character is lowercase.
    inline constexpr bool ascii_islower(unsigned char c) {
        return c >= 'a' && c <= 'z';
    }

    // ascii_isascii()
    //
    // Determines whether the given character is ASCII.
    inline constexpr bool ascii_isascii(unsigned char c) {
        return c < 128;
    }

    // ascii_tolower()
    //
    // Returns an ASCII character, converting to lowercase if uppercase is
    // passed. Note that character values > 127 are simply returned.
    inline char ascii_tolower(unsigned char c) {
        return ascii_internal::kToLower[c];
    }

    // Converts the characters in `s` to lowercase, changing the contents of `s`.
    void str_to_lower(std::string* turbo_nonnull s);

    // Creates a lowercase string from a given std::string_view.
    [[nodiscard]] inline std::string str_to_lower(std::string_view s) {
        std::string result;
        StringResizeAndOverwrite(result, s.size(), [s](char* buf, size_t buf_size) {
            ascii_internal::str_to_lower(buf, s.data(), s.size());
            return buf_size;
        });
        return result;
    }

    // Creates a lowercase string from a given std::string&&.
    //
    // (Template is used to lower priority of this overload.)
    template <int&... DoNotSpecify>
    [[nodiscard]] inline std::string str_to_lower(std::string&& s) {
        std::string result = std::move(s);
        turbo::str_to_lower(&result);
        return result;
    }

    // ascii_toupper()
    //
    // Returns the ASCII character, converting to upper-case if lower-case is
    // passed. Note that characters values > 127 are simply returned.
    inline char ascii_toupper(unsigned char c) {
        return ascii_internal::kToUpper[c];
    }

    // Converts the characters in `s` to uppercase, changing the contents of `s`.
    void str_to_upper(std::string* turbo_nonnull s);

    // Creates an uppercase string from a given std::string_view.
    [[nodiscard]] inline std::string str_to_upper(std::string_view s) {
        std::string result;
        StringResizeAndOverwrite(result, s.size(), [s](char* buf, size_t buf_size) {
            ascii_internal::str_to_upper(buf, s.data(), s.size());
            return buf_size;
        });
        return result;
    }

    // Creates an uppercase string from a given std::string&&.
    //
    // (Template is used to lower priority of this overload.)
    template <int&... DoNotSpecify>
    [[nodiscard]] inline std::string str_to_upper(std::string&& s) {
        std::string result = std::move(s);
        turbo::str_to_upper(&result);
        return result;
    }


    inline bool is_octal_digit(char c) {
        return ('0' <= c) && (c <= '7');
    }

    inline unsigned int hex_digit_to_int(char c) {
        static_assert('0' == 0x30 && 'A' == 0x41 && 'a' == 0x61,
            "Character set must be ASCII.");
        assert(turbo::ascii_isxdigit(static_cast<unsigned char>(c)));
        unsigned int x = static_cast<unsigned char>(c);
        if (x > '9') {
            x += 9;
        }
        return x & 0xf;
    }

    inline char int_to_hex_digit(int i) {
        assert(i >= 0 && i <= 15);
        return ((i < 10) ? (static_cast<char>(i) + '0')
                         : (static_cast<char>(i - 10) + 'A'));
    }

} // namespace turbo

#endif // TURBO_STRINGS_ASCII_H_
