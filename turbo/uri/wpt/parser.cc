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

#include <turbo/uri/wpt/parser.h>
#include <turbo/uri/scheme.h>
#include <turbo/uri/utility.h>
#include <turbo/uri/uri_error.h>
#include <turbo/uri/character_sets.h>
#include <turbo/uri/checkers.h>
#include <turbo/uri/wpt/parse_components.h>
#include <turbo/strings/ascii.h>
#include <turbo/strings/match.h>
#include <turbo/strings/substring.h>
#include <array>
#include <limits>
#include <optional>
#include <string>

namespace turbo {

    UriView parse_wpt_uri(std::string_view user_input) {
        WptParser parser;
        return parser.parse_url_no_base_impl(user_input);
    }

    UriView parse_wpt_uri(std::string_view user_input, const UriView &base_url) {
        WptParser parser;
        return parser.parse_url_with_base_impl(user_input, base_url);
    }

    namespace {

    std::string apply_wpt_codec(std::string_view s, const uint8_t *encode_set,
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

    // WPT: tab/LF/CR are ignored while scanning — never percent-encoded, never kept.
    std::string percent_encode_wpt(std::string_view input, const uint8_t *encode_set) {
        std::string result;
        result.reserve(input.size());
        for (unsigned char c : input) {
            if (is_ascii_tab_or_newline(static_cast<char>(c))) {
                continue;
            }
            if (uri_charsets::bit_at(encode_set, c)) {
                result.append(uri_charsets::hex + c * 4, 3);
            } else {
                result.push_back(static_cast<char>(c));
            }
        }
        return result;
    }

    bool consume_two_slashes(const char *&pos, const char *end) {
        const char *p = pos;
        skip_ascii_tab_or_newline(p, end);
        if (p == end || *p != '/') {
            return false;
        }
        ++p;
        skip_ascii_tab_or_newline(p, end);
        if (p == end || *p != '/') {
            return false;
        }
        ++p;
        pos = p;
        return true;
    }

    void append_wpt_authority(std::string &out, const UriView &u) {
        out.append("//");
        if (u.has_username() || u.has_password()) {
            out.append(u.username());
            if (u.has_password()) {
                out.push_back(':');
                out.append(u.password());
            }
            out.push_back('@');
        }
        out.append(u.host());
        if (u.has_port() && !u.port().empty()) {
            out.push_back(':');
            out.append(u.port());
        }
    }

    std::string serialize_wpt_uri(const UriView &u) {
        std::string out;
        out.append(u.shema());
        out.push_back(':');
        if (u.has_host()) {
            append_wpt_authority(out, u);
        } else if (u.path().size() >= 2 && u.path()[0] == '/' && u.path()[1] == '/') {
            // cannot-be-a-base URL path starting with // → insert "/."
            out.append("/.");
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

    }  // namespace

    UriView merge_wpt_uri(std::string_view user_input, const UriView &base_url) {
        if (!base_url.ok() || !base_url.has_shema()) {
            UriView err;
            err.uri_error({UriErrorCode::kUriInvalidArgs, 0, "relative base"});
            return err;
        }
        return parse_wpt_uri(user_input, base_url);
    }

    UriView encode_wpt_uri(const UriView &uri) {
        if (uri.standard() != StandType::STD_WPT) {
            UriView err = uri;
            err.uri_error({UriErrorCode::kUriInvalidArgs, 0, "standard mismatch"});
            return err;
        }
        if (uri.encode_type() == EnodeType::PRECENT) {
            return uri;
        }
        return WptParser::convert_encode_type(uri, EnodeType::PRECENT);
    }

    UriView decode_wpt_uri(const UriView &uri) {
        if (uri.standard() != StandType::STD_WPT) {
            UriView err = uri;
            err.uri_error({UriErrorCode::kUriInvalidArgs, 0, "standard mismatch"});
            return err;
        }
        if (uri.encode_type() == EnodeType::PLAIN) {
            return uri;
        }
        return WptParser::convert_encode_type(uri, EnodeType::PLAIN);
    }

    std::string get_wpt_href(const UriView &ref_url) {
        if (!ref_url.ok() || !ref_url.has_shema()) {
            return {};
        }
        return serialize_wpt_uri(ref_url);
    }

    std::string get_wpt_href(const UriView &base_url, const UriView &ref_url) {
        UriView merged = merge_wpt_uri(ref_url.origin(), base_url);
        if (!merged.ok()) {
            return {};
        }
        return serialize_wpt_uri(merged);
    }

    UriView WptParser::convert_encode_type(const UriView &uri, EnodeType target) {
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
                    buf.append(apply_wpt_codec(uri.username(),
                        uri_charsets::USERINFO_PERCENT_ENCODE, to_percent));
                    out.username(take(start));
                }
                if (uri.has_password()) {
                    buf.push_back(':');
                    const uint32_t start = static_cast<uint32_t>(buf.size());
                    buf.append(apply_wpt_codec(uri.password(),
                        uri_charsets::USERINFO_PERCENT_ENCODE, to_percent));
                    out.password(take(start));
                }
                buf.push_back('@');
            }
            {
                const uint32_t start = static_cast<uint32_t>(buf.size());
                if (uri.host_type() == UriHostType::DEFAULT) {
                    buf.append(apply_wpt_codec(uri.host(),
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
            const uint8_t *path_set = uri.has_host()
                ? uri_charsets::PATH_PERCENT_ENCODE
                : uri_charsets::C0_CONTROL_PERCENT_ENCODE;
            buf.append(apply_wpt_codec(uri.path(), path_set, to_percent));
            out.path(take(start));
        }
        if (uri.has_query()) {
            buf.push_back('?');
            const uint32_t start = static_cast<uint32_t>(buf.size());
            const uint8_t *qset = uri.is_special()
                ? uri_charsets::SPECIAL_QUERY_PERCENT_ENCODE
                : uri_charsets::QUERY_PERCENT_ENCODE;
            buf.append(apply_wpt_codec(uri.query(), qset, to_percent));
            out.query(take(start));
        }
        if (uri.has_fragment()) {
            buf.push_back('#');
            const uint32_t start = static_cast<uint32_t>(buf.size());
            buf.append(apply_wpt_codec(uri.fragment(),
                uri_charsets::FRAGMENT_PERCENT_ENCODE, to_percent));
            out.fragment(take(start));
        }

        out.set_ownd(std::move(buf));
        out.standard(StandType::STD_WPT);
        out.encode_type(target);
        out.host_type(uri.host_type());
        out.schema_type(uri.schema_type());
        return out;
    }

    UriView WptParser::parse_url_no_base_impl(std::string_view url_data) {
        _base = nullptr;
        UriView uri(url_data);
        uri.standard(StandType::STD_WPT);
        if (url_data.size() > std::numeric_limits<uint32_t>::max()) {
            uri.uri_error({ UriErrorCode::kUriOverflow, 0, "input overflow" });
        }
        if (!uri.ok()) {
            return uri;
        }
        ParserContext ctx(url_data);
        chain_parse_url_schema(ctx, uri);
        return uri;
    }

    UriView WptParser::parse_url_with_base_impl(std::string_view url_data,
        const UriView &base_url) {
        UriView uri(url_data);
        uri.standard(StandType::STD_WPT);
        if (url_data.size() > std::numeric_limits<uint32_t>::max()) {
            uri.uri_error({ UriErrorCode::kUriOverflow, 0, "input overflow" });
        }
        if (!base_url.ok()) {
            uri.uri_error(base_url.uri_error());
        }
        if (!uri.ok()) {
            return uri;
        }
        _base = &base_url;
        ParserContext ctx(url_data);
        chain_parse_url_schema(ctx, uri);
        return uri;
    }

    bool WptParser::base_has_opaque_path() const {
        if (_base == nullptr || _base->is_special() || _base->has_host()) {
            return false;
        }
        // Opaque if path is empty or does not start with '/' (cannot-be-a-base).
        const std::string_view p = _base->path();
        return p.empty() || p[0] != '/';
    }

    const uint8_t *WptParser::query_encode_set(const UriView &uri) const {
        return uri.is_special() ? uri_charsets::SPECIAL_QUERY_PERCENT_ENCODE
                                : uri_charsets::QUERY_PERCENT_ENCODE;
    }

    void WptParser::finish(UriView& uri) {
        if (_pending_fragment.has_value()) {
            _buffer.push_back('#');
            const uint32_t start = static_cast<uint32_t>(_buffer.size());
            _buffer.append(percent_encode_wpt(*_pending_fragment,
                uri_charsets::FRAGMENT_PERCENT_ENCODE));
            uri.fragment({start, static_cast<uint32_t>(_buffer.size())});
            _pending_fragment.reset();
        }
        uri.set_ownd(std::move(_buffer));
    }

    void WptParser::set_scheme_from_base(UriView& uri) {
        _buffer.clear();
        const uint32_t start = static_cast<uint32_t>(_buffer.size());
        _buffer.append(_base->shema());
        uri.shema({start, static_cast<uint32_t>(_buffer.size())});
        uri.schema_type(_base->schema_type());
        _buffer.push_back(':');
    }

    void WptParser::set_empty_host(UriView& uri) {
        _buffer.append("//");
        const uint32_t start = static_cast<uint32_t>(_buffer.size());
        uri.host({start, start});
    }

    std::string_view WptParser::buffer_slice(const std::optional<ComponentView> &c) const {
        if (!c.has_value() || c->start == ComponentView::npos || c->end < c->start) {
            return {};
        }
        return std::string_view(_buffer.data() + c->start,
            static_cast<size_t>(c->end - c->start));
    }

    void WptParser::clear_path(UriView& uri) {
        if (uri._path.has_value()) {
            _buffer.resize(uri._path->start);
            uri.reset_path();
        }
    }

    void WptParser::append_authority_from_base(UriView& uri) {
        if (!_base->has_host()) {
            return;
        }
        _buffer.append("//");
        if (_base->has_username() || _base->has_password()) {
            if (_base->has_username()) {
                const uint32_t start = static_cast<uint32_t>(_buffer.size());
                _buffer.append(_base->username());
                uri.username({start, static_cast<uint32_t>(_buffer.size())});
            }
            if (_base->has_password()) {
                _buffer.push_back(':');
                const uint32_t start = static_cast<uint32_t>(_buffer.size());
                _buffer.append(_base->password());
                uri.password({start, static_cast<uint32_t>(_buffer.size())});
            }
            _buffer.push_back('@');
        }
        {
            const uint32_t start = static_cast<uint32_t>(_buffer.size());
            _buffer.append(_base->host());
            uri.host({start, static_cast<uint32_t>(_buffer.size())});
            uri.host_type(_base->host_type());
        }
        if (_base->has_port()) {
            _buffer.push_back(':');
            const uint32_t start = static_cast<uint32_t>(_buffer.size());
            _buffer.append(_base->port());
            uri.port({start, static_cast<uint32_t>(_buffer.size())});
        }
    }

    void WptParser::append_path_from_base(UriView& uri) {
        if (!_base->has_path()) {
            return;
        }
        const uint32_t start = static_cast<uint32_t>(_buffer.size());
        _buffer.append(_base->path());
        uri.path({start, static_cast<uint32_t>(_buffer.size())});
    }

    void WptParser::append_query_from_base(UriView& uri) {
        if (!_base->has_query()) {
            return;
        }
        _buffer.push_back('?');
        const uint32_t start = static_cast<uint32_t>(_buffer.size());
        _buffer.append(_base->query());
        uri.query({start, static_cast<uint32_t>(_buffer.size())});
    }

    void WptParser::chain_parse_url_schema(ParserContext &ctx, UriView& uri) {
        auto &pos = ctx.pos;
        auto &end = ctx.end;
        static constexpr auto Headerset = []() {
            std::array<uint8_t, 256> table{};
            for (int i = 0; i <= 0x20; ++i) {
                table[static_cast<size_t>(i)] = 1;
            }
            return table;
        }();

        static constexpr auto SchemaSet = []() {
            std::array<uint8_t, 256> table{};
            table[static_cast<uint8_t>('\t')] = 2;
            table[static_cast<uint8_t>('\n')] = 2;
            table[static_cast<uint8_t>('\r')] = 2;
            for (char i = 'a'; i <= 'z'; ++i) {
                table[static_cast<size_t>(i)] = 1;
            }
            for (char i = 'A'; i <= 'Z'; ++i) {
                table[static_cast<size_t>(i)] = 1;
            }
            for (char i = '0'; i <= '9'; ++i) {
                table[static_cast<size_t>(i)] = 1;
            }
            table[static_cast<uint8_t>('+')] = 1;
            table[static_cast<uint8_t>('-')] = 1;
            table[static_cast<uint8_t>('.')] = 1;
            return table;
        }();

        while (pos != end && Headerset[static_cast<uint8_t>(*pos)]) {
            ++pos;
        }

        while (end != pos && Headerset[static_cast<uint8_t>(*(end - 1))]) {
            --end;
        }
        {
            std::string_view rest(pos, static_cast<size_t>(end - pos));
            _pending_fragment = prune_hash(rest);
            end = rest.data() + rest.size();
        }

        _buffer.clear();
        _buffer.reserve(static_cast<size_t>(end - pos) + 64);
        const char *scheme_begin = pos;

        if (pos == end || !ascii_isalpha(*pos)) {
            if (_base != nullptr) {
                chain_parse_no_scheme(ctx, uri);
                return;
            }
            uri.uri_error({ UriErrorCode::kUriNotComplete,
                static_cast<uint32_t>(pos - ctx.start), "no schema" });
            return;
        }
        _buffer.push_back(static_cast<char>(ascii_tolower(*pos)));
        ++pos;

        while (pos != end) {
            const unsigned char c = static_cast<unsigned char>(*pos);
            if (c == ':') {
                break;
            }
            switch (SchemaSet[c]) {
            case 1:
                _buffer.push_back(static_cast<char>(ascii_tolower(*pos)));
                ++pos;
                break;
            case 2:
                ++pos;
                break;
            default:
                if (_base != nullptr) {
                    _buffer.clear();
                    pos = scheme_begin;
                    chain_parse_no_scheme(ctx, uri);
                    return;
                }
                uri.uri_error({ UriErrorCode::kUriNotComplete,
                    static_cast<uint32_t>(pos - ctx.start), "no schema" });
                return;
            }
        }
        if (pos == end || _buffer.empty() || *pos != ':') {
            if (_base != nullptr) {
                _buffer.clear();
                pos = scheme_begin;
                chain_parse_no_scheme(ctx, uri);
                return;
            }
            uri.uri_error({ UriErrorCode::kUriNotComplete,
                static_cast<uint32_t>(pos - ctx.start), "no schema" });
            return;
        }
        ++pos;

        SchemaType type;
        parse_scheme(type);
        uri.schema_type(type);
        uri.shema({0u, static_cast<uint32_t>(_buffer.size())});
        _buffer.push_back(':');

        if (type == SchemaType::FILE) {
            chain_parse_file(ctx, uri);
            return;
        }
        if (uri.is_special()) {
            if (_base != nullptr && _base->schema_type() == type) {
                chain_parse_special_relative_or_authority(ctx, uri);
            } else {
                chain_parse_special_authority_slashes(ctx, uri);
            }
            return;
        }
        if (pos != end && *pos == '/') {
            ++pos;
            chain_parse_path_or_authority(ctx, uri);
            return;
        }
        chain_parse_opaque_path(ctx, uri);
    }

    void WptParser::chain_parse_no_scheme(ParserContext &ctx, UriView& uri) {
        auto &pos = ctx.pos;
        auto &end = ctx.end;

        if (base_has_opaque_path()) {
            // After prune_hash: only a pure "#fragment" leaves remaining empty.
            if (pos != end || !_pending_fragment.has_value()) {
                uri.uri_error({ UriErrorCode::kUriNotComplete,
                    static_cast<uint32_t>(pos - ctx.start), "no schema" });
                return;
            }
            set_scheme_from_base(uri);
            append_path_from_base(uri);
            append_query_from_base(uri);
            finish(uri);
            return;
        }
        if (_base->schema_type() == SchemaType::FILE) {
            chain_parse_file(ctx, uri);
            return;
        }
        chain_parse_relative(ctx, uri);
    }

    void WptParser::chain_parse_special_relative_or_authority(ParserContext &ctx,
        UriView& uri) {
        auto &pos = ctx.pos;
        auto &end = ctx.end;
        std::string_view view(pos, static_cast<size_t>(end - pos));
        if (consume_two_slashes(pos, end)) {
            chain_parse_special_authority_ignore_slashes(ctx, uri);
            return;
        }
        chain_parse_relative(ctx, uri);
    }

    void WptParser::chain_parse_path_or_authority(ParserContext &ctx, UriView& uri) {
        auto &pos = ctx.pos;
        auto &end = ctx.end;
        if (pos != end && *pos == '/') {
            ++pos;
            chain_parse_authority(ctx, uri);
            return;
        }
        chain_parse_path(ctx, uri);
    }

    void WptParser::chain_parse_opaque_path(ParserContext &ctx, UriView& uri) {
        auto &pos = ctx.pos;
        auto &end = ctx.end;

        std::string_view view(pos, static_cast<size_t>(end - pos));
        bool go_query = false;
        size_t location = view.find('?');
        if (location != std::string_view::npos) {
            go_query = true;
            view.remove_suffix(view.size() - location);
            pos += location + 1;
        } else {
            pos = end;
        }

        const uint32_t start = static_cast<uint32_t>(_buffer.size());
        _buffer.append(percent_encode_wpt(view, uri_charsets::C0_CONTROL_PERCENT_ENCODE));
        uri.path({start, static_cast<uint32_t>(_buffer.size())});

        if (go_query) {
            chain_parse_query(ctx, uri);
            return;
        }
        finish(uri);
    }

    void WptParser::chain_parse_file(ParserContext &ctx, UriView& uri) {
        auto &pos = ctx.pos;
        auto &end = ctx.end;

        // no-scheme + file base never wrote scheme yet.
        if (!uri.has_shema()) {
            set_scheme_from_base(uri);
        }
        set_empty_host(uri);

        if (pos != end && (*pos == '/' || *pos == '\\')) {
            ++pos;
            chain_parse_file_slash(ctx, uri);
            return;
        }
        if (_base != nullptr && _base->schema_type() == SchemaType::FILE) {
            {
                const size_t scheme_end = uri._schema->end + 1;
                _buffer.resize(scheme_end);
                uri.reset_host();
                uri.reset_username();
                uri.reset_password();
                uri.reset_port();
                uri.reset_path();
            }
            if (_base->has_host()) {
                append_authority_from_base(uri);
            } else {
                set_empty_host(uri);
            }
            append_path_from_base(uri);

            if (pos != end && *pos == '?') {
                ++pos;
                chain_parse_query(ctx, uri);
                return;
            }
            if (pos != end) {
                std::string_view file_view(pos, static_cast<size_t>(end - pos));
                if (!is_windows_drive_letter(file_view)) {
                    if (uri._path.has_value()) {
                        std::string path(buffer_slice(uri._path));
                        const uint32_t path_start = uri._path->start;
                        shorten_path(path, uri.schema_type());
                        _buffer.resize(path_start);
                        _buffer.append(path);
                        uri.path({path_start, static_cast<uint32_t>(_buffer.size())});
                    }
                } else {
                    clear_path(uri);
                }
                chain_parse_path(ctx, uri);
                return;
            }
            append_query_from_base(uri);
            finish(uri);
            return;
        }
        chain_parse_path(ctx, uri);
    }

    void WptParser::chain_parse_file_slash(ParserContext &ctx, UriView& uri) {
        auto &pos = ctx.pos;
        auto &end = ctx.end;

        if (pos != end && (*pos == '/' || *pos == '\\')) {
            ++pos;
            chain_parse_file_host(ctx, uri);
            return;
        }

        if (_base != nullptr && _base->schema_type() == SchemaType::FILE) {
            // Set url's host to base's host.
            {
                const size_t scheme_end = uri._schema->end + 1;
                _buffer.resize(scheme_end);
                uri.reset_host();
                uri.reset_username();
                uri.reset_password();
                uri.reset_port();
                uri.reset_path();
            }
            if (_base->has_host()) {
                append_authority_from_base(uri);
            } else {
                set_empty_host(uri);
            }

            if (_base->has_path() && !_base->path().empty()) {
                std::string_view rest(pos, static_cast<size_t>(end - pos));
                if (!is_windows_drive_letter(rest)) {
                    std::string_view first_base = _base->path().substr(1);
                    size_t loc = first_base.find('/');
                    if (loc != std::string_view::npos) {
                        resize(first_base, loc);
                    }
                    if (is_normalized_windows_drive_letter(first_base)) {
                        const uint32_t start = static_cast<uint32_t>(_buffer.size());
                        _buffer.push_back('/');
                        _buffer.append(first_base);
                        uri.path({start, static_cast<uint32_t>(_buffer.size())});
                    }
                }
            }
        }
        chain_parse_path(ctx, uri);
    }

    void WptParser::chain_parse_file_host(ParserContext &ctx, UriView& uri) {
        auto &pos = ctx.pos;
        auto &end = ctx.end;

        std::string_view view(pos, static_cast<size_t>(end - pos));
        size_t location = view.find_first_of("/\\?");
        std::string_view file_host_buffer(
            view.data(),
            (location != std::string_view::npos) ? location : view.size());

        if (is_windows_drive_letter(file_host_buffer)) {
            chain_parse_path(ctx, uri);
            return;
        }
        if (file_host_buffer.empty()) {
            // host already empty from set_empty_host in file state;
            // ensure // present (already from file)
            chain_parse_path_start(ctx, uri);
            return;
        }

        pos += file_host_buffer.size();
        std::string tmp_host;
        UriHostType ht = UriHostType::DEFAULT;
        UriError err = uri_wpt::parse_host(file_host_buffer, true, ht, &tmp_host);
        if (!err.ok()) {
            uri.uri_error(err);
            return;
        }
        if (tmp_host == "localhost") {
            tmp_host.clear();
        }
        // replace empty host content after //
        {
            const size_t scheme_end = uri._schema->end + 1;
            _buffer.resize(scheme_end);
            _buffer.append("//");
            const uint32_t start = static_cast<uint32_t>(_buffer.size());
            _buffer.append(tmp_host);
            uri.host({start, static_cast<uint32_t>(_buffer.size())});
            uri.host_type(ht);
        }
        chain_parse_path_start(ctx, uri);
    }

    void WptParser::chain_parse_relative(ParserContext &ctx, UriView& uri) {
        auto &pos = ctx.pos;
        auto &end = ctx.end;

        set_scheme_from_base(uri);
        skip_ascii_tab_or_newline(pos, end);

        if (pos != end && *pos == '/') {
            ++pos;
            chain_parse_relative_slash(ctx, uri);
            return;
        }
        if (uri.is_special() && pos != end && *pos == '\\') {
            ++pos;
            chain_parse_relative_slash(ctx, uri);
            return;
        }

        append_authority_from_base(uri);
        append_path_from_base(uri);

        if (pos != end && *pos == '?') {
            ++pos;
            chain_parse_query(ctx, uri);
            return;
        }
        if (pos != end) {
            if (uri._path.has_value()) {
                std::string path(buffer_slice(uri._path));
                const uint32_t path_start = uri._path->start;
                shorten_path(path, uri.schema_type());
                _buffer.resize(path_start);
                _buffer.append(path);
                uri.path({path_start, static_cast<uint32_t>(_buffer.size())});
            }
            chain_parse_path(ctx, uri);
            return;
        }
        append_query_from_base(uri);
        finish(uri);
    }

    void WptParser::chain_parse_relative_slash(ParserContext &ctx, UriView& uri) {
        auto &pos = ctx.pos;
        auto &end = ctx.end;
        skip_ascii_tab_or_newline(pos, end);

        if (uri.is_special() && pos != end && (*pos == '/' || *pos == '\\')) {
            ++pos;
            chain_parse_special_authority_ignore_slashes(ctx, uri);
            return;
        }
        if (pos != end && *pos == '/') {
            ++pos;
            chain_parse_authority(ctx, uri);
            return;
        }
        append_authority_from_base(uri);
        chain_parse_path(ctx, uri);
    }

    void WptParser::chain_parse_special_authority_slashes(ParserContext &ctx, UriView& uri) {
        auto &pos = ctx.pos;
        auto &end = ctx.end;
        if (consume_two_slashes(pos, end)) {
            // consumed
        }
        chain_parse_special_authority_ignore_slashes(ctx, uri);
    }

    void WptParser::chain_parse_special_authority_ignore_slashes(ParserContext &ctx, UriView& uri) {
        auto &pos = ctx.pos;
        auto &end = ctx.end;
        while (pos != end &&
            (*pos == '/' || *pos == '\\' || is_ascii_tab_or_newline(*pos))) {
            ++pos;
        }
        chain_parse_authority(ctx, uri);
    }

    void WptParser::chain_parse_authority(ParserContext &ctx, UriView& uri) {
        auto &pos = ctx.pos;
        auto &end = ctx.end;
        const bool special = uri.is_special();

        _buffer.append("//");

        const bool contains_ampersand =
            (std::string_view(pos, static_cast<size_t>(end - pos)).find('@') != std::string_view::npos);

        if (!contains_ampersand) {
            chain_parse_host(ctx, uri);
            return;
        }

        bool at_sign_seen = false;
        bool password_token_seen = false;
        std::string username;
        std::string password;

        do {
            std::string_view view(pos, static_cast<size_t>(end - pos));
            size_t location = special ? find_authority_delimiter_special(view)
                                      : find_authority_delimiter(view);
            std::string_view authority_view(view.data(), location);
            const char *end_of_authority = pos + authority_view.size();

            if (end_of_authority != end && *end_of_authority == '@') {
                if (at_sign_seen) {
                    if (password_token_seen) {
                        password += "%40";
                    } else {
                        username += "%40";
                    }
                }
                at_sign_seen = true;

                    if (!password_token_seen) {
                        size_t password_token_location = authority_view.find(':');
                        password_token_seen = password_token_location != std::string_view::npos;
                        if (!password_token_seen) {
                            username += percent_encode_wpt(authority_view,
                                uri_charsets::USERINFO_PERCENT_ENCODE);
                        } else {
                            username += percent_encode_wpt(
                                authority_view.substr(0, password_token_location),
                                uri_charsets::USERINFO_PERCENT_ENCODE);
                            password += percent_encode_wpt(
                                authority_view.substr(password_token_location + 1),
                                uri_charsets::USERINFO_PERCENT_ENCODE);
                        }
                    } else {
                        password += percent_encode_wpt(authority_view,
                            uri_charsets::USERINFO_PERCENT_ENCODE);
                    }
            } else if (end_of_authority == end || *end_of_authority == '/' ||
                *end_of_authority == '?' ||
                (special && *end_of_authority == '\\')) {
                if (at_sign_seen && authority_view.empty()) {
                    uri.uri_error({ UriErrorCode::kUriNotComplete,
                        static_cast<uint32_t>(pos - ctx.start), "no authority" });
                    return;
                }
                if (at_sign_seen) {
                    // Omit credentials entirely when both user and password empty
                    // (e.g. "https://:@host/" → "https://host/").
                    if (!username.empty() || !password.empty()) {
                        const uint32_t u_start = static_cast<uint32_t>(_buffer.size());
                        _buffer.append(username);
                        uri.username({u_start, static_cast<uint32_t>(_buffer.size())});
                        if (!password.empty()) {
                            _buffer.push_back(':');
                            const uint32_t p_start = static_cast<uint32_t>(_buffer.size());
                            _buffer.append(password);
                            uri.password({p_start, static_cast<uint32_t>(_buffer.size())});
                        }
                        _buffer.push_back('@');
                    }
                }
                chain_parse_host(ctx, uri);
                return;
            }
            if (end_of_authority == end) {
                finish(uri);
                return;
            }
            pos = end_of_authority + 1;
        } while (true);
    }

    void WptParser::chain_parse_host(ParserContext &ctx, UriView& uri) {
        auto &pos = ctx.pos;
        auto &end = ctx.end;
        const bool special = uri.is_special();

        std::string_view host_view(pos, static_cast<size_t>(end - pos));
        auto [location, found_colon] =
            uri_wpt::get_host_delimiter_location(special, host_view);
        pos = (location != std::string_view::npos) ? pos + location : end;

        UriHostType ht = UriHostType::DEFAULT;
        if (found_colon) {
            // Empty host before ":" is failure for both special and non-special
            // (e.g. "sc://:/" / "http://:/").
            if (host_view.empty()) {
                uri.uri_error({ UriErrorCode::kUriNotComplete,
                    static_cast<uint32_t>(pos - ctx.start), "no host" });
                return;
            }
            std::string tmp_host;
            if (!host_view.empty()) {
                UriError err = uri_wpt::parse_host(host_view, special, ht, &tmp_host);
                if (!err.ok()) {
                    uri.uri_error(err);
                    return;
                }
            }
            uri.host_type(ht);
            const uint32_t start = static_cast<uint32_t>(_buffer.size());
            _buffer.append(tmp_host);
            uri.host({start, static_cast<uint32_t>(_buffer.size())});
            ++pos;
            chain_parse_port(ctx, uri);
            return;
        }

        if (special && host_view.empty()) {
            uri.uri_error({ UriErrorCode::kUriNotComplete,
                static_cast<uint32_t>(pos - ctx.start), "no host" });
            return;
        }
        std::string tmp_host;
        if (!host_view.empty()) {
            UriError err = uri_wpt::parse_host(host_view, special, ht, &tmp_host);
            if (!err.ok()) {
                uri.uri_error(err);
                return;
            }
        }
        uri.host_type(ht);
        const uint32_t start = static_cast<uint32_t>(_buffer.size());
        _buffer.append(tmp_host);
        uri.host({start, static_cast<uint32_t>(_buffer.size())});
        chain_parse_path_start(ctx, uri);
    }

    void WptParser::chain_parse_port(ParserContext &ctx, UriView& uri) {
        auto &pos = ctx.pos;
        auto &end = ctx.end;
        const bool special = uri.is_special();

        std::string_view port_view(pos, static_cast<size_t>(end - pos));
        std::optional<uint16_t> tmp_port;
        UriError err = uri_wpt::parse_port(port_view, special, uri.schema_type(), true, tmp_port);
        pos += err.error_pos;
        if (!err.ok()) {
            uri.uri_error(err);
            return;
        }
        if (tmp_port.has_value()) {
            _buffer.push_back(':');
            const uint32_t start = static_cast<uint32_t>(_buffer.size());
            _buffer.append(std::to_string(*tmp_port));
            uri.port({start, static_cast<uint32_t>(_buffer.size())});
        }
        chain_parse_path_start(ctx, uri);
    }

    void WptParser::chain_parse_path_start(ParserContext &ctx, UriView& uri) {
        auto &pos = ctx.pos;
        auto &end = ctx.end;
        skip_ascii_tab_or_newline(pos, end);

        if (uri.is_special()) {
            if (pos == end) {
                const uint32_t start = static_cast<uint32_t>(_buffer.size());
                _buffer.push_back('/');
                uri.path({start, static_cast<uint32_t>(_buffer.size())});
                finish(uri);
                return;
            }
            if (*pos != '/' && *pos != '\\') {
                chain_parse_path(ctx, uri);
                return;
            }
            ++pos;
            chain_parse_path(ctx, uri);
            return;
        }

        if (pos != end && *pos == '?') {
            ++pos;
            chain_parse_query(ctx, uri);
            return;
        }
        if (pos != end) {
            if (*pos == '/') {
                ++pos;
            }
            chain_parse_path(ctx, uri);
            return;
        }
        finish(uri);
    }

    void WptParser::chain_parse_path(ParserContext &ctx, UriView& uri) {
        auto &pos = ctx.pos;
        auto &end = ctx.end;

        std::string_view view(pos, static_cast<size_t>(end - pos));
        size_t locofquestionmark = view.find('?');
        bool go_query = false;
        if (locofquestionmark != std::string_view::npos) {
            go_query = true;
            view.remove_suffix(view.size() - locofquestionmark);
            pos += locofquestionmark + 1;
        } else {
            pos = end;
        }

        std::string path;
        uint32_t path_start = static_cast<uint32_t>(_buffer.size());
        if (uri._path.has_value()) {
            path.assign(buffer_slice(uri._path));
            path_start = uri._path->start;
            _buffer.resize(path_start);
        }
        parse_prepared_path(view, uri.schema_type(), path);
        _buffer.append(path);
        uri.path({path_start, static_cast<uint32_t>(_buffer.size())});

        if (go_query) {
            chain_parse_query(ctx, uri);
            return;
        }
        finish(uri);
    }

    void WptParser::chain_parse_query(ParserContext &ctx, UriView& uri) {
        auto &pos = ctx.pos;
        auto &end = ctx.end;

        std::string_view view(pos, static_cast<size_t>(end - pos));
        _buffer.push_back('?');
        const uint32_t start = static_cast<uint32_t>(_buffer.size());
        _buffer.append(percent_encode_wpt(view, query_encode_set(uri)));
        uri.query({start, static_cast<uint32_t>(_buffer.size())});
        pos = end;
        finish(uri);
    }

    void WptParser::chain_parse_fragment(ParserContext &ctx, UriView& uri) {
        auto &pos = ctx.pos;
        auto &end = ctx.end;
        std::string_view view(pos, static_cast<size_t>(end - pos));
        _buffer.push_back('#');
        const uint32_t start = static_cast<uint32_t>(_buffer.size());
        _buffer.append(percent_encode_wpt(view, uri_charsets::FRAGMENT_PERCENT_ENCODE));
        uri.fragment({start, static_cast<uint32_t>(_buffer.size())});
        pos = end;
        uri.set_ownd(std::move(_buffer));
    }

    void WptParser::parse_scheme(SchemaType &type) {
        auto parsed_type = get_scheme_type(_buffer);
        if (parsed_type != SchemaType::NOT_SPECIAL) {
            type = parsed_type;
        } else {
            to_lower_ascii(_buffer.data(), _buffer.size());
            type = SchemaType::NOT_SPECIAL;
        }
    }

    std::string href_from_file(std::string_view input) {
        // parse_prepared_path skips tab/LF/CR while scanning — no pre-copy.
        std::string path;
        if (input.empty()) {
            path = "/";
        } else if ((input[0] == '/') || (input[0] == '\\')) {
            parse_prepared_path(input.substr(1), SchemaType::FILE, path);
        } else {
            parse_prepared_path(input, SchemaType::FILE, path);
        }
        return "file://" + path;
    }

} // namespace turbo
