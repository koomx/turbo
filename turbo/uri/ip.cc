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

#include <turbo/uri/ip.h>
#include <string>
#include <charconv>
#include <turbo/strings/match.h>
#include <turbo/strings/str_cat.h>
#include <turbo/strings/ascii.h>
#include <turbo/uri/utility.h>

namespace turbo {

    void find_longest_sequence_of_ipv6_pieces(
        const std::array<uint16_t, 8>& address, size_t& compress,
        size_t& compress_length) noexcept {
        for (size_t i = 0; i < 8; i++) {
            if (address[i] == 0) {
                size_t next = i + 1;
                while (next != 8 && address[next] == 0)
                    ++next;
                const size_t count = next - i;
                if (compress_length < count) {
                    compress_length = count;
                    compress = i;
                    if (next == 8)
                        break;
                    i = next;
                }
            }
        }
    }

    std::string ipv6_to_string(const std::array<uint16_t, 8>& address) noexcept {
        size_t compress_length = 0; // The length of a long sequence of zeros.
        size_t compress = 0; // The start of a long sequence of zeros.
        find_longest_sequence_of_ipv6_pieces(address, compress, compress_length);

        if (compress_length <= 1) {
            // Optimization opportunity: Find a faster way then snprintf for imploding
            // and return here.
            compress = compress_length = 8;
        }

        std::string output(4 * 8 + 7 + 2, '\0');
        size_t piece_index = 0;
        char* point = output.data();
        char* point_end = output.data() + output.size();
        *point++ = '[';
        while (true) {
            if (piece_index == compress) {
                *point++ = ':';
                // If we skip a value initially, we need to write '::', otherwise
                // a single ':' will do since it follows a previous ':'.
                if (piece_index == 0) {
                    *point++ = ':';
                }
                piece_index += compress_length;
                if (piece_index == 8) {
                    break;
                }
            }
            point = std::to_chars(point, point_end, address[piece_index], 16).ptr;
            piece_index++;
            if (piece_index == 8) {
                break;
            }
            *point++ = ':';
        }
        *point++ = ']';
        output.resize(point - output.data());
        return output;
    }

    std::string ipv4_to_string(uint64_t address) noexcept {
        std::string output(15, '\0');
        char* point = output.data();
        char* point_end = output.data() + output.size();
        point = std::to_chars(point, point_end, uint8_t(address >> 24)).ptr;
        for (int i = 2; i >= 0; i--) {
            *point++ = '.';
            point = std::to_chars(point, point_end, uint8_t(address >> (i * 8))).ptr;
        }
        output.resize(point - output.data());
        return output;
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


    UriError parse_wpt_ipv4(std::string_view input, std::string *result) {
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


    bool is_wpt_ipv4(std::string_view view) noexcept {
        // The string is not empty and does not contain upper case ASCII characters.
        //
        // Optimization. To be considered as a possible ipv4, the string must end
        // with 'x' or a lowercase hex character.
        // Most of the time, this will be false so this simple check will save a lot
        // of effort.
        char last_char = view.back();
        // If the address ends with a dot, we need to prune it (special case).
        if (last_char == '.') {
            view.remove_suffix(1);
            if (view.empty()) {
                return false;
            }
            last_char = view.back();
        }
        bool possible_ipv4 = (last_char >= '0' && last_char <= '9') || (last_char >= 'a' && last_char <= 'f') || last_char == 'x';
        if (!possible_ipv4) {
            return false;
        }
        /// From the last character, find the last dot.
        size_t last_dot = view.rfind('.');
        if (last_dot != std::string_view::npos) {
            /// We have at least one dot.
            view = view.substr(last_dot + 1);
        }
        /// Optimization opportunity: we have basically identified the last number of
        /// the ipv4 if we return true here. We might as well parse it and have at
        /// least one number parsed when we get to parse_ipv4.
        if (std::all_of(view.begin(), view.end(), turbo::ascii_isdigit)) {
            return true;
        }
        // It could be hex (0x), but not if there is a single character.
        if (view.size() == 1) {
            return false;
        }
        // It must start with 0x.
        if (!std::equal(view.begin(), view.begin() + 2, "0x")) {
            return false;
        }
        // We must allow "0x".
        if (view.size() == 2) {
            return true;
        }
        // We have 0x followed by some characters, we need to check that they are
        // hexadecimals.
        return std::all_of(view.begin() + 2, view.end(),
            turbo::is_lowercase_hex);
    }

    bool is_rfc_ipv4(std::string_view view) noexcept {
        if (view.empty()) {
            return false;
        }
        // RFC IPv4address ends with a decimal digit only (no hex / trailing dot).
        char last_char = view.back();
        if (last_char < '0' || last_char > '9') {
            return false;
        }
        size_t last_dot = view.rfind('.');
        if (last_dot != std::string_view::npos) {
            view = view.substr(last_dot + 1);
        }
        return !view.empty()
            && std::all_of(view.begin(), view.end(), turbo::ascii_isdigit);
    }

    UriError try_parse_wpt_ip_v4(const char* start, const char* end, std::optional<IpAddr>& out) {
        size_t digit_count { 0 };
        int pure_decimal_count = 0;
        uint64_t ipv4 { 0 };

        std::string_view input(start, end - start);
        for (; (digit_count < 4) && !input.empty(); digit_count++) {
            // If any number exceeds 32 bits, we have an error.
            uint32_t segment_result { };
            std::from_chars_result rc { };
            bool is_hex = turbo::has_hex_prefix(input);
            if (is_hex) {
                // special case
                if (((input.length() == 2) || ((input.length() > 2) && (input[2] == '.')))) {
                    segment_result = 0;
                    rc = {input.data() +2, std::errc()};
                } else {
                    rc = std::from_chars(input.data() + 2, input.data() + input.size(),
                        segment_result, 16);
                }
            } else {
                if ((input.length() >= 2) && input[0] == '0' && turbo::ascii_isdigit(input[1])) {
                    rc = std::from_chars(input.data() + 1, input.data() + input.size(),
                        segment_result, 8);
                } else {
                    pure_decimal_count++;
                    rc = std::from_chars(input.data(), input.data() + input.size(),
                        segment_result, 10);
                }
            }

            if (rc.ec != std::errc()) {
                return {UriErrorCode::kUriInvalidArgs,
                   0,
                "r.ec != std::errc()"
                       };
            }
            input.remove_prefix(rc.ptr - input.data());

            // There is more, so that the value must no be larger than 255
            // and we must have a '.'.
            if ((segment_result > 255)) {
                return {UriErrorCode::kUriOverflow,
                   static_cast<uint32_t>(input.data() - start),
                turbo::str_cat("parse ipv4 ", digit_count,
                    " element overflow with ", segment_result)
                       };
            }
            ipv4 <<= 8;
            ipv4 |= segment_result;

            if (digit_count < 3) {
                if (input.empty() || input[0] != '.') {
                    return {UriErrorCode::kUriNotComplete,
                        0,
                     "ipv4 not complete"
                            };
                }
                input.remove_prefix(1); // remove '.'
            }

        }  /// for
        if (digit_count != 4) {
            return {UriErrorCode::kUriNotComplete,
                       0,
                    turbo::str_cat("ipv4 not complete, got ", digit_count,
                        (digit_count > 1 ? " segments" : " segment"))
                           };
        }

        IpAddr addr;
        addr.setup_ipv4(ipv4);
        if (pure_decimal_count == 4) {
            addr.normalized.assign(start, input.data());
        } else {
            addr.normalized = turbo::ipv4_to_string(ipv4);
        }
        out = std::move(addr);
        return {UriErrorCode::kUriSuccess, static_cast<uint32_t>(input.data() - start), ""};
    }

    UriError try_parse_rfc_ip_v4(const char* start, const char* end, std::optional<IpAddr>& out) {
        if (start == end || !turbo::ascii_isdigit(static_cast<unsigned char>(*start))) {
            return {UriErrorCode::kUriSuccess, 0, ""};
        }

        size_t digit_count { 0 };
        uint64_t ipv4 { 0 };

        std::string_view input(start, end - start);
        for (; (digit_count < 4) && !input.empty(); digit_count++) {
            const char* seg = input.data();
            if (input[0] == '0' && input.size() > 1 && turbo::ascii_isdigit(input[1])) {
                // RFC dec-octet: leading zero only allowed for lone "0"
                return {UriErrorCode::kUriInvalidArgs, 0, "rfc ipv4 leading zero"};
            }

            uint32_t segment_result { };
            auto rc = std::from_chars(input.data(), input.data() + input.size(),
                segment_result, 10);

            if (rc.ec != std::errc()) {
                if (digit_count == 0) {
                    return {UriErrorCode::kUriSuccess, 0, ""};
                }
                return {UriErrorCode::kUriInvalidArgs,
                   0,
                "r.ec != std::errc()"
                       };
            }
            // Reject if from_chars ate a leading-zero form we missed (e.g. only "0" is ok)
            if (rc.ptr - seg > 1 && *seg == '0') {
                return {UriErrorCode::kUriInvalidArgs, 0, "rfc ipv4 leading zero"};
            }
            input.remove_prefix(rc.ptr - input.data());

            if ((segment_result > 255)) {
                return {UriErrorCode::kUriOverflow,
                   static_cast<uint32_t>(input.data() - start),
                turbo::str_cat("parse ipv4 ", digit_count,
                    " element overflow with ", segment_result)
                       };
            }
            ipv4 <<= 8;
            ipv4 |= segment_result;

            if (digit_count < 3) {
                if (input.empty() || input[0] != '.') {
                    return {UriErrorCode::kUriNotComplete,
                        0,
                     "ipv4 not complete"
                            };
                }
                input.remove_prefix(1); // remove '.'
            }

        }  /// for
        if (digit_count != 4) {
            return {UriErrorCode::kUriNotComplete,
                       0,
                    turbo::str_cat("ipv4 not complete, got ", digit_count,
                        (digit_count > 1 ? " segments" : " segment"))
                           };
        }

        IpAddr addr;
        addr.setup_ipv4(ipv4);
        addr.normalized.assign(start, input.data());
        out = std::move(addr);
        return {UriErrorCode::kUriSuccess, static_cast<uint32_t>(input.data() - start), ""};
    }


    UriError try_parse_wpt_ip_v6(const char* start, const char* end, std::optional<IpAddr>& out) {
        if (start == end || *start != '[') {
            return {UriErrorCode::kUriSuccess, 0, ""};
        }

        const char* pointer = start + 1;
        auto at_eof = [end](const char* p) {
            return p == end || *p == ']';
        };
        auto err_pos = [start](const char* p) {
            return static_cast<uint32_t>(p - start);
        };

        if (at_eof(pointer)) {
            return {UriErrorCode::kUriNotComplete, err_pos(pointer), "ipv6 empty"};
        }

        std::array<uint16_t, 8> address {};
        int piece_index = 0;
        std::optional<int> compress {};

        if (*pointer == ':') {
            if (pointer + 1 == end || pointer[1] != ':') {
                return {UriErrorCode::kUriInvalidArgs, err_pos(pointer + 1),
                    "parse_ipv6 starts with : but the rest does not start with :"};
            }
            pointer += 2;
            compress = ++piece_index;
        }

        while (!at_eof(pointer)) {
            if (piece_index == 8) {
                return {UriErrorCode::kUriOverflow, err_pos(pointer), "parse_ipv6 piece_index == 8"};
            }

            if (*pointer == ':') {
                if (compress.has_value()) {
                    return {UriErrorCode::kUriInvalidArgs, err_pos(pointer),
                        "parse_ipv6 compress is non-null"};
                }
                pointer++;
                compress = ++piece_index;
                continue;
            }

            uint16_t value = 0, length = 0;
            while (length < 4 && !at_eof(pointer) && turbo::ascii_isxdigit(*pointer)) {
                value = uint16_t(value * 0x10 + turbo::convert_hex_to_binary(*pointer));
                pointer++;
                length++;
            }

            if (!at_eof(pointer) && *pointer == '.') {
                if (length == 0) {
                    return {UriErrorCode::kUriOverflow, err_pos(pointer), "parse_ipv6 length is 0"};
                }
                pointer -= length;
                if (piece_index > 6) {
                    return {UriErrorCode::kUriOverflow, err_pos(pointer), "parse_ipv6 piece_index > 6"};
                }

                int numbers_seen = 0;
                while (!at_eof(pointer)) {
                    std::optional<uint16_t> ipv4_piece {};
                    if (numbers_seen > 0) {
                        if (*pointer == '.' && numbers_seen < 4) {
                            pointer++;
                        } else {
                            return {UriErrorCode::kUriOverflow, err_pos(pointer),
                                "parse_ipv6 Otherwise, validation error, return failure"};
                        }
                    }

                    if (at_eof(pointer) || !turbo::ascii_isdigit(*pointer)) {
                        return {UriErrorCode::kUriOverflow, err_pos(pointer),
                            "parse_ipv6 If c is not an ASCII digit, validation error, return failure"};
                    }

                    while (!at_eof(pointer) && turbo::ascii_isdigit(*pointer)) {
                        int number = *pointer - '0';
                        if (!ipv4_piece.has_value()) {
                            ipv4_piece = number;
                        } else if (ipv4_piece == 0) {
                            return {UriErrorCode::kUriInvalidArgs, err_pos(pointer),
                                "parse_ipv6 if ipv4Piece is 0, validation error"};
                        } else {
                            ipv4_piece = *ipv4_piece * 10 + number;
                        }
                        if (ipv4_piece > 255) {
                            return {UriErrorCode::kUriInvalidArgs, err_pos(pointer),
                                "parse_ipv6 ipv4_piece > 255"};
                        }
                        pointer++;
                    }

                    address[piece_index] = uint16_t(address[piece_index] * 0x100 + *ipv4_piece);
                    numbers_seen++;
                    if (numbers_seen == 2 || numbers_seen == 4) {
                        piece_index++;
                    }
                }

                if (numbers_seen != 4) {
                    return {UriErrorCode::kUriInvalidArgs, err_pos(pointer),
                        turbo::str_cat("numbers_seen != 4, ", numbers_seen)};
                }
                break;
            } else if (!at_eof(pointer) && *pointer == ':') {
                pointer++;
                if (at_eof(pointer)) {
                    return {UriErrorCode::kUriInvalidArgs, err_pos(pointer),
                        "parse_ipv6 If c is the EOF code point, validation error, return failure"};
                }
            } else if (!at_eof(pointer)) {
                return {UriErrorCode::kUriInvalidArgs, err_pos(pointer),
                    "parse_ipv6 Otherwise, if c is not the EOF code point, validation error, return failure"};
            }

            address[piece_index] = value;
            piece_index++;
        }

        if (pointer == end || *pointer != ']') {
            return {UriErrorCode::kUriNotComplete, err_pos(pointer), "missing ]"};
        }

        if (compress.has_value()) {
            int swaps = piece_index - *compress;
            piece_index = 7;
            while (piece_index != 0 && swaps > 0) {
                std::swap(address[piece_index], address[*compress + swaps - 1]);
                piece_index--;
                swaps--;
            }
        } else if (piece_index != 8) {
            return {UriErrorCode::kUriInvalidArgs, err_pos(pointer),
                "parse_ipv6 if compress is null and pieceIndex is not 8, validation error, return failure"};
        }

        pointer++;  // consume ']'

        IpAddr addr;
        addr.type = IpType::IP_V6;
        addr.data = address;
        addr.normalized = turbo::ipv6_to_string(address);
        out = std::move(addr);
        return {UriErrorCode::kUriSuccess, err_pos(pointer), ""};
    }

    UriError try_parse_rfc_ip_v6(const char* start, const char* end, std::optional<IpAddr>& out) {
        return try_parse_wpt_ip_v6(start, end, out);
    }


}  // namespace turbo
