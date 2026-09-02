// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>

#include <string_view>
#include <turbo/base/nullability.h>
#include <turbo/macros/config.h>
#include <turbo/base/resize_and_overwrite.h>
#include <turbo/unicode/engine/portability.h>
#include <turbo/unicode/text_encoding.h>
#include <turbo/unicode/error.h>

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

    // ascii_isword()
    //
    // Determines whether the given character is an alphanumeric character.
    inline bool ascii_isword(unsigned char c) {
        return (ascii_internal::kPropertyBits[c] & 0x04) != 0 || c == '_';
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

    KUMO_FORCE_INLINE constexpr bool is_lowercase_hex(const char c) noexcept {
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
    }

    /// Validate the ASCII string.
    ///
    /// Overridden by each implementation.
    ///
    /// @param buf the ASCII string to validate.
    /// @param len the length of the string in bytes.
    /// @return true if and only if the string is valid ASCII.
     [[nodiscard]] bool validate_ascii(const char* buf, size_t len) noexcept;

    inline bool validate_ascii(std::string_view buf) noexcept {
        return validate_ascii(buf.data(),buf.size());
    }

    inline bool constexpr validate_ascii(const char16_t* buf, size_t len) {
        auto end = buf + len;
        for (; buf != end; buf++) {
            if (static_cast<uint32_t>(*buf) >= 0x80) {
                return false;
            }
        }
        return true;
    }

    inline bool constexpr validate_ascii(std::u16string_view view) {
        return validate_ascii(view.data(), view.size());
    }


    inline bool constexpr validate_ascii(const char32_t* buf, size_t len) {
        auto end = buf + len;
        for (; buf != end; buf++) {
            if (static_cast<uint32_t>(*buf) >= 0x80) {
                return false;
            }
        }
        return true;
    }

    inline bool constexpr validate_ascii(std::u32string_view view) {
        return validate_ascii(view.data(), view.size());
    }



    /// Validate the ASCII string and stop on error. It might be faster than
    /// validate_utf8 when an error is expected to occur early.
    ///
    /// Overridden by each implementation.
    ///
    /// @param buf the ASCII string to validate.
    /// @param len the length of the string in bytes.
    /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of code units validated if
    /// successful.
     [[nodiscard]] UnicodeResult validate_ascii_with_errors(const char* buf,
        size_t len) noexcept;

}  // namespace turbo
