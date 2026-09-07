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
#include <vector>
#include <turbo/uri/utility.h>

namespace turbo {

    enum class UriSearchParamsIterType {
        KEYS,
        VALUES,
        ENTRIES,
    };

    template <typename T, UriSearchParamsIterType Type>
    struct UriSearchParamsIter;

    typedef std::pair<std::string_view, std::string_view> key_value_view_pair;

    using url_search_params_keys_iter = UriSearchParamsIter<std::string_view, UriSearchParamsIterType::KEYS>;
    using url_search_params_values_iter = UriSearchParamsIter<std::string_view,
        UriSearchParamsIterType::VALUES>;
    using url_search_params_entries_iter = UriSearchParamsIter<key_value_view_pair,
        UriSearchParamsIterType::ENTRIES>;

    /**
     * @see https://url.spec.whatwg.org/#interface-urlsearchparams
     */
    struct UriSearchParams {
        UriSearchParams() = default;

        /**
         * @see
         * https://github.com/web-platform-tests/wpt/blob/master/url/urlsearchparams-constructor.any.js
         */
        UriSearchParams(const std::string_view input) { initialize(input); }

        UriSearchParams(const UriSearchParams& u) = default;
        UriSearchParams(UriSearchParams&& u) noexcept = default;
        UriSearchParams& operator=(UriSearchParams&& u) noexcept = default;
        UriSearchParams& operator=(const UriSearchParams& u) = default;
        ~UriSearchParams() = default;

        [[nodiscard]] inline size_t size() const noexcept;

        /**
         * @see https://url.spec.whatwg.org/#dom-urlsearchparams-append
         */
        inline void append(std::string_view key, std::string_view value);

        /**
         * @see https://url.spec.whatwg.org/#dom-urlsearchparams-delete
         */
        inline void remove(std::string_view key);
        inline void remove(std::string_view key, std::string_view value);

        /**
         * @see https://url.spec.whatwg.org/#dom-urlsearchparams-get
         */
        inline std::optional<std::string_view> get(std::string_view key);

        /**
         * @see https://url.spec.whatwg.org/#dom-urlsearchparams-getall
         */
        inline std::vector<std::string> get_all(std::string_view key);

        /**
         * @see https://url.spec.whatwg.org/#dom-urlsearchparams-has
         */
        inline bool has(std::string_view key) noexcept;
        inline bool has(std::string_view key, std::string_view value) noexcept;

        /**
         * @see https://url.spec.whatwg.org/#dom-urlsearchparams-set
         */
        inline void set(std::string_view key, std::string_view value);

        /**
         * @see https://url.spec.whatwg.org/#dom-urlsearchparams-sort
         */
        inline void sort();

        /**
         * @see https://url.spec.whatwg.org/#urlsearchparams-stringification-behavior
         */
        inline std::string to_string() const;

        /**
         * Returns a simple JS-style iterator over all of the keys in this
         * UriSearchParams. The keys in the iterator are not unique. The valid
         * lifespan of the iterator is tied to the UriSearchParams. The iterator
         * must be freed when you're done with it.
         * @see https://url.spec.whatwg.org/#interface-urlsearchparams
         */
        inline url_search_params_keys_iter get_keys();

        /**
         * Returns a simple JS-style iterator over all of the values in this
         * UriSearchParams. The valid lifespan of the iterator is tied to the
         * UriSearchParams. The iterator must be freed when you're done with it.
         * @see https://url.spec.whatwg.org/#interface-urlsearchparams
         */
        inline url_search_params_values_iter get_values();

        /**
         * Returns a simple JS-style iterator over all of the entries in this
         * UriSearchParams. The entries are pairs of keys and corresponding values.
         * The valid lifespan of the iterator is tied to the UriSearchParams. The
         * iterator must be freed when you're done with it.
         * @see https://url.spec.whatwg.org/#interface-urlsearchparams
         */
        inline url_search_params_entries_iter get_entries();

        /**
         * C++ style conventional iterator support. const only because we
         * do not really want the params to be modified via the iterator.
         */
        inline auto begin() const { return params.begin(); }
        inline auto end() const { return params.end(); }
        inline auto front() const { return params.front(); }
        inline auto back() const { return params.back(); }
        inline auto operator[](size_t index) const { return params[index]; }

        /**
         * @private
         * Used to reset the search params to a new input.
         * Used primarily for C API.
         * @param input
         */
        void reset(std::string_view input);

    private:
        typedef std::pair<std::string, std::string> key_value_pair;
        std::vector<key_value_pair> params { };

        /**
         * @see https://url.spec.whatwg.org/#concept-urlencoded-parser
         */
        void initialize(std::string_view init);

        template <typename T, UriSearchParamsIterType Type>
        friend struct UriSearchParamsIter;
    }; // UriSearchParams

    /**
     * Implements a non-conventional iterator pattern that is closer in style to
     * JavaScript's definition of an iterator.
     *
     * @see https://webidl.spec.whatwg.org/#idl-iterable
     */
    template <typename T, UriSearchParamsIterType Type>
    struct UriSearchParamsIter {
        inline UriSearchParamsIter()
            : params(EMPTY) { }
        UriSearchParamsIter(const UriSearchParamsIter& u) = default;
        UriSearchParamsIter(UriSearchParamsIter&& u) noexcept = default;
        UriSearchParamsIter& operator=(UriSearchParamsIter&& u) noexcept = default;
        UriSearchParamsIter& operator=(const UriSearchParamsIter& u) = default;
        ~UriSearchParamsIter() = default;

        /**
         * Return the next item in the iterator or std::nullopt if done.
         */
        inline std::optional<T> next();

        inline bool has_next();

    private:
        static UriSearchParams EMPTY;
        inline UriSearchParamsIter(UriSearchParams& params_)
            : params(params_) { }

        UriSearchParams& params;
        size_t pos = 0;

        friend struct UriSearchParams;
    };

     // A default, empty UriSearchParams for use with empty iterators.
    template <typename T, UriSearchParamsIterType Type>
    UriSearchParams UriSearchParamsIter<T, Type>::EMPTY;

    inline void UriSearchParams::reset(std::string_view input) {
        params.clear();
        initialize(input);
    }

    inline void UriSearchParams::initialize(std::string_view input) {
        if (!input.empty() && input.front() == '?') {
            input.remove_prefix(1);
        }

        auto process_key_value = [&](const std::string_view current) {
            auto equal = current.find('=');

            if (equal == std::string_view::npos) {
                std::string name(current);
                std::replace(name.begin(), name.end(), '+', ' ');
                params.emplace_back(turbo::percent_decode(name, name.find('%')), "");
            } else {
                std::string name(current.substr(0, equal));
                std::string value(current.substr(equal + 1));

                std::replace(name.begin(), name.end(), '+', ' ');
                std::replace(value.begin(), value.end(), '+', ' ');

                params.emplace_back(turbo::percent_decode(name, name.find('%')),
                    turbo::percent_decode(value, value.find('%')));
            }
        };

        while (!input.empty()) {
            auto ampersand_index = input.find('&');

            if (ampersand_index == std::string_view::npos) {
                if (!input.empty()) {
                    process_key_value(input);
                }
                break;
            } else if (ampersand_index != 0) {
                process_key_value(input.substr(0, ampersand_index));
            }

            input.remove_prefix(ampersand_index + 1);
        }
    }

    inline void UriSearchParams::append(const std::string_view key,
        const std::string_view value) {
        params.emplace_back(key, value);
    }

    inline size_t UriSearchParams::size() const noexcept {
        return params.size();
    }

    inline std::optional<std::string_view> UriSearchParams::get(
        const std::string_view key) {
        auto entry = std::find_if(params.begin(), params.end(),
            [&key](auto& param) { return param.first == key; });

        if (entry == params.end()) {
            return std::nullopt;
        }

        return entry->second;
    }

    inline std::vector<std::string> UriSearchParams::get_all(
        const std::string_view key) {
        std::vector<std::string> out { };

        for (auto& param : params) {
            if (param.first == key) {
                out.emplace_back(param.second);
            }
        }

        return out;
    }

    inline bool UriSearchParams::has(const std::string_view key) noexcept {
        auto entry = std::find_if(params.begin(), params.end(),
            [&key](auto& param) { return param.first == key; });
        return entry != params.end();
    }

    inline bool UriSearchParams::has(std::string_view key,
        std::string_view value) noexcept {
        auto entry = std::find_if(params.begin(), params.end(), [&key, &value](auto& param) {
            return param.first == key && param.second == value;
        });
        return entry != params.end();
    }

    inline std::string UriSearchParams::to_string() const {
        auto character_set = turbo::uri_charsets::WWW_FORM_URLENCODED_PERCENT_ENCODE;
        std::string out { };
        for (size_t i = 0; i < params.size(); i++) {
            auto key = turbo::percent_encode(params[i].first, character_set);
            auto value = turbo::percent_encode(params[i].second, character_set);

            // Performance optimization: Move this inside percent_encode.
            std::replace(key.begin(), key.end(), ' ', '+');
            std::replace(value.begin(), value.end(), ' ', '+');

            if (i != 0) {
                out += "&";
            }
            out.append(key);
            out += "=";
            out.append(value);
        }
        return out;
    }

    inline void UriSearchParams::set(const std::string_view key,
        const std::string_view value) {
        const auto find = [&key](auto& param) { return param.first == key; };

        auto it = std::find_if(params.begin(), params.end(), find);

        if (it == params.end()) {
            params.emplace_back(key, value);
        } else {
            it->second = value;
            params.erase(std::remove_if(std::next(it), params.end(), find),
                params.end());
        }
    }

    inline void UriSearchParams::remove(const std::string_view key) {
        params.erase(
            std::remove_if(params.begin(), params.end(),
                [&key](auto& param) { return param.first == key; }),
            params.end());
    }

    inline void UriSearchParams::remove(const std::string_view key,
        const std::string_view value) {
        params.erase(std::remove_if(params.begin(), params.end(),
                         [&key, &value](auto& param) {
                             return param.first == key && param.second == value;
                         }),
            params.end());
    }

    inline void UriSearchParams::sort() {
        std::stable_sort(params.begin(), params.end(),
            [](const key_value_pair& lhs, const key_value_pair& rhs) {
                return lhs.first < rhs.first;
            });
    }

    inline url_search_params_keys_iter UriSearchParams::get_keys() {
        return url_search_params_keys_iter(*this);
    }

    /**
     * @see https://url.spec.whatwg.org/#interface-urlsearchparams
     */
    inline url_search_params_values_iter UriSearchParams::get_values() {
        return url_search_params_values_iter(*this);
    }

    /**
     * @see https://url.spec.whatwg.org/#interface-urlsearchparams
     */
    inline url_search_params_entries_iter UriSearchParams::get_entries() {
        return url_search_params_entries_iter(*this);
    }

    template <typename T, UriSearchParamsIterType Type>
    inline bool UriSearchParamsIter<T, Type>::has_next() {
        return pos < params.params.size();
    }

    template <>
    inline std::optional<std::string_view> url_search_params_keys_iter::next() {
        if (!has_next()) {
            return std::nullopt;
        }
        return params.params[pos++].first;
    }

    template <>
    inline std::optional<std::string_view> url_search_params_values_iter::next() {
        if (!has_next()) {
            return std::nullopt;
        }
        return params.params[pos++].second;
    }

    template <>
    inline std::optional<key_value_view_pair>
    url_search_params_entries_iter::next() {
        if (!has_next()) {
            return std::nullopt;
        }
        return params.params[pos++];
    }

} // namespace turbo
