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
#include <turbo/uri/wpt/uri.h>

namespace turbo {
    /**
     * Parses a url. The parameter user_input is the input to be parsed:
     * it should be a valid UTF-8 string. The parameter base_url is an optional
     * parameter that can be used to resolve relative URLs. If the base_url is
     * provided, the user_input is resolved against the base_url.
     */
    WptUri parse_wpt_uri(std::string_view user_input,
        const WptUri* base_url = nullptr);

    std::string href_from_file(std::string_view path);

    class WptParser {
    public:
        /// private
       static WptUri parse_url_no_base_impl(std::string_view user_input);

        /// private
       static WptUri parse_url_with_base_impl(std::string_view user_input, const WptUri& base_url);
    };


} // namespace turbo
