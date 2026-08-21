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

#include <string>
#include <string_view>
#include <optional>

namespace turbo {

    // url_encode()
    //
    // Escapes a string so it can be safely used as a value in a URL component by
    // replacing all characters that are not "unreserved characters" with
    // percent-escapes. See https://tools.ietf.org/html/rfc3986
    //
    // Usage note: URLs use "reserved characters" (like ?, &, =, /) as structural
    // syntax. This function escapes these syntax characters. The correct use of
    // this function is to clean individual URL components *before* assembling them
    // into the final URL structure. Do not run it on a fully constructed URL, as
    // this will turn structural delimiters into URL component data.
    //
    // Example (encoding "gift for mom & dad" as a URL query parameter):
    //
    //   std::string url = turbo::str_sprintf("https://www.google.com/search?q=%s",
    //                                     turbo::url_encode("gift for mom & dad"));
    //   assert(url ==
    //     "https://www.google.com/search?q=gift%20for%20mom%20%26%20dad");
    [[nodiscard]] std::string url_encode(std::string_view input);

    // url_decode()
    //
    // Performs the inverse transformation of url_encode(), converting each
    // percent-encoded sequence of the form "%AB" into the character with the
    // hexadecimal value 0xAB. It returns `std::nullopt` if any % is not followed by
    // two hexadecimal digits.
    //
    // url_decode() is identical to url_decode_plus() except that it does not
    // unescape '+' to ' '.
    [[nodiscard]] std::optional<std::string> url_decode(std::string_view input);

    // url_encode_plus()
    //
    // Escapes a string so it can be safely used as a value for
    // application/x-www-form-urlencoded (HTML form submissions).
    //
    // Historically web browsers have also used this form of escaping for query
    // parameters.
    //
    // url_encode_plus() differs from url_encode() in that space (' ') is encoded to
    // plus ("+") instead of "%20". According to the URI specification (RFC 3986),
    // the correct way to escape a space anywhere in a URL (including the query
    // string) is "%20". Using "%20" in a query parameter will work universally.
    //
    // Some strict URL parsers (especially outside of web browsers/web servers)
    // follow RFC 3986 strictly and will treat a literal '+' in the query string as
    // a literal plus sign, rather than decoding it to a space.
    //
    // Recommendation: Use url_encode_plus() only if you are specifically implementing
    // or interacting with a system that strictly expects
    // "application/x-www-form-urlencoded" formatting. For general URL construction,
    // url_encode() is the correct and safest choice.
    //
    // Example (encoding "gift for mom & dad" as a URL query parameter):
    //
    //   std::string url = turbo::str_sprintf("https://www.google.com/search?q=%s",
    //                                     turbo::url_encode_plus(
    //                                         "gift for mom & dad"));
    //   assert(url == "https://www.google.com/search?q=gift+for+mom+%26+dad");
    [[nodiscard]] std::string url_encode_plus(std::string_view input);

    // url_decode_plus()
    //
    // Performs the inverse transformation of url_encode_plus(). It returns
    // `std::nullopt` if any % is not followed by two hexadecimal digits.
    [[nodiscard]] std::optional<std::string> url_decode_plus(
        std::string_view input);
}  // namespace turbo
