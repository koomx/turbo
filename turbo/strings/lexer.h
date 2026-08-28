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

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace turbo {

    enum class TokenType : uint8_t {
        Whitespace,
        Comment,
        /// Either keyword (SELECT) or identifier (column)
        BareWord,
        /// Always non-negative. No leading plus. 123 or something like 123.456e12, 0x123p12
        Number,
        /// 'hello word', 'hello''word', 'hello\'word\\'
        StringLiteral,
        /// "x", `x`
        QuotedIdentifier,
        OpeningRoundBracket,
        ClosingRoundBracket,
        OpeningSquareBracket,
        ClosingSquareBracket,
        OpeningCurlyBrace,
        ClosingCurlyBrace,
        Comma,
        Semicolon,
        /// Vertical delimiter \G
        VerticalDelimiter,
        /// Compound identifiers, like a.b or tuple access operator a.1, (x, y).2.
        /// Need to be distinguished from floating point number with omitted integer part: .1
        Dot,
        /// Could be used as multiplication operator or on it's own: "SELECT *"
        Asterisk,
        HereDoc,
        DollarSign,
        Plus,
        Minus,
        Slash,
        Percent,
        /// ->. Should be distinguished from minus operator.
        Arrow,
        QuestionMark,
        Colon,
        Caret,
        DoubleColon,
        Equals,
        NotEquals,
        Less,
        Greater,
        LessOrEquals,
        GreaterOrEquals,
        /// <=>. Used in MySQL for NULL-safe equality comparison.
        Spaceship,
        PipeMark,
        /// |>. Pipe operator: FROM t |> WHERE x |> SELECT y
        PipeOperator,
        /// String concatenation operator: ||
        Concatenation,
        /// @. Used for specifying user names and also for MySQL-style variables.
        At,
        /// @@. Used for MySQL-style global variables.
        DoubleAt,
        /// Order is important. EndOfStream goes after all usual tokens,
        /// and special error tokens goes after EndOfStream.
        EndOfStream,
        /// Something unrecognized.
        Error,
        /// Something is wrong and we have more information.
        ErrorMultilineCommentIsNotClosed,
        ErrorSingleQuoteIsNotClosed,
        ErrorDoubleQuoteIsNotClosed,
        ErrorBackQuoteIsNotClosed,
        ErrorSingleExclamationMark,
        ErrorSinglePipeMark,
        ErrorWrongNumber,
        ErrorMaxQuerySizeExceeded,
        KMaxTokenType,
    };

    const char* get_token_name(TokenType type);
    const char* get_error_token_description(TokenType type);

    struct Token {
        TokenType type;
        const char* begin;
        const char* end;

        size_t size() const { return end - begin; }

        Token() = default;
        Token(TokenType type_, const char* begin_, const char* end_)
            : type(type_)
            , begin(begin_)
            , end(end_) { }

        bool is_significant() const {
            return type != TokenType::Whitespace && type != TokenType::Comment;
        }
        bool is_error() const {
            return type > TokenType::EndOfStream;
        }
        bool is_end() const {
            return type == TokenType::EndOfStream;
        }
    };

    class ExpressionLexer {
    public:
        ExpressionLexer(const char* begin_, const char* end_, size_t max_query_size_ = 0)
            : _begin(begin_)
            , _pos(begin_)
            , _end(end_)
            , _max_query_size(max_query_size_ <= kMaxQuerySizeLimit ? max_query_size_ : kMaxQuerySizeLimit) {
        }

        explicit ExpressionLexer(std::string_view query, size_t max_query_size_ = 0)
            : ExpressionLexer(query.data(), query.data() + query.size(), max_query_size_) {
        }
        Token next_token();

    private:
        const char* const _begin;
        const char* _pos;
        const char* const _end;

        const size_t _max_query_size;

        /// Some reasonable size to at least avoid pointer overflows.
        static constexpr size_t kMaxQuerySizeLimit = 1'000'000'000;

        Token next_token_impl();

        /// This is needed to disambiguate tuple access operator from floating point number (.1).
        TokenType prev_significant_token_type = TokenType::Whitespace; /// No previous token.
    };


    template<typename Core = ExpressionLexer>
    class Lexer : public Core{
    public:
        Lexer(const char* begin, const char* end, size_t max_query_size = 0)
            : Core(begin, end, max_query_size) {

        }

        explicit Lexer(std::string_view query, size_t max_query_size = 0)
           : Lexer(query.data(), query.data() + query.size(), max_query_size) {
        }

        using Core::next_token;
    };
} // namespace turbo
