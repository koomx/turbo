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

#include <limits>
#include <turbo/bits/bits.h>
#include <turbo/strings/match.h>
#include <turbo/uri/character_sets.h>
#include <turbo/uri/checkers.h>
#include <turbo/uri/utility.h>
#include <turbo/uri/wpt/parse_components.h>
#include <turbo/uri/wpt/parser.h>
#include <turbo/uri/wpt/state.h>
#include <turbo/uri/wpt/uri.h>

namespace turbo {

   void WptParser::parse_url_with_base_impl(std::string_view user_input,
        const WptUri& base_uri, WptUri&uri) {

        turbo::WptState state = turbo::WptState::SCHEME_START;

        // We refuse to parse URL strings that exceed 4GB. Such strings are almost
        // surely the result of a bug or are otherwise a security concern.
        if (user_input.size() > std::numeric_limits<uint32_t>::max()) {
            uri.uri_error() = { UriErrorCode::kUriOverflow, 0, "input overflow" };
        }
        // Going forward, user_input.size() is in [0,
        // std::numeric_limits<uint32_t>::max). If we are provided with an invalid
        // base, or the optional_url was invalid, we must return.
        uri.uri_error() &= base_uri.uri_error();
        if (!uri.ok()) {
            return;
        }
        std::string tmp_buffer;
        std::string_view internal_input;
        if (turbo::has_tabs_or_newline(user_input)) {
            tmp_buffer = user_input;
            // Optimization opportunity: Instead of copying and then pruning, we could
            // just directly build the string from user_input.
            turbo::remove_ascii_tab_or_newline(tmp_buffer);
            internal_input = tmp_buffer;
        } else {
            internal_input = user_input;
        }

        // Leading and trailing control characters are uncommon and easy to deal with
        // (no performance concern).
        std::string_view url_data = internal_input;
        turbo::trim_c0_whitespace(url_data);

        // Optimization opportunity. Most websites do not have fragment.
        std::optional<std::string_view> fragment = turbo::prune_hash(url_data);

        // Here url_data no longer has its fragment.
        // We are going to access the data from url_data (it is immutable).
        // At any given time, we are pointing at byte 'input_position' in url_data.
        // The input_position variable should range from 0 to input_size.
        // It is illegal to access url_data at input_size.
        size_t input_position = 0;
        const size_t input_size = url_data.size();
        // Keep running the following state machine by switching on state.
        // If after a run pointer points to the EOF code point, go to the next step.
        // Otherwise, increase pointer by 1 and continue with the state machine.
        // We never decrement input_position.
        while (input_position <= input_size) {
            switch (state) {
            case turbo::WptState::SCHEME_START: {
                // If c is an ASCII alpha, append c, lowercased, to buffer, and set
                // state to scheme state.
                if ((input_position != input_size) && turbo::ascii_isalpha(url_data[input_position])) {
                    state = turbo::WptState::SCHEME;
                    input_position++;
                } else {
                    // Otherwise, if state override is not given, set state to no scheme
                    // state and decrease pointer by 1.
                    state = turbo::WptState::NO_SCHEME;
                }
                break;
            }
            case turbo::WptState::SCHEME: {
                // If c is an ASCII alphanumeric, U+002B (+), U+002D (-), or U+002E (.),
                // append c, lowercased, to buffer.
                while ((input_position != input_size) && (turbo::is_valid_schema_alnum(url_data[input_position]))) {
                    input_position++;
                }
                // Otherwise, if c is U+003A (:), then:
                if ((input_position != input_size) && (url_data[input_position] == ':')) {
                    std::string scheme;
                    auto scheme_view = url_data.substr(0, input_position);
                    if (!turbo::uri_wpt::parse_scheme(
                            scheme_view, uri.type, &scheme)) {
                        return;
                    }
                    if (uri.type == turbo::SchemaType::NOT_SPECIAL) {
                        uri.set_scheme(std::move(scheme));
                    }

                    // If url's scheme is "file", then:
                    if (uri.type == turbo::SchemaType::FILE) {
                        // Set state to file state.
                        state = turbo::WptState::FILE;
                    }
                    // Otherwise, if url is special, base is non-null, and base's scheme
                    // is url's scheme: Note: Doing base_url->scheme is unsafe if base_url
                    // != nullptr is false.
                    else if (uri.is_special() && base_uri.type == uri.type) {
                        // Set state to special relative or authority state.
                        state = turbo::WptState::SPECIAL_RELATIVE_OR_AUTHORITY;
                    }
                    // Otherwise, if url is special, set state to special authority
                    // slashes state.
                    else if (uri.is_special()) {
                        state = turbo::WptState::SPECIAL_AUTHORITY_SLASHES;
                    }
                    // Otherwise, if remaining starts with an U+002F (/), set state to
                    // path or authority state and increase pointer by 1.
                    else if (input_position + 1 < input_size && url_data[input_position + 1] == '/') {
                        state = turbo::WptState::PATH_OR_AUTHORITY;
                        input_position++;
                    }
                    // Otherwise, set url's path to the empty string and set state to
                    // opaque path state.
                    else {
                        state = turbo::WptState::OPAQUE_PATH;
                    }
                }
                // Otherwise, if state override is not given, set buffer to the empty
                // string, state to no scheme state, and start over (from the first code
                // point in input).
                else {
                    state = turbo::WptState::NO_SCHEME;
                    input_position = 0;
                    break;
                }
                input_position++;
                break;
            }
            case turbo::WptState::NO_SCHEME: {
                // If base is null, or base has an opaque path and c is not U+0023 (#),
                // validation error, return failure.
                if (base_uri.has_opaque_path() && !fragment.has_value()) {
                    uri.uri_error() = { UriErrorCode::kUriNotComplete, 0, "no schema" };
                    return;
                } else if (base_uri.has_opaque_path() && fragment.has_value() && input_position == input_size) {
                    // Otherwise, if base has an opaque path and c is U+0023 (#),
                    // set url's scheme to base's scheme, url's path to base's path, url's
                    // query to base's query, and set state to fragment state.
                    uri.copy_scheme(base_uri);
                    uri.has_opaque_path() = base_uri.has_opaque_path();

                    uri.path = base_uri.path;
                    uri.query = base_uri.query;
                    uri.update_unencoded_base_hash(*fragment);
                    return;
                }else if (base_uri.type != turbo::SchemaType::FILE) {
                    // Otherwise, if base's scheme is not "file", set state to relative
                    // state and decrease pointer by 1.
                    state = turbo::WptState::RELATIVE_SCHEME;
                } else {
                    // Otherwise, set state to file state and decrease pointer by 1.
                    state = turbo::WptState::FILE;
                }
                break;
            }
            case turbo::WptState::AUTHORITY: {
                // most URLs have no @. Having no @ tells us that we don't have to worry
                // about AUTHORITY. Of course, we could have @ and still not have to
                // worry about AUTHORITY.
                // TODO: Instead of just collecting a bool, collect the location of the
                // '@' and do something useful with it.
                // TODO: We could do various processing early on, using a single pass
                // over the string to collect information about it, e.g., telling us
                // whether there is a @ and if so, where (or how many).
                const bool contains_ampersand = (url_data.find('@', input_position) != std::string_view::npos);

                if (!contains_ampersand) {
                    state = turbo::WptState::HOST;
                    break;
                }
                bool at_sign_seen { false };
                bool password_token_seen { false };
                ///
                /// We expect something of the sort...
                /// https://user:pass@example.com:1234/foo/bar?baz#quux
                /// --------^
                ///
                do {
                    std::string_view view = turbo::subview(url_data, input_position);
                    // The delimiters are @, /, ? \\.
                    size_t location = uri.is_special() ? turbo::find_authority_delimiter_special(view)
                                                       : turbo::find_authority_delimiter(view);
                    std::string_view authority_view(view.data(), location);
                    size_t end_of_authority = input_position + authority_view.size();
                    // If c is U+0040 (@), then:
                    if ((end_of_authority != input_size) && (url_data[end_of_authority] == '@')) {
                        // If atSignSeen is true, then prepend "%40" to buffer.
                        if (at_sign_seen) {
                            if (password_token_seen) {
                                uri.password += "%40";
                            } else {
                                uri.username += "%40";
                            }
                        }

                        at_sign_seen = true;

                        if (!password_token_seen) {
                            size_t password_token_location = authority_view.find(':');
                            password_token_seen = password_token_location != std::string_view::npos;

                            if (!password_token_seen) {
                                uri.username += turbo::percent_encode(
                                    authority_view,
                                    turbo::uri_charsets::USERINFO_PERCENT_ENCODE);
                            } else {
                                uri.username += turbo::percent_encode(
                                    authority_view.substr(0, password_token_location),
                                    turbo::uri_charsets::USERINFO_PERCENT_ENCODE);
                                uri.password += turbo::percent_encode(
                                    authority_view.substr(password_token_location + 1),
                                    turbo::uri_charsets::USERINFO_PERCENT_ENCODE);
                            }
                        } else {
                            uri.password += turbo::percent_encode(
                                authority_view, turbo::uri_charsets::USERINFO_PERCENT_ENCODE);
                        }
                    } else if (end_of_authority == input_size || url_data[end_of_authority] == '/' ||
                        url_data[end_of_authority] == '?' || (uri.is_special() && url_data[end_of_authority] == '\\')) {
                        // Otherwise, if one of the following is true:
                        // - c is the EOF code point, U+002F (/), U+003F (?), or U+0023 (#)
                        // - url is special and c is U+005C (\)
                        //
                        // If atSignSeen is true and authority_view is the empty string,
                        // validation error, return failure.
                        if (at_sign_seen && authority_view.empty()) {
                            uri.uri_error() = { UriErrorCode::kUriNotComplete, 0, "no authority" };
                            return;
                        }
                        state = turbo::WptState::HOST;
                        break;
                    }
                    if (end_of_authority == input_size) {
                        if (fragment.has_value()) {
                            uri.update_unencoded_base_hash(*fragment);
                        }
                        return;
                    }
                    input_position = end_of_authority + 1;
                } while (true);

                break;
            }
            case turbo::WptState::SPECIAL_RELATIVE_OR_AUTHORITY: {

                // If c is U+002F (/) and remaining starts with U+002F (/),
                // then set state to special authority ignore slashes state and increase
                // pointer by 1.
                std::string_view view = turbo::subview(url_data, input_position);
                if (turbo::starts_with(view, "//")) {
                    state = turbo::WptState::SPECIAL_AUTHORITY_IGNORE_SLASHES;
                    input_position += 2;
                } else {
                    // Otherwise, validation error, set state to relative state and
                    // decrease pointer by 1.
                    state = turbo::WptState::RELATIVE_SCHEME;
                }

                break;
            }
            case turbo::WptState::PATH_OR_AUTHORITY: {

                // If c is U+002F (/), then set state to authority state.
                if ((input_position != input_size) && (url_data[input_position] == '/')) {
                    state = turbo::WptState::AUTHORITY;
                    input_position++;
                } else {
                    // Otherwise, set state to path state, and decrease pointer by 1.
                    state = turbo::WptState::PATH;
                }

                break;
            }
            case turbo::WptState::RELATIVE_SCHEME: {

                // Set url's scheme to base's scheme.
                uri.copy_scheme(base_uri);

                // If c is U+002F (/), then set state to relative slash state.
                if ((input_position != input_size) && (url_data[input_position] == '/')) {
                    state = turbo::WptState::RELATIVE_SLASH;
                } else if (uri.is_special() && (input_position != input_size) && (url_data[input_position] == '\\')) {
                    // Otherwise, if url is special and c is U+005C (\), validation error,
                    // set state to relative slash state.
                    state = turbo::WptState::RELATIVE_SLASH;
                } else {
                    // Set url's username to base's username, url's password to base's
                    // password, url's host to base's host, url's port to base's port,
                    // url's path to a clone of base's path, and url's query to base's
                    // query.
                    uri.username = base_uri.username;
                    uri.password = base_uri.password;
                    uri.host = base_uri.host;
                    uri.port = base_uri.port;
                    // cloning the base path includes cloning the has_opaque_path flag
                    uri.path = base_uri.path;
                    uri.query = base_uri.query;

                    uri.has_opaque_path() = base_uri.has_opaque_path();

                    // If c is U+003F (?), then set url's query to the empty string, and
                    // state to query state.
                    if ((input_position != input_size) && (url_data[input_position] == '?')) {
                        state = turbo::WptState::QUERY;
                    }
                    // Otherwise, if c is not the EOF code point:
                    else if (input_position != input_size) {
                        // Set url's query to null.
                        uri.clear_search();
                        // Shorten url's path.
                        turbo::shorten_path(uri.path, uri.type);

                        // Set state to path state and decrease pointer by 1.
                        state = turbo::WptState::PATH;
                        break;
                    }
                }
                input_position++;
                break;
            }
            case turbo::WptState::RELATIVE_SLASH: {

                // If url is special and c is U+002F (/) or U+005C (\), then:
                if (uri.is_special() && (input_position != input_size) && (url_data[input_position] == '/' || url_data[input_position] == '\\')) {
                    // Set state to special authority ignore slashes state.
                    state = turbo::WptState::SPECIAL_AUTHORITY_IGNORE_SLASHES;
                }
                // Otherwise, if c is U+002F (/), then set state to authority state.
                else if ((input_position != input_size) && (url_data[input_position] == '/')) {
                    state = turbo::WptState::AUTHORITY;
                } else {
                    // Otherwise, set
                    // - url's username to base's username,
                    // - url's password to base's password,
                    // - url's host to base's host,
                    // - url's port to base's port,
                    // - state to path state, and then, decrease pointer by 1.
                    uri.username = base_uri.username;
                    uri.password = base_uri.password;
                    uri.host = base_uri.host;
                    uri.port = base_uri.port;
                    state = turbo::WptState::PATH;
                    break;
                }

                input_position++;
                break;
            }
            case turbo::WptState::SPECIAL_AUTHORITY_SLASHES: {

                // If c is U+002F (/) and remaining starts with U+002F (/),
                // then set state to special authority ignore slashes state and increase
                // pointer by 1.
                std::string_view view = turbo::subview(url_data, input_position);
                if (turbo::starts_with(view, "//")) {
                    input_position += 2;
                }

                [[fallthrough]];
            }
            case turbo::WptState::SPECIAL_AUTHORITY_IGNORE_SLASHES: {

                // If c is neither U+002F (/) nor U+005C (\), then set state to
                // authority state and decrease pointer by 1.
                while ((input_position != input_size) && ((url_data[input_position] == '/') || (url_data[input_position] == '\\'))) {
                    input_position++;
                }
                state = turbo::WptState::AUTHORITY;

                break;
            }
            case turbo::WptState::QUERY: {
                // Let queryPercentEncodeSet be the special-query percent-encode set
                // if url is special; otherwise the query percent-encode set.
                const uint8_t* query_percent_encode_set = uri.is_special()
                    ? turbo::uri_charsets::SPECIAL_QUERY_PERCENT_ENCODE
                    : turbo::uri_charsets::QUERY_PERCENT_ENCODE;

                // Percent-encode after encoding, with encoding, buffer, and
                // queryPercentEncodeSet, and append the result to url's query.
                uri.update_base_search(turbo::subview(url_data, input_position),
                    query_percent_encode_set);
                if (fragment.has_value()) {
                    uri.update_unencoded_base_hash(*fragment);
                }
                return;
            }
            case turbo::WptState::HOST: {

                std::string_view host_view = turbo::subview(url_data, input_position);
                auto [location, found_colon] = uri_wpt::get_host_delimiter_location(uri.is_special(), host_view);
                input_position = (location != std::string_view::npos)
                    ? input_position + location
                    : input_size;
                // Otherwise, if c is U+003A (:) and insideBrackets is false, then:
                // Note: the 'found_colon' value is true if and only if a colon was
                // encountered while not inside brackets.
                if (found_colon) {
                    // If buffer is the empty string, validation error, return failure.
                    // Let host be the result of host parsing buffer with url is not
                    // special.
                    std::string tmp_host;
                    uri.uri_error() = turbo::uri_wpt::parse_host(host_view, uri.is_special(), uri.host_type(), &tmp_host);
                    if (!uri.ok()) {
                        return;
                    }
                    uri.host = std::move(tmp_host);
                    // Set url's host to host, buffer to the empty string, and state to
                    // port state.
                    state = turbo::WptState::PORT;
                    input_position++;
                }
                // Otherwise, if one of the following is true:
                // - c is the EOF code point, U+002F (/), U+003F (?), or U+0023 (#)
                // - url is special and c is U+005C (\)
                // The get_host_delimiter_location function either brings us to
                // the colon outside of the bracket, or to one of those characters.
                else {
                    // If url is special and host_view is the empty string, validation
                    // error, return failure.
                    if (uri.is_special() && host_view.empty()) {
                        uri.uri_error() = { UriErrorCode::kUriNotComplete, 0, "no host" };
                        ;
                        return;
                    }
                    // Let host be the result of host parsing host_view with url is not
                    // special.
                    if (host_view.empty()) {
                        uri.update_base_hostname("");
                    } else {
                        std::string tmp_host;
                        uri.uri_error() = turbo::uri_wpt::parse_host(host_view, uri.is_special(), uri.host_type(), &tmp_host);
                        if (!uri.ok()) {
                            return;
                        }
                        uri.host = tmp_host;
                    }

                    // Set url's host to host, and state to path start state.
                    state = turbo::WptState::PATH_START;
                }

                break;
            }
            case turbo::WptState::OPAQUE_PATH: {
                std::string_view view = turbo::subview(url_data, input_position);
                // If c is U+003F (?), then set url's query to the empty string and
                // state to query state.
                size_t location = view.find('?');
                if (location != std::string_view::npos) {
                    view.remove_suffix(view.size() - location);
                    state = turbo::WptState::QUERY;
                    input_position += location + 1;
                } else {
                    input_position = input_size + 1;
                }
                uri.has_opaque_path() = true;
                // This is a really unlikely scenario in real world. We should not seek
                // to optimize it.
                uri.update_base_pathname(turbo::percent_encode(
                    view, turbo::uri_charsets::C0_CONTROL_PERCENT_ENCODE));
                break;
            }
            case turbo::WptState::PORT: {
                std::string_view port_view = turbo::subview(url_data, input_position);
                std::optional<uint16_t> tmp_port;
                uri.uri_error() = uri_wpt::parse_port(port_view, uri.is_special(), uri.type, true, tmp_port);
                input_position += uri.uri_error().error_pos;
                if (!uri.ok()) {
                    return;
                }
                uri.port = tmp_port;
                state = turbo::WptState::PATH_START;
                [[fallthrough]];
            }
            case turbo::WptState::PATH_START: {

                // If url is special, then:
                if (uri.is_special()) {
                    // Set state to path state.
                    state = turbo::WptState::PATH;

                    // Optimization: Avoiding going into PATH state improves the
                    // performance of urls ending with /.
                    if (input_position == input_size) {
                        uri.update_base_pathname("/");
                        if (fragment.has_value()) {
                            uri.update_unencoded_base_hash(*fragment);
                        }
                        return;
                    }
                    // If c is neither U+002F (/) nor U+005C (\), then decrease pointer
                    // by 1. We know that (input_position == input_size) is impossible
                    // here, because of the previous if-check.
                    if ((url_data[input_position] != '/') && (url_data[input_position] != '\\')) {
                        break;
                    }
                } else if ((input_position != input_size) && (url_data[input_position] == '?')) {
                    // Otherwise, if state override is not given and c is U+003F (?),
                    // set url's query to the empty string and state to query state.
                    state = turbo::WptState::QUERY;
                } else if (input_position != input_size) {
                    // Otherwise, if c is not the EOF code point:
                    // Set state to path state.
                    state = turbo::WptState::PATH;

                    // If c is not U+002F (/), then decrease pointer by 1.
                    if (url_data[input_position] != '/') {
                        break;
                    }
                }

                input_position++;
                break;
            }
            case turbo::WptState::PATH: {
                std::string_view view = turbo::subview(url_data, input_position);

                // Most time, we do not need percent encoding.
                // Furthermore, we can immediately locate the '?'.
                size_t locofquestionmark = view.find('?');
                if (locofquestionmark != std::string_view::npos) {
                    state = turbo::WptState::QUERY;
                    view.remove_suffix(view.size() - locofquestionmark);
                    input_position += locofquestionmark + 1;
                } else {
                    input_position = input_size + 1;
                }

                turbo::parse_prepared_path(view, uri.type, uri.path);
                break;
            }
            case turbo::WptState::FILE_SLASH: {

                // If c is U+002F (/) or U+005C (\), then:
                if ((input_position != input_size) && (url_data[input_position] == '/' || url_data[input_position] == '\\')) {
                    // Set state to file host state.
                    state = turbo::WptState::FILE_HOST;
                    input_position++;
                } else {
                    // If base is non-null and base's scheme is "file", then:
                    // Note: it is unsafe to do base_url->scheme unless you know that
                    // base_url_has_value() is true.
                    if (base_uri.type == turbo::SchemaType::FILE) {
                        // Set url's host to base's host.
                        uri.host = base_uri.host;

                        // If the code point substring from pointer to the end of input does
                        // not start with a Windows drive letter and base's path[0] is a
                        // normalized Windows drive letter, then append base's path[0] to
                        // url's path.
                        if (!base_uri.get_pathname().empty()) {
                            if (!turbo::is_windows_drive_letter(
                                    turbo::subview(url_data, input_position))) {
                                std::string_view first_base_url_path = base_uri.get_pathname().substr(1);
                                size_t loc = first_base_url_path.find('/');
                                if (loc != std::string_view::npos) {
                                    turbo::resize(first_base_url_path, loc);
                                }
                                if (turbo::is_normalized_windows_drive_letter(
                                        first_base_url_path)) {
                                    uri.path += '/';
                                    uri.path += first_base_url_path;
                                }
                            }
                        }
                    }

                    // Set state to path state, and decrease pointer by 1.
                    state = turbo::WptState::PATH;
                }

                break;
            }
            case turbo::WptState::FILE_HOST: {
                std::string_view view = turbo::subview(url_data, input_position);

                size_t location = view.find_first_of("/\\?");
                std::string_view file_host_buffer(
                    view.data(),
                    (location != std::string_view::npos) ? location : view.size());

                if (turbo::is_windows_drive_letter(file_host_buffer)) {
                    state = turbo::WptState::PATH;
                } else if (file_host_buffer.empty()) {
                    // Set url's host to the empty string.
                    uri.host = "";
                    // Set state to path start state.
                    state = turbo::WptState::PATH_START;
                } else {
                    size_t consumed_bytes = file_host_buffer.size();
                    input_position += consumed_bytes;
                    // Let host be the result of host parsing buffer with url is not
                    // special.
                    std::string tmp_host;
                    uri.uri_error() = turbo::uri_wpt::parse_host(file_host_buffer, uri.is_special(), uri.host_type(), &tmp_host);
                    if (!uri.ok()) {
                        return;
                    }

                    uri.host = std::move(tmp_host);

                    // If host is "localhost", then set host to the empty string.
                    if (uri.host.has_value() && uri.host.value() == "localhost") {
                        uri.host = "";
                    }

                    // Set buffer to the empty string and state to path start state.
                    state = turbo::WptState::PATH_START;
                }

                break;
            }
            case turbo::WptState::FILE: {
                std::string_view file_view = turbo::subview(url_data, input_position);

                uri.set_protocol_as_file();
                // Set url's host to the empty string.
                uri.host = "";
                // If c is U+002F (/) or U+005C (\), then:
                if (input_position != input_size && (url_data[input_position] == '/' || url_data[input_position] == '\\')) {
                    // Set state to file slash state.
                    state = turbo::WptState::FILE_SLASH;
                } else if (base_uri.type == turbo::SchemaType::FILE) {
                    // Otherwise, if base is non-null and base's scheme is "file":
                    // Set url's host to base's host, url's path to a clone of base's
                    // path, and url's query to base's query.
                    uri.host = base_uri.host;
                    uri.path = base_uri.path;
                    uri.query = base_uri.query;
                    uri.has_opaque_path() = base_uri.has_opaque_path();

                    // If c is U+003F (?), then set url's query to the empty string and
                    // state to query state.
                    if (input_position != input_size && url_data[input_position] == '?') {
                        state = turbo::WptState::QUERY;
                    }
                    // Otherwise, if c is not the EOF code point:
                    else if (input_position != input_size) {
                        // Set url's query to null.
                        uri.clear_search();
                        // If the code point substring from pointer to the end of input does
                        // not start with a Windows drive letter, then shorten url's path.
                        if (!turbo::is_windows_drive_letter(file_view)) {
                            turbo::shorten_path(uri.path, uri.type);

                        }
                        // Otherwise:
                        else {
                            // Set url's path to an empty list.
                            uri.clear_pathname();
                            uri.has_opaque_path() = true;
                        }

                        // Set state to path state and decrease pointer by 1.
                        state = turbo::WptState::PATH;
                        break;
                    }
                } else {
                    // Otherwise, set state to path state, and decrease pointer by 1.
                    state = turbo::WptState::PATH;
                    break;
                }

                input_position++;
                break;
            }
            default:
                KUMO_UNREACHABLE();
            }
        }

        if (fragment.has_value()) {
            uri.update_unencoded_base_hash(*fragment);
        }
        return;
    }


    //////////////////////////////////////////////////////////////////////
    void WptParser::parse_url_no_base_impl(std::string_view user_input, WptUri& uri) {

        turbo::WptState state = turbo::WptState::SCHEME_START;
       
        // We refuse to parse URL strings that exceed 4GB. Such strings are almost
        // surely the result of a bug or are otherwise a security concern.
        if (user_input.size() > std::numeric_limits<uint32_t>::max()) {
            uri.uri_error() = { UriErrorCode::kUriOverflow, 0, "input overflow" };
        }
        if (!uri.ok()) {
            return;
        }
        std::string tmp_buffer;
        std::string_view internal_input;
        if (turbo::has_tabs_or_newline(user_input)) {
            tmp_buffer = user_input;
            // Optimization opportunity: Instead of copying and then pruning, we could
            // just directly build the string from user_input.
            turbo::remove_ascii_tab_or_newline(tmp_buffer);
            internal_input = tmp_buffer;
        } else {
            internal_input = user_input;
        }

        // Leading and trailing control characters are uncommon and easy to deal with
        // (no performance concern).
        std::string_view url_data = internal_input;
        turbo::trim_c0_whitespace(url_data);

        // Optimization opportunity. Most websites do not have fragment.
        std::optional<std::string_view> fragment = turbo::prune_hash(url_data);

        // Here url_data no longer has its fragment.
        // We are going to access the data from url_data (it is immutable).
        // At any given time, we are pointing at byte 'input_position' in url_data.
        // The input_position variable should range from 0 to input_size.
        // It is illegal to access url_data at input_size.
        size_t input_position = 0;
        const size_t input_size = url_data.size();
        // Keep running the following state machine by switching on state.
        // If after a run pointer points to the EOF code point, go to the next step.
        // Otherwise, increase pointer by 1 and continue with the state machine.
        // We never decrement input_position.
        while (input_position <= input_size) {
            switch (state) {
            case turbo::WptState::SCHEME_START: {
                // If c is an ASCII alpha, append c, lowercased, to buffer, and set
                // state to scheme state.
                if ((input_position != input_size) && turbo::ascii_isalpha(url_data[input_position])) {
                    state = turbo::WptState::SCHEME;
                    input_position++;
                } else {
                    // Otherwise, if state override is not given, set state to no scheme
                    // state and decrease pointer by 1.
                    state = turbo::WptState::NO_SCHEME;
                }
                break;
            }
            case turbo::WptState::SCHEME: {
                // If c is an ASCII alphanumeric, U+002B (+), U+002D (-), or U+002E (.),
                // append c, lowercased, to buffer.
                while ((input_position != input_size) && (turbo::is_valid_schema_alnum(url_data[input_position]))) {
                    input_position++;
                }
                // Otherwise, if c is U+003A (:), then:
                if ((input_position != input_size) && (url_data[input_position] == ':')) {
                    std::string scheme;
                    auto scheme_view = url_data.substr(0, input_position);
                    if (!turbo::uri_wpt::parse_scheme(
                            scheme_view, uri.type, &scheme)) {
                        return;
                    }
                    if (uri.type == turbo::SchemaType::NOT_SPECIAL) {
                        uri.set_scheme(std::move(scheme));
                    }

                    // If url's scheme is "file", then:
                    if (uri.type == turbo::SchemaType::FILE) {
                        // Set state to file state.
                        state = turbo::WptState::FILE;
                    }
                    // Otherwise, if url is special, set state to special authority
                    // slashes state.
                    else if (uri.is_special()) {
                        state = turbo::WptState::SPECIAL_AUTHORITY_SLASHES;
                    }
                    // Otherwise, if remaining starts with an U+002F (/), set state to
                    // path or authority state and increase pointer by 1.
                    else if (input_position + 1 < input_size && url_data[input_position + 1] == '/') {
                        state = turbo::WptState::PATH_OR_AUTHORITY;
                        input_position++;
                    }
                    // Otherwise, set url's path to the empty string and set state to
                    // opaque path state.
                    else {
                        state = turbo::WptState::OPAQUE_PATH;
                    }
                }
                // Otherwise, if state override is not given, set buffer to the empty
                // string, state to no scheme state, and start over (from the first code
                // point in input).
                else {
                    state = turbo::WptState::NO_SCHEME;
                    input_position = 0;
                    break;
                }
                input_position++;
                break;
            }
            case turbo::WptState::NO_SCHEME: {
                // If base is null, or base has an opaque path and c is not U+0023 (#),
                // validation error, return failure.
                uri.uri_error() = { UriErrorCode::kUriNotComplete, 0, "no schema" };
                return;
                break;
            }
            case turbo::WptState::AUTHORITY: {
                // most URLs have no @. Having no @ tells us that we don't have to worry
                // about AUTHORITY. Of course, we could have @ and still not have to
                // worry about AUTHORITY.
                // TODO: Instead of just collecting a bool, collect the location of the
                // '@' and do something useful with it.
                // TODO: We could do various processing early on, using a single pass
                // over the string to collect information about it, e.g., telling us
                // whether there is a @ and if so, where (or how many).
                const bool contains_ampersand = (url_data.find('@', input_position) != std::string_view::npos);

                if (!contains_ampersand) {
                    state = turbo::WptState::HOST;
                    break;
                }
                bool at_sign_seen { false };
                bool password_token_seen { false };
                /**
                 * We expect something of the sort...
                 * https://user:pass@example.com:1234/foo/bar?baz#quux
                 * --------^
                 */
                do {
                    std::string_view view = turbo::subview(url_data, input_position);
                    // The delimiters are @, /, ? \\.
                    size_t location = uri.is_special() ? turbo::find_authority_delimiter_special(view)
                                                       : turbo::find_authority_delimiter(view);
                    std::string_view authority_view(view.data(), location);
                    size_t end_of_authority = input_position + authority_view.size();
                    // If c is U+0040 (@), then:
                    if ((end_of_authority != input_size) && (url_data[end_of_authority] == '@')) {
                        // If atSignSeen is true, then prepend "%40" to buffer.
                        if (at_sign_seen) {
                            if (password_token_seen) {
                                uri.password += "%40";
                            } else {
                                uri.username += "%40";
                            }
                        }

                        at_sign_seen = true;

                        if (!password_token_seen) {
                            size_t password_token_location = authority_view.find(':');
                            password_token_seen = password_token_location != std::string_view::npos;

                            if (!password_token_seen) {
                                uri.username += turbo::percent_encode(
                                    authority_view,
                                    turbo::uri_charsets::USERINFO_PERCENT_ENCODE);
                            } else {
                                uri.username += turbo::percent_encode(
                                    authority_view.substr(0, password_token_location),
                                    turbo::uri_charsets::USERINFO_PERCENT_ENCODE);
                                uri.password += turbo::percent_encode(
                                    authority_view.substr(password_token_location + 1),
                                    turbo::uri_charsets::USERINFO_PERCENT_ENCODE);
                            }
                        } else {
                            uri.password += turbo::percent_encode(
                                authority_view, turbo::uri_charsets::USERINFO_PERCENT_ENCODE);
                        }
                    }
                    // Otherwise, if one of the following is true:
                    // - c is the EOF code point, U+002F (/), U+003F (?), or U+0023 (#)
                    // - url is special and c is U+005C (\)
                    else if (end_of_authority == input_size || url_data[end_of_authority] == '/' || url_data[end_of_authority] == '?' || (uri.is_special() && url_data[end_of_authority] == '\\')) {
                        // If atSignSeen is true and authority_view is the empty string,
                        // validation error, return failure.
                        if (at_sign_seen && authority_view.empty()) {
                            uri.uri_error() = { UriErrorCode::kUriNotComplete, 0, "no authority" };
                            return;
                        }
                        state = turbo::WptState::HOST;
                        break;
                    }
                    if (end_of_authority == input_size) {
                        if (fragment.has_value()) {
                            uri.update_unencoded_base_hash(*fragment);
                        }
                        return;
                    }
                    input_position = end_of_authority + 1;
                } while (true);

                break;
            }
            case turbo::WptState::PATH_OR_AUTHORITY: {

                // If c is U+002F (/), then set state to authority state.
                if ((input_position != input_size) && (url_data[input_position] == '/')) {
                    state = turbo::WptState::AUTHORITY;
                    input_position++;
                } else {
                    // Otherwise, set state to path state, and decrease pointer by 1.
                    state = turbo::WptState::PATH;
                }

                break;
            }
            case turbo::WptState::SPECIAL_AUTHORITY_SLASHES: {

                // If c is U+002F (/) and remaining starts with U+002F (/),
                // then set state to special authority ignore slashes state and increase
                // pointer by 1.
                std::string_view view = turbo::subview(url_data, input_position);
                if (turbo::starts_with(view, "//")) {
                    input_position += 2;
                }

                [[fallthrough]];
            }
            case turbo::WptState::SPECIAL_AUTHORITY_IGNORE_SLASHES: {

                // If c is neither U+002F (/) nor U+005C (\), then set state to
                // authority state and decrease pointer by 1.
                while ((input_position != input_size) && ((url_data[input_position] == '/') || (url_data[input_position] == '\\'))) {
                    input_position++;
                }
                state = turbo::WptState::AUTHORITY;

                break;
            }
            case turbo::WptState::QUERY: {
                // Let queryPercentEncodeSet be the special-query percent-encode set
                // if url is special; otherwise the query percent-encode set.
                const uint8_t* query_percent_encode_set = uri.is_special()
                    ? turbo::uri_charsets::SPECIAL_QUERY_PERCENT_ENCODE
                    : turbo::uri_charsets::QUERY_PERCENT_ENCODE;

                // Percent-encode after encoding, with encoding, buffer, and
                // queryPercentEncodeSet, and append the result to url's query.
                uri.update_base_search(turbo::subview(url_data, input_position),
                    query_percent_encode_set);
                if (fragment.has_value()) {
                    uri.update_unencoded_base_hash(*fragment);
                }
                return;
            }
            case turbo::WptState::HOST: {

                std::string_view host_view = turbo::subview(url_data, input_position);
                auto [location, found_colon] = uri_wpt::get_host_delimiter_location(uri.is_special(), host_view);
                input_position = (location != std::string_view::npos)
                    ? input_position + location
                    : input_size;
                // Otherwise, if c is U+003A (:) and insideBrackets is false, then:
                // Note: the 'found_colon' value is true if and only if a colon was
                // encountered while not inside brackets.
                if (found_colon) {
                    // If buffer is the empty string, validation error, return failure.
                    // Let host be the result of host parsing buffer with url is not
                    // special.
                    std::string tmp_host;
                    uri.uri_error() = turbo::uri_wpt::parse_host(host_view, uri.is_special(), uri.host_type(), &tmp_host);
                    if (!uri.ok()) {
                        return;
                    }
                    uri.host = std::move(tmp_host);
                    // Set url's host to host, buffer to the empty string, and state to
                    // port state.
                    state = turbo::WptState::PORT;
                    input_position++;
                } else {
                    // Otherwise, if one of the following is true:
                    // - c is the EOF code point, U+002F (/), U+003F (?), or U+0023 (#)
                    // - url is special and c is U+005C (\)
                    // The get_host_delimiter_location function either brings us to
                    // the colon outside of the bracket, or to one of those characters.
                    // If url is special and host_view is the empty string, validation
                    // error, return failure.
                    if (uri.is_special() && host_view.empty()) {
                        uri.uri_error() = { UriErrorCode::kUriNotComplete, 0, "no host" };
                        ;
                        return;
                    }
                    // Let host be the result of host parsing host_view with url is not
                    // special.
                    if (host_view.empty()) {
                        uri.update_base_hostname("");
                    } else {
                        std::string tmp_host;
                        uri.uri_error() = turbo::uri_wpt::parse_host(host_view, uri.is_special(), uri.host_type(), &tmp_host);
                        if (!uri.ok()) {
                            return;
                        }
                        uri.host = tmp_host;
                    }

                    // Set url's host to host, and state to path start state.
                    state = turbo::WptState::PATH_START;
                }

                break;
            }
            case turbo::WptState::OPAQUE_PATH: {
                std::string_view view = turbo::subview(url_data, input_position);
                // If c is U+003F (?), then set url's query to the empty string and
                // state to query state.
                size_t location = view.find('?');
                if (location != std::string_view::npos) {
                    view.remove_suffix(view.size() - location);
                    state = turbo::WptState::QUERY;
                    input_position += location + 1;
                } else {
                    input_position = input_size + 1;
                }
                uri.has_opaque_path() = true;
                // This is a really unlikely scenario in real world. We should not seek
                // to optimize it.
                uri.update_base_pathname(turbo::percent_encode(
                    view, turbo::uri_charsets::C0_CONTROL_PERCENT_ENCODE));
                break;
            }
            case turbo::WptState::PORT: {
                std::string_view port_view = turbo::subview(url_data, input_position);
                std::optional<uint16_t> tmp_port;
                uri.uri_error() = uri_wpt::parse_port(port_view, uri.is_special(), uri.type, true, tmp_port);
                input_position += uri.uri_error().error_pos;
                if (!uri.ok()) {
                    return;
                }
                uri.port = tmp_port;
                state = turbo::WptState::PATH_START;
                [[fallthrough]];
            }
            case turbo::WptState::PATH_START: {

                // If url is special, then:
                if (uri.is_special()) {
                    // Set state to path state.
                    state = turbo::WptState::PATH;

                    // Optimization: Avoiding going into PATH state improves the
                    // performance of urls ending with /.
                    if (input_position == input_size) {
                        uri.update_base_pathname("/");
                        if (fragment.has_value()) {
                            uri.update_unencoded_base_hash(*fragment);
                        }
                        return;
                    }
                    // If c is neither U+002F (/) nor U+005C (\), then decrease pointer
                    // by 1. We know that (input_position == input_size) is impossible
                    // here, because of the previous if-check.
                    if ((url_data[input_position] != '/') && (url_data[input_position] != '\\')) {
                        break;
                    }
                }
                // Otherwise, if state override is not given and c is U+003F (?),
                // set url's query to the empty string and state to query state.
                else if ((input_position != input_size) && (url_data[input_position] == '?')) {
                    state = turbo::WptState::QUERY;
                }
                // Otherwise, if c is not the EOF code point:
                else if (input_position != input_size) {
                    // Set state to path state.
                    state = turbo::WptState::PATH;

                    // If c is not U+002F (/), then decrease pointer by 1.
                    if (url_data[input_position] != '/') {
                        break;
                    }
                }

                input_position++;
                break;
            }
            case turbo::WptState::PATH: {
                std::string_view view = turbo::subview(url_data, input_position);

                // Most time, we do not need percent encoding.
                // Furthermore, we can immediately locate the '?'.
                size_t locofquestionmark = view.find('?');
                if (locofquestionmark != std::string_view::npos) {
                    state = turbo::WptState::QUERY;
                    view.remove_suffix(view.size() - locofquestionmark);
                    input_position += locofquestionmark + 1;
                } else {
                    input_position = input_size + 1;
                }

                turbo::parse_prepared_path(view, uri.type, uri.path);
                break;
            }
            case turbo::WptState::FILE_SLASH: {

                // If c is U+002F (/) or U+005C (\), then:
                if ((input_position != input_size) && (url_data[input_position] == '/' || url_data[input_position] == '\\')) {
                    // Set state to file host state.
                    state = turbo::WptState::FILE_HOST;
                    input_position++;
                } else {
                    // Set state to path state, and decrease pointer by 1.
                    state = turbo::WptState::PATH;
                }

                break;
            }
            case turbo::WptState::FILE_HOST: {
                std::string_view view = turbo::subview(url_data, input_position);

                size_t location = view.find_first_of("/\\?");
                std::string_view file_host_buffer(
                    view.data(),
                    (location != std::string_view::npos) ? location : view.size());

                if (turbo::is_windows_drive_letter(file_host_buffer)) {
                    state = turbo::WptState::PATH;
                } else if (file_host_buffer.empty()) {
                    // Set url's host to the empty string.
                    uri.host = "";
                    // Set state to path start state.
                    state = turbo::WptState::PATH_START;
                } else {
                    size_t consumed_bytes = file_host_buffer.size();
                    input_position += consumed_bytes;
                    // Let host be the result of host parsing buffer with url is not
                    // special.
                    std::string tmp_host;
                    uri.uri_error() = turbo::uri_wpt::parse_host(file_host_buffer, uri.is_special(), uri.host_type(), &tmp_host);
                    if (!uri.ok()) {
                        return;
                    }

                    uri.host = std::move(tmp_host);

                    // If host is "localhost", then set host to the empty string.
                    if (uri.host.has_value() && uri.host.value() == "localhost") {
                        uri.host = "";
                    }

                    // Set buffer to the empty string and state to path start state.
                    state = turbo::WptState::PATH_START;
                }

                break;
            }
            case turbo::WptState::FILE: {
                std::string_view file_view = turbo::subview(url_data, input_position);

                uri.set_protocol_as_file();
                // Set url's host to the empty string.
                uri.host = "";
                // If c is U+002F (/) or U+005C (\), then:
                if (input_position != input_size && (url_data[input_position] == '/' || url_data[input_position] == '\\')) {
                    // Set state to file slash state.
                    state = turbo::WptState::FILE_SLASH;
                } else {
                    // Otherwise, set state to path state, and decrease pointer by 1.
                    state = turbo::WptState::PATH;
                    break;
                }

                input_position++;
                break;
            }
            default:
                KUMO_UNREACHABLE();
            }
        }

        if (fragment.has_value()) {
            uri.update_unencoded_base_hash(*fragment);
        }
        return;
    }

    WptUri parse_wpt_uri(std::string_view user_input,
        const WptUri* base_url) {
       WptUri ret;
       if (base_url) {
           WptParser::parse_url_with_base_impl(user_input, *base_url, ret);
       } else {
           WptParser::parse_url_no_base_impl(user_input, ret);
       }
        return ret;
    }

    bool parse_wpt_uri(std::string_view user_input,WptUri&uri,
       const WptUri* base_url) {
       if (base_url) {
           WptParser::parse_url_with_base_impl(user_input, *base_url, uri);
       } else {
           WptParser::parse_url_no_base_impl(user_input, uri);
       }
       return uri.ok();
   }

    std::string href_from_file(std::string_view input) {
        // This is going to be much faster than constructing a URL.
        std::string tmp_buffer;
        std::string_view internal_input;
        if (turbo::has_tabs_or_newline(input)) {
            tmp_buffer = input;
            turbo::remove_ascii_tab_or_newline(tmp_buffer);
            internal_input = tmp_buffer;
        } else {
            internal_input = input;
        }
        std::string path;
        if (internal_input.empty()) {
            path = "/";
        } else if ((internal_input[0] == '/') || (internal_input[0] == '\\')) {
            turbo::parse_prepared_path(internal_input.substr(1),
                turbo::SchemaType::FILE, path);
        } else {
            turbo::parse_prepared_path(internal_input, turbo::SchemaType::FILE, path);
        }
        return "file://" + path;
    }
} // namespace turbo
