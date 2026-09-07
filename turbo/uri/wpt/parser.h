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
// WHATWG URL parsing (WPT). Implementation is Turbo's own UriView + chain parser.
// Early design and some component helpers were informed by Ada
// (https://github.com/ada-url/ada, Apache-2.0 OR MIT); this is not a vendored
// copy of Ada.
//

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <turbo/uri/uri_view.h>
#include <turbo/uri/parser_context.h>

namespace turbo {

    UriView parse_wpt_uri(std::string_view user_input);

    UriView parse_wpt_uri(std::string_view user_input, const UriView& base_url);

    UriView merge_wpt_uri(std::string_view user_input, const UriView& base_url);

    UriView encode_wpt_uri(const UriView& uri);

    UriView decode_wpt_uri(const UriView& uri);

    std::string get_wpt_href(const UriView &base_url, const UriView &ref_url);

    std::string get_wpt_href(const UriView &ref_url);

    std::string href_from_file(std::string_view path);

    class WptParser {
    public:
        /// private
        UriView parse_url_no_base_impl(std::string_view user_input);

        /// private
        UriView parse_url_with_base_impl(std::string_view user_input, const UriView& base_url);

        static UriView convert_encode_type(const UriView& uri, EnodeType target);
    private:
        ///////////////////////////////////////////////////////////////////////
        /// 1
        void chain_parse_url_schema(ParserContext &ctx, UriView& uri);
        void chain_parse_no_scheme(ParserContext &ctx, UriView& uri);
        void chain_parse_relative(ParserContext &ctx, UriView& uri);
        void chain_parse_relative_slash(ParserContext &ctx, UriView& uri);
        void chain_parse_special_relative_or_authority(ParserContext &ctx, UriView& uri);
        void chain_parse_path_or_authority(ParserContext &ctx, UriView& uri);
        void chain_parse_opaque_path(ParserContext &ctx, UriView& uri);
        void chain_parse_file(ParserContext &ctx, UriView& uri);
        void chain_parse_file_slash(ParserContext &ctx, UriView& uri);
        void chain_parse_file_host(ParserContext &ctx, UriView& uri);
        void chain_parse_special_authority_slashes(ParserContext &ctx, UriView& uri);
        void chain_parse_special_authority_ignore_slashes(ParserContext &ctx, UriView& uri);
        void chain_parse_authority(ParserContext &ctx, UriView& uri);
        void chain_parse_host(ParserContext &ctx, UriView& uri);
        void chain_parse_port(ParserContext &ctx, UriView& uri);
        void chain_parse_path_start(ParserContext &ctx, UriView& uri);
        void chain_parse_path(ParserContext &ctx, UriView& uri);
        void chain_parse_query(ParserContext &ctx, UriView& uri);
        void chain_parse_fragment(ParserContext &ctx, UriView& uri);

    private:
        void parse_scheme(SchemaType &type);
        void finish(UriView& uri);
        void set_scheme_from_base(UriView& uri);
        void append_authority_from_base(UriView& uri);
        void append_path_from_base(UriView& uri);
        void append_query_from_base(UriView& uri);
        void set_empty_host(UriView& uri);
        void clear_path(UriView& uri);
        bool base_has_opaque_path() const;
        const uint8_t *query_encode_set(const UriView& uri) const;
        std::string_view buffer_slice(const std::optional<ComponentView> &c) const;
    private:
        std::string _buffer;
        std::string _encode_buffer;
        std::optional<std::string_view> _pending_fragment;
        const UriView *_base = nullptr;
    };

} // namespace turbo
