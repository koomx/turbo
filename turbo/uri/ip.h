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
#include <string_view>
#include <turbo/macros/macros.h>
#include <turbo/strings/ascii.h>
#include <turbo/uri/uri_error.h>

namespace turbo {

    //////////////////////////////////////////////////////////////////////////
    /// ipv6
    std::string ipv6_to_string(const std::array<uint16_t, 8>& address) noexcept;

    UriError parse_ipv6(std::string_view input, std::string *result);

    //////////////////////////////////////////////////////////////////////////
    /// ipv4
    std::string ipv4_to_string(uint64_t address) noexcept;
    UriError parse_wpt_ipv4(std::string_view input, std::string* result);

    //////////////////////////////////////////////////////////////////////////
    ///
    /// Returns true if an input is an ipv4 address. It is assumed that the string
    /// does not contain uppercase ASCII characters (the input should have been
    /// lowered cased before calling this function) and is not empty.
    ///
    bool is_wpt_ipv4(std::string_view view) noexcept;
    bool is_rfc_ipv4(std::string_view view) noexcept;

    enum class IpType {
        IP_NONE,
        IP_V4,
        IP_V6
    };

    struct IpAddr {
        IpType type{IpType::IP_NONE};
        /// ipv6 all
        /// ipv4 first 4 bytes
        std::array<uint16_t,8> data;
        std::string            normalized;

        void setup_ipv4(uint64_t add) {
            type = IpType::IP_V4;
            data.fill(0);
            data[0] = static_cast<uint16_t>((add >> 16) & 0xffff);
            data[1] = static_cast<uint16_t>(add & 0xffff);
        }
        uint64_t get_ipv4() const {
            return (uint64_t(data[0]) << 16) | uint64_t(data[1]);
        }
    };

    //////////////////////////////////////////////////////////////////////////
    /// Try-parse IP from [start, end). Single forward scan; no separate is_* pass.
    ///
    /// Return semantics (error_pos is relative to start):
    ///   ok() && error_pos == 0  -> not an IP address (caller may treat as reg-name)
    ///   ok() && error_pos > 0   -> IP parsed; error_pos = consumed bytes; out is set
    ///   !ok()                   -> illegal IP syntax; error_pos may be 0 or the fault offset
    ///
    /// IPv4: input should be lowercase ASCII. Must consume the entire [start, end) span.
    /// IPv6: expects a full IP-literal including '[' and ']'; must consume the entire span.

    UriError try_parse_wpt_ip_v4(const char* start, const char* end, std::optional<IpAddr>& out);
    UriError try_parse_wpt_ip_v6(const char* start, const char* end, std::optional<IpAddr>& out);

    UriError try_parse_rfc_ip_v4(const char* start, const char* end, std::optional<IpAddr>& out);
    UriError try_parse_rfc_ip_v6(const char* start, const char* end, std::optional<IpAddr>& out);

    inline UriError try_parse_wpt_ip(const char* start, const char* end, std::optional<IpAddr>& out) {
        if (start == end) {
            return {};
        }
        if (*start == '[') {
            return try_parse_wpt_ip_v6(start, end, out);
        }
        return try_parse_wpt_ip_v4(start, end, out);
    }

    inline UriError try_parse_rfc_ip(const char* start, const char* end, std::optional<IpAddr>& out) {
        if (start == end) {
            return {};
        }
        if (*start == '[') {
            return try_parse_rfc_ip_v6(start, end, out);
        }
        return try_parse_rfc_ip_v4(start, end, out);
    }

    inline UriError try_parse_wpt_ip_v4(std::string_view input, std::optional<IpAddr>& out) {
        return try_parse_wpt_ip_v4(input.data(), input.data() + input.size(), out);
    }
    inline UriError try_parse_wpt_ip_v6(std::string_view input, std::optional<IpAddr>& out) {
        return try_parse_wpt_ip_v6(input.data(), input.data() + input.size(), out);
    }
    inline UriError try_parse_rfc_ip_v4(std::string_view input, std::optional<IpAddr>& out) {
        return try_parse_rfc_ip_v4(input.data(), input.data() + input.size(), out);
    }
    inline UriError try_parse_rfc_ip_v6(std::string_view input, std::optional<IpAddr>& out) {
        return try_parse_rfc_ip_v6(input.data(), input.data() + input.size(), out);
    }
    inline UriError try_parse_wpt_ip(std::string_view input, std::optional<IpAddr>& out) {
        return try_parse_wpt_ip(input.data(), input.data() + input.size(), out);
    }
    inline UriError try_parse_rfc_ip(std::string_view input, std::optional<IpAddr>& out) {
        return try_parse_rfc_ip(input.data(), input.data() + input.size(), out);
    }
} // namespace turbo
