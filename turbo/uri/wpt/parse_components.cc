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
#include <charconv>
#include <turbo/strings/str_cat.h>

namespace turbo::uri_wpt {

    UriError check_opaque_host(std::string_view input) {
        auto it = std::find_if(input.begin(), input.end(), turbo::is_forbidden_host_code_point);
        if (it != input.end()) {
            return {UriErrorCode::kUriForbiddenHostCodePoint,
                    static_cast<uint32_t>(it - input.begin()), ""};
        }
        return {};
    }

    std::string encode_opaque_host(std::string_view input) {
        // Return the result of running UTF-8 percent-encode on input using the C0
        // control percent-encode set.
        auto ret = turbo::percent_encode(
            input, turbo::uri_charsets::C0_CONTROL_PERCENT_ENCODE);
        return ret;
    }



    std::string parse_path(std::string_view input, bool is_special, bool have_host, turbo::SchemaType type) {
        std::string tmp_buffer;
        std::string_view internal_input;
        std::string path;
        if (turbo::has_tabs_or_newline(input)) {
            tmp_buffer = input;
            // Optimization opportunity: Instead of copying and then pruning, we could
            // just directly build the string from user_input.
            turbo::remove_ascii_tab_or_newline(tmp_buffer);
            internal_input = tmp_buffer;
        } else {
            internal_input = input;
        }

        // If url is special, then:
        if (is_special) {
            if (internal_input.empty()) {
                path = "/";
            } else if ((internal_input[0] == '/') || (internal_input[0] == '\\')) {
                turbo::parse_prepared_path(internal_input.substr(1), type, path);
            } else {
                turbo::parse_prepared_path(internal_input, type, path);
            }
        } else if (!internal_input.empty()) {
            if (internal_input[0] == '/') {
                turbo::parse_prepared_path(internal_input.substr(1), type, path);
            } else {
                turbo::parse_prepared_path(internal_input, type, path);
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
        if (input.empty()) {
            return {UriErrorCode::kUriNotComplete, 0, "inout empty"};
        } // technically unnecessary.
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
        std::string buffer = std::string(input);
        // This next function checks that the result is ascii, but we are going to
        // to check anyhow with is_forbidden.
        // bool is_ascii =
        turbo::to_lower_ascii(buffer.data(), buffer.size());
        bool is_forbidden = turbo::contains_forbidden_domain_code_point(
            buffer.data(), buffer.size());
        if (is_forbidden == 0 && buffer.find("xn-") == std::string_view::npos) {
            // fast path
            if (is_wpt_ipv4(buffer)) {
                auto r = parse_wpt_ipv4(buffer, result);
                if (r.ok()) {
                    ht = UriHostType::IPV4;
                }
                return r;
            }

            if (result) {
                *result = std::move(buffer);
            }

            return {};
        }

        std::optional<std::string> tmp_host;
        auto valid = turbo::to_ascii(tmp_host, input, input.find('%'));
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
        if (!view.empty() && view[0] == '-') {
            return {UriErrorCode::kUriInvalidArgs, 0, ""};
        }
        uint16_t parsed_port { };
        auto r = std::from_chars(view.data(), view.data() + view.size(), parsed_port);
        if (r.ec == std::errc::result_out_of_range) {
            return {UriErrorCode::kUriOverflow, 0, ""};
        }

        const size_t consumed = size_t(r.ptr - view.data());

        if (check_trailing_content) {
            auto valid = (consumed == view.size() || view[consumed] == '/' || view[consumed] == '?' || (is_special && view[consumed] == '\\'));
           if (!valid) {
               return {UriErrorCode::kUriInvalidArgs, static_cast<uint32_t>(consumed), ""};
           }
        }

        // scheme_default_port can return 0, and we should allow 0 as a base port.
        auto default_port = turbo::get_special_port(type);
        bool is_port_valid = (default_port == 0 && parsed_port == 0) || (default_port != parsed_port);
        port = (r.ec == std::errc() && is_port_valid)
            ? std::optional<uint16_t>(parsed_port)
            : std::nullopt;

        return {UriErrorCode::kUriSuccess, static_cast<uint32_t>(consumed), ""};
    }
}  // namespace turbo::uri_wpt

