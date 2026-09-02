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

#include <turbo/unicode/api/ascii.h>

namespace turbo {

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

    bool ascii_has_upper_case(const char* input, size_t length);

    inline bool ascii_has_upper_case(std::string_view str) {
        return ascii_has_upper_case(str.data(), str.size());
    }

    bool ascii_has_lower_case(const char* input, size_t length);

    inline bool ascii_has_lower_case(std::string_view str) {
        return ascii_has_lower_case(str.data(), str.size());
    }
} // namespace turbo

#endif // TURBO_STRINGS_ASCII_H_
