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
#include <turbo/uri/uri_base.h>
#include <turbo/uri/utility.h>
#include <turbo/format/fast_to_buffer.h>

#if KUMO_COMPILER_MSVC_CLANG
#include <intrin.h>
#endif // KUMO_COMPILER_MSVC_CLANG


namespace turbo {

    struct RfcUri : public UriBase {
        RfcUri():UriBase(StandType::STD_RFC){}
        RfcUri(const RfcUri& u) = default;
        RfcUri(RfcUri&& u) noexcept = default;
        RfcUri& operator=(RfcUri&& u) noexcept = default;
        RfcUri& operator=(const RfcUri& u) = default;
        ~RfcUri() override = default;

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

        friend class RfcParser;

    }; // struct url


}  // namespace turbo
