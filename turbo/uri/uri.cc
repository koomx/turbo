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

#include <turbo/uri/uri.h>
#include <turbo/uri/utility.h>
#include <turbo/uri/character_sets.h>
#include <turbo/uri/scheme.h>
#include <turbo/uri/rfc/parser.h>

namespace turbo {

    ComponentView Uri::append_to_store(std::string_view s) {
        const uint32_t start = static_cast<uint32_t>(_store.size());
        _store.append(s);
        return {start, static_cast<uint32_t>(_store.size())};
    }

    const uint8_t *Uri::wpt_query_encode_set() const {
        if (_uri_view.has_shema()) {
            const auto &c = *_uri_view._schema;
            const std::string_view scheme(_store.data() + c.start, c.end - c.start);
            if (is_special(scheme)) {
                return uri_charsets::SPECIAL_QUERY_PERCENT_ENCODE;
            }
        }
        return uri_charsets::QUERY_PERCENT_ENCODE;
    }

    Uri& Uri::schema(std::string_view s) {
        _uri_view.shema(append_to_store(s));
        return *this;
    }

    Uri& Uri::username(std::string_view s) {
        _uri_view.username(append_to_store(s));
        return *this;
    }

    Uri& Uri::password(std::string_view s) {
        _uri_view.password(append_to_store(s));
        return *this;
    }

    Uri& Uri::host(std::string_view s) {
        _uri_view.host(append_to_store(s));
        return *this;
    }

    Uri& Uri::port(std::string_view s) {
        _uri_view.port(append_to_store(s));
        return *this;
    }

    Uri& Uri::path(std::string_view s) {
        _uri_view.path(append_to_store(s));
        return *this;
    }

    Uri& Uri::query(std::string_view s) {
        _uri_view.query(append_to_store(s));
        return *this;
    }

    Uri& Uri::fragment(std::string_view s) {
        _uri_view.fragment(append_to_store(s));
        return *this;
    }

    Uri& Uri::append_query(std::string_view key, std::string_view value) {
        const ComponentView k = append_to_store(key);
        const ComponentView v = append_to_store(value);
        if (!_uri_view._query_params.has_value()) {
            _uri_view._query_params = QueryParams{};
        }
        _uri_view._query_params->emplace_back(k, v);
        return *this;
    }

    Uri& Uri::rfc_plain_schema(std::string_view s) {
        return schema(s);
    }

    Uri& Uri::rfc_plain_username(std::string_view s) {
        return username(percent_encode(s, uri_charsets::USERINFO_PERCENT_ENCODE));
    }

    Uri& Uri::rfc_plain_password(std::string_view s) {
        return password(percent_encode(s, uri_charsets::USERINFO_PERCENT_ENCODE));
    }

    Uri& Uri::rfc_plain_host(std::string_view s) {
        return host(percent_encode(s, uri_charsets::C0_CONTROL_PERCENT_ENCODE));
    }

    Uri& Uri::rfc_plain_port(std::string_view s) {
        return port(s);
    }

    Uri& Uri::rfc_plain_path(std::string_view s) {
        return path(percent_encode(s, uri_charsets::PATH_PERCENT_ENCODE));
    }

    Uri& Uri::rfc_plain_query(std::string_view s) {
        return query(percent_encode(s, uri_charsets::QUERY_PERCENT_ENCODE));
    }

    Uri& Uri::rfc_plain_fragment(std::string_view s) {
        return fragment(percent_encode(s, uri_charsets::FRAGMENT_PERCENT_ENCODE));
    }

    Uri& Uri::rfc_plain_append_query(std::string_view key, std::string_view value) {
        return append_query(
            percent_encode(key, uri_charsets::QUERY_PERCENT_ENCODE),
            percent_encode(value, uri_charsets::QUERY_PERCENT_ENCODE));
    }

    Uri& Uri::wpt_plain_schema(std::string_view s) {
        return schema(s);
    }

    Uri& Uri::wpt_plain_username(std::string_view s) {
        return username(percent_encode(s, uri_charsets::USERINFO_PERCENT_ENCODE));
    }

    Uri& Uri::wpt_plain_password(std::string_view s) {
        return password(percent_encode(s, uri_charsets::USERINFO_PERCENT_ENCODE));
    }

    Uri& Uri::wpt_plain_host(std::string_view s) {
        return host(percent_encode(s, uri_charsets::C0_CONTROL_PERCENT_ENCODE));
    }

    Uri& Uri::wpt_plain_port(std::string_view s) {
        return port(s);
    }

    Uri& Uri::wpt_plain_path(std::string_view s) {
        return path(percent_encode(s, uri_charsets::PATH_PERCENT_ENCODE));
    }

    Uri& Uri::wpt_plain_query(std::string_view s) {
        return query(percent_encode(s, wpt_query_encode_set()));
    }

    Uri& Uri::wpt_plain_fragment(std::string_view s) {
        return fragment(percent_encode(s, uri_charsets::FRAGMENT_PERCENT_ENCODE));
    }

    Uri& Uri::wpt_plain_append_query(std::string_view key, std::string_view value) {
        const uint8_t *set = wpt_query_encode_set();
        return append_query(percent_encode(key, set), percent_encode(value, set));
    }

    UriView Uri::build_rfc() const {
        auto slice = [this](const ComponentView &c) {
            return std::string_view(_store.data() + c.start, c.end - c.start);
        };
        auto slice_opt = [&](const std::optional<ComponentView> &c) -> std::string_view {
            if (!c.has_value()) {
                return {};
            }
            return slice(*c);
        };

        std::string buf;
        buf.reserve(_store.size() + 16);
        UriView out;

        auto take = [&](uint32_t start) {
            return ComponentView{start, static_cast<uint32_t>(buf.size())};
        };
        auto append_part = [&](std::string_view s) {
            const uint32_t start = static_cast<uint32_t>(buf.size());
            buf.append(s);
            return take(start);
        };

        if (_uri_view.has_shema()) {
            out.shema(append_part(slice_opt(_uri_view._schema)));
        }
        buf.push_back(':');

        if (_uri_view.has_host()) {
            buf.append("//");
            if (_uri_view.has_username() || _uri_view.has_password()) {
                if (_uri_view.has_username()) {
                    out.username(append_part(slice_opt(_uri_view._username)));
                }
                if (_uri_view.has_password()) {
                    buf.push_back(':');
                    out.password(append_part(slice_opt(_uri_view._password)));
                }
                buf.push_back('@');
            }
            out.host(append_part(slice_opt(_uri_view._host)));
            if (_uri_view.has_port()) {
                buf.push_back(':');
                out.port(append_part(slice_opt(_uri_view._port)));
            }
        }

        if (_uri_view.has_path()) {
            out.path(append_part(slice_opt(_uri_view._path)));
        }

        if (_uri_view.has_query()) {
            buf.push_back('?');
            out.query(append_part(slice_opt(_uri_view._query)));
        } else if (_uri_view.has_query_params()) {
            buf.push_back('?');
            const uint32_t qstart = static_cast<uint32_t>(buf.size());
            QueryParams params;
            params.reserve(_uri_view._query_params->size());
            bool first = true;
            for (const auto &kv : *_uri_view._query_params) {
                if (!first) {
                    buf.push_back('&');
                }
                first = false;
                const uint32_t ks = static_cast<uint32_t>(buf.size());
                buf.append(slice(kv.first));
                const ComponentView key{ks, static_cast<uint32_t>(buf.size())};
                buf.push_back('=');
                const uint32_t vs = static_cast<uint32_t>(buf.size());
                buf.append(slice(kv.second));
                const ComponentView val{vs, static_cast<uint32_t>(buf.size())};
                params.emplace_back(key, val);
            }
            out.query(take(qstart));
            out.query_params(std::move(params));
        }

        if (_uri_view.has_fragment()) {
            buf.push_back('#');
            out.fragment(append_part(slice_opt(_uri_view._fragment)));
        }

        out.set_ownd(std::move(buf));
        out.standard(StandType::STD_RFC);
        out.encode_type(EnodeType::PRECENT);
        out.host_type(_uri_view.host_type());
        return out;
    }

    UriView Uri::build_wpt() const {
        auto slice = [this](const ComponentView &c) {
            return std::string_view(_store.data() + c.start, c.end - c.start);
        };
        auto slice_opt = [&](const std::optional<ComponentView> &c) -> std::string_view {
            if (!c.has_value()) {
                return {};
            }
            return slice(*c);
        };

        std::string buf;
        buf.reserve(_store.size() + 16);
        UriView out;

        auto take = [&](uint32_t start) {
            return ComponentView{start, static_cast<uint32_t>(buf.size())};
        };
        auto append_part = [&](std::string_view s) {
            const uint32_t start = static_cast<uint32_t>(buf.size());
            buf.append(s);
            return take(start);
        };

        if (_uri_view.has_shema()) {
            out.shema(append_part(slice_opt(_uri_view._schema)));
        }
        buf.push_back(':');

        if (_uri_view.has_host()) {
            buf.append("//");
            if (_uri_view.has_username() || _uri_view.has_password()) {
                if (_uri_view.has_username()) {
                    out.username(append_part(slice_opt(_uri_view._username)));
                }
                if (_uri_view.has_password()) {
                    buf.push_back(':');
                    out.password(append_part(slice_opt(_uri_view._password)));
                }
                buf.push_back('@');
            }
            out.host(append_part(slice_opt(_uri_view._host)));
            if (_uri_view.has_port()) {
                buf.push_back(':');
                out.port(append_part(slice_opt(_uri_view._port)));
            }
        } else {
            const std::string_view path = slice_opt(_uri_view._path);
            if (path.size() >= 2 && path[0] == '/' && path[1] == '/') {
                buf.append("/.");
            }
        }

        if (_uri_view.has_path()) {
            out.path(append_part(slice_opt(_uri_view._path)));
        }

        if (_uri_view.has_query()) {
            buf.push_back('?');
            out.query(append_part(slice_opt(_uri_view._query)));
        } else if (_uri_view.has_query_params()) {
            buf.push_back('?');
            const uint32_t qstart = static_cast<uint32_t>(buf.size());
            QueryParams params;
            params.reserve(_uri_view._query_params->size());
            bool first = true;
            for (const auto &kv : *_uri_view._query_params) {
                if (!first) {
                    buf.push_back('&');
                }
                first = false;
                const uint32_t ks = static_cast<uint32_t>(buf.size());
                buf.append(slice(kv.first));
                const ComponentView key{ks, static_cast<uint32_t>(buf.size())};
                buf.push_back('=');
                const uint32_t vs = static_cast<uint32_t>(buf.size());
                buf.append(slice(kv.second));
                const ComponentView val{vs, static_cast<uint32_t>(buf.size())};
                params.emplace_back(key, val);
            }
            out.query(take(qstart));
            out.query_params(std::move(params));
        }

        if (_uri_view.has_fragment()) {
            buf.push_back('#');
            out.fragment(append_part(slice_opt(_uri_view._fragment)));
        }

        out.set_ownd(std::move(buf));
        out.standard(StandType::STD_WPT);
        out.encode_type(EnodeType::PRECENT);
        out.host_type(_uri_view.host_type());
        return out;
    }

    UriView Uri::build_plain_rfc() const {
        return decode_rfc_uri(build_rfc());
    }

    UriView Uri::build_plain_wpt() const {
        UriView out = RfcParser::convert_encode_type(build_wpt(), EnodeType::PLAIN);
        out.standard(StandType::STD_WPT);
        return out;
    }

}  // namespace turbo
