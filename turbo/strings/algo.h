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

#include <turbo/strings/ascii.h>
#include <string>
#include <string_view>

namespace turbo {

    // ByString
    //
    // A sub-string delimiter. If `str_split()` is passed a string in place of a
    // `Delimiter` object, the string will be implicitly converted into a
    // `ByString` delimiter.
    //
    // Example:
    //
    //   // Because a string literal is converted to an `turbo::ByString`,
    //   // the following two splits are equivalent.
    //
    //   std::vector<std::string> v1 = turbo::str_split("a, b, c", ", ");
    //
    //   using turbo::ByString;
    //   std::vector<std::string> v2 = turbo::str_split("a, b, c",
    //                                                ByString(", "));
    //   // v[0] == "a", v[1] == "b", v[2] == "c"
    class ByString {
    public:
        explicit ByString(std::string_view sp);
        std::string_view Find(std::string_view text, size_t pos) const;

    private:
        std::string delimiter_;
    };

    // ByAsciiWhitespace
    //
    // A sub-string delimiter that splits by ASCII whitespace
    // (space, tab, vertical tab, formfeed, linefeed, or carriage return).
    // Note: you probably want to use turbo::SkipEmpty() as well!
    //
    // This class is equivalent to ByAnyChar with ASCII whitespace chars.
    //
    // Example:
    //
    //   std::vector<std::string> v = turbo::str_split(
    //       "a b\tc\n  d  \n", turbo::ByAsciiWhitespace(), turbo::SkipEmpty());
    //   // v[0] == "a", v[1] == "b", v[2] == "c", v[3] == "d"
    class ByAsciiWhitespace {
    public:
        std::string_view Find(std::string_view text, size_t pos) const;
    };

    // ByChar
    //
    // A single character delimiter. `ByChar` is functionally equivalent to a
    // 1-char string within a `ByString` delimiter, but slightly more efficient.
    //
    // Example:
    //
    //   // Because a char literal is converted to a turbo::ByChar,
    //   // the following two splits are equivalent.
    //   std::vector<std::string> v1 = turbo::str_split("a,b,c", ',');
    //   using turbo::ByChar;
    //   std::vector<std::string> v2 = turbo::str_split("a,b,c", ByChar(','));
    //   // v[0] == "a", v[1] == "b", v[2] == "c"
    //
    // `ByChar` is also the default delimiter if a single character is given
    // as the delimiter to `str_split()`. For example, the following calls are
    // equivalent:
    //
    //   std::vector<std::string> v = turbo::str_split("a-b", '-');
    //
    //   using turbo::ByChar;
    //   std::vector<std::string> v = turbo::str_split("a-b", ByChar('-'));
    //
    class ByChar {
    public:
        explicit ByChar(char c)
            : c_(c) { }
        std::string_view Find(std::string_view text, size_t pos) const;

    private:
        char c_;
    };

    // ByAnyChar
    //
    // A delimiter that will match any of the given byte-sized characters within
    // its provided string.
    //
    // Note: this delimiter works with single-byte string data, but does not work
    // with variable-width encodings, such as UTF-8.
    //
    // Example:
    //
    //   using turbo::ByAnyChar;
    //   std::vector<std::string> v = turbo::str_split("a,b=c", ByAnyChar(",="));
    //   // v[0] == "a", v[1] == "b", v[2] == "c"
    //
    // If `ByAnyChar` is given the empty string, it behaves exactly like
    // `ByString` and matches each individual character in the input string.
    //
    class ByAnyChar {
    public:
        explicit ByAnyChar(std::string_view sp);
        std::string_view Find(std::string_view text, size_t pos) const;

    private:
        const std::string delimiters_;
    };


    // ByLength
    //
    // A delimiter for splitting into equal-length strings. The length argument to
    // the constructor must be greater than 0.
    //
    // Note: this delimiter works with single-byte string data, but does not work
    // with variable-width encodings, such as UTF-8.
    //
    // Example:
    //
    //   using turbo::ByLength;
    //   std::vector<std::string> v = turbo::str_split("123456789", ByLength(3));

    //   // v[0] == "123", v[1] == "456", v[2] == "789"
    //
    // Note that the string does not have to be a multiple of the fixed split
    // length. In such a case, the last substring will be shorter.
    //
    //   using turbo::ByLength;
    //   std::vector<std::string> v = turbo::str_split("12345", ByLength(2));
    //
    //   // v[0] == "12", v[1] == "34", v[2] == "5"
    class ByLength {
    public:
        explicit ByLength(ptrdiff_t length);
        std::string_view Find(std::string_view text, size_t pos) const;

    private:
        const ptrdiff_t length_;
    };


}  // namespace turbo
