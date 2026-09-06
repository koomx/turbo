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
#include <turbo/uri/uri_view.h>
#include <turbo/uri/parser_context.h>

namespace turbo {

    UriView parse_rfc_uri(std::string_view user_input);

    UriView parse_rfc_uri(std::string_view user_input, const UriView& base_url);

    UriView merge_rfc_uri(std::string_view user_input, const UriView& base_url);

    UriView encode_rfc_uri(const UriView& uri);

    UriView decode_rfc_uri(const UriView& uri);

    std::string href_from_file(std::string_view path);

    std::string get_rfc_href(const UriView &base_url, const UriView &ref_url);

    std::string get_rfc_href(const UriView &ref_url);

    class RfcParser {
    public:
        /// private
        static UriView parse_url_no_base_impl(std::string_view user_input);

        /// private
        static UriView parse_url_with_base_impl(std::string_view user_input, const UriView& base_url);

        static UriView convert_encode_type(const UriView& uri, EnodeType target);
    private:
        ///////////////////////////////////////////////////////////////////////
        /// 1
        /// 1
        static void chain_parse_url_schema(ParserContext &ctx, UriView& uri);
        /// 2
        /// 1-2
        static void chain_parse_url_hier_part(ParserContext &ctx, UriView& uri);
        /// 3
        /// 1-2-3
        static void chain_parse_url_hier_part_two(ParserContext &ctx, UriView& uri);
        /// 4
        /// 1-2-4
        static void chain_parse_path_root_less(ParserContext &ctx, UriView& uri);
        /// 5
        /// 1-2-3-5
        static void chain_parse_authority(ParserContext &ctx, UriView& uri);
        /// 6
        /// 1-2-3-5-6
        static void chain_parse_host(ParserContext &ctx, UriView& uri);

        /// 7
        /// 1-2-3-5-7
        static void chain_parse_port(ParserContext &ctx, UriView& uri);

        /// 8
        /// 1-2-3-7-8
        static void chain_parse_path_abs_empty(ParserContext &ctx, UriView& uri);

        /// 9
        /// 1-2-3-7-8-9
        static void chain_parse_query(ParserContext &ctx, UriView& uri);

        /// 10
        /// 1-2-3-7-8-10
        /// 1-2-3-7-8-9-10
        static void chain_parse_fragment(ParserContext &ctx, UriView& uri);


        static  UriError try_parse_rfc_reg_host(const char* start, const char* end);

    };


} // namespace turbo
