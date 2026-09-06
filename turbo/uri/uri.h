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

#include <string>
#include <string_view>
#include <turbo/uri/uri_view.h>

namespace turbo {

    class Uri {
    public:
        Uri() = default;

        ////////////////////////////////////////////
        /// encoded, s should be
        Uri& schema(std::string_view s);
        Uri& username(std::string_view s);
        Uri& password(std::string_view s);
        Uri& host(std::string_view s);
        Uri& port(std::string_view s);
        Uri& path(std::string_view s);
        Uri& query(std::string_view s);
        Uri& fragment(std::string_view s);
        Uri& append_query(std::string_view key, std::string_view value);

        ////////////////////////////////////////////
        /// unencoded, s should be
        Uri& rfc_plain_schema(std::string_view s);
        Uri& rfc_plain_username(std::string_view s);
        Uri& rfc_plain_password(std::string_view s);
        Uri& rfc_plain_host(std::string_view s);
        Uri& rfc_plain_port(std::string_view s);
        Uri& rfc_plain_path(std::string_view s);
        Uri& rfc_plain_query(std::string_view s);
        Uri& rfc_plain_fragment(std::string_view s);
        Uri& rfc_plain_append_query(std::string_view key, std::string_view value);

        Uri& wpt_plain_schema(std::string_view s);
        Uri& wpt_plain_username(std::string_view s);
        Uri& wpt_plain_password(std::string_view s);
        Uri& wpt_plain_host(std::string_view s);
        Uri& wpt_plain_port(std::string_view s);
        Uri& wpt_plain_path(std::string_view s);
        Uri& wpt_plain_query(std::string_view s);
        Uri& wpt_plain_fragment(std::string_view s);
        Uri& wpt_plain_append_query(std::string_view key, std::string_view value);

        UriView build_rfc() const;
        UriView build_wpt() const;

        UriView build_plain_rfc() const;
        UriView build_plain_wpt() const;
    private:
        ComponentView append_to_store(std::string_view s);
        const uint8_t *wpt_query_encode_set() const;

        UriView _uri_view;
        std::string _store;
    };
}  // namespace turbo
