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
#include <optional>
#include <string>
#include <string_view>
#include <turbo/macros/macros.h>
#include <turbo/strings/find_symbols.h>
#include <turbo/strings/substring.h>
#include <turbo/uri/character_sets.h>
#include <turbo/uri/checkers.h>
#include <turbo/uri/scheme.h>
#include <turbo/uri/uri_error.h>

namespace turbo {

    /////////////////////////////////////////////////////////////////////////
    ///
    /// This function is used to prune a fragment from a url, and returning the
    /// removed string if input has fragment.
    ///
    ///  @details prune_hash seeks the first '#' and returns everything after it
    ///  as a string_view, and modifies (in place) the input so that it points at
    ///  everything before the '#'. If no '#' is found, the input is left unchanged
    ///  and std::nullopt is returned.
    ///
    ///  @attention The function is non-allocating and it does not throw.
    ///  @returns Note that the returned string_view might be empty!
    ///
    KUMO_FORCE_INLINE std::optional<std::string_view> prune_hash(
        std::string_view& input) noexcept {
        // compiles down to 20--30 instructions including a class to memchr (C
        // function). this function should be quite fast.
        size_t location_of_first = input.find('#');
        if (location_of_first == std::string_view::npos) {
            return std::nullopt;
        }
        std::string_view hash = input;
        hash.remove_prefix(location_of_first + 1);
        input.remove_suffix(input.size() - location_of_first);
        return hash;
    }

    //////////////////////////////////////////////////////////////////
    /// Defined by the URL specification, shorten a URLs paths.
    /// @see https://url.spec.whatwg.org/#shorten-a-urls-path
    /// @returns Returns true if path is shortened.
    KUMO_FORCE_INLINE bool shorten_path(std::string& path,
        turbo::SchemaType type) noexcept {
        size_t first_delimiter = path.find_first_of('/', 1);

        // Let path be url's path.
        // If url's scheme is "file", path's size is 1, and path[0] is a normalized
        // Windows drive letter, then return.
        if (type == turbo::SchemaType::FILE && first_delimiter == std::string_view::npos && !path.empty()) {
            if (turbo::is_normalized_windows_drive_letter(
                    turbo::subview(path, 1))) {
                return false;
            }
        }

        // Remove path's last item, if any.
        size_t last_delimiter = path.rfind('/');
        if (last_delimiter != std::string::npos) {
            path.erase(last_delimiter);
            return true;
        }

        return false;
    }

    //////////////////////////////////////////////////////////////////
    /// Defined by the URL specification, shorten a URLs paths.
    /// @see https://url.spec.whatwg.org/#shorten-a-urls-path
    /// @returns Returns true if path is shortened.
    KUMO_FORCE_INLINE bool shorten_path(std::string_view& path,
        turbo::SchemaType type) noexcept {
        size_t first_delimiter = path.find_first_of('/', 1);

        // Let path be url's path.
        // If url's scheme is "file", path's size is 1, and path[0] is a normalized
        // Windows drive letter, then return.
        if (type == turbo::SchemaType::FILE && first_delimiter == std::string_view::npos && !path.empty()) {
            if (turbo::is_normalized_windows_drive_letter(
                    turbo::subview(path, 1))) {
                return false;
            }
        }

        // Remove path's last item, if any.
        if (!path.empty()) {
            size_t slash_loc = path.rfind('/');
            if (slash_loc != std::string_view::npos) {
                path.remove_suffix(path.size() - slash_loc);
                return true;
            }
        }

        return false;
    }

    KUMO_FORCE_INLINE constexpr bool is_ascii_tab_or_newline(const char c) noexcept {
        return c == '\t' || c == '\n' || c == '\r';
    }

    KUMO_FORCE_INLINE void remove_ascii_tab_or_newline(std::string& input) noexcept {
        // if this ever becomes a performance issue, we could use an approach similar
        // to has_tabs_or_newline
        input.erase(std::remove_if(input.begin(), input.end(),
                        [](char c) {
                            return is_ascii_tab_or_newline(c);
                        }),
            input.end());
    }

    KUMO_FORCE_INLINE constexpr bool is_c0_control_or_space(const char c) noexcept {
        return (unsigned char)c <= ' ';
    }

    KUMO_FORCE_INLINE void trim_c0_whitespace(std::string_view& input) noexcept {
        while (!input.empty() && turbo::is_c0_control_or_space(input.front())) {
            input.remove_prefix(1);
        }
        while (!input.empty() && turbo::is_c0_control_or_space(input.back())) {
            input.remove_suffix(1);
        }
    }

    ////////////////////////////////////////////////////////
    ///
    /// Returns the index at which percent encoding should start, or (equivalently),
    /// the length of the prefix that does not require percent encoding.
    KUMO_FORCE_INLINE size_t percent_encode_index(const std::string_view input,
        const uint8_t character_set[]) {
        return std::distance(
            input.begin(),
            std::find_if(input.begin(), input.end(), [character_set](const char c) {
                return turbo::uri_charsets::bit_at(character_set, c);
            }));
    }

    void parse_prepared_path(std::string_view input,
        turbo::SchemaType type,
        std::string& path);

    std::string percent_encode(std::string_view input,
        const uint8_t character_set[]);

    std::string percent_encode(std::string_view input,
        const uint8_t character_set[], size_t index);

    template <bool append>
    bool percent_encode(const std::string_view input, const uint8_t character_set[],
        std::string& out) {
        auto pointer = std::find_if(input.begin(), input.end(), [character_set](const char c) {
            return turbo::uri_charsets::bit_at(character_set, c);
        });
        // Optimization: Don't iterate if percent encode is not required
        if (pointer == input.end()) {
            return false;
        }
        if (!append) {
            out.clear();
        }
        out.append(input.data(), std::distance(input.begin(), pointer));
        for (; pointer != input.end(); pointer++) {
            if (turbo::uri_charsets::bit_at(character_set, *pointer)) {
                out.append(turbo::uri_charsets::hex + uint8_t(*pointer) * 4, 3);
            } else {
                out += *pointer;
            }
        }
        return true;
    }

    unsigned constexpr convert_hex_to_binary(const char c) noexcept {
        constexpr static char hex_to_binary_table[] = {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 10, 11,
            12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15
        };
        return hex_to_binary_table[c - '0'];
    }

    std::string percent_decode(std::string_view input, size_t first_percent);

    bool to_ascii(std::optional<std::string>& out, std::string_view plain,
        size_t first_percent);

    bool to_lower_ascii(char* input, size_t length) noexcept;

    KUMO_FORCE_INLINE bool has_tabs_or_newline(std::string_view user_input) noexcept {
        auto begin = user_input.data();
        auto end = user_input.data() + user_input.size();
        find_first_symbols<'\t', '\n', '\r'>(begin, end) != end;
    }

    // credit: @the-moisrex recommended a table-based approach
    KUMO_FORCE_INLINE size_t find_authority_delimiter_special(std::string_view view) noexcept {
        // @ / \\ ?
        static constexpr std::array<uint8_t, 256> authority_delimiter_special =
            []() constexpr {
                std::array<uint8_t, 256> result { };
                for (uint8_t i : { '@', '/', '\\', '?' }) {
                    result[i] = 1;
                }
                return result;
            }();
        // performance note: we might be able to gain further performance
        // with SIMD instrinsics.
        for (auto pos = view.begin(); pos != view.end(); ++pos) {
            if (authority_delimiter_special[(uint8_t)*pos]) {
                return pos - view.begin();
            }
        }
        return size_t(view.size());
    }

    // credit: @the-moisrex recommended a table-based approach
    KUMO_FORCE_INLINE size_t find_authority_delimiter(std::string_view view) noexcept {
        static constexpr std::array<uint8_t, 256> authority_delimiter = []() constexpr {
            std::array<uint8_t, 256> result { };
            for (uint8_t i : { '@', '/', '?' }) {
                result[i] = 1;
            }
            return result;
        }();
        // performance note: we might be able to gain further performance
        // with SIMD instrinsics.
        for (auto pos = view.begin(); pos != view.end(); ++pos) {
            if (authority_delimiter[(uint8_t)*pos]) {
                return pos - view.begin();
            }
        }
        return size_t(view.size());
    }

    KUMO_FORCE_INLINE size_t find_next_host_delimiter(std::string_view view,
        size_t location) noexcept {
        auto begin = view.data() + location;
        auto end = view.data() + view.size();
        auto pos = find_first_symbols<':', '/', '?', '['>(begin, end);
        return pos - view.data();
    }

    KUMO_FORCE_INLINE size_t find_next_host_delimiter_special(std::string_view view, size_t location) noexcept {
        auto begin = view.data() + location;
        auto end = view.data() + view.size();
        auto pos = find_first_symbols<':', '/', '\\', '?', '['>(begin, end);
        return pos - view.data();
    }


} // namespace turbo
