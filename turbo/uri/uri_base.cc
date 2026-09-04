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
        if (!ok()) {
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
        if (!query.has_value()) {
            return "";
        }
        if (_standard == StandType::STD_WPT && query.value().empty()) {
            return "";
        }
        return "?" + query.value();
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


    [[nodiscard]] std::string UriBase::get_href() const noexcept {
        std::string output = get_protocol();

        if (host.has_value()) {
            output += "//";
            if (has_credentials()) {
                output += username;
                if (!password.empty()) {
                    output += ":" + get_password();
                }
                output += "@";
            }
            output += host.value();
            if (port.has_value()) {
                output += ":" + get_port();
            }
        } else if (_standard == StandType::STD_WPT && !has_opaque_path()
            && turbo::starts_with(path, "//")) {
            output += "/.";
        }
        output += path;
        if (query.has_value()) {
            output += "?" + query.value();
        }
        if (hash.has_value()) {
            output += "#" + hash.value();
        }
        return output;
    }

} // namespace turbo
