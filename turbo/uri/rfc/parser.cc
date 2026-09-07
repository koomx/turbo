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

#include <turbo/uri/uri_view.h>
#include <turbo/uri/rfc/parser.h>
#include <turbo/uri/uri_error.h>
#include <turbo/uri/ip.h>
#include <turbo/uri/utility.h>
#include <turbo/uri/character_sets.h>
#include <turbo/strings/find_symbols.h>
#include <turbo/strings/ascii.h>
#include <array>
#include <limits>
#include <optional>
#include <string>


namespace turbo {
namespace {

    // RFC 3986 §5.2.4 — scan path, single output buffer
    std::string remove_dot_segments(std::string_view path) {
        std::string output;
        output.reserve(path.size());
        const char* p = path.data();
        const char* end = p + path.size();
        while (p != end) {
            const size_t rem = static_cast<size_t>(end - p);
            if (rem >= 3 && p[0] == '.' && p[1] == '.' && p[2] == '/') {
                p += 3;
            } else if (rem >= 2 && p[0] == '.' && p[1] == '/') {
                p += 2;
            } else if (rem >= 3 && p[0] == '/' && p[1] == '.' && p[2] == '/') {
                p += 2;  // leave '/' for next
            } else if (rem == 2 && p[0] == '/' && p[1] == '.') {
                output.push_back('/');
                p = end;
            } else if (rem >= 4 && p[0] == '/' && p[1] == '.' && p[2] == '.'
                && p[3] == '/') {
                p += 3;  // leave '/'
                const auto slash = output.rfind('/');
                if (slash == std::string::npos) {
                    output.clear();
                } else {
                    output.erase(slash);
                }
            } else if (rem == 3 && p[0] == '/' && p[1] == '.' && p[2] == '.') {
                p = end;
                const auto slash = output.rfind('/');
                if (slash == std::string::npos) {
                    output.clear();
                } else {
                    output.erase(slash);
                }
                output.push_back('/');
            } else if ((rem == 1 && p[0] == '.') || (rem == 2 && p[0] == '.' && p[1] == '.')) {
                p = end;
            } else {
                const char* seg = p;
                if (*p == '/') {
                    ++p;
                    while (p != end && *p != '/') {
                        ++p;
                    }
                } else {
                    while (p != end && *p != '/') {
                        ++p;
                    }
                }
                output.append(seg, p);
            }
        }
        return output;
    }

    std::string merge_paths(std::string_view base_path, std::string_view rel_path,
        bool base_has_host) {
        if (base_has_host && base_path.empty()) {
            std::string out;
            out.reserve(rel_path.size() + 1);
            out.push_back('/');
            out.append(rel_path);
            return out;
        }
        const auto slash = base_path.rfind('/');
        std::string out;
        if (slash == std::string_view::npos) {
            out.assign(rel_path);
        } else {
            out.reserve(slash + 1 + rel_path.size());
            out.append(base_path.data(), slash + 1);
            out.append(rel_path);
        }
        return out;
    }

    void append_authority(std::string &out, const UriView &u) {
        out.append("//");
        if (!u.username().empty() || !u.password().empty()) {
            out.append(u.username());
            if (!u.password().empty()) {
                out.push_back(':');
                out.append(u.password());
            }
            out.push_back('@');
        }
        out.append(u.host());
        if (!u.port().empty()) {
            out.push_back(':');
            out.append(u.port());
        }
    }

    std::string serialize_uri(const UriView &u) {
        std::string out;
        out.append(u.shema());
        out.push_back(':');
        if (u.has_host()) {
            append_authority(out, u);
        }
        out.append(u.path());
        if (u.has_query()) {
            out.push_back('?');
            out.append(u.query());
        }
        if (u.has_fragment()) {
            out.push_back('#');
            out.append(u.fragment());
        }
        return out;
    }

    std::string apply_rfc_codec(std::string_view s, const uint8_t *encode_set,
        bool to_percent) {
        if (to_percent) {
            return percent_encode(s, encode_set);
        }
        const auto pos = s.find('%');
        if (pos == std::string_view::npos) {
            return std::string(s);
        }
        return percent_decode(s, pos);
    }

}  // namespace

    UriView parse_rfc_uri(std::string_view user_input) {
        return RfcParser::parse_url_no_base_impl(user_input);
    }

    UriView parse_rfc_uri(std::string_view user_input, const UriView &base) {
        return RfcParser::parse_url_with_base_impl(user_input, base);
    }

    UriView merge_rfc_uri(std::string_view user_input, const UriView &base_url) {
        if (base_url.shema().empty()) {
            UriView err;
            err.uri_error({UriErrorCode::kUriInvalidArgs, 0, "relative base"});
            return err;
        }
        UriView rel = RfcParser::parse_url_with_base_impl(user_input, base_url);
        if (!rel.ok()) {
            return rel;
        }

        std::string scheme;
        std::string path;
        std::string query;
        std::string frag;
        bool has_auth = false;
        bool has_query = false;
        bool has_frag = rel.has_fragment();
        const UriView *auth = &base_url;

        if (has_frag) {
            frag = std::string(rel.fragment());
        }

        if (!rel.shema().empty()) {
            scheme = std::string(rel.shema());
            has_auth = rel.has_host();
            auth = &rel;
            path = remove_dot_segments(rel.path());
            has_query = rel.has_query();
            if (has_query) {
                query = std::string(rel.query());
            }
        } else {
            scheme = std::string(base_url.shema());
            if (rel.has_host()) {
                has_auth = true;
                auth = &rel;
                path = remove_dot_segments(rel.path());
                has_query = rel.has_query();
                if (has_query) {
                    query = std::string(rel.query());
                }
            } else {
                has_auth = base_url.has_host();
                auth = &base_url;
                if (rel.path().empty() && !rel.has_path()) {
                    path = std::string(base_url.path());
                    if (rel.has_query()) {
                        has_query = true;
                        query = std::string(rel.query());
                    } else if (base_url.has_query()) {
                        has_query = true;
                        query = std::string(base_url.query());
                    }
                } else if (!rel.path().empty() && rel.path()[0] == '/') {
                    path = remove_dot_segments(rel.path());
                    has_query = rel.has_query();
                    if (has_query) {
                        query = std::string(rel.query());
                    }
                } else if (rel.has_path()) {
                    path = remove_dot_segments(
                        merge_paths(base_url.path(), rel.path(), base_url.has_host()));
                    has_query = rel.has_query();
                    if (has_query) {
                        query = std::string(rel.query());
                    }
                } else {
                    path = std::string(base_url.path());
                    if (rel.has_query()) {
                        has_query = true;
                        query = std::string(rel.query());
                    } else if (base_url.has_query()) {
                        has_query = true;
                        query = std::string(base_url.query());
                    }
                }
            }
        }

        std::string merged;
        merged.reserve(scheme.size() + path.size() + query.size() + frag.size() + 32);
        merged.append(scheme);
        merged.push_back(':');
        if (has_auth) {
            append_authority(merged, *auth);
        }
        merged.append(path);
        if (has_query) {
            merged.push_back('?');
            merged.append(query);
        }
        if (has_frag) {
            merged.push_back('#');
            merged.append(frag);
        }

        UriView out = RfcParser::parse_url_no_base_impl(merged);
        out.set_ownd(std::move(merged));
        return out;
    }

    UriView encode_rfc_uri(const UriView &uri) {
        if (uri.standard() != StandType::STD_RFC) {
            UriView err = uri;
            err.uri_error({UriErrorCode::kUriInvalidArgs, 0, "standard mismatch"});
            return err;
        }
        if (uri.encode_type() == EnodeType::PRECENT) {
            return uri;
        }
        return RfcParser::convert_encode_type(uri, EnodeType::PRECENT);
    }

    UriView decode_rfc_uri(const UriView &uri) {
        if (uri.standard() != StandType::STD_RFC) {
            UriView err = uri;
            err.uri_error({UriErrorCode::kUriInvalidArgs, 0, "standard mismatch"});
            return err;
        }
        if (uri.encode_type() == EnodeType::PLAIN) {
            return uri;
        }
        return RfcParser::convert_encode_type(uri, EnodeType::PLAIN);
    }

    std::string get_rfc_href(const UriView &ref_url) {
        if (!ref_url.ok() || !ref_url.has_shema()) {
            return {};
        }
        return serialize_uri(ref_url);
    }

    std::string get_rfc_href(const UriView &base_url, const UriView &ref_url) {
        UriView merged = merge_rfc_uri(ref_url.origin(), base_url);
        if (!merged.ok()) {
            return {};
        }
        return serialize_uri(merged);
    }

    UriView RfcParser::convert_encode_type(const UriView &uri, EnodeType target) {
        const bool to_percent = target == EnodeType::PRECENT;
        std::string buf;
        buf.reserve(uri.origin().size() * (to_percent ? 3 : 1));
        UriView out;

        auto take = [&](uint32_t start) {
            return ComponentView{start, static_cast<uint32_t>(buf.size())};
        };

        if (uri.has_shema()) {
            const uint32_t start = static_cast<uint32_t>(buf.size());
            buf.append(uri.shema());
            out.shema(take(start));
        }
        buf.push_back(':');

        if (uri.has_host()) {
            buf.append("//");
            if (uri.has_username() || uri.has_password()) {
                if (uri.has_username()) {
                    const uint32_t start = static_cast<uint32_t>(buf.size());
                    buf.append(apply_rfc_codec(uri.username(),
                        uri_charsets::USERINFO_PERCENT_ENCODE, to_percent));
                    out.username(take(start));
                }
                if (uri.has_password()) {
                    buf.push_back(':');
                    const uint32_t start = static_cast<uint32_t>(buf.size());
                    buf.append(apply_rfc_codec(uri.password(),
                        uri_charsets::USERINFO_PERCENT_ENCODE, to_percent));
                    out.password(take(start));
                }
                buf.push_back('@');
            }
            {
                const uint32_t start = static_cast<uint32_t>(buf.size());
                if (uri.host_type() == UriHostType::DEFAULT) {
                    buf.append(apply_rfc_codec(uri.host(),
                        uri_charsets::C0_CONTROL_PERCENT_ENCODE, to_percent));
                } else {
                    buf.append(uri.host());
                }
                out.host(take(start));
            }
            if (uri.has_port()) {
                buf.push_back(':');
                const uint32_t start = static_cast<uint32_t>(buf.size());
                buf.append(uri.port());
                out.port(take(start));
            }
        }

        if (uri.has_path()) {
            const uint32_t start = static_cast<uint32_t>(buf.size());
            buf.append(apply_rfc_codec(uri.path(),
                uri_charsets::PATH_PERCENT_ENCODE, to_percent));
            out.path(take(start));
        }
        if (uri.has_query()) {
            buf.push_back('?');
            const uint32_t start = static_cast<uint32_t>(buf.size());
            buf.append(apply_rfc_codec(uri.query(),
                uri_charsets::QUERY_PERCENT_ENCODE, to_percent));
            out.query(take(start));
        }
        if (uri.has_fragment()) {
            buf.push_back('#');
            const uint32_t start = static_cast<uint32_t>(buf.size());
            buf.append(apply_rfc_codec(uri.fragment(),
                uri_charsets::FRAGMENT_PERCENT_ENCODE, to_percent));
            out.fragment(take(start));
        }

        out.set_ownd(std::move(buf));
        out.standard(StandType::STD_RFC);
        out.encode_type(target);
        out.host_type(uri.host_type());
        return out;
    }

    UriView RfcParser::parse_url_no_base_impl(std::string_view url_data) {
        UriView uri(url_data);
        uri.standard(StandType::STD_RFC);
        if (url_data.size() > std::numeric_limits<uint32_t>::max()) {
            uri.uri_error({ UriErrorCode::kUriOverflow, 0, "input overflow" });
        }
        if (!uri.ok()) {
            return uri;
        }
        ParserContext ctx(url_data);
        chain_parse_url_schema(ctx,uri);
        return uri;
    }

    void RfcParser::chain_parse_url_schema(ParserContext &ctx, UriView& uri) {
        auto & pos = ctx.pos;
        auto  &end = ctx.end;

        if ((pos != end) && ascii_isalpha(*pos)) {
            pos++;
        } else {
            // Otherwise, if state override is not given, set state to no scheme
            // state and decrease pointer by 1.
            uri.uri_error({ UriErrorCode::kUriNotComplete, 0, "no schema" });
            return;
        }

        while ((pos != end) && (turbo::is_valid_schema_alnum(*pos))) {
            ++pos;
        }
        if ((pos != end) && (*pos == ':')) {
            uri.shema({0, static_cast<uint32_t>(pos - ctx.start)});
            ++pos;
            chain_parse_url_hier_part(ctx, uri);
        } else {
            uri.uri_error({ UriErrorCode::kUriNotComplete, 0, "no schema" });
        }
    }

    void RfcParser::chain_parse_url_hier_part(ParserContext &ctx, UriView& uri) {
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

    void RfcParser::chain_parse_url_hier_part_two(ParserContext &ctx, UriView& uri) {
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

    void RfcParser::chain_parse_authority(ParserContext &ctx, UriView& uri) {
        auto & pos = ctx.pos;
        auto & end = ctx.end;

        auto at_pos = turbo::find_first_symbols<'@'>(pos, end);

        if (at_pos != end) {
            // have auth
            auto c_pos = turbo::find_first_symbols<':'>(pos, at_pos);
            uri.username({static_cast<uint32_t>(pos - ctx.start),
                static_cast<uint32_t>(c_pos - ctx.start)});
            if (c_pos != at_pos) {
                uri.password({static_cast<uint32_t>(c_pos + 1 - ctx.start),
                    static_cast<uint32_t>(at_pos - ctx.start)});
            }
            pos = at_pos + 1;
        }
        if (pos == end) {
            return;
        }
        chain_parse_host(ctx,uri);
    }

    void RfcParser::chain_parse_host(ParserContext &ctx, UriView& uri) {
        auto & pos = ctx.pos;
        auto & end = ctx.end;

        std::optional<IpAddr> ip;
        auto r = try_parse_rfc_ip(pos, end, ip);
        if (!r.ok()) {
            uri.uri_error({r.code,
                static_cast<uint32_t>((pos - ctx.start) + r.error_pos), r.payload});
            return;
        }

        if (r.error_pos > 0) {
            uri.host({static_cast<uint32_t>(pos - ctx.start),
                static_cast<uint32_t>(pos - ctx.start + r.error_pos)});
            if (ip.has_value()) {
                if (ip->type == IpType::IP_V4) {
                    uri.host_type(UriHostType::IPV4);
                } else if (ip->type == IpType::IP_V6) {
                    uri.host_type(UriHostType::IPV6);
                }
            }
            pos += r.error_pos;
        } else {
            r = try_parse_rfc_reg_host(pos, end);
            if (!r.ok()) {
                uri.uri_error({r.code,
                    static_cast<uint32_t>((pos - ctx.start) + r.error_pos), r.payload});
                return;
            }
            uri.host({static_cast<uint32_t>(pos - ctx.start),
                static_cast<uint32_t>(pos - ctx.start + r.error_pos)});
            uri.host_type(UriHostType::DEFAULT);
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
            uri.uri_error({UriErrorCode::kUriInvalidArgs,
                static_cast<uint32_t>(pos - ctx.start), ""});
            break;
        }
    }

    void RfcParser::chain_parse_port(ParserContext &ctx, UriView& uri) {
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
                    uri.uri_error({UriErrorCode::kUriOverflow,
                        static_cast<uint32_t>(p - ctx.start), ""});
                    return;
                }
            }
            uri.port({static_cast<uint32_t>(port_start - ctx.start),
                static_cast<uint32_t>(pos - ctx.start)});
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
            uri.uri_error({UriErrorCode::kUriInvalidArgs,
                static_cast<uint32_t>(pos - ctx.start), ""});
            break;
        }
    }


    void RfcParser::chain_parse_path_abs_empty(ParserContext &ctx, UriView& uri) {
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
                    uri.uri_error({UriErrorCode::kUriInvalidArgs,
                        static_cast<uint32_t>(pos - ctx.start), ""});
                    return;
                }
                pos += 3;
                continue;
            }
            case 2:
                break;
            default:
                uri.uri_error({UriErrorCode::kUriInvalidArgs,
                    static_cast<uint32_t>(pos - ctx.start), ""});
                return;
            }
            break;
        }

        uri.path({static_cast<uint32_t>(path_start - ctx.start),
            static_cast<uint32_t>(pos - ctx.start)});

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

    void RfcParser::chain_parse_query(ParserContext &ctx, UriView& uri) {
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
                    uri.uri_error({UriErrorCode::kUriInvalidArgs,
                        static_cast<uint32_t>(pos - ctx.start), ""});
                    return;
                }
                pos += 3;
                continue;
            }
            case 2:
                break;
            default:
                uri.uri_error({UriErrorCode::kUriInvalidArgs,
                    static_cast<uint32_t>(pos - ctx.start), ""});
                return;
            }
            break;
        }

        uri.query({static_cast<uint32_t>(query_start - ctx.start),
            static_cast<uint32_t>(pos - ctx.start)});

        if (pos == end) {
            return;
        }
        ++pos;  // '#'
            chain_parse_fragment(ctx, uri);
    }

    void RfcParser::chain_parse_fragment(ParserContext &ctx, UriView& uri) {
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
                    uri.uri_error({UriErrorCode::kUriInvalidArgs,
                        static_cast<uint32_t>(pos - ctx.start), ""});
                    return;
                }
                pos += 3;
                continue;
            }
            default:
                uri.uri_error({UriErrorCode::kUriInvalidArgs,
                    static_cast<uint32_t>(pos - ctx.start), ""});
                return;
            }
        }

        uri.fragment({static_cast<uint32_t>(frag_start - ctx.start),
            static_cast<uint32_t>(pos - ctx.start)});
    }

    UriView RfcParser::parse_url_with_base_impl(std::string_view user_input,
        const UriView& base_url) {
        UriView uri(user_input);
        uri.standard(StandType::STD_RFC);
        if (user_input.size() > std::numeric_limits<uint32_t>::max()) {
            uri.uri_error({UriErrorCode::kUriOverflow, 0, "input overflow"});
            return uri;
        }
        if (base_url.shema().empty()) {
            uri.uri_error({UriErrorCode::kUriInvalidArgs, 0, "relative base"});
            return uri;
        }
        if (user_input.empty()) {
            return uri;
        }

        ParserContext ctx(user_input);
        auto & pos = ctx.pos;
        auto & end = ctx.end;

        if (ascii_isalpha(*pos)) {
            const char* p = pos + 1;
            while (p != end && turbo::is_valid_schema_alnum(*p)) {
                ++p;
            }
            if (p != end && *p == ':') {
                uri.shema({0, static_cast<uint32_t>(p - ctx.start)});
                pos = p + 1;
                chain_parse_url_hier_part(ctx, uri);
                return uri;
            }
        }

        switch (*pos) {
        case '/':
            ++pos;
            if (pos != end && *pos == '/') {
                ++pos;
                chain_parse_authority(ctx, uri);
            } else {
                --pos;
                chain_parse_path_abs_empty(ctx, uri);
            }
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
            chain_parse_path_root_less(ctx, uri);
            break;
        }
        return uri;
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


   void RfcParser::chain_parse_path_root_less(ParserContext &ctx, UriView& uri) {
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
            uri.uri_error({UriErrorCode::kUriInvalidArgs,
                static_cast<uint32_t>(pos - ctx.start), "path-rootless empty"});
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
                    uri.uri_error({UriErrorCode::kUriInvalidArgs,
                        static_cast<uint32_t>(pos - ctx.start), ""});
                    return;
                }
                pos += 3;
                continue;
            }
            case 2:
                break;
            default:
                uri.uri_error({UriErrorCode::kUriInvalidArgs,
                    static_cast<uint32_t>(pos - ctx.start), ""});
                return;
            }
            break;
        }

        uri.path({static_cast<uint32_t>(path_start - ctx.start),
            static_cast<uint32_t>(pos - ctx.start)});

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
