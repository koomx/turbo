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

#include <cstring>
#include <string_view>
#include <turbo/macros/macros.h>
#include <turbo/strings/ascii.h>
#include <turbo/strings/match.h>

namespace turbo {

    /**
     * Returns true if the length of the domain name and its labels are according to
     * the specifications. The length of the domain must be 255 octets (253
     * characters not including the last 2 which are the empty label reserved at the
     * end). When the empty label is included (a dot at the end), the domain name
     * can have 254 characters. The length of a label must be at least 1 and at most
     * 63 characters.
     * @see section 3.1. of https://www.rfc-editor.org/rfc/rfc1034
     * @see https://www.unicode.org/reports/tr46/#ToASCII
     */

    KUMO_FORCE_INLINE constexpr bool verify_dns_length(
        std::string_view input) noexcept {
        if (input.back() == '.') {
            if (input.size() > 254)
                return false;
        } else if (input.size() > 253)
            return false;

        size_t start = 0;
        while (start < input.size()) {
            auto dot_location = input.find('.', start);
            // If not found, it's likely the end of the domain
            if (dot_location == std::string_view::npos)
                dot_location = input.size();

            auto label_size = dot_location - start;
            if (label_size > 63 || label_size == 0)
                return false;

            start = dot_location + 1;
        }

        return true;
    }


    //////////////////////////////////////////////////////////////////////////
    ///
    /// Returns true if an input is an ipv4 address. It is assumed that the string
    /// does not contain uppercase ASCII characters (the input should have been
    /// lowered cased before calling this function) and is not empty.
    ///
     KUMO_FORCE_INLINE bool is_ipv4(std::string_view view) noexcept {
        // The string is not empty and does not contain upper case ASCII characters.
        //
        // Optimization. To be considered as a possible ipv4, the string must end
        // with 'x' or a lowercase hex character.
        // Most of the time, this will be false so this simple check will save a lot
        // of effort.
        char last_char = view.back();
        // If the address ends with a dot, we need to prune it (special case).
        if (last_char == '.') {
            view.remove_suffix(1);
            if (view.empty()) {
                return false;
            }
            last_char = view.back();
        }
        bool possible_ipv4 = (last_char >= '0' && last_char <= '9') ||
                             (last_char >= 'a' && last_char <= 'f') ||
                             last_char == 'x';
        if (!possible_ipv4) {
            return false;
        }
        // From the last character, find the last dot.
        size_t last_dot = view.rfind('.');
        if (last_dot != std::string_view::npos) {
            // We have at least one dot.
            view = view.substr(last_dot + 1);
        }
        /** Optimization opportunity: we have basically identified the last number of
           the ipv4 if we return true here. We might as well parse it and have at
           least one number parsed when we get to parse_ipv4. */
        if (std::all_of(view.begin(), view.end(), turbo::ascii_isdigit)) {
            return true;
        }
        // It could be hex (0x), but not if there is a single character.
        if (view.size() == 1) {
            return false;
        }
        // It must start with 0x.
        if (!std::equal(view.begin(), view.begin() + 2, "0x")) {
            return false;
        }
        // We must allow "0x".
        if (view.size() == 2) {
            return true;
        }
        // We have 0x followed by some characters, we need to check that they are
        // hexadecimals.
        return std::all_of(view.begin() + 2, view.end(),
                           turbo::is_lowercase_hex);
    }

    inline constexpr bool is_windows_drive_letter(std::string_view input) noexcept {
        return input.size() >= 2 &&
               (turbo::ascii_isalpha(input[0]) && ((input[1] == ':') || (input[1] == '|'))) &&
               ((input.size() == 2) || (input[2] == '/' || input[2] == '\\' ||
                                        input[2] == '?' || input[2] == '#'));
    }

    inline constexpr bool is_normalized_windows_drive_letter(
    std::string_view input) noexcept {
        return input.size() >= 2 && (turbo::ascii_isalpha(input[0]) && (input[1] == ':'));
    }




    KUMO_FORCE_INLINE constexpr uint8_t path_signature(std::string_view input) noexcept {
        // for use with path_signature, we include all characters that need percent
        // encoding.
        static constexpr std::array<uint8_t, 256> path_signature_table =
            []() constexpr {
            std::array<uint8_t, 256> result{};
            for (size_t i = 0; i < 256; i++) {
                if (i <= 0x20 || i == 0x22 || i == 0x23 || i == 0x3c || i == 0x3e ||
                    i == 0x3f || i == 0x60 || i == 0x7b || i == 0x7d || i > 0x7e) {
                    result[i] = 1;
                    } else if (i == 0x25) {
                        result[i] = 8;
                    } else if (i == 0x2e) {
                        result[i] = 4;
                    } else if (i == 0x5c) {
                        result[i] = 2;
                    } else {
                        result[i] = 0;
                    }
            }
            return result;
            }();
        // The path percent-encode set is the query percent-encode set and U+003F (?),
        // U+0060 (`), U+007B ({), and U+007D (}). The query percent-encode set is the
        // C0 control percent-encode set and U+0020 SPACE, U+0022 ("), U+0023 (#),
        // U+003C (<), and U+003E (>). The C0 control percent-encode set are the C0
        // controls and all code points greater than U+007E (~).
        size_t i = 0;
        uint8_t accumulator{};
        for (; i + 7 < input.size(); i += 8) {
            accumulator |= uint8_t(path_signature_table[uint8_t(input[i])] |
                                   path_signature_table[uint8_t(input[i + 1])] |
                                   path_signature_table[uint8_t(input[i + 2])] |
                                   path_signature_table[uint8_t(input[i + 3])] |
                                   path_signature_table[uint8_t(input[i + 4])] |
                                   path_signature_table[uint8_t(input[i + 5])] |
                                   path_signature_table[uint8_t(input[i + 6])] |
                                   path_signature_table[uint8_t(input[i + 7])]);
        }
        for (; i < input.size(); i++) {
            accumulator |= uint8_t(path_signature_table[uint8_t(input[i])]);
        }
        return accumulator;
    }



    constexpr static std::array<uint8_t, 256> is_forbidden_domain_code_point_table =
        []() constexpr {
        std::array<uint8_t, 256> result{};
        for (uint8_t c : {'\0', '\x09', '\x0a', '\x0d', ' ', '#', '/', ':', '<',
                          '>', '?', '@', '[', '\\', ']', '^', '|', '%'}) {
            result[c] = true;
                          }
        for (uint8_t c = 0; c <= 32; c++) {
            result[c] = true;
        }
        for (size_t c = 127; c < 255; c++) {
            result[c] = true;
        }
        return result;
        }();

    static_assert(sizeof(is_forbidden_domain_code_point_table) == 256);

    KUMO_FORCE_INLINE constexpr bool contains_forbidden_domain_code_point(const char* input, size_t length) noexcept {
        size_t i = 0;
        uint8_t accumulator{};
        for (; i + 4 <= length; i += 4) {
            accumulator |= is_forbidden_domain_code_point_table[uint8_t(input[i])];
            accumulator |= is_forbidden_domain_code_point_table[uint8_t(input[i + 1])];
            accumulator |= is_forbidden_domain_code_point_table[uint8_t(input[i + 2])];
            accumulator |= is_forbidden_domain_code_point_table[uint8_t(input[i + 3])];
        }
        for (; i < length; i++) {
            accumulator |= is_forbidden_domain_code_point_table[uint8_t(input[i])];
        }
        return accumulator;
    }

    KUMO_FORCE_INLINE constexpr bool is_forbidden_domain_code_point(const char c) noexcept {
        return is_forbidden_domain_code_point_table[uint8_t(c)];
    }

    constexpr static std::array<uint8_t, 256>
    is_forbidden_domain_code_point_table_or_upper = []() constexpr {
        std::array<uint8_t, 256> result{};
        for (uint8_t c : {'\0', '\x09', '\x0a', '\x0d', ' ', '#', '/', ':', '<',
                          '>', '?', '@', '[', '\\', ']', '^', '|', '%'}) {
            result[c] = 1;
                          }
        for (uint8_t c = 'A'; c <= 'Z'; c++) {
            result[c] = 2;
        }
        for (uint8_t c = 0; c <= 32; c++) {
            result[c] = 1;
        }
        for (size_t c = 127; c < 255; c++) {
            result[c] = 1;
        }
        return result;
    }();

    KUMO_FORCE_INLINE constexpr uint8_t contains_forbidden_domain_code_point_or_upper(const char* input,
                                                  size_t length) noexcept {
        size_t i = 0;
        uint8_t accumulator{};
        for (; i + 4 <= length; i += 4) {
            accumulator |=
                is_forbidden_domain_code_point_table_or_upper[uint8_t(input[i])];
            accumulator |=
                is_forbidden_domain_code_point_table_or_upper[uint8_t(input[i + 1])];
            accumulator |=
                is_forbidden_domain_code_point_table_or_upper[uint8_t(input[i + 2])];
            accumulator |=
                is_forbidden_domain_code_point_table_or_upper[uint8_t(input[i + 3])];
        }
        for (; i < length; i++) {
            accumulator |=
                is_forbidden_domain_code_point_table_or_upper[uint8_t(input[i])];
        }
        return accumulator;
    }

    constexpr std::string_view table_is_double_dot_path_segment[] = {
        "..", "%2e.", ".%2e", "%2e%2e"};

    KUMO_FORCE_INLINE bool is_double_dot_path_segment(
        std::string_view input) noexcept {
        // This will catch most cases:
        // The length must be 2,4 or 6.
        // We divide by two and require
        // that the result be between 1 and 3 inclusively.
        uint64_t half_length = uint64_t(input.size()) / 2;
        if (half_length - 1 > 2) {
            return false;
        }
        // We have a string of length 2, 4 or 6.
        // We now check the first character:
        if ((input[0] != '.') && (input[0] != '%')) {
            return false;
        }
        // We are unlikely the get beyond this point.
        int hash_value = (input.size() + (unsigned)(input[0])) & 3;
        const std::string_view target = table_is_double_dot_path_segment[hash_value];
        if (target.size() != input.size()) {
            return false;
        }
        // We almost never get here.
        // Optimizing the rest is relatively unimportant.
        auto prefix_equal_unsafe = [](std::string_view a, std::string_view b) {
            uint16_t A, B;
            memcpy(&A, a.data(), sizeof(A));
            memcpy(&B, b.data(), sizeof(B));
            return A == B;
        };
        if (!prefix_equal_unsafe(input, target)) {
            return false;
        }
        for (size_t i = 2; i < input.size(); i++) {
            char c = input[i];
            if ((uint8_t((c | 0x20) - 0x61) <= 25 ? (c | 0x20) : c) != target[i]) {
                return false;
            }
        }
        return true;
        // The above code might be a bit better than the code below. Compilers
        // are not stupid and may use the fact that these strings have length 2,4 and
        // 6 and other tricks.
        // return input == ".." ||
        //  input == ".%2e" || input == ".%2E" ||
        //  input == "%2e." || input == "%2E." ||
        //  input == "%2e%2e" || input == "%2E%2E" || input == "%2E%2e" || input ==
        //  "%2e%2E";
    }


    KUMO_FORCE_INLINE constexpr bool is_forbidden_host_code_point(
        const char c) noexcept {
        // A forbidden host code point is U+0000 NULL, U+0009 TAB, U+000A LF, U+000D CR,
        // U+0020 SPACE, U+0023 (#), U+002F (/), U+003A (:), U+003C (<), U+003E (>),
        // U+003F (?), U+0040 (@), U+005B ([), U+005C (\), U+005D (]), U+005E (^), or
        // U+007C (|).
        constexpr static std::array<uint8_t, 256> is_forbidden_host_code_point_table =
            []() constexpr {
            std::array<uint8_t, 256> result{};
            for (uint8_t c : {'\0', '\x09', '\x0a', '\x0d', ' ', '#', '/', ':', '<',
                              '>', '?', '@', '[', '\\', ']', '^', '|'}) {
                result[c] = true;
                              }
            return result;
            }();

        return is_forbidden_host_code_point_table[uint8_t(c)];
    }




    KUMO_FORCE_INLINE constexpr bool is_valid_schema_alnum(const char c) noexcept {
        // std::isalnum(c) || c == '+' || c == '-' || c == '.') is true for
        constexpr static std::array<bool, 256> is_alnum_plus_table = []() constexpr {
            std::array<bool, 256> result{};
            for (size_t c = 0; c < 256; c++) {
                result[c] = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
                            (c >= 'A' && c <= 'Z') || c == '+' || c == '-' || c == '.';
            }
            return result;
        }();

        return is_alnum_plus_table[uint8_t(c)];
        // A table is almost surely much faster than the
        // following under most compilers: return
        // return (std::isalnum(c) || c == '+' || c == '-' || c == '.');
    }

    KUMO_FORCE_INLINE constexpr bool is_single_dot_path_segment(std::string_view input) noexcept {
        return input == "." || input == "%2e" || input == "%2E";
    }
} // namespace turbo
