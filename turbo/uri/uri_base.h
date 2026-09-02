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

#include <algorithm>
#include <charconv>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

#include <turbo/uri/scheme.h>
#include <turbo/uri/uri_components.h>
#include <turbo/uri/uri_common_base.h>
#include <turbo/uri/utility.h>
#include <turbo/format/fast_to_buffer.h>

#if KUMO_COMPILER_MSVC_CLANG
#include <intrin.h>
#endif // KUMO_COMPILER_MSVC_CLANG


namespace turbo {

    class  UriBase : public turbo::UriCommonBase {
    protected:
        explicit UriBase(StandType t) : UriCommonBase(t){}
    public:
        UriBase(const UriBase& u) = default;
        UriBase(UriBase&& u) noexcept = default;
        UriBase& operator=(UriBase&& u) noexcept = default;
        UriBase& operator=(const UriBase& u) = default;
        ~UriBase() override = default;


        std::string username { };
        std::string password { };
        std::optional<std::string> host { };
        std::optional<uint16_t> port { };
        std::string path { };
        std::optional<std::string> query { };
        std::optional<std::string> hash { };

        [[nodiscard]] inline bool has_empty_hostname() const noexcept;
        [[nodiscard]] inline bool has_port() const noexcept;
        [[nodiscard]] inline bool has_hostname() const noexcept;

        [[nodiscard]] bool host_empty() const noexcept {
            return host.has_value() && host.value().empty();
        }

        [[nodiscard]] std::string to_string() const override;

        [[nodiscard]] std::string get_href() const noexcept;

        [[nodiscard]] std::string get_protocol() const noexcept;

        [[nodiscard]] std::string get_host() const noexcept;

        [[nodiscard]] std::string get_hostname() const noexcept;

        [[nodiscard]] std::string_view get_pathname() const noexcept;

        [[nodiscard]] KUMO_FORCE_INLINE size_t get_pathname_length() const noexcept;

        [[nodiscard]] std::string get_search() const noexcept;

        [[nodiscard]] const std::string& get_username() const noexcept;

        [[nodiscard]] const std::string& get_password() const noexcept;

        [[nodiscard]] std::string get_port() const noexcept;

        [[nodiscard]] std::string get_hash() const noexcept;

        [[nodiscard]] KUMO_FORCE_INLINE bool has_credentials() const noexcept;

        [[nodiscard]] KUMO_FORCE_INLINE turbo::UriComponents get_components() const noexcept;
        [[nodiscard]] inline bool has_hash() const noexcept override;
        [[nodiscard]] inline bool has_search() const noexcept override;


        inline void clear_pathname() override;
        inline void clear_search() override;

        //////////////////////////////////////////////////////////////////////////////

        inline void update_base_hostname(std::string_view input);
        inline void update_base_search(std::string_view input);
        inline void update_base_search(std::optional<std::string> input);
        inline void update_base_pathname(std::string_view input);
        inline void update_base_username(std::string_view input);
        inline void update_base_password(std::string_view input);
        inline void update_base_port(std::optional<uint16_t> input);
    private:
        /// for parsers
        virtual bool set_username(std::string_view input) = 0;

        virtual bool set_password(std::string_view input) = 0;

        virtual  bool set_port(std::string_view input) = 0;

        virtual void set_hash(std::string_view input) = 0;

        virtual void set_search(std::string_view input) = 0;

        virtual bool set_pathname(std::string_view input) = 0;

        virtual bool set_host(std::string_view input) = 0;

        virtual  bool set_hostname(std::string_view input) = 0;

        virtual bool set_protocol(std::string_view input) = 0;

        virtual bool set_href(std::string_view input) = 0;
    protected:
        std::string non_special_scheme { };

        friend class WptParser;

        [[nodiscard]] inline bool cannot_have_credentials_or_port() const;

        inline void copy_scheme(const UriBase& u);
        inline void set_protocol_as_file();

        inline void set_scheme(std::string&& new_scheme) noexcept;
        inline void copy_scheme(UriBase&& u) noexcept;

    };

    inline std::ostream& operator<<(std::ostream& out, const UriBase& u);

    [[nodiscard]] KUMO_FORCE_INLINE bool UriBase::has_credentials() const noexcept {
        return !username.empty() || !password.empty();
    }
    [[nodiscard]] KUMO_FORCE_INLINE bool UriBase::has_port() const noexcept {
        return port.has_value();
    }
    [[nodiscard]] inline bool UriBase::cannot_have_credentials_or_port() const {
        return !host.has_value() || host.value().empty()
            || (_standard == StandType::STD_WPT && type == turbo::SchemaType::FILE);
    }
    [[nodiscard]] inline bool UriBase::has_empty_hostname() const noexcept {
        if (!host.has_value()) {
            return false;
        }
        return host.value().empty();
    }
    [[nodiscard]] inline bool UriBase::has_hostname() const noexcept {
        return host.has_value();
    }
    inline std::ostream& operator<<(std::ostream& out, const UriBase& u) {
        return out << u.to_string();
    }

    [[nodiscard]] size_t UriBase::get_pathname_length() const noexcept {
        return path.size();
    }

    [[nodiscard]] KUMO_FORCE_INLINE turbo::UriComponents UriBase::get_components()
        const noexcept {
        turbo::UriComponents out { };

        // protocol ends with ':'. for example: "https:"
        out.protocol_end = uint32_t(get_protocol().size());

        // Trailing index is always the next character of the current one.
        size_t running_index = out.protocol_end;

        if (host.has_value()) {
            // 2 characters for "//" and 1 character for starting index
            out.host_start = out.protocol_end + 2;

            if (has_credentials()) {
                out.username_end = uint32_t(out.host_start + username.size());

                out.host_start += uint32_t(username.size());

                if (!password.empty()) {
                    out.host_start += uint32_t(password.size() + 1);
                }

                out.host_end = uint32_t(out.host_start + host.value().size());
            } else {
                out.username_end = out.host_start;

                // Host does not start with "@" if it does not include credentials.
                out.host_end = uint32_t(out.host_start + host.value().size()) - 1;
            }

            running_index = out.host_end + 1;
        } else {
            // Update host start and end date to the same index, since it does not
            // exist.
            out.host_start = out.protocol_end;
            out.host_end = out.host_start;

            if (_standard == StandType::STD_WPT && !has_opaque_path()
                && turbo::starts_with(path, "//")) {
                running_index = out.protocol_end + 2;
            } else {
                running_index = out.protocol_end;
            }
        }

        if (port.has_value()) {
            out.port = *port;
            running_index += turbo::format_internal::fast_digit_count(*port) + 1; // Port omits ':'
        }

        out.pathname_start = uint32_t(running_index);

        running_index += path.size();

        if (query.has_value()) {
            out.search_start = uint32_t(running_index);
            running_index += get_search().size();
            if (get_search().empty()) {
                running_index++;
            }
        }

        if (hash.has_value()) {
            out.hash_start = uint32_t(running_index);
        }

        return out;
    }

    inline void UriBase::update_base_hostname(std::string_view input) {
        host = input;
    }

    inline void UriBase::update_base_search(std::optional<std::string> input) {
        query = input;
    }

    inline void UriBase::update_base_search(std::string_view input) {
        std::string s(input);
        query = s;
    }

    inline void UriBase::update_base_pathname(const std::string_view input) {
        path = input;
    }

    inline void UriBase::update_base_username(const std::string_view input) {
        username = input;
    }

    inline void UriBase::update_base_password(const std::string_view input) {
        password = input;
    }

    inline void UriBase::update_base_port(std::optional<uint16_t> input) {
        port = input;
    }

    inline void UriBase::clear_pathname() {
        path.clear();
    }

    inline void UriBase::clear_search() {
        query = std::nullopt;
    }

    [[nodiscard]] inline bool UriBase::has_hash() const noexcept {
        return hash.has_value();
    }

    [[nodiscard]] inline bool UriBase::has_search() const noexcept {
        return query.has_value();
    }

    inline void UriBase::set_protocol_as_file() {
        type = turbo::SchemaType::FILE;
    }

    inline void UriBase::set_scheme(std::string&& new_scheme) noexcept {
        type = turbo::get_scheme_type(new_scheme);
        // We only move the 'scheme' if it is non-special.
        if (!is_special()) {
            non_special_scheme = std::move(new_scheme);
        }

    }

    inline void UriBase::copy_scheme(UriBase&& u) noexcept {
        non_special_scheme = u.non_special_scheme;
        type = u.type;
    }

    inline void UriBase::copy_scheme(const UriBase& u) {
        non_special_scheme = u.non_special_scheme;
        type = u.type;
    }

}  // namespace turbo
