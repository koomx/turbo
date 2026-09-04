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

#include <optional>
#include <string_view>
#include <turbo/strings/substring.h>

namespace turbo {

    ///////////////////////////////////////////////////////////////////////
    ///@brief URL Component representations using offsets.
    ///
    /// @details We design the UriComponents struct so that it is as small
    /// and simple as possible. This version uses 32 bytes.
    ///
    /// This struct is used to extract components from a single 'href'.
    /// [start, end)
    struct UriComponent {
        constexpr static uint32_t npos = std::numeric_limits<uint32_t>::max();
        uint32_t start{npos};
        uint32_t end{npos};

        [[nodiscard]] bool is_null() const noexcept {
            return start == npos && end == npos;
        }

        [[nodiscard]] bool is_valid() const noexcept {
            return start != npos && end != npos && start <= end;
        }

        constexpr bool has_value() const {
            return start!= npos && end != npos;
        }

        constexpr bool empty() const {
            return has_value() && start == end;
        }

        constexpr void set_empty(uint32_t off) {
            start = off;
            end = off;
        }
        constexpr void clear() {
            start = npos;
            end = npos;
        }

        static UriComponent null_value() {
            return {};
        }

        constexpr size_t size() const {
            return end - start;
        }

        std::string_view extract_view(std::string_view input) const {
            if (!has_value()) {
                return {};
            }
            return subview(input,start, end);
        }

        std::string extract_string(std::string_view input) const {
            if (!has_value()) {
                return {};
            }
            return substring(input,start, end);
        }

        void append_to(std::string_view src, std::string * out) const {
            auto str = extract_view(src);
            out->append(str);
        }

        void refine_to(std::string_view src, std::string * out) {
            auto str = extract_view(src);
            start = static_cast<uint32_t>(out->size());
            out->append(str);
            end = static_cast<uint32_t>(out->size());
        }

    };
    struct UriComponents {
        UriComponents() = default;
        UriComponents(const UriComponents& u) = default;
        UriComponents(UriComponents&& u) noexcept = default;
        UriComponents& operator=(UriComponents&& u) noexcept = default;
        UriComponents& operator=(const UriComponents& u) = default;
        ~UriComponents() = default;

        UriComponent schema;
        UriComponent username;
        UriComponent password;
        UriComponent host;
        UriComponent port;
        UriComponent pathname;
        UriComponent query;
        UriComponent fragment;


        // Only meaningful after format(); dirty working-buffer offsets are unordered.
        [[nodiscard]] bool check_offset_consistency() const noexcept;


        [[nodiscard]] std::string to_string() const;

        // Strict href into dst; rewrites this object's offsets onto dst.
        bool refine_to(std::string_view src, std::string& dst, UriComponents& dst_com) const;

    }; // struct UriComponents

} // namespace turbo
