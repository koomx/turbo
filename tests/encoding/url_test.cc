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


#include <string_view>
#include <turbo/encoding/url.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <turbo/strings/charset.h>
#include <turbo/format/str_format.h>

namespace turbo {

        using ::testing::Eq;
        using ::testing::Optional;

        TEST(url_encode, Basics) {
        EXPECT_EQ(turbo::url_encode(""), "");
        EXPECT_THAT(turbo::url_decode(""), Optional(Eq("")));

        EXPECT_EQ(turbo::url_encode("abc"), "abc");
        EXPECT_THAT(turbo::url_decode("abc"), Optional(Eq("abc")));

        EXPECT_EQ(turbo::url_encode("a/b"), "a%2Fb");
        EXPECT_THAT(turbo::url_decode("a%2Fb"), Optional(Eq("a/b")));

        EXPECT_EQ(turbo::url_encode("one two"), "one%20two");
        EXPECT_THAT(turbo::url_decode("one%20two"), Optional(Eq("one two")));

        EXPECT_EQ(turbo::url_encode("10%"), "10%25");
        EXPECT_THAT(turbo::url_decode("10%25"), Optional(Eq("10%")));

        EXPECT_EQ(turbo::url_encode(" ?&=#+%!<>#\"{}|\\^[]`☺\t:/@$'()*,;"),
            "%20%3F%26%3D%23%2B%25%21%3C%3E%23%22%7B%7D%7C%5C%5E%5B%5D%60%E2%"
            "98%BA%09%3A%2F%40%24%27%28%29%2A%2C%3B");
        EXPECT_THAT(turbo::url_decode("%20%3F%26%3D%23%2B%25%21%3C%3E%23%22%7B%7D%7C%"
                                       "5C%5E%5B%5D%60%E2%98%BA%"
                                       "09%3A%2F%40%24%27%28%29%2A%2C%3B"),
            Optional(Eq(" ?&=#+%!<>#\"{}|\\^[]`☺\t:/@$'()*,;")));

        // Test all characters.
        static constexpr turbo::CharSet kDoNotEscape = turbo::CharSet::AsciiAlphanumerics() | turbo::CharSet("-._~");
        for (int i = 0; i < 256; ++i) {
            char c = static_cast<char>(i);
            std::string expected = kDoNotEscape.contains(c)
                ? std::string(1, c)
                : turbo::str_sprintf("%%%02X", c);
            EXPECT_EQ(turbo::url_encode(std::string_view(&c, 1)), expected);
            EXPECT_EQ(turbo::url_decode(expected), std::string_view(&c, 1));
        }
    }

    TEST(url_decode, SuccessCases) {
        EXPECT_THAT(turbo::url_decode(""), Optional(Eq("")));
        EXPECT_THAT(turbo::url_decode("abc"), Optional(Eq("abc")));
        EXPECT_THAT(turbo::url_decode("1%41"), Optional(Eq("1A")));
        EXPECT_THAT(turbo::url_decode("1%41%42%43"), Optional(Eq("1ABC")));
        EXPECT_THAT(turbo::url_decode("%4a"), Optional(Eq("J")));
        EXPECT_THAT(turbo::url_decode("%6F"), Optional(Eq("o")));
        EXPECT_THAT(turbo::url_decode("a%20b"), Optional(Eq("a b")));
        EXPECT_THAT(turbo::url_decode("a+b"), Optional(Eq("a+b")));
    }

    TEST(url_decode, NotEnoughCharsAfterPercent) {
        EXPECT_EQ(turbo::url_decode("%"), std::nullopt);
        EXPECT_EQ(turbo::url_decode("%a"), std::nullopt);
        EXPECT_EQ(turbo::url_decode("%1"), std::nullopt);
        EXPECT_EQ(turbo::url_decode("123%45%6"), std::nullopt);
    }

    TEST(url_decode, InvalidHexDigits) {
        EXPECT_EQ(turbo::url_decode("%zzzzz"), std::nullopt);
    }

    TEST(url_decode, NoErrorWithNoEscapeSequence) {
        // Any string that does not contain '%' should not produce an error, even if
        // turbo::url_encode() would never produce a string with certain characters.
        std::string no_percent;
        for (int c = 0; c < 256; ++c) {
            if (c != '%') {
                no_percent.push_back(static_cast<char>(c));
            }
        }
        EXPECT_THAT(turbo::url_decode(no_percent), Optional(no_percent));
    }

    TEST(url_encode_plus, Basics) {
        EXPECT_EQ(turbo::url_encode_plus(""), "");
        EXPECT_THAT(turbo::url_decode_plus(""), Optional(Eq("")));

        EXPECT_EQ(turbo::url_encode_plus("abc"), "abc");
        EXPECT_THAT(turbo::url_decode_plus("abc"), Optional(Eq("abc")));

        EXPECT_EQ(turbo::url_encode_plus("one two"), "one+two");
        EXPECT_THAT(turbo::url_decode_plus("one+two"), Optional(Eq("one two")));

        EXPECT_EQ(turbo::url_encode_plus("gift for mom & dad"), "gift+for+mom+%26+dad");
        EXPECT_THAT(turbo::url_decode_plus("gift+for+mom+%26+dad"),
            Optional(Eq("gift for mom & dad")));

        EXPECT_EQ(turbo::url_encode_plus("10%"), "10%25");
        EXPECT_THAT(turbo::url_decode_plus("10%25"), Optional(Eq("10%")));

        EXPECT_EQ(turbo::url_encode_plus(" ?&=#+%!<>#\"{}|\\^[]`☺\t:/@$'()*,;"),
            "+%3F%26%3D%23%2B%25%21%3C%3E%23%22%7B%7D%7C%5C%5E%5B%5D%60%E2%"
            "98%BA%09%3A%2F%40%24%27%28%29%2A%2C%3B");
        EXPECT_THAT(turbo::url_decode_plus("+%3F%26%3D%23%2B%25%21%3C%3E%23%22%7B%7D%"
                                           "7C%5C%5E%5B%5D%60%E2%98%BA%"
                                           "09%3A%2F%40%24%27%28%29%2A%2C%3B"),
            Optional(Eq(" ?&=#+%!<>#\"{}|\\^[]`☺\t:/@$'()*,;")));

        // Test all characters.
        static constexpr turbo::CharSet kDoNotEscape = turbo::CharSet::AsciiAlphanumerics() | turbo::CharSet("-._~");
        for (int i = 0; i < 256; ++i) {
            char c = static_cast<char>(i);
            std::string expected = kDoNotEscape.contains(c)
                ? std::string(1, c)
                : turbo::str_sprintf("%%%02X", c);
            if (c == ' ')
                expected = '+';
            EXPECT_EQ(turbo::url_encode_plus(std::string_view(&c, 1)), expected);
            EXPECT_EQ(turbo::url_decode_plus(expected), std::string_view(&c, 1));
        }
    }

    TEST(url_decode_plus, SuccessCases) {
        EXPECT_THAT(turbo::url_decode_plus(""), Optional(Eq("")));
        EXPECT_THAT(turbo::url_decode_plus("abc"), Optional(Eq("abc")));
        EXPECT_THAT(turbo::url_decode_plus("1%41"), Optional(Eq("1A")));
        EXPECT_THAT(turbo::url_decode_plus("1%41%42%43"), Optional(Eq("1ABC")));
        EXPECT_THAT(turbo::url_decode_plus("%4a"), Optional(Eq("J")));
        EXPECT_THAT(turbo::url_decode_plus("%6F"), Optional(Eq("o")));
        EXPECT_THAT(turbo::url_decode_plus("a%20b"), Optional(Eq("a b")));
        EXPECT_THAT(turbo::url_decode_plus("a+b"), Optional(Eq("a b")));
    }

    TEST(url_decode_plus, NotEnoughCharsAfterPercent) {
        EXPECT_EQ(turbo::url_decode_plus("%"), std::nullopt);
        EXPECT_EQ(turbo::url_decode_plus("%a"), std::nullopt);
        EXPECT_EQ(turbo::url_decode_plus("%1"), std::nullopt);
        EXPECT_EQ(turbo::url_decode_plus("123%45%6"), std::nullopt);
    }

    TEST(url_decode_plus, InvalidHexDigits) {
        EXPECT_EQ(turbo::url_decode_plus("%zzzzz"), std::nullopt);
    }

    TEST(url_decode_plus, NoErrorWithNoEscapeSequence) {
        // Any string that does not contain '%' should not produce an error, even if
        // turbo::url_encode_plus() would never produce a string with certain
        // characters.
        std::string no_percent;
        std::string no_percent_expected;
        for (int c = 0; c < 256; ++c) {
            if (c != '%') {
                no_percent.push_back(static_cast<char>(c));
                no_percent_expected.push_back(c != '+' ? static_cast<char>(c) : ' ');
            }
        }
        EXPECT_THAT(turbo::url_decode_plus(no_percent), Optional(no_percent_expected));
    }
}  // namespace turbo
