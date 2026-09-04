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

#include <turbo/uri/rfc/uri.h>

namespace turbo {

    bool RfcUri::has_valid_domain() const noexcept {
        return host.has_value() && !host.value().empty();
    }

    std::string RfcUri::get_origin() const noexcept {
        return {};
    }

    bool RfcUri::set_username(std::string_view input) {
        update_base_username(input);
        return true;
    }

    bool RfcUri::set_password(std::string_view input) {
        update_base_password(input);
        return true;
    }

    bool RfcUri::set_port(std::string_view) {
        return false;
    }

    void RfcUri::set_hash(std::string_view input) {
        hash = std::string(input);
    }

    void RfcUri::set_search(std::string_view input) {
        update_base_search(input);
    }

    bool RfcUri::set_pathname(std::string_view input) {
        update_base_pathname(input);
        return true;
    }

    bool RfcUri::set_host(std::string_view input) {
        update_base_hostname(input);
        return true;
    }

    bool RfcUri::set_hostname(std::string_view input) {
        update_base_hostname(input);
        return true;
    }

    bool RfcUri::set_protocol(std::string_view) {
        return false;
    }

    bool RfcUri::set_href(std::string_view) {
        return false;
    }


}  // namespace turbo
