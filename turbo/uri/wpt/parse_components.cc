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

#include <turbo/uri/wpt/parse_components.h>
#include <turbo/uri/checkers.h>
#include <turbo/uri/utility.h>
#include <turbo/uri/ip.h>
#include <turbo/strings/ascii.h>
#include <turbo/strings/str_cat.h>

namespace turbo::uri_wpt {

    UriError check_opaque_host(std::string_view input) {
        for (size_t i = 0; i < input.size(); ++i) {
            if (turbo::is_ascii_tab_or_newline(input[i])) {
                continue;
            }
            if (turbo::is_forbidden_host_code_point(input[i])) {
                return {UriErrorCode::kUriForbiddenHostCodePoint,
                        static_cast<uint32_t>(i), ""};
            }
        }
        return {};
    }

    std::string encode_opaque_host(std::string_view input) {
        // Skip tab/LF/CR while scanning; percent-encode remaining with C0 set.
        std::string out;
        out.reserve(input.size());
        for (unsigned char c : input) {
            if (turbo::is_ascii_tab_or_newline(static_cast<char>(c))) {
                continue;
            }
            if (turbo::uri_charsets::bit_at(
                    turbo::uri_charsets::C0_CONTROL_PERCENT_ENCODE, c)) {
                out.append(turbo::uri_charsets::hex + c * 4, 3);
            } else {
                out.push_back(static_cast<char>(c));
            }
        }
        return out;
    }



    std::string parse_path(std::string_view input, bool is_special, bool have_host, turbo::SchemaType type) {
        std::string path;
        // Tab/LF/CR are skipped inside parse_prepared_path while scanning — no pre-copy.
        if (is_special) {
            if (input.empty()) {
                path = "/";
            } else if ((input[0] == '/') || (input[0] == '\\')) {
                turbo::parse_prepared_path(input.substr(1), type, path);
            } else {
                turbo::parse_prepared_path(input, type, path);
            }
        } else if (!input.empty()) {
            if (input[0] == '/') {
                turbo::parse_prepared_path(input.substr(1), type, path);
            } else {
                turbo::parse_prepared_path(input, type, path);
            }
        } else {
            if (!have_host) {
                path = "/";
            }
        }
        return path;
    }

    UriError parse_host(std::string_view input, bool is_special, UriHostType& ht, std::string * result) {
        ht = UriHostType::DEFAULT;
        // Scan once into host buffer, skipping tab/LF/CR (no whole-URL pre-strip).
        std::string buffer;
        buffer.reserve(input.size());
        for (char c : input) {
            if (!turbo::is_ascii_tab_or_newline(c)) {
                buffer.push_back(c);
            }
        }
        if (buffer.empty()) {
            return {UriErrorCode::kUriNotComplete, 0, "inout empty"};
        }
        input = buffer;

        // If input starts with U+005B ([), then:
        if (input[0] == '[') {
            // If input does not end with U+005D (]), validation error, return failure.
            if (input.back() != ']') {
                return {UriErrorCode::kUriNotComplete, static_cast<uint32_t>(input.size() -1), "input.back() != ']'"};
            }

            // Return the result of IPv6 parsing input with its leading U+005B ([) and
            // trailing U+005D (]) removed.
            input.remove_prefix(1);
            input.remove_suffix(1);
            auto r =  parse_ipv6(input, result);
            if (r.ok()) {
                ht = UriHostType::IPV6;
            }
            return r;
        }

        // If isNotSpecial is true, then return the result of opaque-host parsing
        // input.
        if (!is_special) {
            UriError opaque = turbo::uri_wpt::check_opaque_host(input);
            if (!opaque.ok()) {
                return opaque;
            }
            if (result) {
                *result = turbo::uri_wpt::encode_opaque_host(input);
            }
            return {};
        }
        // Let domain be the result of running UTF-8 decode without BOM on the
        // percent-decoding of input. Let asciiDomain be the result of running domain
        // to ASCII with domain and false. The most common case is an ASCII input, in
        // which case we do not need to call the expensive 'to_ascii' if a few
        // conditions are met: no '%' and no 'xn-' subsequence.
        //
        // to_lower only for the ASCII fast path — never mutate UTF-8 before to_ascii.
        std::string lowered = buffer;
        turbo::to_lower_ascii(lowered.data(), lowered.size());
        bool is_forbidden = turbo::contains_forbidden_domain_code_point(
            lowered.data(), lowered.size());
        if (is_forbidden == 0 && lowered.find("xn-") == std::string_view::npos) {
            // fast path
            if (is_wpt_ipv4(lowered)) {
                auto r = parse_wpt_ipv4(lowered, result);
                if (r.ok()) {
                    ht = UriHostType::IPV4;
                }
                return r;
            }

            if (result) {
                *result = std::move(lowered);
            }

            return {};
        }

        std::optional<std::string> tmp_host;
        auto valid = turbo::to_ascii(tmp_host, buffer, buffer.find('%'));
        if (!valid) {
            return {UriErrorCode::kUriNotComplete, 0, "parse_host to_ascii returns false"};
        }

        if (std::any_of(tmp_host.value().begin(), tmp_host.value().end(),
                turbo::is_forbidden_domain_code_point)) {
            return {UriErrorCode::kUriForbiddenHostCodePoint, 0, ""};
        }


        // If asciiDomain ends in a number, then return the result of IPv4 parsing
        // asciiDomain.
        if (is_wpt_ipv4(tmp_host.value())) {
            auto r=  parse_wpt_ipv4(tmp_host.value(), result);
            if (r.ok()) {
                ht = UriHostType::IPV4;
            }
            return r;
        }

        if (result) {
            *result = tmp_host.value();
        }

        return {};
    }

    bool parse_scheme(const std::string_view input, SchemaType &type, std::string *result) {
        auto parsed_type = turbo::get_scheme_type(input);
        bool is_input_special = (parsed_type != turbo::SchemaType::NOT_SPECIAL);
        ///
        /// In the common case, we will immediately recognize a special scheme (e.g.,
        /// http, https), in which case, we can go really fast.
        ///
        if (is_input_special) {
            // fast path!!!
            type = parsed_type;
        } else {
            // slow path
            std::string _buffer(input);
            // Next function is only valid if the input is ASCII and returns false
            // otherwise, but it seems that we always have ascii content so we do not
            // need to check the return value.
            // bool is_ascii =
            turbo::to_lower_ascii(_buffer.data(), _buffer.size());

            if (result) {
              *result = std::move(_buffer);
            }
        }

        return true;
    }

    bool parse_scheme_state_override(const std::string_view input, bool is_special, bool has_credentials, bool host_empty,std::optional<uint16_t> & port, SchemaType &type, std::string *result) {
        auto parsed_type = turbo::get_scheme_type(input);
        bool is_input_special = (parsed_type != turbo::SchemaType::NOT_SPECIAL);
        /**
         * In the common case, we will immediately recognize a special scheme (e.g.,
         *http, https), in which case, we can go really fast.
         **/
        if (is_input_special) { // fast path!!!
            // If url's scheme is not a special scheme and buffer is a special scheme,
            // then return.
            if (is_special != is_input_special) {
                return false;
            }

            // If url includes credentials or has a non-null port, and buffer is
            // "file", then return.
            if ((has_credentials || port.has_value()) && parsed_type == turbo::SchemaType::FILE) {
                return false;
            }

            // If url's scheme is "file" and its host is an empty host, then return.
            // An empty host is the empty string.
            if (type == turbo::SchemaType::FILE && host_empty) {
                return false;
            }
            type = parsed_type;

            // This is uncommon.
            uint16_t urls_scheme_port = turbo::get_special_port(type);

            if (urls_scheme_port) {
                // If url's port is url's scheme's default port, then set url's port to
                // null.
                if (port.has_value() && *port == urls_scheme_port) {
                    port = std::nullopt;
                }
            }
        } else {
            // slow path
            std::string _buffer(input);
            // Next function is only valid if the input is ASCII and returns false
            // otherwise, but it seems that we always have ascii content so we do not
            // need to check the return value.
            // bool is_ascii =
            turbo::to_lower_ascii(_buffer.data(), _buffer.size());

            // If url's scheme is a special scheme and buffer is not a special scheme,
            // then return. If url's scheme is not a special scheme and buffer is a
            // special scheme, then return.
            if (is_special != turbo::is_special(_buffer)) {
                return true;
            }

            // If url includes credentials or has a non-null port, and buffer is
            // "file", then return.
            if ((has_credentials || port.has_value()) && _buffer == "file") {
                return true;
            }

            // If url's scheme is "file" and its host is an empty host, then return.
            // An empty host is the empty string.
            if (type == turbo::SchemaType::FILE && host_empty) {
                return true;
            }

            if (result) {
                *result = std::move(_buffer);
            }

            // This is uncommon.
            uint16_t urls_scheme_port = turbo::get_special_port(type);

            if (urls_scheme_port) {
                // If url's port is url's scheme's default port, then set url's port to
                // null.
                if (port.has_value() && *port == urls_scheme_port) {
                    port = std::nullopt;
                }
            }
        }

        return true;
    }


    UriError parse_port(std::string_view view, bool is_special,SchemaType type, bool check_trailing_content, std::optional<uint16_t> &port) noexcept {
        const char *p = view.data();
        const char *const e = view.data() + view.size();
        skip_ascii_tab_or_newline(p, e);
        if (p != e && *p == '-') {
            return {UriErrorCode::kUriInvalidArgs, 0, ""};
        }

        uint32_t value = 0;
        bool saw_digit = false;
        while (p != e) {
            if (is_ascii_tab_or_newline(*p)) {
                ++p;
                continue;
            }
            if (!turbo::ascii_isdigit(static_cast<unsigned char>(*p))) {
                break;
            }
            saw_digit = true;
            value = value * 10u + static_cast<uint32_t>(*p - '0');
            if (value > 65535u) {
                return {UriErrorCode::kUriOverflow, 0, ""};
            }
            ++p;
        }

        if (check_trailing_content) {
            skip_ascii_tab_or_newline(p, e);
            const bool valid =
                (p == e || *p == '/' || *p == '?' || (is_special && *p == '\\'));
            if (!valid) {
                return {UriErrorCode::kUriInvalidArgs,
                    static_cast<uint32_t>(p - view.data()), ""};
            }
        }

        auto default_port = turbo::get_special_port(type);
        const uint16_t parsed_port = static_cast<uint16_t>(value);
        bool is_port_valid = (default_port == 0 && parsed_port == 0) ||
            (default_port != parsed_port);
        port = (saw_digit && is_port_valid)
            ? std::optional<uint16_t>(parsed_port)
            : std::nullopt;

        return {UriErrorCode::kUriSuccess,
            static_cast<uint32_t>(p - view.data()), ""};
    }
}  // namespace turbo::uri_wpt

