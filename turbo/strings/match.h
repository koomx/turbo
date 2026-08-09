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
//   assert(turbo::StrContains(s, sv));
//
// Note: The order of parameters in these functions is designed to mimic the
// order an equivalent member function would exhibit;
// e.g. `s.Contains(x)` ==> `turbo::StrContains(s, x).
#ifndef TURBO_STRINGS_MATCH_H_
#define TURBO_STRINGS_MATCH_H_

#include <cstring>

#include <string_view>

namespace turbo {

    // StrContains()
    //
    // Returns whether a given string `haystack` contains the substring `needle`.
    inline bool StrContains(std::string_view haystack,
        std::string_view needle) noexcept {
        return haystack.find(needle, 0) != haystack.npos;
    }

    inline bool StrContains(std::string_view haystack, char needle) noexcept {
        return haystack.find(needle) != haystack.npos;
    }

    // StartsWith()
    //
    // Returns whether a given string `text` begins with `prefix`.
    inline constexpr bool StartsWith(std::string_view text,
        std::string_view prefix) noexcept {
        if (prefix.empty()) {
            return true;
        }
        if (text.size() < prefix.size()) {
            return false;
        }
        std::string_view possible_match = text.substr(0, prefix.size());

        return possible_match == prefix;
    }

    // EndsWith()
    //
    // Returns whether a given string `text` ends with `suffix`.
    inline constexpr bool EndsWith(std::string_view text,
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
    // StrContainsIgnoreCase()
    //
    // Returns whether a given ASCII string `haystack` contains the ASCII substring
    // `needle`, ignoring case in the comparison.
    bool StrContainsIgnoreCase(std::string_view haystack,
        std::string_view needle) noexcept;

    bool StrContainsIgnoreCase(std::string_view haystack,
        char needle) noexcept;

    // EqualsIgnoreCase()
    //
    // Returns whether given ASCII strings `piece1` and `piece2` are equal, ignoring
    // case in the comparison.
    bool EqualsIgnoreCase(std::string_view piece1,
        std::string_view piece2) noexcept;

    // StartsWithIgnoreCase()
    //
    // Returns whether a given ASCII string `text` starts with `prefix`,
    // ignoring case in the comparison.
    bool StartsWithIgnoreCase(std::string_view text,
        std::string_view prefix) noexcept;

    // EndsWithIgnoreCase()
    //
    // Returns whether a given ASCII string `text` ends with `suffix`, ignoring
    // case in the comparison.
    bool EndsWithIgnoreCase(std::string_view text,
        std::string_view suffix) noexcept;

    // Yields the longest prefix in common between both input strings.
    // Pointer-wise, the returned result is a subset of input "a".
    std::string_view FindLongestCommonPrefix(std::string_view a,
        std::string_view b);

    // Yields the longest suffix in common between both input strings.
    // Pointer-wise, the returned result is a subset of input "a".
    std::string_view FindLongestCommonSuffix(std::string_view a,
        std::string_view b);

} // namespace turbo

#endif // TURBO_STRINGS_MATCH_H_
