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


#include <turbo/strings/substring.h>
#include <turbo/uri/uri_view.h>
namespace turbo {

    namespace {
        std::string_view component_slice(std::string_view data,
            const std::optional<ComponentView>& comp) {
            if (!comp.has_value()) {
                return "";
            }
            return subview(data, comp.value().start, comp.value().end);
        }
    } // namespace

    UriView::UriView(std::string_view s)
        : _uri_data(s) {
    }

    UriView::UriView(const UriView &u)
        : _type(u._type)
        , _encode(u._encode)
        , _host_type(u._host_type)
        , _error(u._error)
        , _schema_type(u._schema_type)
        , _schema(u._schema)
        , _username(u._username)
        , _password(u._password)
        , _host(u._host)
        , _port(u._port)
        , _path(u._path)
        , _query(u._query)
        , _fragment(u._fragment)
        , _query_params(u._query_params) {
        if (u._store.has_value()) {
            _store = u._store;
            _uri_data = *_store;
        } else {
            _uri_data = u._uri_data;
        }
    }

    UriView::UriView(UriView &&u) noexcept
        : _store(std::move(u._store))
        , _type(u._type)
        , _encode(u._encode)
        , _host_type(u._host_type)
        , _error(std::move(u._error))
        , _schema_type(u._schema_type)
        , _schema(u._schema)
        , _username(u._username)
        , _password(u._password)
        , _host(u._host)
        , _port(u._port)
        , _path(u._path)
        , _query(u._query)
        , _fragment(u._fragment)
        , _query_params(std::move(u._query_params)) {
        if (_store.has_value()) {
            _uri_data = *_store;
        } else {
            _uri_data = u._uri_data;
        }
        u._uri_data = {};
        u._store.reset();
    }

    UriView &UriView::operator=(const UriView &u) {
        if (this == &u) {
            return *this;
        }
        _type = u._type;
        _encode = u._encode;
        _host_type = u._host_type;
        _error = u._error;
        _schema_type = u._schema_type;
        _schema = u._schema;
        _username = u._username;
        _password = u._password;
        _host = u._host;
        _port = u._port;
        _path = u._path;
        _query = u._query;
        _fragment = u._fragment;
        _query_params = u._query_params;
        if (u._store.has_value()) {
            _store = u._store;
            _uri_data = *_store;
        } else {
            _store.reset();
            _uri_data = u._uri_data;
        }
        return *this;
    }

    UriView &UriView::operator=(UriView &&u) noexcept {
        if (this == &u) {
            return *this;
        }
        _store = std::move(u._store);
        _type = u._type;
        _encode = u._encode;
        _host_type = u._host_type;
        _error = std::move(u._error);
        _schema_type = u._schema_type;
        _schema = u._schema;
        _username = u._username;
        _password = u._password;
        _host = u._host;
        _port = u._port;
        _path = u._path;
        _query = u._query;
        _fragment = u._fragment;
        _query_params = std::move(u._query_params);
        if (_store.has_value()) {
            _uri_data = *_store;
        } else {
            _uri_data = u._uri_data;
        }
        u._uri_data = {};
        u._store.reset();
        return *this;
    }

    void UriView::make_ownd() {
        _store = std::string(_uri_data);
        _uri_data = *_store;
    }

    void UriView::set_ownd(std::string&& str) {
        _store = std::move(str);
        _uri_data = *_store;
    }

    void UriView::release(std::string &out) {
        if (_store.has_value()) {
            out = std::move(*_store);
            _store.reset();
            _uri_data = {};
        } else {
            out.assign(_uri_data);
        }
    }

    std::string UriView::release() {
        std::string out;
        release(out);
        return out;
    }

    void UriView::release_append(std::string &out) const {
        out.append(_uri_data);
    }

    std::string_view UriView::shema() const {
        return component_slice(_uri_data, _schema);
    }

    std::string_view UriView::username() const {
        return component_slice(_uri_data, _username);
    }

    std::string_view UriView::password() const {
        return component_slice(_uri_data, _password);
    }

    bool UriView::ok() const noexcept {
        return _error.ok();
    }

    StandType UriView::standard() const noexcept {
        return _type;
    }

    EnodeType UriView::encode_type() const noexcept {
        return _encode;
    }

    UriHostType UriView::host_type() const noexcept {
        return _host_type;
    }

    const UriError& UriView::uri_error() const noexcept {
        return _error;
    }

    std::string_view UriView::host() const {
        return component_slice(_uri_data, _host);
    }

    std::string_view UriView::port() const {
        return component_slice(_uri_data, _port);
    }

    std::string_view UriView::path() const {
        return component_slice(_uri_data, _path);
    }

    std::string_view UriView::query() const {
        return component_slice(_uri_data, _query);
    }

    std::string_view UriView::fragment() const {
        return component_slice(_uri_data, _fragment);
    }

    const QueryParams& UriView::query_params() const noexcept {
        static const QueryParams kEmpty;
        if (!_query_params.has_value()) {
            return kEmpty;
        }
        return *_query_params;
    }

    std::string_view UriView::origin() const {
        return _uri_data;
    }
    bool UriView::has_shema() const noexcept {
        return _schema.has_value();
    }

    bool UriView::has_username() const noexcept {
        return _username.has_value();
    }

    bool UriView::has_password() const noexcept {
        return _password.has_value();
    }

    bool UriView::has_host() const noexcept {
        return _host.has_value();
    }

    bool UriView::has_port() const noexcept {
        return _port.has_value();
    }

    bool UriView::has_path() const noexcept {
        return _path.has_value();
    }

    bool UriView::has_query() const noexcept {
        return _query.has_value();
    }

    bool UriView::has_fragment() const noexcept {
        return _fragment.has_value();
    }

    bool UriView::has_query_params() const noexcept {
        return _query_params.has_value();
    }

    void UriView::encode_type(EnodeType et) {
        _encode = et;
    }

    void UriView::standard(StandType st) {
        _type = st;
    }

    void UriView::host_type(UriHostType ht) {
        _host_type = ht;
    }

    void UriView::uri_error(UriError err) {
        _error = std::move(err);
    }

    void UriView::shema(ComponentView view) {
        _schema = view;
    }

    void UriView::username(ComponentView view) {
        _username = view;
    }

    void UriView::password(ComponentView view) {
        _password = view;
    }

    void UriView::host(ComponentView view) {
        _host = view;
    }

    void UriView::port(ComponentView view) {
        _port = view;
    }

    void UriView::path(ComponentView view) {
        _path = view;
    }

    void UriView::query(ComponentView view) {
        _query = view;
    }

    void UriView::fragment(ComponentView view) {
        _fragment = view;
    }

    void UriView::query_params(QueryParams params) {
        _query_params = std::move(params);
    }

    void UriView::reset_shema() {
        _schema = std::nullopt;
    }

    void UriView::reset_username() {
        _username = std::nullopt;
    }

    void UriView::reset_password() {
        _password = std::nullopt;
    }

    void UriView::reset_host() {
        _host = std::nullopt;
    }

    void UriView::reset_port() {
        _port = std::nullopt;
    }

    void UriView::reset_path() {
        _path = std::nullopt;
    }

    void UriView::reset_query() {
        _query = std::nullopt;
    }

    void UriView::reset_fragment() {
        _fragment = std::nullopt;
    }

    void UriView::reset_query_params() {
        _query_params = std::nullopt;
    }

    bool UriView::is_special() const noexcept {
        return _schema_type != turbo::SchemaType::NOT_SPECIAL;
    }

    SchemaType UriView::schema_type() const {
        return _schema_type;
    }

    void UriView::schema_type(SchemaType type) {
        _schema_type = type;
    }
    void UriView::reset() {
        _uri_data = { };
        _store.reset();
        _error = { };
        _schema = std::nullopt;
        _username = std::nullopt;
        _password = std::nullopt;
        _host = std::nullopt;
        _port = std::nullopt;
        _path = std::nullopt;
        _query = std::nullopt;
        _fragment = std::nullopt;
        _query_params = std::nullopt;
    }
} // namespace turbo
