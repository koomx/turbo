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

#include <turbo/uri/rfc/uri.h>
#include <turbo/uri/rfc/parser.h>
#include <turbo/uri/uri_error.h>
#include <turbo/uri/ip.h>
#include <turbo/strings/find_symbols.h>
#include <turbo/strings/ascii.h>
#include <array>
#include <limits>
#include <optional>
#include <string>


namespace turbo {
namespace {

    // RFC 3986 §5.2.4
    std::string remove_dot_segments(std::string_view path) {
        std::string input(path);
        std::string output;
        while (!input.empty()) {
            if (input.size() >= 3 && input.compare(0, 3, "../") == 0) {
                input.erase(0, 3);
            } else if (input.size() >= 2 && input.compare(0, 2, "./") == 0) {
                input.erase(0, 2);
            } else if (input.size() >= 3 && input.compare(0, 3, "/./") == 0) {
                input.replace(0, 3, "/");
            } else if (input == "/.") {
                input = "/";
            } else if (input.size() >= 4 && input.compare(0, 4, "/../") == 0) {
                input.replace(0, 4, "/");
                const auto slash = output.rfind('/');
                if (slash == std::string::npos) {
                    output.clear();
                } else {
                    output.erase(slash);
                }
            } else if (input == "/..") {
                input = "/";
                const auto slash = output.rfind('/');
                if (slash == std::string::npos) {
                    output.clear();
                } else {
                    output.erase(slash);
                }
            } else if (input == "." || input == "..") {
                input.clear();
            } else {
                size_t seg_end = 0;
                if (input[0] == '/') {
                    seg_end = input.find('/', 1);
                } else {
                    seg_end = input.find('/');
                }
                if (seg_end == std::string::npos) {
                    output += input;
                    input.clear();
                } else {
                    output.append(input, 0, seg_end);
                    input.erase(0, seg_end);
                }
            }
        }
        return output;
    }

}  // namespace

    RfcUri parse_rfc_uri(std::string_view user_input) {
        RfcUri ret;
        RfcParser::parse_url_no_base_impl(user_input, ret);
        return ret;
    }

    RfcUri parse_rfc_uri(std::string_view user_input, const RfcUri &base) {
        RfcUri ret;
        RfcParser::parse_url_with_base_impl(user_input, base, ret);
        return ret;
    }

    bool parse_rfc_uri(std::string_view user_input,RfcUri&uri) {
        RfcParser::parse_url_no_base_impl(user_input, uri);
        return uri.ok();
    }

    bool parse_rfc_uri(std::string_view user_input,RfcUri&uri,const RfcUri& base_url) {
        RfcParser::parse_url_with_base_impl(user_input, base_url, uri);
        return uri.ok();
    }


     void RfcParser::parse_url_no_base_impl(std::string_view url_data, RfcUri& uri) {
        if (url_data.size() > std::numeric_limits<uint32_t>::max()) {
            uri.uri_error() = { UriErrorCode::kUriOverflow, 0, "input overflow" };
        }
        if (!uri.ok()) {
            return;
        }
        ParserContext ctx(url_data);
        chain_parse_url_schema(ctx,uri);
    }

    void RfcParser::chain_parse_url_schema(ParserContext &ctx, RfcUri& uri) {
        auto & pos = ctx.pos;
        auto  &end = ctx.end;

        if ((pos != end) && ascii_isalpha(*pos)) {
            pos++;
        } else {
            // Otherwise, if state override is not given, set state to no scheme
            // state and decrease pointer by 1.
            uri.uri_error() = { UriErrorCode::kUriNotComplete, 0, "no schema" };
            return;
        }

        while ((pos != end) && (turbo::is_valid_schema_alnum(*pos))) {
            ++pos;
        }
        if ((pos != end) && (*pos == ':')) {
            std::string schema(ctx.start,pos);
            uri.set_scheme(std::move(schema));
            ++pos;
            chain_parse_url_hier_part(ctx, uri);
        } else {
            uri.uri_error() = { UriErrorCode::kUriNotComplete, 0, "no schema" };
        }
    }

    void RfcParser::chain_parse_url_hier_part(ParserContext &ctx, RfcUri& uri) {
        auto & pos = ctx.pos;
        auto & end = ctx.end;

        if (KUMO_UNLIKELY(pos == end)) {
            return;
        }
        if (*pos == '/') {
            ++pos;
            chain_parse_url_hier_part_two(ctx, uri);
        } else {
            chain_parse_path_root_less(ctx, uri);
        }
    }

    void RfcParser::chain_parse_url_hier_part_two(ParserContext &ctx, RfcUri& uri) {
        auto & pos = ctx.pos;
        auto & end = ctx.end;

        if (pos != end && *pos == '/') {
            /// http://
            ++pos;
            chain_parse_authority(ctx, uri);
        } else {
            /// http:/
            /// http:/a
            chain_parse_path_root_less(ctx, uri);
        }
    }

    void RfcParser::chain_parse_authority(ParserContext &ctx, RfcUri& uri) {
        auto & pos = ctx.pos;
        auto & end = ctx.end;

        auto at_pos = turbo::find_first_symbols<'@'>(pos, end);

        if (at_pos != end) {
            // have auth
            auto c_pos = turbo::find_first_symbols<':'>(pos, at_pos);
            uri.set_username(std::string_view(pos, c_pos - pos));
            if (c_pos != at_pos) {
                uri.set_password(std::string_view(c_pos + 1, at_pos - c_pos - 1));
            }
            pos = at_pos + 1;
        }
        if (pos == end) {
            return;
        }
        chain_parse_host(ctx,uri);
    }

    void RfcParser::chain_parse_host(ParserContext &ctx, RfcUri& uri) {
        auto & pos = ctx.pos;
        auto & end = ctx.end;

        std::optional<IpAddr> ip;
        auto r = try_parse_rfc_ip(pos, end, ip);
        if (!r.ok()) {
            uri.uri_error() = {r.code,
                static_cast<uint32_t>((pos - ctx.start) + r.error_pos), r.payload};
            return;
        }

        if (r.error_pos > 0) {
            uri.update_base_hostname(std::string_view(pos, r.error_pos));
            if (ip.has_value()) {
                if (ip->type == IpType::IP_V4) {
                    uri.host_type() = UriHostType::IPV4;
                } else if (ip->type == IpType::IP_V6) {
                    uri.host_type() = UriHostType::IPV6;
                }
            }
            pos += r.error_pos;
        } else {
            r = try_parse_rfc_reg_host(pos, end);
            if (!r.ok()) {
                uri.uri_error() = {r.code,
                    static_cast<uint32_t>((pos - ctx.start) + r.error_pos), r.payload};
                return;
            }
            uri.update_base_hostname(std::string_view(pos, r.error_pos));
            uri.host_type() = UriHostType::DEFAULT;
            pos += r.error_pos;
        }

        if (pos == end) {
            return;
        }

        switch (*pos) {
        case ':':
            ++pos;
            chain_parse_port(ctx, uri);
            break;
        case '/':
            chain_parse_path_abs_empty(ctx, uri);
            break;
        case '?':
            ++pos;
            chain_parse_query(ctx, uri);
            break;
        case '#':
            ++pos;
            chain_parse_fragment(ctx, uri);
            break;
        default:
            uri.uri_error() = {UriErrorCode::kUriInvalidArgs,
                static_cast<uint32_t>(pos - ctx.start), ""};
            break;
        }
    }

    void RfcParser::chain_parse_port(ParserContext &ctx, RfcUri& uri) {
        auto & pos = ctx.pos;
        auto & end = ctx.end;

        const char* port_start = pos;
        while (pos != end && turbo::ascii_isdigit(static_cast<unsigned char>(*pos))) {
            ++pos;
        }

        if (pos != port_start) {
            uint32_t value = 0;
            for (const char* p = port_start; p != pos; ++p) {
                value = value * 10u + static_cast<uint32_t>(*p - '0');
                if (value > 65535u) {
                    uri.uri_error() = {UriErrorCode::kUriOverflow,
                        static_cast<uint32_t>(p - ctx.start), ""};
                    return;
                }
            }
            uri.update_base_port(static_cast<uint16_t>(value));
        }

        if (pos == end) {
            return;
        }

        switch (*pos) {
        case '/':
            chain_parse_path_abs_empty(ctx, uri);
            break;
        case '?':
            ++pos;
            chain_parse_query(ctx, uri);
            break;
        case '#':
            ++pos;
            chain_parse_fragment(ctx, uri);
            break;
        default:
            uri.uri_error() = {UriErrorCode::kUriInvalidArgs,
                static_cast<uint32_t>(pos - ctx.start), ""};
            break;
        }
    }


    void RfcParser::chain_parse_path_abs_empty(ParserContext &ctx, RfcUri& uri) {
        // 0 illegal, 1 path char ('/' + pchar), 2 stop ('?'/'#'), 3 '%'
        static const std::array<uint8_t, 256> kClass = [] {
            std::array<uint8_t, 256> t {};
            for (int c = 'A'; c <= 'Z'; ++c) {
                t[static_cast<size_t>(c)] = 1;
            }
            for (int c = 'a'; c <= 'z'; ++c) {
                t[static_cast<size_t>(c)] = 1;
            }
            for (int c = '0'; c <= '9'; ++c) {
                t[static_cast<size_t>(c)] = 1;
            }
            for (unsigned char c : std::string_view("-._~!$&'()*+,;=:@/")) {
                t[c] = 1;
            }
            t[static_cast<unsigned char>('%')] = 3;
            t[static_cast<unsigned char>('?')] = 2;
            t[static_cast<unsigned char>('#')] = 2;
            return t;
        }();

        auto & pos = ctx.pos;
        auto & end = ctx.end;
        const char* path_start = pos;

        while (pos != end) {
            switch (kClass[static_cast<unsigned char>(*pos)]) {
            case 1:
                ++pos;
                continue;
            case 3: {
                if (pos + 2 >= end
                    || !turbo::ascii_isxdigit(static_cast<unsigned char>(pos[1]))
                    || !turbo::ascii_isxdigit(static_cast<unsigned char>(pos[2]))) {
                    uri.uri_error() = {UriErrorCode::kUriInvalidArgs,
                        static_cast<uint32_t>(pos - ctx.start), ""};
                    return;
                }
                pos += 3;
                continue;
            }
            case 2:
                break;
            default:
                uri.uri_error() = {UriErrorCode::kUriInvalidArgs,
                    static_cast<uint32_t>(pos - ctx.start), ""};
                return;
            }
            break;
        }

        uri.update_base_pathname(std::string_view(path_start, pos - path_start));

        if (pos == end) {
            return;
        }
        if (*pos == '?') {
            ++pos;
            chain_parse_query(ctx, uri);
        } else {
            ++pos;  // '#'
            chain_parse_fragment(ctx, uri);
        }
    }

    void RfcParser::chain_parse_query(ParserContext &ctx, RfcUri& uri) {
        // query = *( pchar / "/" / "?" ); stop at '#'
        // 0 illegal, 1 query char, 2 stop('#'), 3 '%'
        static const std::array<uint8_t, 256> kClass = [] {
            std::array<uint8_t, 256> t {};
            for (int c = 'A'; c <= 'Z'; ++c) {
                t[static_cast<size_t>(c)] = 1;
            }
            for (int c = 'a'; c <= 'z'; ++c) {
                t[static_cast<size_t>(c)] = 1;
            }
            for (int c = '0'; c <= '9'; ++c) {
                t[static_cast<size_t>(c)] = 1;
            }
            for (unsigned char c : std::string_view("-._~!$&'()*+,;=:@/?")) {
                t[c] = 1;
            }
            t[static_cast<unsigned char>('%')] = 3;
            t[static_cast<unsigned char>('#')] = 2;
            return t;
        }();

        auto & pos = ctx.pos;
        auto & end = ctx.end;
        const char* query_start = pos;

        while (pos != end) {
            switch (kClass[static_cast<unsigned char>(*pos)]) {
            case 1:
                ++pos;
                continue;
            case 3: {
                if (pos + 2 >= end
                    || !turbo::ascii_isxdigit(static_cast<unsigned char>(pos[1]))
                    || !turbo::ascii_isxdigit(static_cast<unsigned char>(pos[2]))) {
                    uri.uri_error() = {UriErrorCode::kUriInvalidArgs,
                        static_cast<uint32_t>(pos - ctx.start), ""};
                    return;
                }
                pos += 3;
                continue;
            }
            case 2:
                break;
            default:
                uri.uri_error() = {UriErrorCode::kUriInvalidArgs,
                    static_cast<uint32_t>(pos - ctx.start), ""};
                return;
            }
            break;
        }

        uri.update_base_search(std::string_view(query_start, pos - query_start));

        if (pos == end) {
            return;
        }
        ++pos;  // '#'
            chain_parse_fragment(ctx, uri);
    }

    void RfcParser::chain_parse_fragment(ParserContext &ctx, RfcUri& uri) {
        // fragment = *( pchar / "/" / "?" )
        // 0 illegal, 1 fragment char, 3 '%'
        static const std::array<uint8_t, 256> kClass = [] {
            std::array<uint8_t, 256> t {};
            for (int c = 'A'; c <= 'Z'; ++c) {
                t[static_cast<size_t>(c)] = 1;
            }
            for (int c = 'a'; c <= 'z'; ++c) {
                t[static_cast<size_t>(c)] = 1;
            }
            for (int c = '0'; c <= '9'; ++c) {
                t[static_cast<size_t>(c)] = 1;
            }
            for (unsigned char c : std::string_view("-._~!$&'()*+,;=:@/?")) {
                t[c] = 1;
            }
            t[static_cast<unsigned char>('%')] = 3;
            return t;
        }();

        auto & pos = ctx.pos;
        auto & end = ctx.end;
        const char* frag_start = pos;

        while (pos != end) {
            switch (kClass[static_cast<unsigned char>(*pos)]) {
            case 1:
                ++pos;
                continue;
            case 3: {
                if (pos + 2 >= end
                    || !turbo::ascii_isxdigit(static_cast<unsigned char>(pos[1]))
                    || !turbo::ascii_isxdigit(static_cast<unsigned char>(pos[2]))) {
                    uri.uri_error() = {UriErrorCode::kUriInvalidArgs,
                        static_cast<uint32_t>(pos - ctx.start), ""};
                    return;
                }
                pos += 3;
                continue;
            }
            default:
                uri.uri_error() = {UriErrorCode::kUriInvalidArgs,
                    static_cast<uint32_t>(pos - ctx.start), ""};
                return;
            }
        }

        uri.hash = std::string(frag_start, pos - frag_start);
    }

    void RfcParser::parse_url_with_base_impl(std::string_view user_input,
        const RfcUri& base_url, RfcUri& uri) {
        if (user_input.size() > std::numeric_limits<uint32_t>::max()) {
            uri.uri_error() = {UriErrorCode::kUriOverflow, 0, "input overflow"};
            return;
        }
        if (base_url.get_protocol().size() <= 1) {
            uri.uri_error() = {UriErrorCode::kUriInvalidArgs, 0, "relative base"};
            return;
        }
        if (user_input.empty()) {
            uri = base_url;
            return;
        }

        uri = base_url;
        ParserContext ctx(user_input);
        auto & pos = ctx.pos;
        auto & end = ctx.end;

        if (ascii_isalpha(*pos)) {
            const char* p = pos + 1;
            while (p != end && turbo::is_valid_schema_alnum(*p)) {
                ++p;
            }
            if (p != end && *p == ':') {
                uri = RfcUri{};
                uri.set_scheme(std::string(ctx.start, p));
                pos = p + 1;
                chain_parse_url_hier_part(ctx, uri);
                return;
            }
        }

        switch (*pos) {
        case '/':
            ++pos;
            if (pos != end && *pos == '/') {
                uri.username.clear();
                uri.password.clear();
                uri.host = std::nullopt;
                uri.port = std::nullopt;
                uri.clear_pathname();
                uri.clear_search();
                uri.hash = std::nullopt;
                uri.host_type() = UriHostType::DEFAULT;
                ++pos;
                chain_parse_authority(ctx, uri);
            } else {
                uri.clear_pathname();
                uri.clear_search();
                uri.hash = std::nullopt;
                --pos;
                chain_parse_path_abs_empty(ctx, uri);
                if (uri.ok()) {
                    uri.update_base_pathname(remove_dot_segments(uri.path));
                }
            }
            break;
        case '?':
            uri.clear_search();
            uri.hash = std::nullopt;
            ++pos;
            chain_parse_query(ctx, uri);
            break;
        case '#':
            uri.hash = std::nullopt;
            ++pos;
            chain_parse_fragment(ctx, uri);
            break;
        default: {
            const std::string base_path = uri.path;
            uri.clear_pathname();
            uri.clear_search();
            uri.hash = std::nullopt;
            chain_parse_path_root_less(ctx, uri);
            if (!uri.ok()) {
                return;
            }
            const std::string & rel = uri.path;
            std::string merged;
            if (uri.host.has_value() && base_path.empty()) {
                merged = std::string("/") + rel;
            } else {
                const auto slash = base_path.rfind('/');
                if (slash == std::string::npos) {
                    merged = rel;
                } else {
                    merged = base_path.substr(0, slash + 1) + rel;
                }
            }
            uri.update_base_pathname(remove_dot_segments(merged));
            break;
        }
        }
    }

    UriError RfcParser::try_parse_rfc_reg_host(const char* start, const char* end) {
        // 0 illegal, 1 reg-name char, 2 delimiter, 3 '%'
        static const std::array<uint8_t, 256> kClass = [] {
            std::array<uint8_t, 256> t {};
            for (int c = 'A'; c <= 'Z'; ++c) {
                t[static_cast<size_t>(c)] = 1;
            }
            for (int c = 'a'; c <= 'z'; ++c) {
                t[static_cast<size_t>(c)] = 1;
            }
            for (int c = '0'; c <= '9'; ++c) {
                t[static_cast<size_t>(c)] = 1;
            }
            for (unsigned char c : std::string_view("-._~!$&'()*+,;=")) {
                t[c] = 1;
            }
            t[static_cast<unsigned char>('%')] = 3;
            t[static_cast<unsigned char>(':')] = 2;
            t[static_cast<unsigned char>('/')] = 2;
            t[static_cast<unsigned char>('?')] = 2;
            t[static_cast<unsigned char>('#')] = 2;
            return t;
        }();

        const char* p = start;
        while (p != end) {
            switch (kClass[static_cast<unsigned char>(*p)]) {
            case 1:
                ++p;
                continue;
            case 3: {
                if (p + 2 >= end
                    || !turbo::ascii_isxdigit(static_cast<unsigned char>(p[1]))
                    || !turbo::ascii_isxdigit(static_cast<unsigned char>(p[2]))) {
                    return {UriErrorCode::kUriInvalidArgs,
                        static_cast<uint32_t>(p - start), ""};
                }
                p += 3;
                continue;
            }
            case 2:
                return {UriErrorCode::kUriSuccess, static_cast<uint32_t>(p - start), ""};
            default:
                return {UriErrorCode::kUriInvalidArgs,
                    static_cast<uint32_t>(p - start), ""};
            }
        }
        return {UriErrorCode::kUriSuccess, static_cast<uint32_t>(p - start), ""};
    }


   void RfcParser::chain_parse_path_root_less(ParserContext &ctx, RfcUri& uri) {
        // path-rootless = segment-nz *( "/" segment )
        // 0 illegal, 1 path char, 2 stop ('?'/'#'), 3 '%'
        static const std::array<uint8_t, 256> kClass = [] {
            std::array<uint8_t, 256> t {};
            for (int c = 'A'; c <= 'Z'; ++c) {
                t[static_cast<size_t>(c)] = 1;
            }
            for (int c = 'a'; c <= 'z'; ++c) {
                t[static_cast<size_t>(c)] = 1;
            }
            for (int c = '0'; c <= '9'; ++c) {
                t[static_cast<size_t>(c)] = 1;
            }
            for (unsigned char c : std::string_view("-._~!$&'()*+,;=:@/")) {
                t[c] = 1;
            }
            t[static_cast<unsigned char>('%')] = 3;
            t[static_cast<unsigned char>('?')] = 2;
            t[static_cast<unsigned char>('#')] = 2;
            return t;
        }();

        auto & pos = ctx.pos;
        auto & end = ctx.end;

        if (pos == end
            || kClass[static_cast<unsigned char>(*pos)] == 0
            || kClass[static_cast<unsigned char>(*pos)] == 2) {
            uri.uri_error() = {UriErrorCode::kUriInvalidArgs,
                static_cast<uint32_t>(pos - ctx.start), "path-rootless empty"};
            return;
        }

        const char* path_start = pos;
        while (pos != end) {
            switch (kClass[static_cast<unsigned char>(*pos)]) {
            case 1:
                ++pos;
                continue;
            case 3: {
                if (pos + 2 >= end
                    || !turbo::ascii_isxdigit(static_cast<unsigned char>(pos[1]))
                    || !turbo::ascii_isxdigit(static_cast<unsigned char>(pos[2]))) {
                    uri.uri_error() = {UriErrorCode::kUriInvalidArgs,
                        static_cast<uint32_t>(pos - ctx.start), ""};
                    return;
                }
                pos += 3;
                continue;
            }
            case 2:
                break;
            default:
                uri.uri_error() = {UriErrorCode::kUriInvalidArgs,
                    static_cast<uint32_t>(pos - ctx.start), ""};
                return;
            }
            break;
        }

        uri.update_base_pathname(std::string_view(path_start, pos - path_start));

        if (pos == end) {
            return;
        }
        if (*pos == '?') {
            ++pos;
            chain_parse_query(ctx, uri);
        } else {
            ++pos;  // '#'
            chain_parse_fragment(ctx, uri);
        }
    }
}  // namespace turbo
