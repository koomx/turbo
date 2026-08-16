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

#include <turbo/encoding/url.h>
#include <turbo/strings/charset.h>
#include <turbo/base/internal/raw_logging.h>
#include <turbo/base/resize_and_overwrite.h>
#include <turbo/strings/ascii.h>
#include <turbo/strings/escaping.h>

namespace turbo {

        static std::string UrlEscapeInternal(std::string_view input,
        const bool escape_space_to_plus) {
        // Unreserved characters from RFC 3986.
        // See https://www.rfc-editor.org/info/rfc3986/#section-2.3.
        static constexpr turbo::CharSet kRfc3986Unreserved = turbo::CharSet::AsciiAlphanumerics() | turbo::CharSet("-._~");

        std::string output;
        std::string_view::iterator in = input.begin();

        // Fast path for when we don't need to do any escaping.
        while (in < input.end() && kRfc3986Unreserved.contains(*in)) {
            ++in;
        }

        std::size_t initial_portion = static_cast<std::size_t>(std::distance(input.begin(), in));

        if (initial_portion == input.size()) {
            return std::string(input);
        }

        // We need a buffer with enough space to store at most the initial portion
        // plus 3 bytes for each remaining character since escapes use 3 characters.
        TURBO_INTERNAL_CHECK(
            (input.size() - initial_portion) <= (std::numeric_limits<size_t>::max() - initial_portion) / 3,
            "url_encode() overflow");
        StringResizeAndOverwrite(
            output, initial_portion + 3 * (input.size() - initial_portion),
            [&](char* buf, size_t) {
                char* out = buf;

                // Copy the initial portion that did not need escaping.
                out = std::copy(input.begin(), in, out);

                // Handle the rest of the string.
                while (in < input.end()) {
                    char c = *in++;
                    if (kRfc3986Unreserved.contains(c)) {
                        *out++ = c;
                    } else if (escape_space_to_plus && c == ' ') {
                        *out++ = '+';
                    } else {
                        *out++ = '%';
                        *out++ = static_cast<char>(
                            int_to_hex_digit((static_cast<unsigned char>(c) >> 4) & 0xf));
                        *out++ = static_cast<char>(
                            int_to_hex_digit(static_cast<unsigned char>(c) & 0xf));
                    }
                }
                return static_cast<size_t>(std::distance(buf, out));
            });

        return output;
    }


    static std::optional<std::string> UrlUnescapeInternal(
        std::string_view input, const bool unescape_plus_to_space) {
        std::string output;

        // Fast path for when we don't need to do any unescaping.
        // This case includes empty input, which allows us to return 0 from the
        // lambda below to signal the error case.
        size_t in = unescape_plus_to_space ? input.find_first_of("%+") : input.find('%');
        if (in == input.npos) {
            return std::string(input);
        }

        StringResizeAndOverwrite(output, input.size(), [&](char* buf, size_t) {
            char* out = buf;

            // Copy the initial portion that did not need unescaping.
            out = std::copy_n(input.data(), in, out);

            // Handle the rest of the string.
            while (in < input.size()) {
                char c = input[in++];
                if (unescape_plus_to_space && c == '+') {
                    *out++ = ' ';
                } else if (c == '%') {
                    if (in + 1 >= input.size() || !turbo::ascii_isxdigit(static_cast<unsigned char>(input[in])) || !turbo::ascii_isxdigit(static_cast<unsigned char>(input[in + 1]))) {
                        return size_t { 0 }; // Error.
                    }
                    int x = static_cast<int>(hex_digit_to_int(input[in++])) << 4;
                    x += static_cast<int>(hex_digit_to_int(input[in++]));
                    *out++ = static_cast<char>(x);
                } else {
                    *out++ = c;
                }
            }
            return static_cast<size_t>(std::distance(buf, out));
        });

        if (output.empty()) {
            // Empty output is only valid if the input was empty, and that case is
            // handled above.
            return std::nullopt;
        }

        return output;
    }

    std::string url_encode(std::string_view input) {
        constexpr bool kEscapeSpaceToPlus = false;
        return UrlEscapeInternal(input, kEscapeSpaceToPlus);
    }

    std::string url_encode_plus(std::string_view input) {
        constexpr bool kEscapeSpaceToPlus = true;
        return UrlEscapeInternal(input, kEscapeSpaceToPlus);
    }



    std::optional<std::string> url_decode(std::string_view input) {
        constexpr bool kUnescapePlusToSpace = false;
        return UrlUnescapeInternal(input, kUnescapePlusToSpace);
    }

    std::optional<std::string> url_decode_plus(std::string_view input) {
        constexpr bool kUnescapePlusToSpace = true;
        return UrlUnescapeInternal(input, kUnescapePlusToSpace);
    }


}  // namespace turbo
