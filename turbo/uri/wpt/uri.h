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

    /**
     * @brief Generic URL struct reliant on std::string instantiation.
     *
     * @details To disambiguate from a valid URL string it can also be referred to
     * as a URL record. A URL is a struct that represents a universal identifier.
     * Unlike the url_aggregator, the WptUri represents the different components
     * of a parsed URL as independent std::string instances. This makes the
     * structure heavier and more reliant on memory allocations. When getting
     * components from the parsed URL, a new std::string is typically constructed.
     *
     * @see https://url.spec.whatwg.org/#url-representation
     */
    struct WptUri : turbo::UriCommonBase {
        WptUri() = default;
        WptUri(const WptUri& u) = default;
        WptUri(WptUri&& u) noexcept = default;
        WptUri& operator=(WptUri&& u) noexcept = default;
        WptUri& operator=(const WptUri& u) = default;
        ~WptUri() override = default;

        /**
         * @private
         * A URL's username is an ASCII string identifying a username. It is initially
         * the empty string.
         */
        std::string username { };

        /**
         * @private
         * A URL's password is an ASCII string identifying a password. It is initially
         * the empty string.
         */
        std::string password { };

        /**
         * @private
         * A URL's host is null or a host. It is initially null.
         */
        std::optional<std::string> host { };

        /**
         * @private
         * A URL's port is either null or a 16-bit unsigned integer that identifies a
         * networking port. It is initially null.
         */
        std::optional<uint16_t> port { };

        /**
         * @private
         * A URL's path is either an ASCII string or a list of zero or more ASCII
         * strings, usually identifying a location.
         */
        std::string path { };

        /**
         * @private
         * A URL's query is either null or an ASCII string. It is initially null.
         */
        std::optional<std::string> query { };

        /**
         * @private
         * A URL's fragment is either null or an ASCII string that can be used for
         * further processing on the resource the URL's other components identify. It
         * is initially null.
         */
        std::optional<std::string> hash { };

        /** @return true if it has an host but it is the empty string */
        [[nodiscard]] inline bool has_empty_hostname() const noexcept;
        /** @return true if the URL has a (non default) port */
        [[nodiscard]] inline bool has_port() const noexcept;
        /** @return true if it has a host (included an empty host) */
        [[nodiscard]] inline bool has_hostname() const noexcept;
        [[nodiscard]] bool has_valid_domain() const noexcept override;

        [[nodiscard]] bool host_empty() const noexcept {
            return host.has_value() && host.value().empty();
        }

        /**
         * Returns a JSON string representation of this URL.
         */
        [[nodiscard]] std::string to_string() const override;

        /**
         * @see https://url.spec.whatwg.org/#dom-url-href
         * @see https://url.spec.whatwg.org/#concept-url-serializer
         */
        [[nodiscard]] KUMO_FORCE_INLINE std::string get_href() const noexcept;

        /**
         * The origin getter steps are to return the serialization of this's URL's
         * origin. [HTML]
         * @return a newly allocated string.
         * @see https://url.spec.whatwg.org/#concept-url-origin
         */
        [[nodiscard]] std::string get_origin() const noexcept override;

        /**
         * The protocol getter steps are to return this's URL's scheme, followed by
         * U+003A (:).
         * @return a newly allocated string.
         * @see https://url.spec.whatwg.org/#dom-url-protocol
         */
        [[nodiscard]] std::string get_protocol() const noexcept;

        /**
         * Return url's host, serialized, followed by U+003A (:) and url's port,
         * serialized.
         * When there is no host, this function returns the empty string.
         * @return a newly allocated string.
         * @see https://url.spec.whatwg.org/#dom-url-host
         */
        [[nodiscard]] std::string get_host() const noexcept;

        /**
         * Return this's URL's host, serialized.
         * When there is no host, this function returns the empty string.
         * @return a newly allocated string.
         * @see https://url.spec.whatwg.org/#dom-url-hostname
         */
        [[nodiscard]] std::string get_hostname() const noexcept;

        /**
         * The pathname getter steps are to return the result of URL path serializing
         * this's URL.
         * @return a newly allocated string.
         * @see https://url.spec.whatwg.org/#dom-url-pathname
         */
        [[nodiscard]] std::string_view get_pathname() const noexcept;

        /**
         * Compute the pathname length in bytes without instantiating a view or a
         * string.
         * @return size of the pathname in bytes
         * @see https://url.spec.whatwg.org/#dom-url-pathname
         */
        [[nodiscard]] KUMO_FORCE_INLINE size_t get_pathname_length() const noexcept;

        /**
         * Return U+003F (?), followed by this's URL's query.
         * @return a newly allocated string.
         * @see https://url.spec.whatwg.org/#dom-url-search
         */
        [[nodiscard]] std::string get_search() const noexcept;

        /**
         * The username getter steps are to return this's URL's username.
         * @return a constant reference to the underlying string.
         * @see https://url.spec.whatwg.org/#dom-url-username
         */
        [[nodiscard]] const std::string& get_username() const noexcept;

        /**
         * @return Returns true on successful operation.
         * @see https://url.spec.whatwg.org/#dom-url-username
         */
        bool set_username(std::string_view input);

        /**
         * @return Returns true on success.
         * @see https://url.spec.whatwg.org/#dom-url-password
         */
        bool set_password(std::string_view input);

        /**
         * @return Returns true on success.
         * @see https://url.spec.whatwg.org/#dom-url-port
         */
        bool set_port(std::string_view input);

        /**
         * This function always succeeds.
         * @see https://url.spec.whatwg.org/#dom-url-hash
         */
        void set_hash(std::string_view input);

        /**
         * This function always succeeds.
         * @see https://url.spec.whatwg.org/#dom-url-search
         */
        void set_search(std::string_view input);

        /**
         * @return Returns true on success.
         * @see https://url.spec.whatwg.org/#dom-url-search
         */
        bool set_pathname(std::string_view input);

        /**
         * @return Returns true on success.
         * @see https://url.spec.whatwg.org/#dom-url-host
         */
        bool set_host(std::string_view input);

        /**
         * @return Returns true on success.
         * @see https://url.spec.whatwg.org/#dom-url-hostname
         */
        bool set_hostname(std::string_view input);

        /**
         * @return Returns true on success.
         * @see https://url.spec.whatwg.org/#dom-url-protocol
         */
        bool set_protocol(std::string_view input);

        /**
         * @see https://url.spec.whatwg.org/#dom-url-href
         */
        bool set_href(std::string_view input);

        /**
         * The password getter steps are to return this's URL's password.
         * @return a constant reference to the underlying string.
         * @see https://url.spec.whatwg.org/#dom-url-password
         */
        [[nodiscard]] const std::string& get_password() const noexcept;

        /**
         * Return this's URL's port, serialized.
         * @return a newly constructed string representing the port.
         * @see https://url.spec.whatwg.org/#dom-url-port
         */
        [[nodiscard]] std::string get_port() const noexcept;

        /**
         * Return U+0023 (#), followed by this's URL's fragment.
         * @return a newly constructed string representing the hash.
         * @see https://url.spec.whatwg.org/#dom-url-hash
         */
        [[nodiscard]] std::string get_hash() const noexcept;

        /**
         * A URL includes credentials if its username or password is not the empty
         * string.
         */
        [[nodiscard]] KUMO_FORCE_INLINE bool has_credentials() const noexcept;

        /**
         * Useful for implementing efficient serialization for the URL.
         *
         * https://user:pass@example.com:1234/foo/bar?baz#quux
         *       |     |    |          | ^^^^|       |   |
         *       |     |    |          | |   |       |   `----- hash_start
         *       |     |    |          | |   |       `--------- search_start
         *       |     |    |          | |   `----------------- pathname_start
         *       |     |    |          | `--------------------- port
         *       |     |    |          `----------------------- host_end
         *       |     |    `---------------------------------- host_start
         *       |     `--------------------------------------- username_end
         *       `--------------------------------------------- protocol_end
         *
         * Inspired after servo/url
         *
         * @return a newly constructed component.
         *
         * @see
         * https://github.com/servo/rust-url/blob/b65a45515c10713f6d212e6726719a020203cc98/url/src/quirks.rs#L31
         */
        [[nodiscard]] KUMO_FORCE_INLINE turbo::UriComponents get_components()
            const noexcept;
        /** @return true if the URL has a hash component */
        [[nodiscard]] inline bool has_hash() const noexcept override;
        /** @return true if the URL has a search component */
        [[nodiscard]] inline bool has_search() const noexcept override;

    private:
        void strip_trailing_spaces_from_opaque_path() noexcept;

        friend class WptParser;
        inline void update_unencoded_base_hash(std::string_view input);
        inline void update_base_hostname(std::string_view input);
        inline void update_base_search(std::string_view input);
        inline void update_base_search(std::string_view input,
            const uint8_t query_percent_encode_set[]);
        inline void update_base_search(std::optional<std::string> input);
        inline void update_base_pathname(std::string_view input);
        inline void update_base_username(std::string_view input);
        inline void update_base_password(std::string_view input);
        inline void update_base_port(std::optional<uint16_t> input);

        /**
         * Sets the host or hostname according to override condition.
         * Return true on success.
         * @see https://url.spec.whatwg.org/#hostname-state
         */

        bool set_host_or_hostname(std::string_view input, bool override_hostname);

        /**
         * A URL's scheme is an ASCII string that identifies the type of URL and can
         * be used to dispatch a URL for further processing after parsing. It is
         * initially the empty string. We only set non_special_scheme when the scheme
         * is non-special, otherwise we avoid constructing string.
         *
         * Special schemes are stored in turbo::details::is_special_list so we
         * typically do not need to store them in each url instance.
         */
        std::string non_special_scheme { };

        /**
         * A URL cannot have a username/password/port if its host is null or the empty
         * string, or its scheme is "file".
         */
        [[nodiscard]] inline bool cannot_have_credentials_or_port() const;

        /**
         * Take the scheme from another URL. The scheme string is copied from the
         * provided url.
         */
        inline void copy_scheme(const WptUri& u);

        inline void clear_pathname() override;
        inline void clear_search() override;
        inline void set_protocol_as_file();

        /**
         * Set the scheme for this URL. The provided scheme should be a valid
         * scheme string, be lower-cased, not contain spaces or tabs. It should
         * have no spurious trailing or leading content.
         */
        inline void set_scheme(std::string&& new_scheme) noexcept;

        /**
         * Take the scheme from another URL. The scheme string is moved from the
         * provided url.
         */
        inline void copy_scheme(WptUri&& u) noexcept;

    }; // struct url

    inline std::ostream& operator<<(std::ostream& out, const WptUri& u);

    [[nodiscard]] KUMO_FORCE_INLINE bool WptUri::has_credentials() const noexcept {
        return !username.empty() || !password.empty();
    }
    [[nodiscard]] KUMO_FORCE_INLINE bool WptUri::has_port() const noexcept {
        return port.has_value();
    }
    [[nodiscard]] inline bool WptUri::cannot_have_credentials_or_port() const {
        return !host.has_value() || host.value().empty() || type == turbo::SchemaType::FILE;
    }
    [[nodiscard]] inline bool WptUri::has_empty_hostname() const noexcept {
        if (!host.has_value()) {
            return false;
        }
        return host.value().empty();
    }
    [[nodiscard]] inline bool WptUri::has_hostname() const noexcept {
        return host.has_value();
    }
    inline std::ostream& operator<<(std::ostream& out, const WptUri& u) {
        return out << u.to_string();
    }

    [[nodiscard]] size_t WptUri::get_pathname_length() const noexcept {
        return path.size();
    }

    [[nodiscard]] KUMO_FORCE_INLINE turbo::UriComponents WptUri::get_components()
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

            if (!has_opaque_path() && turbo::starts_with(path, "//")) {
                // If url's host is null, url does not have an opaque path, url's path's
                // size is greater than 1, and url's path[0] is the empty string, then
                // append U+002F (/) followed by U+002E (.) to output.
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

    inline void WptUri::update_base_hostname(std::string_view input) {
        host = input;
    }

    inline void WptUri::update_unencoded_base_hash(std::string_view input) {
        // We do the percent encoding
        hash = turbo::percent_encode(input,
            turbo::uri_charsets::FRAGMENT_PERCENT_ENCODE);
    }

    inline void WptUri::update_base_search(std::string_view input,
        const uint8_t   query_percent_encode_set[]) {
        query = turbo::percent_encode(input, query_percent_encode_set);
    }

    inline void WptUri::update_base_search(std::optional<std::string> input) {
        query = input;
    }

    inline void WptUri::update_base_pathname(const std::string_view input) {
        path = input;
    }

    inline void WptUri::update_base_username(const std::string_view input) {
        username = input;
    }

    inline void WptUri::update_base_password(const std::string_view input) {
        password = input;
    }

    inline void WptUri::update_base_port(std::optional<uint16_t> input) {
        port = input;
    }

    inline void WptUri::clear_pathname() {
        path.clear();
    }

    inline void WptUri::clear_search() {
        query = std::nullopt;
    }

    [[nodiscard]] inline bool WptUri::has_hash() const noexcept {
        return hash.has_value();
    }

    [[nodiscard]] inline bool WptUri::has_search() const noexcept {
        return query.has_value();
    }

    inline void WptUri::set_protocol_as_file() {
        type = turbo::SchemaType::FILE;
    }

    inline void WptUri::set_scheme(std::string&& new_scheme) noexcept {
        type = turbo::get_scheme_type(new_scheme);
        // We only move the 'scheme' if it is non-special.
        if (!is_special()) {
            non_special_scheme = std::move(new_scheme);
        }

    }

    inline void WptUri::copy_scheme(WptUri&& u) noexcept {
        non_special_scheme = u.non_special_scheme;
        type = u.type;
    }

    inline void WptUri::copy_scheme(const WptUri& u) {
        non_special_scheme = u.non_special_scheme;
        type = u.type;
    }

    [[nodiscard]] KUMO_FORCE_INLINE std::string WptUri::get_href() const noexcept {
        std::string output = get_protocol();

        if (host.has_value()) {
            output += "//";
            if (has_credentials()) {
                output += username;
                if (!password.empty()) {
                    output += ":" + get_password();
                }
                output += "@";
            }
            output += host.value();
            if (port.has_value()) {
                output += ":" + get_port();
            }
        } else if (!has_opaque_path() && turbo::starts_with(path, "//")) {
            // If url's host is null, url does not have an opaque path, url's path's
            // size is greater than 1, and url's path[0] is the empty string, then
            // append U+002F (/) followed by U+002E (.) to output.
            output += "/.";
        }
        output += path;
        if (query.has_value()) {
            output += "?" + query.value();
        }
        if (hash.has_value()) {
            output += "#" + hash.value();
        }
        return output;
    }

}  // namespace turbo
