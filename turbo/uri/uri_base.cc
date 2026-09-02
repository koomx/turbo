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

#include <turbo/uri/ip.h>

#include <algorithm>
#include <numeric>
#include <string>
#include <turbo/uri/checkers.h>
#include <turbo/uri/json.h>
#include <turbo/uri/scheme.h>
#include <turbo/uri/uri_base.h>
#include <turbo/uri/wpt/parser.h>
#include <turbo/uri/wpt/parse_components.h>
#include <turbo/strings/str_cat.h>

namespace turbo {

    [[nodiscard]] std::string UriBase::to_string() const {
        if (!uri_error.ok()) {
            return "null";
        }
        std::string answer;
        auto back = std::back_insert_iterator(answer);
        answer.append("{\n");
        answer.append("\t\"protocol\":\"");
        turbo::encode_json(get_protocol(), back);
        answer.append("\",\n");
        if (has_credentials()) {
            answer.append("\t\"username\":\"");
            turbo::encode_json(username, back);
            answer.append("\",\n");
            answer.append("\t\"password\":\"");
            turbo::encode_json(password, back);
            answer.append("\",\n");
        }
        if (host.has_value()) {
            answer.append("\t\"host\":\"");
            turbo::encode_json(host.value(), back);
            answer.append("\",\n");
        }
        if (port.has_value()) {
            answer.append("\t\"port\":\"");
            answer.append(std::to_string(port.value()));
            answer.append("\",\n");
        }
        answer.append("\t\"path\":\"");
        turbo::encode_json(path, back);
        answer.append("\",\n");
        answer.append("\t\"opaque path\":");
        answer.append((has_opaque_path() ? "true" : "false"));
        if (has_search()) {
            answer.append(",\n");
            answer.append("\t\"query\":\"");
            turbo::encode_json(query.value(), back);
            answer.append("\"");
        }
        if (hash.has_value()) {
            answer.append(",\n");
            answer.append("\t\"hash\":\"");
            turbo::encode_json(hash.value(), back);
            answer.append("\"");
        }
        answer.append("\n}");
        return answer;
    }

    [[nodiscard]] bool UriBase::has_valid_domain() const noexcept {
        if (!host.has_value()) {
            return false;
        }
        return turbo::verify_dns_length(host.value());
    }

    void UriBase::strip_trailing_spaces_from_opaque_path() noexcept {
        if (!has_opaque_path()) return;
        if (has_hash()) return;
        if (has_search()) return;

        auto path = std::string(get_pathname());
        while (!path.empty() && path.back() == ' ') {
            path.resize(path.size() - 1);
        }
        update_base_pathname(path);
    }

     [[nodiscard]] std::string UriBase::get_origin() const noexcept {
        if (is_special()) {
            // Return a new opaque origin.
            if (type == turbo::SchemaType::FILE) {
                return "null";
            }
            return turbo::str_cat(get_protocol(), "//", get_host());
        }

        if (non_special_scheme == "blob") {
            if (!path.empty()) {
                auto result = parse_wpt_uri(path);
                if (result.uri_error.ok() && (result.type == turbo::SchemaType::HTTP ||
                    result.type == turbo::SchemaType::HTTPS)) {
                    // If pathURL's scheme is not "http" and not "https", then return a
                    // new opaque origin.
                    return turbo::str_cat(result.get_protocol(), "//",
                        result.get_host());
                }
            }
        }

        // Return a new opaque origin.
        return "null";
    }

    [[nodiscard]] std::string UriBase::get_protocol() const noexcept {
        if (is_special()) {
            return turbo::str_cat(turbo::details::is_special_list[type], ":");
        }
        // We only move the 'scheme' if it is non-special.
        return turbo::str_cat(non_special_scheme, ":");
    }

    [[nodiscard]] std::string UriBase::get_host() const noexcept {
        // If url's host is null, then return the empty string.
        // If url's port is null, return url's host, serialized.
        // Return url's host, serialized, followed by U+003A (:) and url's port,
        // serialized.
        if (!host.has_value()) {
            return "";
        }
        if (port.has_value()) {
            return host.value() + ":" + get_port();
        }
        return host.value();
    }

    [[nodiscard]] std::string UriBase::get_hostname() const noexcept {
        return host.value_or("");
    }

    [[nodiscard]] std::string_view UriBase::get_pathname() const noexcept {
        return path;
    }

    [[nodiscard]] std::string UriBase::get_search() const noexcept {
        // If this's URL's query is either null or the empty string, then return the
        // empty string. Return U+003F (?), followed by this's URL's query.
        return (!query.has_value() || (query.value().empty())) ? ""
                                                               : "?" + query.value();
    }

    [[nodiscard]] const std::string& UriBase::get_username() const noexcept {
        return username;
    }

    [[nodiscard]] const std::string& UriBase::get_password() const noexcept {
        return password;
    }

    [[nodiscard]] std::string UriBase::get_port() const noexcept {
        return port.has_value() ? std::to_string(port.value()) : "";
    }

    [[nodiscard]] std::string UriBase::get_hash() const noexcept {
        // If this's URL's fragment is either null or the empty string, then return
        // the empty string. Return U+0023 (#), followed by this's URL's fragment.
        return (!hash.has_value() || (hash.value().empty())) ? ""
                                                             : "#" + hash.value();
    }


    bool UriBase::set_host_or_hostname(const std::string_view input, bool override_hostname) {
        if (has_opaque_path()) {
            return false;
        }

        std::optional<std::string> previous_host = host;
        std::optional<uint16_t> previous_port = port;

        size_t host_end_pos = input.find('#');
        std::string _host(input.data(), host_end_pos != std::string_view::npos ? host_end_pos : input.size());
        turbo::remove_ascii_tab_or_newline(_host);
        std::string_view new_host(_host);

        // If url's scheme is "file", then set state to file host state, instead of
        // host state.
        if (type != turbo::SchemaType::FILE) {
            std::string_view host_view(_host.data(), _host.length());
            auto [location, found_colon] = turbo::get_host_delimiter_location(is_special(), host_view);

            // Otherwise, if c is U+003A (:) and insideBrackets is false, then:
            // Note: the 'found_colon' value is true if and only if a colon was
            // encountered while not inside brackets.
            if (found_colon) {
                if (override_hostname) {
                    return false;
                }
                std::string_view buffer = new_host.substr(location + 1);
                if (!buffer.empty()) {
                    set_port(buffer);
                }
            }
            // If url is special and host_view is the empty string, validation error,
            // return failure. Otherwise, if state override is given, host_view is the
            // empty string, and either url includes credentials or url's port is
            // non-null, return.
            else if (host_view.empty() && (is_special() || has_credentials() || port.has_value())) {
                return false;
            }

            // Let host be the result of host parsing host_view with url is not special.
            if (host_view.empty() && !is_special()) {
                host = "";
                return true;
            }

            std::string tmp_host;
            bool succeeded = turbo::uri_wpt::parse_host(host_view, is_special(), host_type(),&tmp_host).ok();
            if (!succeeded) {
                host = previous_host;
                update_base_port(previous_port);
                return false;
            }
            host = std::move(tmp_host);
            return true;
        }

        size_t location = new_host.find_first_of("/\\?");
        if (location != std::string_view::npos) {
            new_host.remove_suffix(new_host.length() - location);
        }

        if (new_host.empty()) {
            // Set url's host to the empty string.
            host = "";
        } else {
            // Let host be the result of host parsing buffer with url is not special.
            std::string tmp_host;

            if (!turbo::uri_wpt::parse_host(new_host, is_special(), host_type(), &tmp_host).ok()) {
                host = previous_host;
                update_base_port(previous_port);
                return false;
            }

            host = std::move(tmp_host);
            // If host is "localhost", then set host to the empty string.
            if (host.has_value() && host.value() == "localhost") {
                host = "";
            }
        }
        return true;
    }

    bool UriBase::set_host(const std::string_view input) {
        return set_host_or_hostname(input, false);
    }

    bool UriBase::set_hostname(const std::string_view input) {
        return set_host_or_hostname(input, true);
    }

    bool UriBase::set_username(const std::string_view input) {
        if (cannot_have_credentials_or_port()) {
            return false;
        }
        username = turbo::percent_encode(
            input, turbo::uri_charsets::USERINFO_PERCENT_ENCODE);
        return true;
    }

    bool UriBase::set_password(const std::string_view input) {
        if (cannot_have_credentials_or_port()) {
            return false;
        }
        password = turbo::percent_encode(
            input, turbo::uri_charsets::USERINFO_PERCENT_ENCODE);
        return true;
    }

    bool UriBase::set_port(const std::string_view input) {
        if (cannot_have_credentials_or_port()) {
            return false;
        }
        std::string trimmed(input);
        turbo::remove_ascii_tab_or_newline(trimmed);
        if (trimmed.empty()) {
            port = std::nullopt;
            return true;
        }
        // Input should not start with control characters.
        if (turbo::is_c0_control_or_space(trimmed.front())) {
            return false;
        }
        // Input should contain at least one ascii digit.
        if (input.find_first_of("0123456789") == std::string_view::npos) {
            return false;
        }

        // Revert changes if parse_port fails.
        std::optional<uint16_t> tmp_port;
        auto r = uri_wpt::parse_port(trimmed, is_special(),type,false,tmp_port);
        if (!r.ok()) {
            return false;
        }
        port = tmp_port;
        uri_error = {};
        return true;
    }

    void UriBase::set_hash(const std::string_view input) {
        if (input.empty()) {
            hash = std::nullopt;
            strip_trailing_spaces_from_opaque_path();
            return;
        }

        std::string new_value;
        new_value = input[0] == '#' ? input.substr(1) : input;
        turbo::remove_ascii_tab_or_newline(new_value);
        hash = turbo::percent_encode(new_value,
            turbo::uri_charsets::FRAGMENT_PERCENT_ENCODE);
    }

    void UriBase::set_search(const std::string_view input) {
        if (input.empty()) {
            query = std::nullopt;
            strip_trailing_spaces_from_opaque_path();
            return;
        }

        std::string new_value;
        new_value = input[0] == '?' ? input.substr(1) : input;
        turbo::remove_ascii_tab_or_newline(new_value);

        auto query_percent_encode_set = is_special() ? turbo::uri_charsets::SPECIAL_QUERY_PERCENT_ENCODE
                                                     : turbo::uri_charsets::QUERY_PERCENT_ENCODE;

        query = turbo::percent_encode(std::string_view(new_value),
            query_percent_encode_set);
    }

    bool UriBase::set_pathname(const std::string_view input) {
        if (has_opaque_path()) {
            return false;
        }
        path =turbo::uri_wpt::parse_path(input, is_special(), host.has_value(),type);
        return true;
    }

    bool UriBase::set_protocol(const std::string_view input) {
        std::string view(input);
        turbo::remove_ascii_tab_or_newline(view);
        if (view.empty()) {
            return true;
        }

        // Schemes should start with alpha values.
        if (!turbo::ascii_isalpha(view[0])) {
            return false;
        }

        view.append(":");

        std::string::iterator pointer = std::find_if_not(view.begin(), view.end(), turbo::is_valid_schema_alnum);

        if (pointer != view.end() && *pointer == ':') {

            std::string tmp_proto;
            auto tmp_port = port;
            auto tmp_type= type;
            if (!turbo::uri_wpt::parse_scheme_state_override(
                std::string_view(view.data(), pointer - view.begin()),
                is_special(),
                has_credentials(),
                host_empty(),
                tmp_port,
                tmp_type,
                &tmp_proto)
                ) {
                return false;
                }
            if (tmp_type == turbo::SchemaType::NOT_SPECIAL) {
                set_scheme(std::move(tmp_proto));
            } else {
                type = tmp_type;
            }
            port = tmp_port;
        }
        return true;
    }

    bool UriBase::set_href(const std::string_view input) {
        auto out = parse_wpt_uri(input);

        if (out.uri_error.ok()) {
            username = out.username;
            password = out.password;
            host = out.host;
            port = out.port;
            path = out.path;
            query = out.query;
            hash = out.hash;
            type = out.type;
            non_special_scheme = out.non_special_scheme;
            has_opaque_path() = out.has_opaque_path();
        }

        return out.uri_error.ok();
    }
} // namespace turbo
