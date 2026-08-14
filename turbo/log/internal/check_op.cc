// Copyright 2022 The Abseil Authors.
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

#include <turbo/log/internal/check_op.h>

#include <cstdint>
#include <cstring>
#include <ostream>
#include <string>
#include <utility>

#include <turbo/macros/config.h>
#include <turbo/base/nullability.h>
#include <turbo/debugging/leak_check.h>
#include <turbo/strings/str_cat.h>
#include <string_view>

#ifdef _MSC_VER
#define strcasecmp _stricmp
#else
#include <strings.h>  // for strcasecmp, but msvc does not have this header
#endif

namespace turbo {
    namespace log_internal {
#define TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(x) \
  template const char* turbo_nonnull make_check_op_string(   \
      x, x, const char* turbo_nonnull)
        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(bool);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(int64_t);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(uint64_t);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(float);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(double);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(char);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(unsigned char);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(const std::string&);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(const std::string_view&);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(const char*);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(const signed char*);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(const unsigned char*);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(const void*);
#undef TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING

        CheckOpMessageBuilder::CheckOpMessageBuilder(
            const char * turbo_nonnull exprtext) {
            stream_ << exprtext << " (";
        }

        std::ostream &CheckOpMessageBuilder::ForVar2() {
            stream_ << " vs. ";
            return stream_;
        }

        const char * turbo_nonnull CheckOpMessageBuilder::NewString() {
            stream_ << ")";
            // There's no need to free this string since the process is crashing.
            return turbo::IgnoreLeak(new std::string(std::move(stream_).str()))->c_str();
        }

        void make_check_op_value_string(std::ostream &os, const char v) {
            if (v >= 32 && v <= 126) {
                os << "'" << v << "'";
            } else {
                os << "char value " << int{v};
            }
        }

        void make_check_op_value_string(std::ostream &os, const signed char v) {
            if (v >= 32 && v <= 126) {
                os << "'" << v << "'";
            } else {
                os << "signed char value " << int{v};
            }
        }

        void make_check_op_value_string(std::ostream &os, const unsigned char v) {
            if (v >= 32 && v <= 126) {
                os << "'" << v << "'";
            } else {
                os << "unsigned char value " << int{v};
            }
        }

        void make_check_op_value_string(std::ostream &os, const void *p) {
            if (p == nullptr) {
                os << "(null)";
            } else {
                os << p;
            }
        }

        std::ostream &operator<<(std::ostream &os, UnprintableWrapper) {
            return os << "UNPRINTABLE";
        }

        // Helper functions for string comparisons.
#define DEFINE_CHECK_STROP_IMPL(name, func, expected)                          \
  const char* turbo_nullable Check##func##expected##Impl(                       \
      const char* turbo_nullable s1, const char* turbo_nullable s2,              \
      const char* turbo_nonnull exprtext) {                                     \
    bool equal = s1 == s2 || (s1 && s2 && !func(s1, s2));                      \
    if (equal == expected) {                                                   \
      return nullptr;                                                          \
    } else {                                                                   \
      /* There's no need to free this string since the process is crashing. */ \
      return turbo::IgnoreLeak(new std::string(turbo::str_cat(exprtext, " (", s1, \
                                                           " vs. ", s2, ")"))) \
          ->c_str();                                                           \
    }                                                                          \
  }
        DEFINE_CHECK_STROP_IMPL(KCHECK_STREQ, strcmp, true)
        DEFINE_CHECK_STROP_IMPL(KCHECK_STRNE, strcmp, false)
        DEFINE_CHECK_STROP_IMPL(KCHECK_STRCASEEQ, strcasecmp, true)
        DEFINE_CHECK_STROP_IMPL(KCHECK_STRCASENE, strcasecmp, false)
#undef DEFINE_CHECK_STROP_IMPL

        namespace detect_specialization {
            StringifySink::StringifySink(std::ostream &os) : os_(os) {
            }

            void StringifySink::Append(std::string_view text) { os_ << text; }

            void StringifySink::Append(size_t length, char ch) {
                for (size_t i = 0; i < length; ++i) os_.put(ch);
            }

            void TurboFormatFlush(StringifySink *sink, std::string_view text) {
                sink->Append(text);
            }
        } // namespace detect_specialization
    } // namespace log_internal
} // namespace turbo
