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
// File: match.h
// -----------------------------------------------------------------------------
//
// This file contains simple utilities for performing string matching checks.
// All of these function parameters are specified as `std::string_view`,
// meaning that these functions can accept `std::string`, `std::string_view` or
// NUL-terminated C-style strings.
//
// Examples:
//   std::string s = "foo";
//   std::string_view sv = "f";
//   assert(turbo::str_contains(s, sv));
//
// Note: The order of parameters in these functions is designed to mimic the
// order an equivalent member function would exhibit;
// e.g. `s.Contains(x)` ==> `turbo::str_contains(s, x).
#ifndef TURBO_STRINGS_MATCH_H_
#define TURBO_STRINGS_MATCH_H_

#include <cstring>

#include <string_view>

namespace turbo {

    // str_contains()
    //
    // Returns whether a given string `haystack` contains the substring `needle`.
    inline bool str_contains(std::string_view haystack,
        std::string_view needle) noexcept {
        return haystack.find(needle, 0) != haystack.npos;
    }

    inline bool str_contains(std::string_view haystack, char needle) noexcept {
        return haystack.find(needle) != haystack.npos;
    }

    // starts_with()
    //
    // Returns whether a given string `text` begins with `prefix`.
    inline constexpr bool starts_with(std::string_view text,std::string_view prefix) noexcept {
        if (prefix.empty()) {
            return true;
        }
        if (text.size() < prefix.size()) {
            return false;
        }
        std::string_view possible_match = text.substr(0, prefix.size());

        return possible_match == prefix;
    }

    inline constexpr bool starts_with(std::u32string_view text,std::u32string_view prefix) noexcept {
        if (prefix.empty()) {
            return true;
        }
        if (text.size() < prefix.size()) {
            return false;
        }
        auto possible_match = text.substr(0, prefix.size());

        return possible_match == prefix;
    }

    inline constexpr bool starts_with(std::u16string_view text,std::u16string_view prefix) noexcept {
        if (prefix.empty()) {
            return true;
        }
        if (text.size() < prefix.size()) {
            return false;
        }
        auto possible_match = text.substr(0, prefix.size());

        return possible_match == prefix;
    }

    inline constexpr bool starts_with(std::wstring_view text,std::wstring_view prefix) noexcept {
        if (prefix.empty()) {
            return true;
        }
        if (text.size() < prefix.size()) {
            return false;
        }
        auto possible_match = text.substr(0, prefix.size());

        return possible_match == prefix;
    }

    inline constexpr bool starts_with(std::string_view text,char prefix) noexcept {
        if (text.empty()) {
            return false;
        }
        return text.front() == prefix;
    }

    inline constexpr bool starts_with(std::u32string_view text,char32_t prefix) noexcept {
        if (text.empty()) {
            return false;
        }
        return text.front() == prefix;
    }

    inline constexpr bool starts_with(std::u16string_view text,char16_t prefix) noexcept {
        if (text.empty()) {
            return false;
        }
        return text.front() == prefix;
    }

    inline constexpr bool starts_with(std::wstring_view text,wchar_t prefix) noexcept {
        if (text.empty()) {
            return false;
        }
        return text.front() == prefix;
    }

    // ends_with()
    //
    // Returns whether a given string `text` ends with `suffix`.
    inline constexpr bool ends_with(std::string_view text,
        std::string_view suffix) noexcept {
        if (suffix.empty()) {
            return true;
        }
        if (text.size() < suffix.size()) {
            return false;
        }
        std::string_view possible_match = text.substr(text.size() - suffix.size());
        return possible_match == suffix;
    }

    inline constexpr bool ends_with(std::u32string_view text,
        std::u32string_view suffix) noexcept {
        if (suffix.empty()) {
            return true;
        }
        if (text.size() < suffix.size()) {
            return false;
        }
        auto possible_match = text.substr(text.size() - suffix.size());
        return possible_match == suffix;
    }

    inline constexpr bool ends_with(std::u16string_view text,
        std::u16string_view suffix) noexcept {
        if (suffix.empty()) {
            return true;
        }
        if (text.size() < suffix.size()) {
            return false;
        }
        auto possible_match = text.substr(text.size() - suffix.size());
        return possible_match == suffix;
    }

    inline constexpr bool ends_with(std::wstring_view text,
        std::wstring_view suffix) noexcept {
        if (suffix.empty()) {
            return true;
        }
        if (text.size() < suffix.size()) {
            return false;
        }
        auto possible_match = text.substr(text.size() - suffix.size());
        return possible_match == suffix;
    }

    inline constexpr bool ends_with(std::string_view text,
        char suffix) noexcept {

        if (text.empty()) {
            return false;
        }
        return text.back() == suffix;
    }

    inline constexpr bool ends_with(std::u32string_view text,
        char32_t suffix) noexcept {
        if (text.empty()) {
            return false;
        }
        return text.back() == suffix;
    }

    inline constexpr bool ends_with(std::u16string_view text,
        char16_t suffix) noexcept {
        if (text.empty()) {
            return false;
        }
        return text.back() == suffix;
    }

    inline constexpr bool ends_with(std::wstring_view text,
        wchar_t suffix) noexcept {
        if (text.empty()) {
            return false;
        }
        return text.back() == suffix;
    }

    // str_contains_ignore_case()
    //
    // Returns whether a given ASCII string `haystack` contains the ASCII substring
    // `needle`, ignoring case in the comparison.
    bool str_contains_ignore_case(std::string_view haystack,
        std::string_view needle) noexcept;

    bool str_contains_ignore_case(std::string_view haystack,
        char needle) noexcept;

    // equals_ignore_case()
    //
    // Returns whether given ASCII strings `piece1` and `piece2` are equal, ignoring
    // case in the comparison.
    bool equals_ignore_case(std::string_view piece1,
        std::string_view piece2) noexcept;

    // starts_with_ignore_case()
    //
    // Returns whether a given ASCII string `text` starts with `prefix`,
    // ignoring case in the comparison.
    bool starts_with_ignore_case(std::string_view text,
        std::string_view prefix) noexcept;

    // ends_with_ignore_case()
    //
    // Returns whether a given ASCII string `text` ends with `suffix`, ignoring
    // case in the comparison.
    bool ends_with_ignore_case(std::string_view text,
        std::string_view suffix) noexcept;

    // Yields the longest prefix in common between both input strings.
    // Pointer-wise, the returned result is a subset of input "a".
    std::string_view find_longest_common_prefix(std::string_view a,
        std::string_view b);

    // Yields the longest suffix in common between both input strings.
    // Pointer-wise, the returned result is a subset of input "a".
    std::string_view find_longest_common_suffix(std::string_view a,
        std::string_view b);

    // Like POSIX `fnmatch`, but:
    // * accepts `string_view`
    // * does not allocate any dynamic memory
    // * only supports * and ? wildcards and not bracket expressions [...]
    // * wildcards may match /
    // * no backslash-escaping
    bool fnmatch(std::string_view pattern, std::string_view str);



    inline bool has_hex_prefix_unsafe(std::string_view input) {
        // This is actually efficient code, see has_hex_prefix for the assembly.
        uint32_t value_one = 1;
        bool is_little_endian = (reinterpret_cast<char*>(&value_one)[0] == 1);
        uint16_t word0x{};
        std::memcpy(&word0x, "0x", 2);  // we would use bit_cast in C++20 and the
        // function could be constexpr.
        uint16_t two_first_bytes{};
        std::memcpy(&two_first_bytes, input.data(), 2);
        if (is_little_endian) {
            two_first_bytes |= 0x2000;
        } else {
            two_first_bytes |= 0x020;
        }
        return two_first_bytes == word0x;
    }

    inline bool has_hex_prefix(std::string_view input) {
        return input.size() >= 2 && has_hex_prefix_unsafe(input);
    }

} // namespace turbo

#endif // TURBO_STRINGS_MATCH_H_
