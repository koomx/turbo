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
#include <string>
#include <string_view>
#include <vector>
#include <turbo/uri/types.h>
#include <turbo/uri/uri_error.h>
#include <turbo/uri/scheme.h>

namespace turbo {

    class WptParser;
    class RfcParser;
    class Uri;
    //////////////////////////////////////////////////
    /// setters privode for paser or other builtin
    /// and not for user api
    struct ComponentView {
        static constexpr uint32_t npos = std::numeric_limits<uint32_t>::max();
        uint32_t start{npos};
        uint32_t end{npos};
    };

    using QueryParams = std::vector<std::pair<ComponentView,ComponentView>>;

    class UriView {
    public:
        UriView() = default;
        UriView(std::string_view);
        UriView(const UriView& u);
        UriView(UriView&& u) noexcept;
        UriView& operator=(UriView&& u) noexcept;
        UriView& operator=(const UriView& u);
        ~UriView() = default;


        void make_ownd();
        void set_ownd(std::string &&str);
        void release(std::string &out);
        std::string release();
        void release_append(std::string &out) const;

        [[nodiscard]] bool is_special() const noexcept;
        ///////////////////////
        /// getters
        [[nodiscard]] bool ok() const noexcept;
        [[nodiscard]] StandType standard() const noexcept;
        [[nodiscard]] EnodeType encode_type() const noexcept;
        [[nodiscard]] UriHostType host_type() const noexcept;
        [[nodiscard]] const UriError& uri_error() const noexcept;
        SchemaType schema_type() const;

        std::string_view origin() const;
        std::string_view shema() const;
        std::string_view username() const;
        std::string_view password() const;
        std::string_view host() const;
        std::string_view port() const;
        std::string_view path() const;
        std::string_view query() const;
        std::string_view fragment() const;
        [[nodiscard]] const QueryParams& query_params() const noexcept;

        ///////////////////////
        /// checkers
        [[nodiscard]] bool has_shema() const noexcept;
        [[nodiscard]] bool has_username() const noexcept;
        [[nodiscard]] bool has_password() const noexcept;
        [[nodiscard]] bool has_host() const noexcept;
        [[nodiscard]] bool has_port() const noexcept;
        [[nodiscard]] bool has_path() const noexcept;
        [[nodiscard]] bool has_query() const noexcept;
        [[nodiscard]] bool has_fragment() const noexcept;
        [[nodiscard]] bool has_query_params() const noexcept;

    private:
        friend class WptParser;
        friend class RfcParser;
        friend class Uri;
        friend UriView merge_rfc_uri(std::string_view user_input, const UriView& base_url);
        friend UriView encode_rfc_uri(const UriView& uri);
        friend UriView decode_rfc_uri(const UriView& uri);
        friend UriView merge_wpt_uri(std::string_view user_input, const UriView& base_url);
        friend UriView encode_wpt_uri(const UriView& uri);
        friend UriView decode_wpt_uri(const UriView& uri);
        ///////////////////////
        /// cleaner
        void reset();
        void reset_shema();
        void reset_username();
        void reset_password();
        void reset_host();
        void reset_port();
        void reset_path();
        void reset_query();
        void reset_fragment();
        void reset_query_params();
        ///////////////////////
        /// setters
        void encode_type(EnodeType et);
        void standard(StandType st);
        void host_type(UriHostType ht);
        void uri_error(UriError err);
        void shema(ComponentView view);
        void username(ComponentView view);
        void password(ComponentView view);
        void host(ComponentView view);
        void port(ComponentView view);
        void path(ComponentView view);
        void query(ComponentView view);
        void fragment(ComponentView view);
        void query_params(QueryParams params);
        void schema_type(SchemaType type);
    protected:
        std::optional<std::string>   _store;
        std::string_view             _uri_data;
        StandType                    _type{StandType::STD_NONE};
        EnodeType                    _encode{EnodeType::PRECENT};
        UriHostType                  _host_type{UriHostType::DEFAULT};
        UriError                     _error;
        SchemaType                   _schema_type{SchemaType::NOT_SPECIAL};

        std::optional<ComponentView> _schema;
        std::optional<ComponentView> _username;
        std::optional<ComponentView> _password;
        std::optional<ComponentView> _host;
        std::optional<ComponentView> _port;
        std::optional<ComponentView> _path;
        std::optional<ComponentView> _query;
        std::optional<ComponentView> _fragment;
        std::optional<QueryParams>   _query_params;
    };
}  // namespace turbo
