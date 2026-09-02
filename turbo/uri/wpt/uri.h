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

#include <algorithm>
#include <charconv>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

#include <turbo/uri/scheme.h>
#include <turbo/uri/uri_components.h>
#include <turbo/uri/uri_base.h>
#include <turbo/uri/utility.h>
#include <turbo/format/fast_to_buffer.h>

#if KUMO_COMPILER_MSVC_CLANG
#include <intrin.h>
#endif // KUMO_COMPILER_MSVC_CLANG


namespace turbo {

    struct WptUri : public UriBase {
        WptUri():UriBase(StandType::STD_WPT){}
        WptUri(const WptUri& u) = default;
        WptUri(WptUri&& u) noexcept = default;
        WptUri& operator=(WptUri&& u) noexcept = default;
        WptUri& operator=(const WptUri& u) = default;
        ~WptUri() override = default;

        [[nodiscard]] bool has_valid_domain() const noexcept override;

        [[nodiscard]] std::string get_origin() const noexcept override;

    private:
        bool set_username(std::string_view input) override;

        bool set_password(std::string_view input) override;

        bool set_port(std::string_view input) override;

        void set_hash(std::string_view input) override;

        void set_search(std::string_view input) override;

        bool set_pathname(std::string_view input) override;

        bool set_host(std::string_view input) override;

        bool set_hostname(std::string_view input) override;

        bool set_protocol(std::string_view input) override;

        bool set_href(std::string_view input) override;

    private:

        friend class WptParser;

        void strip_trailing_spaces_from_opaque_path() noexcept;

        inline void update_unencoded_base_hash(std::string_view input);

        inline void update_base_search(std::string_view input,const uint8_t query_percent_encode_set[]);

        bool set_host_or_hostname(std::string_view input, bool override_hostname);


    }; // struct url

    inline void WptUri::update_unencoded_base_hash(std::string_view input) {
        // We do the percent encoding
        hash = turbo::percent_encode(input,
            turbo::uri_charsets::FRAGMENT_PERCENT_ENCODE);
    }

    inline void WptUri::update_base_search(std::string_view input,
        const uint8_t   query_percent_encode_set[]) {
        query = turbo::percent_encode(input, query_percent_encode_set);
    }


}  // namespace turbo
