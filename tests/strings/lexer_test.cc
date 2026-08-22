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

#include <turbo/strings/lexer.h>

#include <string_view>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

namespace {

using turbo::Lexer;
using turbo::Token;
using turbo::TokenType;

std::string_view TokenText(const Token& t) {
    return std::string_view(t.begin, t.size());
}

std::vector<std::pair<TokenType, std::string_view>> Tokenize(std::string_view query,
    size_t max_query_size = 0) {
    Lexer lexer(query, max_query_size);
    std::vector<std::pair<TokenType, std::string_view>> out;
    for (;;) {
        Token t = lexer.next_token();
        out.emplace_back(t.type, TokenText(t));
        if (t.is_end())
            break;
    }
    return out;
}

TEST(Lexer, Empty) {
    auto tokens = Tokenize("");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].first, TokenType::EndOfStream);
    EXPECT_TRUE(tokens[0].second.empty());
}

TEST(Lexer, SimpleSelect) {
    auto tokens = Tokenize("SELECT a FROM t");
    ASSERT_EQ(tokens.size(), 8u);
    EXPECT_EQ(tokens[0], std::make_pair(TokenType::BareWord, std::string_view("SELECT")));
    EXPECT_EQ(tokens[1], std::make_pair(TokenType::Whitespace, std::string_view(" ")));
    EXPECT_EQ(tokens[2], std::make_pair(TokenType::BareWord, std::string_view("a")));
    EXPECT_EQ(tokens[3], std::make_pair(TokenType::Whitespace, std::string_view(" ")));
    EXPECT_EQ(tokens[4], std::make_pair(TokenType::BareWord, std::string_view("FROM")));
    EXPECT_EQ(tokens[5], std::make_pair(TokenType::Whitespace, std::string_view(" ")));
    EXPECT_EQ(tokens[6], std::make_pair(TokenType::BareWord, std::string_view("t")));
    EXPECT_EQ(tokens[7].first, TokenType::EndOfStream);
}

TEST(Lexer, Numbers) {
    auto tokens = Tokenize("123 1.5 1e10 0x1f 1_000");
    EXPECT_EQ(tokens[0], std::make_pair(TokenType::Number, std::string_view("123")));
    EXPECT_EQ(tokens[2], std::make_pair(TokenType::Number, std::string_view("1.5")));
    EXPECT_EQ(tokens[4], std::make_pair(TokenType::Number, std::string_view("1e10")));
    EXPECT_EQ(tokens[6], std::make_pair(TokenType::Number, std::string_view("0x1f")));
    EXPECT_EQ(tokens[8], std::make_pair(TokenType::Number, std::string_view("1_000")));
}

TEST(Lexer, DotVsFloat) {
    auto chained = Tokenize("x.1.1");
    EXPECT_EQ(chained[0].first, TokenType::BareWord);
    EXPECT_EQ(chained[1].first, TokenType::Dot);
    EXPECT_EQ(chained[2], std::make_pair(TokenType::Number, std::string_view("1")));
    EXPECT_EQ(chained[3].first, TokenType::Dot);
    EXPECT_EQ(chained[4], std::make_pair(TokenType::Number, std::string_view("1")));

    auto leading = Tokenize(".1");
    EXPECT_EQ(leading[0], std::make_pair(TokenType::Number, std::string_view(".1")));
}

TEST(Lexer, StringsAndQuotedIdentifiers) {
    auto tokens = Tokenize("'hello''world' \"x\" `y`");
    EXPECT_EQ(tokens[0], std::make_pair(TokenType::StringLiteral, std::string_view("'hello''world'")));
    EXPECT_EQ(tokens[2], std::make_pair(TokenType::QuotedIdentifier, std::string_view("\"x\"")));
    EXPECT_EQ(tokens[4], std::make_pair(TokenType::QuotedIdentifier, std::string_view("`y`")));
}

TEST(Lexer, UnclosedQuotes) {
    EXPECT_EQ(Tokenize("'abc")[0].first, TokenType::ErrorSingleQuoteIsNotClosed);
    EXPECT_EQ(Tokenize("\"abc")[0].first, TokenType::ErrorDoubleQuoteIsNotClosed);
    EXPECT_EQ(Tokenize("`abc")[0].first, TokenType::ErrorBackQuoteIsNotClosed);
}

TEST(Lexer, Comments) {
    auto line = Tokenize("a-- c\nb");
    EXPECT_EQ(line[0].first, TokenType::BareWord);
    EXPECT_EQ(line[1], std::make_pair(TokenType::Comment, std::string_view("-- c")));
    EXPECT_EQ(line[2].first, TokenType::Whitespace);
    EXPECT_EQ(line[3], std::make_pair(TokenType::BareWord, std::string_view("b")));

    auto slash = Tokenize("a/* x */b");
    EXPECT_EQ(slash[1], std::make_pair(TokenType::Comment, std::string_view("/* x */")));
    EXPECT_EQ(slash[2], std::make_pair(TokenType::BareWord, std::string_view("b")));

    EXPECT_EQ(Tokenize("/*")[0].first, TokenType::ErrorMultilineCommentIsNotClosed);

    auto hash_comment = Tokenize("# hello");
    EXPECT_EQ(hash_comment[0].first, TokenType::Comment);
    EXPECT_EQ(Tokenize("#hello")[0].first, TokenType::Error);
}

TEST(Lexer, Operators) {
    auto tokens = Tokenize("( ) + - * / % < > = <=> != <> -> :: || |> @ @@ ?");
    std::vector<TokenType> types;
    for (const auto& t : tokens) {
        if (t.first != TokenType::Whitespace && t.first != TokenType::EndOfStream)
            types.push_back(t.first);
    }

    EXPECT_EQ(types, (std::vector<TokenType> {
                          TokenType::OpeningRoundBracket,
                          TokenType::ClosingRoundBracket,
                          TokenType::Plus,
                          TokenType::Minus,
                          TokenType::Asterisk,
                          TokenType::Slash,
                          TokenType::Percent,
                          TokenType::Less,
                          TokenType::Greater,
                          TokenType::Equals,
                          TokenType::Spaceship,
                          TokenType::NotEquals,
                          TokenType::NotEquals,
                          TokenType::Arrow,
                          TokenType::DoubleColon,
                          TokenType::Concatenation,
                          TokenType::PipeOperator,
                          TokenType::At,
                          TokenType::DoubleAt,
                          TokenType::QuestionMark,
                      }));
}

TEST(Lexer, BracketsCommaSemicolon) {
    auto tokens = Tokenize("[]{},;");
    EXPECT_EQ(tokens[0].first, TokenType::OpeningSquareBracket);
    EXPECT_EQ(tokens[1].first, TokenType::ClosingSquareBracket);
    EXPECT_EQ(tokens[2].first, TokenType::OpeningCurlyBrace);
    EXPECT_EQ(tokens[3].first, TokenType::ClosingCurlyBrace);
    EXPECT_EQ(tokens[4].first, TokenType::Comma);
    EXPECT_EQ(tokens[5].first, TokenType::Semicolon);
}

TEST(Lexer, ArrowMinusColonPipe) {
    EXPECT_EQ(Tokenize("-")[0].first, TokenType::Minus);
    EXPECT_EQ(Tokenize("->")[0].first, TokenType::Arrow);
    EXPECT_EQ(Tokenize(":")[0].first, TokenType::Colon);
    EXPECT_EQ(Tokenize("|")[0].first, TokenType::PipeMark);
    EXPECT_EQ(Tokenize("!")[0].first, TokenType::ErrorSingleExclamationMark);
    EXPECT_EQ(Tokenize("\\G")[0].first, TokenType::VerticalDelimiter);
    EXPECT_EQ(Tokenize("\\X")[0].first, TokenType::Error);
}

TEST(Lexer, DollarAndHereDoc) {
    EXPECT_EQ(Tokenize("$")[0].first, TokenType::DollarSign);
    auto hd = Tokenize("$tag$hello$tag$");
    EXPECT_EQ(hd[0], std::make_pair(TokenType::HereDoc, std::string_view("$tag$hello$tag$")));
}

TEST(Lexer, HexBinString) {
    EXPECT_EQ(Tokenize("x'abc'")[0].first, TokenType::StringLiteral);
    EXPECT_EQ(Tokenize("b'01'")[0].first, TokenType::StringLiteral);
    EXPECT_EQ(Tokenize("x'ab")[0].first, TokenType::ErrorSingleQuoteIsNotClosed);
}

TEST(Lexer, UnicodeWhitespaceAndMinus) {
    auto nbsp = Tokenize("a\xC2\xA0"
                        "b");
    EXPECT_EQ(nbsp[0].first, TokenType::BareWord);
    EXPECT_EQ(nbsp[1].first, TokenType::Whitespace);
    EXPECT_EQ(nbsp[2].first, TokenType::BareWord);

    auto minus = Tokenize("\xE2\x88\x92");
    EXPECT_EQ(minus[0].first, TokenType::Minus);
    EXPECT_EQ(minus[0].second.size(), 3u);
}

TEST(Lexer, UnicodeQuotes) {
    auto s = Tokenize("\xE2\x80\x98"
                      "hi"
                      "\xE2\x80\x99");
    EXPECT_EQ(s[0].first, TokenType::StringLiteral);

    auto q = Tokenize("\xE2\x80\x9C"
                      "id"
                      "\xE2\x80\x9D");
    EXPECT_EQ(q[0].first, TokenType::QuotedIdentifier);
}

TEST(Lexer, MaxQuerySize) {
    auto tokens = Tokenize("abcdef", 3);
    EXPECT_EQ(tokens[0].first, TokenType::ErrorMaxQuerySizeExceeded);
    EXPECT_EQ(tokens[0].second, std::string_view("abcdef"));
}

TEST(Lexer, TokenFlagsAndNames) {
    Token ws(TokenType::Whitespace, nullptr, nullptr);
    EXPECT_FALSE(ws.is_significant());
    EXPECT_FALSE(ws.is_error());
    EXPECT_FALSE(ws.is_end());

    Token err(TokenType::Error, nullptr, nullptr);
    EXPECT_TRUE(err.is_error());

    EXPECT_STREQ(turbo::get_token_name(TokenType::BareWord), "BareWord");
    EXPECT_STREQ(turbo::get_error_token_description(TokenType::ErrorSingleQuoteIsNotClosed),
        "Single quoted string is not closed");
    EXPECT_STREQ(turbo::get_error_token_description(TokenType::BareWord), "Not an error");
}

TEST(Lexer, ExpressionLexerAndWrapper) {
    std::string_view q = "x";
    turbo::ExpressionLexer core(q);
    Lexer wrapped(q);
    EXPECT_EQ(core.next_token().type, TokenType::BareWord);
    EXPECT_EQ(wrapped.next_token().type, TokenType::BareWord);
}

} // namespace
