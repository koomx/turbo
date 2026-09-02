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

#include <turbo/uri/uri_error.h>
#include <turbo/uri/scheme.h>
#include <turbo/uri/uri_common_base.h>

namespace turbo::uri_wpt {

    UriError check_opaque_host(std::string_view input);

    /// input should be validate by @check_opaque_host.
    std::string encode_opaque_host(std::string_view input);

    ///////////////////////////////////////////////////////////////////////////////
    /// Parse the path from the provided input.
    /// Return true on success. Control characters not
    /// trimmed from the ends (they should have
    /// been removed if needed).
    ///
    /// The input is expected to be UTF-8.
    ///
    /// @see https://url.spec.whatwg.org/
    std::string parse_path(std::string_view input, bool is_special, bool have_host, turbo::SchemaType type);

    UriError parse_ipv4(std::string_view input, std::string *result);

    UriError parse_ipv6(std::string_view input, std::string *result);

    UriError parse_host(std::string_view input, bool is_special, UriHostType& ht, std::string * result);

    bool parse_scheme(const std::string_view input, SchemaType &type, std::string *result);

    bool parse_scheme_state_override(const std::string_view input,
        bool is_special, bool has_credentials, bool host_empty,std::optional<uint16_t> & port,
        SchemaType &type, std::string *result);

    UriError parse_port(std::string_view view, bool is_special,SchemaType type, bool check_trailing_content, std::optional<uint16_t> &port) noexcept;

}  // namespace turbo::uri_wpt

