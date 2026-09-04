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
#include <turbo/uri/rfc/uri.h>
#include <turbo/uri/parser_context.h>

namespace turbo {

    RfcUri parse_rfc_uri(std::string_view user_input);

    RfcUri parse_rfc_uri(std::string_view user_input, const RfcUri& base_url);

    bool parse_rfc_uri(std::string_view user_input,RfcUri&uri);

    bool parse_rfc_uri(std::string_view user_input,RfcUri&uri,const RfcUri& base_url);

    std::string href_from_file(std::string_view path);

    class RfcParser {
    public:
        /// private
        static void parse_url_no_base_impl(std::string_view user_input, RfcUri& uri);

        /// private
        static void parse_url_with_base_impl(std::string_view user_input, const RfcUri& base_url, RfcUri& uri);
    private:
        ///////////////////////////////////////////////////////////////////////
        /// 1
        /// 1
        static void chain_parse_url_schema(ParserContext &ctx, RfcUri& uri);
        /// 2
        /// 1-2
        static void chain_parse_url_hier_part(ParserContext &ctx, RfcUri& uri);
        /// 3
        /// 1-2-3
        static void chain_parse_url_hier_part_two(ParserContext &ctx, RfcUri& uri);
        /// 4
        /// 1-2-4
        static void chain_parse_path_root_less(ParserContext &ctx, RfcUri& uri);
        /// 5
        /// 1-2-3-5
        static void chain_parse_authority(ParserContext &ctx, RfcUri& uri);
        /// 6
        /// 1-2-3-5-6
        static void chain_parse_host(ParserContext &ctx, RfcUri& uri);

        /// 7
        /// 1-2-3-5-7
        static void chain_parse_port(ParserContext &ctx, RfcUri& uri);

        /// 8
        /// 1-2-3-7-8
        static void chain_parse_path_abs_empty(ParserContext &ctx, RfcUri& uri);

        /// 9
        /// 1-2-3-7-8-9
        static void chain_parse_query(ParserContext &ctx, RfcUri& uri);

        /// 10
        /// 1-2-3-7-8-10
        /// 1-2-3-7-8-9-10
        static void chain_parse_fragment(ParserContext &ctx, RfcUri& uri);


        static  UriError try_parse_rfc_reg_host(const char* start, const char* end);

    };


} // namespace turbo
