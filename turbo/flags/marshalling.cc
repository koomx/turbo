//
//  Copyright 2019 The Abseil Authors.
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

#include <turbo/flags/marshalling.h>

#include <stddef.h>

#include <cmath>
#include <limits>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include <string_view>
#include <turbo/base/log_severity.h>
#include <turbo/format/str_format.h>
#include <turbo/macros/config.h>
#include <turbo/numeric/int128.h>
#include <turbo/strings/ascii.h>
#include <turbo/strings/match.h>
#include <turbo/strings/numbers.h>
#include <turbo/strings/str_cat.h>
#include <turbo/strings/str_join.h>
#include <turbo/strings/str_split.h>

namespace turbo {

    namespace flags_internal {

        // --------------------------------------------------------------------
        // turbo_parse_flag specializations for boolean type.

        bool turbo_parse_flag(std::string_view text, bool* dst, std::string*) {
            return SimpleAtob(turbo::trim_all(text), dst);
        }

        // --------------------------------------------------------------------
        // turbo_parse_flag for integral types.

        // Return the base to use for parsing text as an integer.  Leading 0x
        // puts us in base 16.  But leading 0 does not put us in base 8. It
        // caused too many bugs when we had that behavior.
        static int NumericBase(std::string_view text) {
            if (text.empty())
                return 0;
            size_t num_start = (text[0] == '-' || text[0] == '+') ? 1 : 0;
            const bool hex = (text.size() >= num_start + 2 && text[num_start] == '0' && (text[num_start + 1] == 'x' || text[num_start + 1] == 'X'));
            return hex ? 16 : 10;
        }

        template <typename IntType>
        inline bool ParseFlagImpl(std::string_view text, IntType& dst) {
            text = turbo::trim_all(text);

            return turbo::numbers_internal::safe_strtoi_base(text, &dst,
                NumericBase(text));
        }

        bool turbo_parse_flag(std::string_view text, short* dst, std::string*) {
            int val;
            if (!ParseFlagImpl(text, val))
                return false;
            if (static_cast<short>(val) != val) // worked, but number out of range
                return false;
            *dst = static_cast<short>(val);
            return true;
        }

        bool turbo_parse_flag(std::string_view text, unsigned short* dst, std::string*) {
            unsigned int val;
            if (!ParseFlagImpl(text, val))
                return false;
            if (static_cast<unsigned short>(val) != val) // worked, but number out of range
                return false;
            *dst = static_cast<unsigned short>(val);
            return true;
        }

        bool turbo_parse_flag(std::string_view text, int* dst, std::string*) {
            return ParseFlagImpl(text, *dst);
        }

        bool turbo_parse_flag(std::string_view text, unsigned int* dst, std::string*) {
            return ParseFlagImpl(text, *dst);
        }

        bool turbo_parse_flag(std::string_view text, long* dst, std::string*) {
            return ParseFlagImpl(text, *dst);
        }

        bool turbo_parse_flag(std::string_view text, unsigned long* dst, std::string*) {
            return ParseFlagImpl(text, *dst);
        }

        bool turbo_parse_flag(std::string_view text, long long* dst, std::string*) {
            return ParseFlagImpl(text, *dst);
        }

        bool turbo_parse_flag(std::string_view text, unsigned long long* dst,
            std::string*) {
            return ParseFlagImpl(text, *dst);
        }

        bool turbo_parse_flag(std::string_view text, turbo::int128* dst, std::string*) {
            text = turbo::trim_all(text);

            // check hex
            int base = NumericBase(text);
            if (!turbo::numbers_internal::safe_strto128_base(text, dst, base)) {
                return false;
            }

            return base == 16 ? turbo::SimpleHexAtoi(text, dst)
                              : turbo::SimpleAtoi(text, dst);
        }

        bool turbo_parse_flag(std::string_view text, turbo::uint128* dst, std::string*) {
            text = turbo::trim_all(text);

            // check hex
            int base = NumericBase(text);
            if (!turbo::numbers_internal::safe_strtou128_base(text, dst, base)) {
                return false;
            }

            return base == 16 ? turbo::SimpleHexAtoi(text, dst)
                              : turbo::SimpleAtoi(text, dst);
        }

        // --------------------------------------------------------------------
        // turbo_parse_flag for floating point types.

        bool turbo_parse_flag(std::string_view text, float* dst, std::string*) {
            return turbo::SimpleAtof(text, dst);
        }

        bool turbo_parse_flag(std::string_view text, double* dst, std::string*) {
            return turbo::SimpleAtod(text, dst);
        }

        // --------------------------------------------------------------------
        // turbo_parse_flag for strings.

        bool turbo_parse_flag(std::string_view text, std::string* dst, std::string*) {
            dst->assign(text.data(), text.size());
            return true;
        }

        // --------------------------------------------------------------------
        // turbo_parse_flag for vector of strings.

        bool turbo_parse_flag(std::string_view text, std::vector<std::string>* dst,
            std::string*) {
            // An empty flag value corresponds to an empty vector, not a vector
            // with a single, empty std::string.
            if (text.empty()) {
                dst->clear();
                return true;
            }
            *dst = turbo::str_split(text, ',', turbo::AllowEmpty());
            return true;
        }

        // --------------------------------------------------------------------
        // turbo_unparse_flag specializations for various builtin flag types.

        std::string Unparse(bool v) {
            return v ? "true" : "false";
        }
        std::string Unparse(short v) {
            return turbo::str_cat(v);
        }
        std::string Unparse(unsigned short v) {
            return turbo::str_cat(v);
        }
        std::string Unparse(int v) {
            return turbo::str_cat(v);
        }
        std::string Unparse(unsigned int v) {
            return turbo::str_cat(v);
        }
        std::string Unparse(long v) {
            return turbo::str_cat(v);
        }
        std::string Unparse(unsigned long v) {
            return turbo::str_cat(v);
        }
        std::string Unparse(long long v) {
            return turbo::str_cat(v);
        }
        std::string Unparse(unsigned long long v) {
            return turbo::str_cat(v);
        }
        std::string Unparse(turbo::int128 v) {
            std::stringstream ss;
            ss << v;
            return ss.str();
        }
        std::string Unparse(turbo::uint128 v) {
            std::stringstream ss;
            ss << v;
            return ss.str();
        }

        template <typename T>
        std::string UnparseFloatingPointVal(T v) {
            // digits10 is guaranteed to roundtrip correctly in string -> value -> string
            // conversions, but may not be enough to represent all the values correctly.
            std::string digit10_str = turbo::str_sprintf("%.*g", std::numeric_limits<T>::digits10, v);
            if (std::isnan(v) || std::isinf(v))
                return digit10_str;

            T roundtrip_val = 0;
            std::string err;
            if (turbo::ParseFlag(digit10_str, &roundtrip_val, &err) && roundtrip_val == v) {
                return digit10_str;
            }

            // max_digits10 is the number of base-10 digits that are necessary to uniquely
            // represent all distinct values.
            return turbo::str_sprintf("%.*g", std::numeric_limits<T>::max_digits10, v);
        }
        std::string Unparse(float v) {
            return UnparseFloatingPointVal(v);
        }
        std::string Unparse(double v) {
            return UnparseFloatingPointVal(v);
        }
        std::string turbo_unparse_flag(std::string_view v) {
            return std::string(v);
        }
        std::string turbo_unparse_flag(const std::vector<std::string>& v) {
            return turbo::StrJoin(v, ",");
        }

    } // namespace flags_internal

    bool turbo_parse_flag(std::string_view text, turbo::LogSeverity* dst,
        std::string* err) {
        text = turbo::trim_all(text);
        if (text.empty()) {
            *err = "no value provided";
            return false;
        }
        if (turbo::equals_ignore_case(text, "dfatal")) {
            *dst = turbo::kLogDebugFatal;
            return true;
        }
        if (turbo::equals_ignore_case(text, "klogdebugfatal")) {
            *dst = turbo::kLogDebugFatal;
            return true;
        }
        if (text.front() == 'k' || text.front() == 'K')
            text.remove_prefix(1);
        if (turbo::equals_ignore_case(text, "trace")) {
            *dst = turbo::LogSeverity::kTrace;
            return true;
        }
        if (turbo::equals_ignore_case(text, "debug")) {
            *dst = turbo::LogSeverity::kDebug;
            return true;
        }
        if (turbo::equals_ignore_case(text, "info")) {
            *dst = turbo::LogSeverity::kInfo;
            return true;
        }
        if (turbo::equals_ignore_case(text, "warning")) {
            *dst = turbo::LogSeverity::kWarning;
            return true;
        }
        if (turbo::equals_ignore_case(text, "error")) {
            *dst = turbo::LogSeverity::kError;
            return true;
        }
        if (turbo::equals_ignore_case(text, "fatal")) {
            *dst = turbo::LogSeverity::kFatal;
            return true;
        }
        std::underlying_type_t<turbo::LogSeverity> numeric_value;
        if (turbo::ParseFlag(text, &numeric_value, err)) {
            *dst = static_cast<turbo::LogSeverity>(numeric_value);
            return true;
        }
        *err = "only integers, turbo::LogSeverity enumerators, and DFATAL are accepted";
        return false;
    }

    std::string turbo_unparse_flag(turbo::LogSeverity v) {
        if (v == turbo::NormalizeLogSeverity(v))
            return turbo::LogSeverityName(v);
        return turbo::UnparseFlag(static_cast<int>(v));
    }

} // namespace turbo
