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

#include <turbo/uri/uri_error.h>
#include <turbo/uri/scheme.h>
#include <turbo/uri/utility.h>
#include <turbo/uri/uri_common_base.h>

namespace turbo::uri_wpt {

    UriError check_opaque_host(std::string_view input);

    /// input should be validate by @check_opaque_host.
    std::string encode_opaque_host(std::string_view input);

    ///////////////////////////////////////////////////////////////////////////////
    /// Parse the path from the provided input.
    /// Return true on success. Control characters not
    /// trimmed from the ends (they should have
    /// been removed if needed).
    ///
    /// The input is expected to be UTF-8.
    ///
    /// @see https://url.spec.whatwg.org/
    std::string parse_path(std::string_view input, bool is_special, bool have_host, turbo::SchemaType type);

    UriError parse_host(std::string_view input, bool is_special, UriHostType& ht, std::string * result);

    bool parse_scheme(const std::string_view input, SchemaType &type, std::string *result);

    bool parse_scheme_state_override(const std::string_view input,
        bool is_special, bool has_credentials, bool host_empty,std::optional<uint16_t> & port,
        SchemaType &type, std::string *result);

    UriError parse_port(std::string_view view, bool is_special,SchemaType type, bool check_trailing_content, std::optional<uint16_t> &port) noexcept;


      KUMO_FORCE_INLINE std::pair<size_t, bool> get_host_delimiter_location(const bool is_special, std::string_view& view) noexcept {
        /**
         * The spec at https://url.spec.whatwg.org/#hostname-state expects us to
         * compute a variable called insideBrackets but this variable is only used
         * once, to check whether a ':' character was found outside brackets. Exact
         * text: "Otherwise, if c is U+003A (:) and insideBrackets is false, then:".
         * It is conceptually simpler and arguably more efficient to just return a
         * Boolean indicating whether ':' was found outside brackets.
         */
        const size_t view_size = view.size();
        size_t location = 0;
        bool found_colon = false;
        ///
        /// Performance analysis:
        ///
         /// We are basically seeking the end of the hostname which can be indicated
         /// by the end of the view, or by one of the characters ':', '/', '?', '\\'
         /// (where '\\' is only applicable for special URLs). However, these must
         /// appear outside a bracket range. E.g., if you have [something?]fd: then the
         /// '?' does not count.
         ///
         /// So we can skip ahead to the next delimiter, as long as we include '[' in
         /// the set of delimiters, and that we handle it first.
         ///
         /// So the trick is to have a fast function that locates the next delimiter.
         /// Unless we find '[', then it only needs to be called once! Ideally, such a
         /// function would be provided by the C++ standard library, but it seems that
         /// find_first_of is not very fast, so we are forced to roll our own.
        ///
         /// We do not break into two loops for speed, but for clarity.
         ///
        if (is_special) {
            // We move to the next delimiter.
            location = find_next_host_delimiter_special(view, location);
            // Unless we find '[' then we are going only going to have to call
            // find_next_host_delimiter_special once.
            for (; location < view_size;
                location = find_next_host_delimiter_special(view, location)) {
                if (view[location] == '[') {
                    location = view.find(']', location);
                    if (location == std::string_view::npos) {
                        // performance: view.find might get translated to a memchr, which
                        // has no notion of std::string_view::npos, so the code does not
                        // reflect the assembly.
                        location = view_size;
                        break;
                    }
                } else {
                    found_colon = view[location] == ':';
                    break;
                }
            }
        } else {
            // We move to the next delimiter.
            location = find_next_host_delimiter(view, location);
            // Unless we find '[' then we are going only going to have to call
            // find_next_host_delimiter_special once.
            for (; location < view_size;
                location = find_next_host_delimiter(view, location)) {
                if (view[location] == '[') {
                    location = view.find(']', location);
                    if (location == std::string_view::npos) {
                        // performance: view.find might get translated to a memchr, which
                        // has no notion of std::string_view::npos, so the code does not
                        // reflect the assembly.
                        location = view_size;
                        break;
                    }
                } else {
                    found_colon = view[location] == ':';
                    break;
                }
            }
        }
        // performance: remove_suffix may translate into a single instruction.
        view.remove_suffix(view_size - location);
        return { location, found_colon };
    }

}  // namespace turbo::uri_wpt

