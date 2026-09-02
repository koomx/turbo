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

namespace turbo {

    /**
     * @brief URL Component representations using offsets.
     *
     * @details We design the UriComponents struct so that it is as small
     * and simple as possible. This version uses 32 bytes.
     *
     * This struct is used to extract components from a single 'href'.
     */
    struct UriComponents {
        constexpr static uint32_t omitted = uint32_t(-1);

        UriComponents() = default;
        UriComponents(const UriComponents& u) = default;
        UriComponents(UriComponents&& u) noexcept = default;
        UriComponents& operator=(UriComponents&& u) noexcept = default;
        UriComponents& operator=(const UriComponents& u) = default;
        ~UriComponents() = default;

        /*
         * By using 32-bit integers, we implicitly assume that the URL string
         * cannot exceed 4 GB.
         *
         * https://user:pass@example.com:1234/foo/bar?baz#quux
         *       |     |    |          | ^^^^|       |   |
         *       |     |    |          | |   |       |   `----- hash_start
         *       |     |    |          | |   |       `--------- search_start
         *       |     |    |          | |   `----------------- pathname_start
         *       |     |    |          | `--------------------- port
         *       |     |    |          `----------------------- host_end
         *       |     |    `---------------------------------- host_start
         *       |     `--------------------------------------- username_end
         *       `--------------------------------------------- protocol_end
         */
        uint32_t protocol_end { 0 };
        /**
         * Username end is not `omitted` by default to make username and password
         * getters less costly to implement.
         */
        uint32_t username_end { 0 };
        uint32_t host_start { 0 };
        uint32_t host_end { 0 };
        uint32_t port { omitted };
        uint32_t pathname_start { 0 };
        uint32_t search_start { omitted };
        uint32_t hash_start { omitted };

        /**
         * Check the following conditions:
         * protocol_end < username_end < ... < hash_start,
         * expect when a value is omitted. It also computes
         * a lower bound on  the possible string length that may match these
         * offsets.
         * @return true if the offset values are
         *  consistent with a possible URL string
         */
        [[nodiscard]] bool check_offset_consistency() const noexcept;

        /**
         * Converts a UriComponents to JSON stringified version.
         */
        [[nodiscard]] std::string to_string() const;

    }; // struct UriComponents

} // namespace turbo
