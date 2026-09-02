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

#include <array>
#include <optional>
#include <string>
#include <turbo/macros/macros.h>


namespace turbo {

    /// Type of the scheme as an enum.
    /// Using strings to represent a scheme type is not ideal because
    /// checking for types involves string comparisons. It is faster to use
    /// a simple integer.
    /// In C++11, we are allowed to specify the underlying type of the enum.
    /// We pick an 8-bit integer (which allows up to 256 types). Specifying the
    /// type of the enum may help integration with other systems if the type
    /// variable is exposed (since its value will not depend on the compiler).
    enum SchemaType : uint8_t {
        HTTP = 0,
        NOT_SPECIAL = 1,
        HTTPS = 2,
        WS = 3,
        FTP = 4,
        WSS = 5,
        FILE = 6
    };

    /// A special scheme is an ASCII string that is listed in the first column of the
    /// following table. The default port for a special scheme is listed in the
    /// second column on the same row. The default port for any other ASCII string is
    /// null.
    ///
    /// @see https://url.spec.whatwg.org/#url-miscellaneous
    /// @param scheme
    /// @return If scheme is a special scheme
    KUMO_FORCE_INLINE constexpr bool is_special(std::string_view scheme);

    /// A special scheme is an ASCII string that is listed in the first column of the
    /// following table. The default port for a special scheme is listed in the
    /// second column on the same row. The default port for any other ASCII string is
    /// null.
    ///
    /// @see https://url.spec.whatwg.org/#url-miscellaneous
    /// @param scheme
    /// @return The special port
   KUMO_FORCE_INLINE constexpr uint16_t get_special_port(std::string_view scheme) noexcept;

    /// Returns the port number of a special scheme.
    /// @see https://url.spec.whatwg.org/#special-scheme
   KUMO_FORCE_INLINE constexpr uint16_t get_special_port(SchemaType type) noexcept;
    /// Returns the scheme of an input, or NOT_SPECIAL if it's not a special scheme
    /// defined by the spec.
   KUMO_FORCE_INLINE constexpr SchemaType get_scheme_type(std::string_view scheme) noexcept;


      ////////////////////////////////////////////////////////////////////////////////
    /// namespace details
    ////////////////////////////////////////////////////////////////////////////////

    namespace details {
        /// for use with is_special and get_special_port
        /// Spaces, if present, are removed from URL.
        static  constexpr std::string_view is_special_list[] = { "http", " ", "https", "ws",
            "ftp", "wss", "file", " " };
        /// for use with get_special_port
        static   constexpr uint16_t special_ports[] = { 80, 0, 443, 80, 21, 443, 0, 0 };
    } /// namespace details

    ///
    /// In is_special, get_scheme_type, and get_special_port, we
    /// use a standard hashing technique to find the index of the scheme in
    /// the is_special_list. The hashing technique is based on the size of
    /// the scheme and the first character of the scheme. It ensures that we
    /// do at most one string comparison per call. If the protocol is
    /// predictible (e.g., it is always "http"), we can get a better average
    /// performance by using a simpler approach where we loop and compare
    /// scheme with all possible protocols starting with the most likely
    /// protocol. Doing multiple comparisons may have a poor worst case
    /// performance, however. In this instance, we choose a potentially
    /// slightly lower best-case performance for a better worst-case
    /// performance. We can revisit this choice at any time.
    ///
    /// Reference:
    /// Schmidt, Douglas C. "Gperf: A perfect hash function generator."
    /// More C++ gems 17 (2000).
    ///
    /// Reference: https://en.wikipedia.org/wiki/Perfect_hash_function
    /// **

    KUMO_FORCE_INLINE constexpr bool is_special(std::string_view scheme) {
        if (scheme.empty()) {
            return false;
        }
        int hash_value = (2 * scheme.size() + (unsigned)(scheme[0])) & 7;
        const std::string_view target = details::is_special_list[hash_value];
        return (target[0] == scheme[0]) && (target.substr(1) == scheme.substr(1));
    }

   KUMO_FORCE_INLINE constexpr uint16_t get_special_port(std::string_view scheme) noexcept {
        if (scheme.empty()) {
            return 0;
        }
        int hash_value = (2 * scheme.size() + (unsigned)(scheme[0])) & 7;
        const std::string_view target = details::is_special_list[hash_value];
        if ((target[0] == scheme[0]) && (target.substr(1) == scheme.substr(1))) {
            return details::special_ports[hash_value];
        } else {
            return 0;
        }
    }
   KUMO_FORCE_INLINE constexpr uint16_t get_special_port(SchemaType type) noexcept {
        return details::special_ports[static_cast<size_t>(type)];
    }

   KUMO_FORCE_INLINE  constexpr SchemaType get_scheme_type(std::string_view scheme) noexcept {
        if (scheme.empty()) {
            return SchemaType::NOT_SPECIAL;
        }
        int hash_value = (2 * scheme.size() + (unsigned)(scheme[0])) & 7;
        const std::string_view target = details::is_special_list[hash_value];
        if ((target[0] == scheme[0]) && (target.substr(1) == scheme.substr(1))) {
            return SchemaType(hash_value);
        } else {
            return SchemaType::NOT_SPECIAL;
        }
    }

}  // namespace turbo
