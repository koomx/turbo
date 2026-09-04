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

#include <string_view>
#include <turbo/macros/macros.h>
#include <turbo/uri/scheme.h>
#include <turbo/uri/uri_error.h>

namespace turbo {

    enum class UriHostType : uint8_t {
         /// Represents common URLs such as "https://www.google.com"
        DEFAULT = 0,
         /// Represents ipv4 addresses such as "http://127.0.0.1"
        IPV4 = 1,
         /// Represents ipv6 addresses such as
         /// "http://[2001:db8:3333:4444:5555:6666:7777:8888]"
        IPV6 = 2,
    };

    enum class StandType {
        STD_NONE,
        STD_RFC,
        STD_WPT,
    };

    class WptParser;
    class RfcParser;

    class UriCommonBase {
    public:
        virtual ~UriCommonBase() = default;
    private:
        friend class WptParser;
        friend class RfcParser;
    protected:
        explicit UriCommonBase(StandType t) :_standard(t){}
        UriError _uri_error { };

        bool _opaque_path { false };

        UriHostType _host_type = UriHostType::DEFAULT;

        SchemaType type { SchemaType::NOT_SPECIAL };

        StandType _standard {StandType::STD_NONE};

    public:

        StandType standard() const {
            return _standard;
        }
        [[nodiscard]] KUMO_FORCE_INLINE bool has_opaque_path() const {
            return _opaque_path;
        }

        KUMO_FORCE_INLINE bool& has_opaque_path() {
            return _opaque_path;
        }

        UriHostType host_type() const {
            return _host_type;
        }

        UriHostType &host_type() {
            return _host_type;
        }

        [[nodiscard]] KUMO_FORCE_INLINE bool is_special() const noexcept;

        [[nodiscard]] virtual std::string get_origin() const noexcept = 0;

        [[nodiscard]] virtual bool has_valid_domain() const noexcept = 0;

        [[nodiscard]] bool ok() const {
            return _uri_error.ok();
        }

        const UriError& uri_error() const {
            return _uri_error;
        }

        UriError& uri_error() {
            return _uri_error;
        }
        [[nodiscard]] inline uint16_t get_special_port() const noexcept;

        [[nodiscard]] KUMO_FORCE_INLINE uint16_t scheme_default_port() const noexcept;

        [[nodiscard]] virtual std::string to_string() const = 0;

        virtual inline void clear_pathname() = 0;

        virtual inline void clear_search() = 0;

        [[nodiscard]] virtual inline bool has_hash() const noexcept = 0;

        [[nodiscard]] virtual inline bool has_search() const noexcept = 0;

    }; // UriCommonBase

    [[nodiscard]] KUMO_FORCE_INLINE bool UriCommonBase::is_special() const noexcept {
        return type != turbo::SchemaType::NOT_SPECIAL;
    }

    [[nodiscard]] inline uint16_t UriCommonBase::get_special_port() const noexcept {
        return turbo::get_special_port(type);
    }

    [[nodiscard]] KUMO_FORCE_INLINE uint16_t UriCommonBase::scheme_default_port() const noexcept {
        return turbo::get_special_port(type);
    }

} // namespace turbo

