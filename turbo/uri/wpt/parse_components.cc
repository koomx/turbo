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

#include <turbo/uri/wpt/parse_components.h>
#include <turbo/uri/checkers.h>
#include <turbo/uri/utility.h>
#include <turbo/uri/ip.h>
#include <charconv>
#include <turbo/strings/str_cat.h>
namespace turbo::uri_wpt {

    UriError check_opaque_host(std::string_view input) {
        auto it = std::find_if(input.begin(), input.end(), turbo::is_forbidden_host_code_point);
        if (it != input.end()) {
            return {UriErrorCode::kUriForbiddenHostCodePoint,
                    static_cast<uint32_t>(it - input.begin()), ""};
        }
        return {};
    }

    std::string encode_opaque_host(std::string_view input) {
        // Return the result of running UTF-8 percent-encode on input using the C0
        // control percent-encode set.
        auto ret = turbo::percent_encode(
            input, turbo::uri_charsets::C0_CONTROL_PERCENT_ENCODE);
        return ret;
    }



    std::string parse_path(std::string_view input, bool is_special, bool have_host, turbo::SchemaType type) {
        std::string tmp_buffer;
        std::string_view internal_input;
        std::string path;
        if (turbo::has_tabs_or_newline(input)) {
            tmp_buffer = input;
            // Optimization opportunity: Instead of copying and then pruning, we could
            // just directly build the string from user_input.
            turbo::remove_ascii_tab_or_newline(tmp_buffer);
            internal_input = tmp_buffer;
        } else {
            internal_input = input;
        }

        // If url is special, then:
        if (is_special) {
            if (internal_input.empty()) {
                path = "/";
            } else if ((internal_input[0] == '/') || (internal_input[0] == '\\')) {
                turbo::parse_prepared_path(internal_input.substr(1), type, path);
            } else {
                turbo::parse_prepared_path(internal_input, type, path);
            }
        } else if (!internal_input.empty()) {
            if (internal_input[0] == '/') {
                turbo::parse_prepared_path(internal_input.substr(1), type, path);
            } else {
                turbo::parse_prepared_path(internal_input, type, path);
            }
        } else {
            if (!have_host) {
                path = "/";
            }
        }
        return path;
    }

    UriError parse_ipv4(std::string_view input, std::string *result) {
        if (input.back() == '.') {
            input.remove_suffix(1);
        }
        size_t digit_count { 0 };
        int pure_decimal_count = 0; // entries that are decimal
        std::string_view original_input = input; // we might use this if pure_decimal_count == 4.
        uint64_t ipv4 { 0 };
        // we could unroll for better performance?
        for (; (digit_count < 4) && !(input.empty()); digit_count++) {
            uint32_t
                segment_result { }; // If any number exceeds 32 bits, we have an error.
            bool is_hex = turbo::has_hex_prefix(input);
            if (is_hex && ((input.length() == 2) || ((input.length() > 2) && (input[2] == '.')))) {
                // special case
                segment_result = 0;
                input.remove_prefix(2);
            } else {
                std::from_chars_result r { };
                if (is_hex) {
                    r = std::from_chars(input.data() + 2, input.data() + input.size(),
                        segment_result, 16);
                } else if ((input.length() >= 2) && input[0] == '0' && turbo::ascii_isdigit(input[1])) {
                    r = std::from_chars(input.data() + 1, input.data() + input.size(),
                        segment_result, 8);
                } else {
                    pure_decimal_count++;
                    r = std::from_chars(input.data(), input.data() + input.size(),
                        segment_result, 10);
                }
                if (r.ec != std::errc()) {
                    return {UriErrorCode::kUriInvalidArgs,
                       0,
                    "r.ec != std::errc()"
                           };
                }
                input.remove_prefix(r.ptr - input.data());
            }
            if (input.empty()) {
                // We have the last value.
                // At this stage, ipv4 contains digit_count*8 bits.
                // So we have 32-digit_count*8 bits left.
                if (segment_result >= (uint64_t(1) << (32 - digit_count * 8))) {
                    return {UriErrorCode::kUriInvalidArgs,
                       0,
                    "segment_result >= (uint64_t(1) << (32 - digit_count * 8))"
                           };
                }
                ipv4 <<= (32 - digit_count * 8);
                ipv4 |= segment_result;
                goto final;
            } else {
                // There is more, so that the value must no be larger than 255
                // and we must have a '.'.
                if ((segment_result > 255) || (input[0] != '.')) {
                    return {UriErrorCode::kUriInvalidArgs,
                       0,
                    "(segment_result > 255) || (input[0] != '.')"
                           };
                }
                ipv4 <<= 8;
                ipv4 |= segment_result;
                input.remove_prefix(1); // remove '.'
            }
        }
        if ((digit_count != 4) || (!input.empty())) {

            return {UriErrorCode::kUriInvalidArgs,
                       0,
                    "(digit_count != 4) || (!input.empty())"
                           };
        }
    final:
        // We could also check r.ptr to see where the parsing ended.
        if (result) {
            if (pure_decimal_count == 4) {
               *result = original_input; // The original input was already all decimal and we
                // validated it.
            } else {
                *result = turbo::ipv4_to_string(ipv4); // We have to reserialize the address.
            }
        }
        return{};
    }

    UriError parse_ipv6(std::string_view input, std::string *result) {
        if (input.empty()) {
            return {UriErrorCode::kUriNotComplete, 0, "inout empty"};
        }
        // Let address be a new IPv6 address whose IPv6 pieces are all 0.
        std::array<uint16_t, 8> address { };

        // Let pieceIndex be 0.
        int piece_index = 0;

        // Let compress be null.
        std::optional<int> compress { };

        // Let pointer be a pointer for input.
        std::string_view::iterator pointer = input.begin();

        // If c is U+003A (:), then:
        if (input[0] == ':') {
            // If remaining does not start with U+003A (:), validation error, return
            // failure.
            if (input.size() == 1 || input[1] != ':') {
                return {UriErrorCode::kUriInvalidArgs, 1, "parse_ipv6 starts with : but the rest does not start with :"};
            }

            // Increase pointer by 2.
            pointer += 2;

            // Increase pieceIndex by 1 and then set compress to pieceIndex.
            compress = ++piece_index;
        }

        // While c is not the EOF code point:
        while (pointer != input.end()) {
            // If pieceIndex is 8, validation error, return failure.
            if (piece_index == 8) {
                return {UriErrorCode::kUriOverflow, static_cast<uint32_t>(pointer - input.begin()), "parse_ipv6 piece_index == 8"};
            }

            // If c is U+003A (:), then:
            if (*pointer == ':') {
                // If compress is non-null, validation error, return failure.
                if (compress.has_value()) {
                    return {UriErrorCode::kUriInvalidArgs, 8, "parse_ipv6 compress is non-null"};
                }

                // Increase pointer and pieceIndex by 1, set compress to pieceIndex, and
                // then continue.
                pointer++;
                compress = ++piece_index;
                continue;
            }

            // Let value and length be 0.
            uint16_t value = 0, length = 0;

            // While length is less than 4 and c is an ASCII hex digit,
            // set value to value times 0x10 + c interpreted as hexadecimal number, and
            // increase pointer and length by 1.
            while (length < 4 && pointer != input.end() && turbo::ascii_isxdigit(*pointer)) {
                // https://stackoverflow.com/questions/39060852/why-does-the-addition-of-two-shorts-return-an-int
                value = uint16_t(value * 0x10 + turbo::convert_hex_to_binary(*pointer));
                pointer++;
                length++;
            }

            // If c is U+002E (.), then:
            if (pointer != input.end() && *pointer == '.') {
                // If length is 0, validation error, return failure.
                if (length == 0) {
                    return {UriErrorCode::kUriOverflow,
                        static_cast<uint32_t>(pointer - input.begin()),
                        "parse_ipv6 length is 0"
                    };
                }

                // Decrease pointer by length.
                pointer -= length;

                // If pieceIndex is greater than 6, validation error, return failure.
                if (piece_index > 6) {
                    return {UriErrorCode::kUriOverflow,
                        static_cast<uint32_t>(pointer - input.begin()),
                        "parse_ipv6 piece_index > 6"
                    };
                }

                // Let numbersSeen be 0.
                int numbers_seen = 0;

                // While c is not the EOF code point:
                while (pointer != input.end()) {
                    // Let ipv4Piece be null.
                    std::optional<uint16_t> ipv4_piece { };

                    // If numbersSeen is greater than 0, then:
                    if (numbers_seen > 0) {
                        // If c is a U+002E (.) and numbersSeen is less than 4, then increase
                        // pointer by 1.
                        if (*pointer == '.' && numbers_seen < 4) {
                            pointer++;
                        } else {
                            // Otherwise, validation error, return failure.
                            return {UriErrorCode::kUriOverflow,
                        static_cast<uint32_t>(pointer - input.begin()),
                                "parse_ipv6 Otherwise, validation error, return failure"
                            };
                        }
                    }

                    // If c is not an ASCII digit, validation error, return failure.
                    if (pointer == input.end() || !turbo::ascii_isdigit(*pointer)) {
                        return {UriErrorCode::kUriOverflow,
                        static_cast<uint32_t>(pointer - input.begin()),
                            "parse_ipv6 If c is not an ASCII digit, validation error, return failure"
                            };
                    }

                    // While c is an ASCII digit:
                    while (pointer != input.end() && turbo::ascii_isdigit(*pointer)) {
                        // Let number be c interpreted as decimal number.
                        int number = *pointer - '0';

                        // If ipv4Piece is null, then set ipv4Piece to number.
                        if (!ipv4_piece.has_value()) {
                            ipv4_piece = number;
                        } else if (ipv4_piece == 0) {
                            return {UriErrorCode::kUriInvalidArgs,
                        static_cast<uint32_t>(pointer - input.begin()),
                                "parse_ipv6 if ipv4Piece is 0, validation error"
                            };
                        }
                        // Otherwise, set ipv4Piece to ipv4Piece times 10 + number.
                        else {
                            ipv4_piece = *ipv4_piece * 10 + number;
                        }

                        // If ipv4Piece is greater than 255, validation error, return failure.
                        if (ipv4_piece > 255) {
                            return {UriErrorCode::kUriInvalidArgs,
                        static_cast<uint32_t>(pointer - input.begin()),
                                "parse_ipv6 ipv4_piece > 255"
                            };
                        }

                        // Increase pointer by 1.
                        pointer++;
                    }

                    // Set address[pieceIndex] to address[pieceIndex] times 0x100 +
                    // ipv4Piece.
                    // https://stackoverflow.com/questions/39060852/why-does-the-addition-of-two-shorts-return-an-int
                    address[piece_index] = uint16_t(address[piece_index] * 0x100 + *ipv4_piece);

                    // Increase numbersSeen by 1.
                    numbers_seen++;

                    // If numbersSeen is 2 or 4, then increase pieceIndex by 1.
                    if (numbers_seen == 2 || numbers_seen == 4) {
                        piece_index++;
                    }
                }

                // If numbersSeen is not 4, validation error, return failure.
                if (numbers_seen != 4) {
                    return {UriErrorCode::kUriInvalidArgs,
                       static_cast<uint32_t>(pointer - input.begin()),
                               turbo::str_cat("numbers_seen != 4, ", numbers_seen)
                           };
                }

                // Break.
                break;
            }
            // Otherwise, if c is U+003A (:):
            else if ((pointer != input.end()) && (*pointer == ':')) {
                // Increase pointer by 1.
                pointer++;

                // If c is the EOF code point, validation error, return failure.
                if (pointer == input.end()) {
                    return {UriErrorCode::kUriInvalidArgs,
                       static_cast<uint32_t>(pointer - input.begin()),
                        "parse_ipv6 If c is the EOF code point, validation error, return failure"
                           };
                }
            }
            // Otherwise, if c is not the EOF code point, validation error, return
            // failure.
            else if (pointer != input.end()) {
                return {UriErrorCode::kUriInvalidArgs,
                       static_cast<uint32_t>(pointer - input.begin()),
                    "parse_ipv6 Otherwise, if c is not the EOF code point, validation error, return failure"
                           };
            }

            // Set address[pieceIndex] to value.
            address[piece_index] = value;

            // Increase pieceIndex by 1.
            piece_index++;
        }

        // If compress is non-null, then:
        if (compress.has_value()) {
            // Let swaps be pieceIndex - compress.
            int swaps = piece_index - *compress;

            // Set pieceIndex to 7.
            piece_index = 7;

            // While pieceIndex is not 0 and swaps is greater than 0,
            // swap address[pieceIndex] with address[compress + swaps - 1], and then
            // decrease both pieceIndex and swaps by 1.
            while (piece_index != 0 && swaps > 0) {
                std::swap(address[piece_index], address[*compress + swaps - 1]);
                piece_index--;
                swaps--;
            }
        } else if (piece_index != 8) {
            // Otherwise, if compress is null and pieceIndex is not 8, validation error,
            // return failure.
            return {UriErrorCode::kUriInvalidArgs,
                       static_cast<uint32_t>(pointer - input.begin()),
                "parse_ipv6 if compress is null and pieceIndex is not 8, validation error, return failure"
                           };
        }
        if (result) {
            *result = turbo::ipv6_to_string(address);
        }

        return {};
    }


    UriError parse_host(std::string_view input, bool is_special, UriHostType& ht, std::string * result) {
        ht = UriHostType::DEFAULT;
        if (input.empty()) {
            return {UriErrorCode::kUriNotComplete, 0, "inout empty"};
        } // technically unnecessary.
        // If input starts with U+005B ([), then:
        if (input[0] == '[') {
            // If input does not end with U+005D (]), validation error, return failure.
            if (input.back() != ']') {
                return {UriErrorCode::kUriNotComplete, static_cast<uint32_t>(input.size() -1), "input.back() != ']'"};
            }

            // Return the result of IPv6 parsing input with its leading U+005B ([) and
            // trailing U+005D (]) removed.
            input.remove_prefix(1);
            input.remove_suffix(1);
            auto r =  parse_ipv6(input, result);
            if (r.ok()) {
                ht = UriHostType::IPV6;
            }
            return r;
        }

        // If isNotSpecial is true, then return the result of opaque-host parsing
        // input.
        if (!is_special) {
            UriError opaque = turbo::uri_wpt::check_opaque_host(input);
            if (!opaque.ok()) {
                return opaque;
            }
            if (result) {
                *result = turbo::uri_wpt::encode_opaque_host(input);
            }
            return {};
        }
        // Let domain be the result of running UTF-8 decode without BOM on the
        // percent-decoding of input. Let asciiDomain be the result of running domain
        // to ASCII with domain and false. The most common case is an ASCII input, in
        // which case we do not need to call the expensive 'to_ascii' if a few
        // conditions are met: no '%' and no 'xn-' subsequence.
        std::string buffer = std::string(input);
        // This next function checks that the result is ascii, but we are going to
        // to check anyhow with is_forbidden.
        // bool is_ascii =
        turbo::to_lower_ascii(buffer.data(), buffer.size());
        bool is_forbidden = turbo::contains_forbidden_domain_code_point(
            buffer.data(), buffer.size());
        if (is_forbidden == 0 && buffer.find("xn-") == std::string_view::npos) {
            // fast path
            if (turbo::is_ipv4(buffer)) {
                auto r = parse_ipv4(buffer, result);
                if (r.ok()) {
                    ht = UriHostType::IPV4;
                }
                return r;
            }

            if (result) {
                *result = std::move(buffer);
            }

            return {};
        }

        std::optional<std::string> tmp_host;
        auto valid = turbo::to_ascii(tmp_host, input, input.find('%'));
        if (!valid) {
            return {UriErrorCode::kUriNotComplete, 0, "parse_host to_ascii returns false"};
        }

        if (std::any_of(tmp_host.value().begin(), tmp_host.value().end(),
                turbo::is_forbidden_domain_code_point)) {
            return {UriErrorCode::kUriForbiddenHostCodePoint, 0, ""};
        }


        // If asciiDomain ends in a number, then return the result of IPv4 parsing
        // asciiDomain.
        if (turbo::is_ipv4(tmp_host.value())) {
            auto r=  parse_ipv4(tmp_host.value(), result);
            if (r.ok()) {
                ht = UriHostType::IPV4;
            }
            return r;
        }

        if (result) {
            *result = tmp_host.value();
        }

        return {};
    }

    bool parse_scheme(const std::string_view input, SchemaType &type, std::string *result) {
        auto parsed_type = turbo::get_scheme_type(input);
        bool is_input_special = (parsed_type != turbo::SchemaType::NOT_SPECIAL);
        ///
        /// In the common case, we will immediately recognize a special scheme (e.g.,
        /// http, https), in which case, we can go really fast.
        ///
        if (is_input_special) {
            // fast path!!!
            type = parsed_type;
        } else {
            // slow path
            std::string _buffer(input);
            // Next function is only valid if the input is ASCII and returns false
            // otherwise, but it seems that we always have ascii content so we do not
            // need to check the return value.
            // bool is_ascii =
            turbo::to_lower_ascii(_buffer.data(), _buffer.size());

            if (result) {
              *result = std::move(_buffer);
            }
        }

        return true;
    }

    bool parse_scheme_state_override(const std::string_view input, bool is_special, bool has_credentials, bool host_empty,std::optional<uint16_t> & port, SchemaType &type, std::string *result) {
        auto parsed_type = turbo::get_scheme_type(input);
        bool is_input_special = (parsed_type != turbo::SchemaType::NOT_SPECIAL);
        /**
         * In the common case, we will immediately recognize a special scheme (e.g.,
         *http, https), in which case, we can go really fast.
         **/
        if (is_input_special) { // fast path!!!
            // If url's scheme is not a special scheme and buffer is a special scheme,
            // then return.
            if (is_special != is_input_special) {
                return false;
            }

            // If url includes credentials or has a non-null port, and buffer is
            // "file", then return.
            if ((has_credentials || port.has_value()) && parsed_type == turbo::SchemaType::FILE) {
                return false;
            }

            // If url's scheme is "file" and its host is an empty host, then return.
            // An empty host is the empty string.
            if (type == turbo::SchemaType::FILE && host_empty) {
                return false;
            }
            type = parsed_type;

            // This is uncommon.
            uint16_t urls_scheme_port = turbo::get_special_port(type);

            if (urls_scheme_port) {
                // If url's port is url's scheme's default port, then set url's port to
                // null.
                if (port.has_value() && *port == urls_scheme_port) {
                    port = std::nullopt;
                }
            }
        } else {
            // slow path
            std::string _buffer(input);
            // Next function is only valid if the input is ASCII and returns false
            // otherwise, but it seems that we always have ascii content so we do not
            // need to check the return value.
            // bool is_ascii =
            turbo::to_lower_ascii(_buffer.data(), _buffer.size());

            // If url's scheme is a special scheme and buffer is not a special scheme,
            // then return. If url's scheme is not a special scheme and buffer is a
            // special scheme, then return.
            if (is_special != turbo::is_special(_buffer)) {
                return true;
            }

            // If url includes credentials or has a non-null port, and buffer is
            // "file", then return.
            if ((has_credentials || port.has_value()) && _buffer == "file") {
                return true;
            }

            // If url's scheme is "file" and its host is an empty host, then return.
            // An empty host is the empty string.
            if (type == turbo::SchemaType::FILE && host_empty) {
                return true;
            }

            if (result) {
                *result = std::move(_buffer);
            }

            // This is uncommon.
            uint16_t urls_scheme_port = turbo::get_special_port(type);

            if (urls_scheme_port) {
                // If url's port is url's scheme's default port, then set url's port to
                // null.
                if (port.has_value() && *port == urls_scheme_port) {
                    port = std::nullopt;
                }
            }
        }

        return true;
    }


    UriError parse_port(std::string_view view, bool is_special,SchemaType type, bool check_trailing_content, std::optional<uint16_t> &port) noexcept {
        if (!view.empty() && view[0] == '-') {
            return {UriErrorCode::kUriInvalidArgs, 0, ""};
        }
        uint16_t parsed_port { };
        auto r = std::from_chars(view.data(), view.data() + view.size(), parsed_port);
        if (r.ec == std::errc::result_out_of_range) {
            return {UriErrorCode::kUriOverflow, 0, ""};
        }

        const size_t consumed = size_t(r.ptr - view.data());

        if (check_trailing_content) {
            auto valid = (consumed == view.size() || view[consumed] == '/' || view[consumed] == '?' || (is_special && view[consumed] == '\\'));
           if (!valid) {
               return {UriErrorCode::kUriInvalidArgs, static_cast<uint32_t>(consumed), ""};
           }
        }

        // scheme_default_port can return 0, and we should allow 0 as a base port.
        auto default_port = turbo::get_special_port(type);
        bool is_port_valid = (default_port == 0 && parsed_port == 0) || (default_port != parsed_port);
        port = (r.ec == std::errc() && is_port_valid)
            ? std::optional<uint16_t>(parsed_port)
            : std::nullopt;

        return {UriErrorCode::kUriSuccess, static_cast<uint32_t>(consumed), ""};
    }
}  // namespace turbo::uri_wpt

