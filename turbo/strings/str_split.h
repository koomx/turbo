//
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
//
// -----------------------------------------------------------------------------
// File: str_split.h
// -----------------------------------------------------------------------------
//
// This file contains functions for splitting strings. It defines the main
// `str_split()` function, several delimiters for determining the boundaries on
// which to split the string, and predicates for filtering delimited results.
// `str_split()` adapts the returned collection to the type specified by the
// caller.
//
// Example:
//
//   // Splits the given string on commas. Returns the results in a
//   // vector of strings.
//   std::vector<std::string> v = turbo::str_split("a,b,c", ',');
//   // Can also use ","
//   // v[0] == "a", v[1] == "b", v[2] == "c"
//
// See str_split() below for more information.
#ifndef TURBO_STRINGS_STR_SPLIT_H_
#define TURBO_STRINGS_STR_SPLIT_H_

#include <algorithm>
#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <string_view>
#include <turbo/base/internal/raw_logging.h>
#include <turbo/macros/config.h>
#include <turbo/strings/internal/str_split_internal.h>
#include <turbo/strings/strip.h>
#include <turbo/strings/algo.h>

namespace turbo {

    //------------------------------------------------------------------------------
    // Delimiters
    //------------------------------------------------------------------------------
    //
    // `str_split()` uses delimiters to define the boundaries between elements in the
    // provided input. Several `Delimiter` types are defined below. If a string
    // (`const char*`, `std::string`, or `std::string_view`) is passed in place of
    // an explicit `Delimiter` object, `str_split()` treats it the same way as if it
    // were passed a `ByString` delimiter.
    //
    // A `Delimiter` is an object with a `Find()` function that knows how to find
    // the first occurrence of itself in a given `std::string_view`.
    //
    // The following `Delimiter` types are available for use within `str_split()`:
    //
    //   - `ByString` (default for string arguments)
    //   - `ByChar` (default for a char argument)
    //   - `ByAnyChar`
    //   - `ByLength`
    //   - `MaxSplits`
    //
    // A Delimiter's `Find()` member function will be passed an input `text` that is
    // to be split and a position (`pos`) to begin searching for the next delimiter
    // in `text`. The returned std::string_view should refer to the next occurrence
    // (after `pos`) of the represented delimiter; this returned std::string_view
    // represents the next location where the input `text` should be broken.
    //
    // The returned std::string_view may be zero-length if the Delimiter does not
    // represent a part of the string (e.g., a fixed-length delimiter). If no
    // delimiter is found in the input `text`, a zero-length std::string_view
    // referring to `text.end()` should be returned (e.g.,
    // `text.substr(text.size())`). It is important that the returned
    // std::string_view always be within the bounds of the input `text` given as an
    // argument--it must not refer to a string that is physically located outside of
    // the given string.
    //
    // The following example is a simple Delimiter object that is created with a
    // single char and will look for that char in the text passed to the `Find()`
    // function:
    //
    //   struct SimpleDelimiter {
    //     const char c_;
    //     explicit SimpleDelimiter(char c) : c_(c) {}
    //     std::string_view Find(std::string_view text, size_t pos) {
    //       auto found = text.find(c_, pos);
    //       if (found == std::string_view::npos)
    //         return text.substr(text.size());
    //
    //       return text.substr(found, 1);
    //     }
    //   };

    namespace strings_internal {

        // A traits-like metafunction for selecting the default Delimiter object type
        // for a particular Delimiter type. The base case simply exposes type Delimiter
        // itself as the delimiter's Type. However, there are specializations for
        // string-like objects that map them to the ByString delimiter object.
        // This allows functions like turbo::str_split() and turbo::MaxSplits() to accept
        // string-like objects (e.g., ',') as delimiter arguments but they will be
        // treated as if a ByString delimiter was given.
        template <typename Delimiter>
        struct SelectDelimiter {
            using type = Delimiter;
        };

        template <>
        struct SelectDelimiter<char> {
            using type = ByChar;
        };
        template <>
        struct SelectDelimiter<char*> {
            using type = ByString;
        };
        template <>
        struct SelectDelimiter<const char*> {
            using type = ByString;
        };
        template <>
        struct SelectDelimiter<std::string_view> {
            using type = ByString;
        };
        template <>
        struct SelectDelimiter<std::string> {
            using type = ByString;
        };

        // Wraps another delimiter and sets a max number of matches for that delimiter.
        template <typename Delimiter>
        class MaxSplitsImpl {
        public:
            MaxSplitsImpl(Delimiter delimiter, int limit)
                : delimiter_(std::move(delimiter))
                , limit_(limit)
                , count_(0) { }
            std::string_view Find(std::string_view text, size_t pos) {
                if (count_++ == limit_) {
                    return std::string_view(text.data() + text.size(),
                        0); // No more matches.
                }
                return delimiter_.Find(text, pos);
            }

        private:
            Delimiter delimiter_;
            const int limit_;
            int count_;
        };

    } // namespace strings_internal

    // MaxSplits()
    //
    // A delimiter that limits the number of matches which can occur to the passed
    // `limit`. The last element in the returned collection will contain all
    // remaining unsplit pieces, which may contain instances of the delimiter.
    // The collection will contain at most `limit` + 1 elements.
    // Example:
    //
    //   using turbo::MaxSplits;
    //   std::vector<std::string> v = turbo::str_split("a,b,c", MaxSplits(',', 1));
    //
    //   // v[0] == "a", v[1] == "b,c"
    template <typename Delimiter>
    inline strings_internal::MaxSplitsImpl<
        typename strings_internal::SelectDelimiter<Delimiter>::type>
    MaxSplits(Delimiter delimiter, int limit) {
        typedef
            typename strings_internal::SelectDelimiter<Delimiter>::type DelimiterType;
        return strings_internal::MaxSplitsImpl<DelimiterType>(
            DelimiterType(delimiter), limit);
    }

    //------------------------------------------------------------------------------
    // Predicates
    //------------------------------------------------------------------------------
    //
    // Predicates filter the results of a `str_split()` by determining whether or not
    // a resultant element is included in the result set. A predicate may be passed
    // as an optional third argument to the `str_split()` function.
    //
    // Predicates are unary functions (or functors) that take a single
    // `std::string_view` argument and return a bool indicating whether the
    // argument should be included (`true`) or excluded (`false`).
    //
    // Predicates are useful when filtering out empty substrings. By default, empty
    // substrings may be returned by `str_split()`, which is similar to the way split
    // functions work in other programming languages.

    // AllowEmpty()
    //
    // Always returns `true`, indicating that all strings--including empty
    // strings--should be included in the split output. This predicate is not
    // strictly needed because this is the default behavior of `str_split()`;
    // however, it might be useful at some call sites to make the intent explicit.
    //
    // Example:
    //
    //  std::vector<std::string> v = turbo::str_split(" a , ,,b,", ',', AllowEmpty());
    //
    //  // v[0] == " a ", v[1] == " ", v[2] == "", v[3] = "b", v[4] == ""
    struct AllowEmpty {
        bool operator()(std::string_view) const { return true; }
    };

    // SkipEmpty()
    //
    // Returns `false` if the given `std::string_view` is empty, indicating that
    // `str_split()` should omit the empty string.
    //
    // Example:
    //
    //   std::vector<std::string> v = turbo::str_split(",a,,b,", ',', SkipEmpty());
    //
    //   // v[0] == "a", v[1] == "b"
    //
    // Note: `SkipEmpty()` does not consider a string containing only whitespace
    // to be empty. To skip such whitespace as well, use the `SkipWhitespace()`
    // predicate.
    struct SkipEmpty {
        bool operator()(std::string_view sp) const { return !sp.empty(); }
    };

    // SkipWhitespace()
    //
    // Returns `false` if the given `std::string_view` is empty *or* contains only
    // whitespace, indicating that `str_split()` should omit the string.
    //
    // Example:
    //
    //   std::vector<std::string> v = turbo::str_split(" a , ,,b,",
    //                                               ',', SkipWhitespace());
    //   // v[0] == " a ", v[1] == "b"
    //
    //   // SkipEmpty() would return whitespace elements
    //   std::vector<std::string> v = turbo::str_split(" a , ,,b,", ',', SkipEmpty());
    //   // v[0] == " a ", v[1] == " ", v[2] == "b"
    struct SkipWhitespace {
        bool operator()(std::string_view sp) const {
            sp = turbo::trim_left(sp);
            return !sp.empty();
        }
    };

    template <typename T>
    using EnableSplitIfString = std::enable_if_t<std::is_same_v<T, std::string> || std::is_same_v<T, const std::string>,
        int>;

    //------------------------------------------------------------------------------
    //                                  str_split()
    //------------------------------------------------------------------------------

    // str_split()
    //
    // Splits a string into a sequence of substrings identified by `Delimiter`. The
    // input is processed sequentially from beginning to end, and each resulting
    // substring is filtered by an optional `Predicate` before inclusion in the
    // result set. `str_split()` returns a lazy range that preserves the substrings
    // original order and is convertible to the collection type specified by the
    // caller.
    //
    // Optionally, you may pass a `Predicate` to `str_split()` indicating whether to
    // include or exclude the resulting element within the final result set. (See
    // the overviews for Delimiters and Predicates above.)
    //
    // Example:
    //
    //   std::vector<std::string> v = turbo::str_split("a,b,c,d", ',');
    //   // v[0] == "a", v[1] == "b", v[2] == "c", v[3] == "d"
    //
    // You can also provide an explicit `Delimiter` object:
    //
    // Example:
    //
    //   using turbo::ByAnyChar;
    //   std::vector<std::string> v = turbo::str_split("a,b=c", ByAnyChar(",="));
    //   // v[0] == "a", v[1] == "b", v[2] == "c"
    //
    // See above for more information on delimiters.
    //
    // By default, empty strings are included in the result set. You can optionally
    // include a third `Predicate` argument to apply a test for whether the
    // resultant element should be included in the result set:
    //
    // Example:
    //
    //   std::vector<std::string> v = turbo::str_split(" a , ,,b,",
    //                                               ',', SkipWhitespace());
    //   // v[0] == " a ", v[1] == "b"
    //
    // See above for more information on predicates.
    //
    //------------------------------------------------------------------------------
    // str_split() Return Types
    //------------------------------------------------------------------------------
    //
    // The `str_split()` function adapts the returned collection to the collection
    // specified by the caller (e.g. `std::vector` above). The returned collections
    // may contain `std::string`, `std::string_view` (in which case the original
    // string being split must ensure that it outlives the collection), or any
    // object that can be explicitly created from an `std::string_view`. This
    // behavior works for:
    //
    // 1) All standard STL containers including `std::vector`, `std::list`,
    //    `std::deque`, `std::set`,`std::multiset`, 'std::map`, and `std::multimap`.
    // 2) `std::pair` (which is not actually a container). See below.
    // 3) `std::array`, which is a container but has different behavior due to its
    //    fixed size. See below.
    //
    // Example:
    //
    //   // The results are returned as `std::string_view` objects. Note that we
    //   // have to ensure that the input string outlives any results.
    //   std::vector<std::string_view> v = turbo::str_split("a,b,c", ',');
    //
    //   // Stores results in a std::set<std::string>, which also performs
    //   // de-duplication and orders the elements in ascending order.
    //   std::set<std::string> a = turbo::str_split("b,a,c,a,b", ',');
    //   // a[0] == "a", a[1] == "b", a[2] == "c"
    //
    //   // `str_split()` can be used within a range-based for loop, in which case
    //   // each element will be of type `std::string_view`.
    //   std::vector<std::string> v;
    //   for (const auto sv : turbo::str_split("a,b,c", ',')) {
    //     if (sv != "b") v.emplace_back(sv);
    //   }
    //   // v[0] == "a", v[1] == "c"
    //
    //   // Stores results in a map. The map implementation assumes that the input
    //   // is provided as a series of key/value pairs. For example, the 0th element
    //   // resulting from the split will be stored as a key to the 1st element. If
    //   // an odd number of elements are resolved, the last element is paired with
    //   // a default-constructed value (e.g., empty string).
    //   std::map<std::string, std::string> m = turbo::str_split("a,b,c", ',');
    //   // m["a"] == "b", m["c"] == ""     // last component value equals ""
    //
    // Splitting to `std::pair` is an interesting case because it can hold only two
    // elements and is not a collection type. When splitting to a `std::pair` the
    // first two split strings become the `std::pair` `.first` and `.second`
    // members, respectively. The remaining split substrings are discarded. If there
    // are less than two split substrings, the empty string is used for the
    // corresponding `std::pair` member.
    //
    // Example:
    //
    //   // Stores first two split strings as the members in a std::pair.
    //   std::pair<std::string, std::string> p = turbo::str_split("a,b,c", ',');
    //   // p.first == "a", p.second == "b"       // "c" is omitted.
    //
    //
    // Splitting to `std::array` is similar to splitting to `std::pair`, but for
    // N elements instead of two; missing elements are filled with the empty string
    // and extra elements are discarded.
    //
    // Examples:
    //
    //   // Stores first two split strings as the elements in a std::array.
    //   std::array<std::string, 2> a = turbo::str_split("a,b,c", ',');
    //   // a[0] == "a", a[1] == "b"   // "c" is omitted.
    //
    //   // The second element is empty.
    //   std::array<std::string, 2> a = turbo::str_split("a,", ',');
    //   // a[0] == "a", a[1] == ""
    //
    // The `str_split()` function can be used multiple times to perform more
    // complicated splitting logic, such as intelligently parsing key-value pairs.
    //
    // Example:
    //
    //   // The input string "a=b=c,d=e,f=,g" becomes
    //   // { "a" => "b=c", "d" => "e", "f" => "", "g" => "" }
    //   std::map<std::string, std::string> m;
    //   for (std::string_view sp : turbo::str_split("a=b=c,d=e,f=,g", ',')) {
    //     m.insert(turbo::str_split(sp, turbo::MaxSplits('=', 1)));
    //   }
    //   EXPECT_EQ("b=c", m.find("a")->second);
    //   EXPECT_EQ("e", m.find("d")->second);
    //   EXPECT_EQ("", m.find("f")->second);
    //   EXPECT_EQ("", m.find("g")->second);
    //
    // WARNING: Due to a legacy bug that is maintained for backward compatibility,
    // splitting the following empty string_views produces different results:
    //
    //   turbo::str_split(std::string_view(""), '-');  // {""}
    //   turbo::str_split(std::string_view(), '-');    // {}, but should be {""}
    //
    // Try not to depend on this distinction because the bug may one day be fixed.
    template <typename Delimiter>
    strings_internal::Splitter<
        typename strings_internal::SelectDelimiter<Delimiter>::type, AllowEmpty,
        std::string_view>
    str_split(strings_internal::ConvertibleToStringView text, Delimiter d) {
        using DelimiterType =
            typename strings_internal::SelectDelimiter<Delimiter>::type;
        return strings_internal::Splitter<DelimiterType, AllowEmpty,
            std::string_view>(
            text.value(), DelimiterType(d), AllowEmpty());
    }

    template <typename Delimiter, typename StringType,
        EnableSplitIfString<StringType> = 0>
    strings_internal::Splitter<
        typename strings_internal::SelectDelimiter<Delimiter>::type, AllowEmpty,
        std::string>
    str_split(StringType&& text, Delimiter d) {
        using DelimiterType =
            typename strings_internal::SelectDelimiter<Delimiter>::type;
        return strings_internal::Splitter<DelimiterType, AllowEmpty, std::string>(
            std::move(text), DelimiterType(d), AllowEmpty());
    }

    template <typename Delimiter, typename Predicate>
    strings_internal::Splitter<
        typename strings_internal::SelectDelimiter<Delimiter>::type, Predicate,
        std::string_view>
    str_split(strings_internal::ConvertibleToStringView text, Delimiter d,
        Predicate p) {
        using DelimiterType =
            typename strings_internal::SelectDelimiter<Delimiter>::type;
        return strings_internal::Splitter<DelimiterType, Predicate,
            std::string_view>(
            text.value(), DelimiterType(std::move(d)), std::move(p));
    }

    template <typename Delimiter, typename Predicate, typename StringType,
        EnableSplitIfString<StringType> = 0>
    strings_internal::Splitter<
        typename strings_internal::SelectDelimiter<Delimiter>::type, Predicate,
        std::string>
    str_split(StringType&& text, Delimiter d, Predicate p) {
        using DelimiterType =
            typename strings_internal::SelectDelimiter<Delimiter>::type;
        return strings_internal::Splitter<DelimiterType, Predicate, std::string>(
            std::move(text), DelimiterType(d), std::move(p));
    }

} // namespace turbo

#endif // TURBO_STRINGS_STR_SPLIT_H_
