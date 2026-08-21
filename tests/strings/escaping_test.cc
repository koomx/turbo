// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <turbo/strings/escaping.h>

#include <array>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string_view>
#include <turbo/format/str_format.h>
#include <turbo/log/kcheck.h>
#include <turbo/strings/charset.h>
#include <turbo/strings/str_cat.h>

namespace {

    using ::testing::Eq;
    using ::testing::Optional;

    struct epair {
        std::string escaped;
        std::string unescaped;
    };

    TEST(c_escape, EscapeAndUnescape) {
        const std::string inputs[] = {
            std::string("foo\nxx\r\b\0023"),
            std::string(""),
            std::string("abc"),
            std::string("\1chad_rules"),
            std::string("\1arnar_drools"),
            std::string("xxxx\r\t'\"\\"),
            std::string("\0xx\0", 4),
            std::string("\x01\x31"),
            std::string("abc\xb\x42\141bc"),
            std::string("123\1\x31\x32\x33"),
            std::string("\xc1\xca\x1b\x62\x19o\xcc\x04"),
            std::string(
                "\\\"\xe8\xb0\xb7\xe6\xad\x8c\\\" is Google\\\'s Chinese name"),
        };
        // Do this twice, once for octal escapes and once for hex escapes.
        for (int kind = 0; kind < 4; kind++) {
            for (const std::string& original : inputs) {
                std::string escaped;
                switch (kind) {
                case 0:
                    escaped = turbo::c_escape(original);
                    break;
                case 1:
                    escaped = turbo::c_hex_escape(original);
                    break;
                case 2:
                    escaped = turbo::utf8_safe_c_escape(original);
                    break;
                case 3:
                    escaped = turbo::utf8_safe_chex_escape(original);
                    break;
                }
                std::string unescaped_str;
                EXPECT_TRUE(turbo::c_unescape(escaped, &unescaped_str));
                EXPECT_EQ(unescaped_str, original);

                unescaped_str.erase();
                std::string error;
                EXPECT_TRUE(turbo::c_unescape(escaped, &unescaped_str, &error));
                EXPECT_EQ(error, "");

                // Check in-place unescaping
                std::string s = escaped;
                EXPECT_TRUE(turbo::c_unescape(s, &s));
                ASSERT_EQ(s, original);
            }
        }
        // Check that all possible two character strings can be escaped then
        // unescaped successfully.
        for (int char0 = 0; char0 < 256; char0++) {
            for (int char1 = 0; char1 < 256; char1++) {
                char chars[2];
                chars[0] = char0;
                chars[1] = char1;
                std::string s(chars, 2);
                std::string escaped = turbo::c_hex_escape(s);
                std::string unescaped;
                EXPECT_TRUE(turbo::c_unescape(escaped, &unescaped));
                EXPECT_EQ(s, unescaped);
            }
        }
    }

    TEST(c_escape, BasicEscaping) {
        epair oct_values[] = {
            { "foo\\rbar\\nbaz\\t", "foo\rbar\nbaz\t" },
            { "\\'full of \\\"sound\\\" and \\\"fury\\\"\\'",
                "'full of \"sound\" and \"fury\"'" },
            { "signi\\\\fying\\\\ nothing\\\\", "signi\\fying\\ nothing\\" },
            { "\\010\\t\\n\\013\\014\\r", "\010\011\012\013\014\015" }
        };
        epair hex_values[] = {
            { "ubik\\rubik\\nubik\\t", "ubik\rubik\nubik\t" },
            { "I\\\'ve just seen a \\\"face\\\"",
                "I've just seen a \"face\"" },
            { "hel\\\\ter\\\\skel\\\\ter\\\\", "hel\\ter\\skel\\ter\\" },
            { "\\x08\\t\\n\\x0b\\x0c\\r", "\010\011\012\013\014\015" }
        };
        epair utf8_oct_values[] = {
            { "\xe8\xb0\xb7\xe6\xad\x8c\\r\xe8\xb0\xb7\xe6\xad\x8c\\nbaz\\t",
                "\xe8\xb0\xb7\xe6\xad\x8c\r\xe8\xb0\xb7\xe6\xad\x8c\nbaz\t" },
            { "\\\"\xe8\xb0\xb7\xe6\xad\x8c\\\" is Google\\\'s Chinese name",
                "\"\xe8\xb0\xb7\xe6\xad\x8c\" is Google\'s Chinese name" },
            { "\xe3\x83\xa1\xe3\x83\xbc\xe3\x83\xab\\\\are\\\\Japanese\\\\chars\\\\",
                "\xe3\x83\xa1\xe3\x83\xbc\xe3\x83\xab\\are\\Japanese\\chars\\" },
            { "\xed\x81\xac\xeb\xa1\xac\\010\\t\\n\\013\\014\\r",
                "\xed\x81\xac\xeb\xa1\xac\010\011\012\013\014\015" }
        };
        epair utf8_hex_values[] = {
            { "\x20\xe4\xbd\xa0\\t\xe5\xa5\xbd,\\r!\\n",
                "\x20\xe4\xbd\xa0\t\xe5\xa5\xbd,\r!\n" },
            { "\xe8\xa9\xa6\xe9\xa8\x93\\\' means \\\"test\\\"",
                "\xe8\xa9\xa6\xe9\xa8\x93\' means \"test\"" },
            { "\\\\\xe6\x88\x91\\\\:\\\\\xe6\x9d\xa8\xe6\xac\xa2\\\\",
                "\\\xe6\x88\x91\\:\\\xe6\x9d\xa8\xe6\xac\xa2\\" },
            { "\xed\x81\xac\xeb\xa1\xac\\x08\\t\\n\\x0b\\x0c\\r",
                "\xed\x81\xac\xeb\xa1\xac\010\011\012\013\014\015" }
        };

        for (const epair& val : oct_values) {
            std::string escaped = turbo::c_escape(val.unescaped);
            EXPECT_EQ(escaped, val.escaped);
        }
        for (const epair& val : hex_values) {
            std::string escaped = turbo::c_hex_escape(val.unescaped);
            EXPECT_EQ(escaped, val.escaped);
        }
        for (const epair& val : utf8_oct_values) {
            std::string escaped = turbo::utf8_safe_c_escape(val.unescaped);
            EXPECT_EQ(escaped, val.escaped);
        }
        for (const epair& val : utf8_hex_values) {
            std::string escaped = turbo::utf8_safe_chex_escape(val.unescaped);
            EXPECT_EQ(escaped, val.escaped);
        }
    }

    TEST(Unescape, BasicFunction) {
        epair tests[] = { { "", "" },
            { "\\u0030", "0" },
            { "\\u00A3", "\xC2\xA3" },
            { "\\u22FD", "\xE2\x8B\xBD" },
            { "\\U00010000", "\xF0\x90\x80\x80" },
            { "\\U0010FFFD", "\xF4\x8F\xBF\xBD" } };
        for (const epair& val : tests) {
            std::string out;
            EXPECT_TRUE(turbo::c_unescape(val.escaped, &out));
            EXPECT_EQ(out, val.unescaped);
        }
        constexpr std::string_view bad[] = {
            "\\u1", // too short
            "\\U1", // too short
            "\\Uffffff", // exceeds 0x10ffff (largest Unicode)
            "\\U00110000", // exceeds 0x10ffff (largest Unicode)
            "\\uD835", // surrogate character (D800-DFFF)
            "\\U0000DD04", // surrogate character (D800-DFFF)
            "\\777", // exceeds 0xff
            "\\xABCD", // exceeds 0xff
            "\\x100000041", // overflows uint32_t
            "endswith\\", // ends with "\"
            "endswith\\x", // ends with "\x"
            "endswith\\X", // ends with "\X"
            "\\x.2345678", // non-hex follows "\x"
            "\\X.2345678", // non-hex follows "\X"
            "\\u.2345678", // non-hex follows "\U"
            "\\U.2345678", // non-hex follows "\U"
            "\\.unknown", // unknown escape sequence
        };
        for (const auto e : bad) {
            std::string error;
            std::string out;
            EXPECT_FALSE(turbo::c_unescape(e, &out, &error));
            EXPECT_FALSE(error.empty());

            out.erase();
            EXPECT_FALSE(turbo::c_unescape(e, &out));
        }
    }

    class CUnescapeTest : public testing::Test {
    protected:
        static const char kStringWithMultipleOctalNulls[];
        static const char kStringWithMultipleHexNulls[];
        static const char kStringWithMultipleUnicodeNulls[];

        std::string result_string_;
    };

    const char CUnescapeTest::kStringWithMultipleOctalNulls[] = "\\0\\n" // null escape \0 plus newline
                                                                "0\\n" // just a number 0 (not a null escape) plus newline
                                                                "\\00\\12" // null escape \00 plus octal newline code
                                                                "\\000"; // null escape \000

    // This has the same ingredients as kStringWithMultipleOctalNulls
    // but with \x hex escapes instead of octal escapes.
    const char CUnescapeTest::kStringWithMultipleHexNulls[] = "\\x0\\n"
                                                              "0\\n"
                                                              "\\x00\\xa"
                                                              "\\x000";

    const char CUnescapeTest::kStringWithMultipleUnicodeNulls[] = "\\u0000\\n" // short-form (4-digit) null escape plus newline
                                                                  "0\\n" // just a number 0 (not a null escape) plus newline
                                                                  "\\U00000000"; // long-form (8-digit) null escape

    TEST_F(CUnescapeTest, Unescapes1CharOctalNull) {
        std::string original_string = "\\0";
        EXPECT_TRUE(turbo::c_unescape(original_string, &result_string_));
        EXPECT_EQ(std::string("\0", 1), result_string_);
    }

    TEST_F(CUnescapeTest, Unescapes2CharOctalNull) {
        std::string original_string = "\\00";
        EXPECT_TRUE(turbo::c_unescape(original_string, &result_string_));
        EXPECT_EQ(std::string("\0", 1), result_string_);
    }

    TEST_F(CUnescapeTest, Unescapes3CharOctalNull) {
        std::string original_string = "\\000";
        EXPECT_TRUE(turbo::c_unescape(original_string, &result_string_));
        EXPECT_EQ(std::string("\0", 1), result_string_);
    }

    TEST_F(CUnescapeTest, Unescapes1CharHexNull) {
        std::string original_string = "\\x0";
        EXPECT_TRUE(turbo::c_unescape(original_string, &result_string_));
        EXPECT_EQ(std::string("\0", 1), result_string_);
    }

    TEST_F(CUnescapeTest, Unescapes2CharHexNull) {
        std::string original_string = "\\x00";
        EXPECT_TRUE(turbo::c_unescape(original_string, &result_string_));
        EXPECT_EQ(std::string("\0", 1), result_string_);
    }

    TEST_F(CUnescapeTest, Unescapes3CharHexNull) {
        std::string original_string = "\\x000";
        EXPECT_TRUE(turbo::c_unescape(original_string, &result_string_));
        EXPECT_EQ(std::string("\0", 1), result_string_);
    }

    TEST_F(CUnescapeTest, Unescapes4CharUnicodeNull) {
        std::string original_string = "\\u0000";
        EXPECT_TRUE(turbo::c_unescape(original_string, &result_string_));
        EXPECT_EQ(std::string("\0", 1), result_string_);
    }

    TEST_F(CUnescapeTest, Unescapes8CharUnicodeNull) {
        std::string original_string = "\\U00000000";
        EXPECT_TRUE(turbo::c_unescape(original_string, &result_string_));
        EXPECT_EQ(std::string("\0", 1), result_string_);
    }

    TEST_F(CUnescapeTest, UnescapesMultipleOctalNulls) {
        std::string original_string(kStringWithMultipleOctalNulls);
        EXPECT_TRUE(turbo::c_unescape(original_string, &result_string_));
        // All escapes, including newlines and null escapes, should have been
        // converted to the equivalent characters.
        EXPECT_EQ(std::string("\0\n"
                              "0\n"
                              "\0\n"
                              "\0",
                      7),
            result_string_);
    }

    TEST_F(CUnescapeTest, UnescapesMultipleHexNulls) {
        std::string original_string(kStringWithMultipleHexNulls);
        EXPECT_TRUE(turbo::c_unescape(original_string, &result_string_));
        EXPECT_EQ(std::string("\0\n"
                              "0\n"
                              "\0\n"
                              "\0",
                      7),
            result_string_);
    }

    TEST_F(CUnescapeTest, UnescapesMultipleUnicodeNulls) {
        std::string original_string(kStringWithMultipleUnicodeNulls);
        EXPECT_TRUE(turbo::c_unescape(original_string, &result_string_));
        EXPECT_EQ(std::string("\0\n"
                              "0\n"
                              "\0",
                      5),
            result_string_);
    }

    TEST(Escaping, HexStringToBytesBackToHex) {
        std::string bytes, hex;

        constexpr std::string_view kTestHexLower = "1c2f0032f40123456789abcdef";
        constexpr std::string_view kTestHexUpper = "1C2F0032F40123456789ABCDEF";
        constexpr std::string_view kTestBytes = std::string_view(
            "\x1c\x2f\x00\x32\xf4\x01\x23\x45\x67\x89\xab\xcd\xef", 13);

        EXPECT_TRUE(turbo::hex_string_to_bytes(kTestHexLower, &bytes));
        EXPECT_EQ(bytes, kTestBytes);

        EXPECT_TRUE(turbo::hex_string_to_bytes(kTestHexUpper, &bytes));
        EXPECT_EQ(bytes, kTestBytes);

        hex = turbo::bytes_to_hex_string(kTestBytes);
        EXPECT_EQ(hex, kTestHexLower);

        // Same buffer.
        // We do not care if this works since we do not promise it in the contract.
        // The purpose of this test is to to see if the program will crash or if
        // sanitizers will catch anything.
        bytes = std::string(kTestHexUpper);
        (void)turbo::hex_string_to_bytes(bytes, &bytes);

        // Length not a multiple of two.
        EXPECT_FALSE(turbo::hex_string_to_bytes("1c2f003", &bytes));

        // Not hex.
        EXPECT_FALSE(turbo::hex_string_to_bytes("1c2f00ft", &bytes));

        // Empty input.
        bytes = "abc";
        EXPECT_TRUE(turbo::hex_string_to_bytes("", &bytes));
        EXPECT_EQ("", bytes); // Results in empty output.

        // Ensure there is no sign extension bug on a signed char.
        hex.assign("\xC8"
                   "b",
            2);
        EXPECT_FALSE(turbo::hex_string_to_bytes(hex, &bytes));
    }

    TEST(HexAndBack, HexStringToBytes_and_BytesToHexString) {
        std::string hex_mixed = "0123456789abcdefABCDEF";
        std::string bytes_expected = "\x01\x23\x45\x67\x89\xab\xcd\xef\xAB\xCD\xEF";
        std::string hex_only_lower = "0123456789abcdefabcdef";

        std::string bytes_result;
        EXPECT_TRUE(turbo::hex_string_to_bytes(hex_mixed, &bytes_result));
        EXPECT_EQ(bytes_expected, bytes_result);

        std::string prefix_valid = hex_mixed + "?";
        std::string prefix_valid_result;
        EXPECT_TRUE(turbo::hex_string_to_bytes(
            std::string_view(prefix_valid.data(), prefix_valid.size() - 1), &prefix_valid_result));
        EXPECT_EQ(bytes_expected, prefix_valid_result);

        std::string infix_valid = "?" + hex_mixed + "???";
        std::string infix_valid_result;
        EXPECT_TRUE(turbo::hex_string_to_bytes(
            std::string_view(infix_valid.data() + 1, hex_mixed.size()), &infix_valid_result));
        EXPECT_EQ(bytes_expected, infix_valid_result);

        std::string hex_result = turbo::bytes_to_hex_string(bytes_expected);
        EXPECT_EQ(hex_only_lower, hex_result);
    }

} // namespace
