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

#ifndef TURBO_BASE_LOG_SEVERITY_H_
#define TURBO_BASE_LOG_SEVERITY_H_

#include <array>
#include <ostream>

#include <turbo/macros/config.h>

namespace turbo {


// turbo::LogSeverity
//
// Six severity levels are defined. Logging APIs should terminate the program
// when a message is logged at severity `kFatal`; the other levels have no
// special semantics.
//
// TRACE/DEBUG are first-class severities gated by `min_log_level` (and compile
// thresholds). VKLOG remains severity INFO plus a separate verbosity knob — it
// is not the same as KLOG(DEBUG).
//
// Values other than the six defined levels (e.g. produced by `static_cast`)
// are valid, but their semantics when passed to a function, macro, or flag
// depend on the function, macro, or flag. The usual behavior is to normalize
// such values to a defined severity level, however in some cases values other
// than the defined levels are useful for comparison.
//
// Example:
//
//   // Effectively disables all logging:
//   set_min_log_level(static_cast<turbo::LogSeverity>(100));
//
// Abseil flags may be defined with type `LogSeverity`. Dependency layering
// constraints require that the `turbo_parse_flag()` overload be declared and
// defined in the flags library itself rather than here. The `turbo_unparse_flag()`
// overload is defined there as well for consistency.
//
// turbo::LogSeverity Flag String Representation
//
// An `turbo::LogSeverity` has a string representation used for parsing
// command-line flags based on the enumerator name (e.g. `kFatal`) or
// its unprefixed name (without the `k`) in any case-insensitive form. (E.g.
// "FATAL", "fatal" or "Fatal" are all valid.) Unparsing such flags produces an
// unprefixed string representation in all caps (e.g. "FATAL") or an integer.
//
// Additionally, the parser accepts arbitrary integers (as if the type were
// `int`). Prefer named values: integer `0` is TRACE (not INFO).
//
// Examples:
//
//   --my_log_level=kInfo
//   --my_log_level=INFO
//   --my_log_level=info
//   --my_log_level=2
//
// `DFATAL` and `kLogDebugFatal` are similarly accepted.
//
// Unparsing a flag produces the same result as `turbo::LogSeverityName()` for
// the standard levels and a base-ten integer otherwise.
enum class LogSeverity : int {
  kTrace = 0,
  kDebug = 1,
  kInfo = 2,
  kWarning = 3,
  kError = 4,
  kFatal = 5,
};

// LogSeverities()
//
// Returns an iterable of all standard `turbo::LogSeverity` values, ordered from
// least to most severe.
constexpr std::array<turbo::LogSeverity, 6> LogSeverities() {
  return {{turbo::LogSeverity::kTrace, turbo::LogSeverity::kDebug,
           turbo::LogSeverity::kInfo, turbo::LogSeverity::kWarning,
           turbo::LogSeverity::kError, turbo::LogSeverity::kFatal}};
}

// `turbo::kLogDebugFatal` equals `turbo::LogSeverity::kFatal` in debug builds
// (i.e. when `NDEBUG` is not defined) and `turbo::LogSeverity::kError`
// otherwise.  Avoid ODR-using this variable as it has internal linkage and thus
// distinct storage in different TUs.
#ifdef NDEBUG
static constexpr turbo::LogSeverity kLogDebugFatal = turbo::LogSeverity::kError;
#else
static constexpr turbo::LogSeverity kLogDebugFatal = turbo::LogSeverity::kFatal;
#endif

// LogSeverityName()
//
// Returns the all-caps string representation (e.g. "INFO") of the specified
// severity level if it is one of the standard levels and "UNKNOWN" otherwise.
constexpr const char* LogSeverityName(turbo::LogSeverity s) {
  switch (s) {
    case turbo::LogSeverity::kTrace: return "TRACE";
    case turbo::LogSeverity::kDebug: return "DEBUG";
    case turbo::LogSeverity::kInfo: return "INFO";
    case turbo::LogSeverity::kWarning: return "WARNING";
    case turbo::LogSeverity::kError: return "ERROR";
    case turbo::LogSeverity::kFatal: return "FATAL";
  }
  return "UNKNOWN";
}

// NormalizeLogSeverity()
//
// Values less than `kTrace` normalize to `kTrace`; values greater than `kFatal`
// normalize to `kError` (**NOT** `kFatal`).
constexpr turbo::LogSeverity NormalizeLogSeverity(turbo::LogSeverity s) {
  turbo::LogSeverity n = s;
  if (n < turbo::LogSeverity::kTrace) n = turbo::LogSeverity::kTrace;
  if (n > turbo::LogSeverity::kFatal) n = turbo::LogSeverity::kError;
  return n;
}
constexpr turbo::LogSeverity NormalizeLogSeverity(int s) {
  return turbo::NormalizeLogSeverity(static_cast<turbo::LogSeverity>(s));
}

// operator<<
//
// The exact representation of a streamed `turbo::LogSeverity` is deliberately
// unspecified; do not rely on it.
std::ostream& operator<<(std::ostream& os, turbo::LogSeverity s);

// Enums representing a lower bound for LogSeverity. APIs that only operate on
// messages of at least a certain level (for example, `set_min_log_level()`) use
// this type to specify that level. turbo::LogSeverityAtLeast::kInfinity is
// a level above all threshold levels and therefore no log message will
// ever meet this threshold.
enum class LogSeverityAtLeast : int {
  kTrace = static_cast<int>(turbo::LogSeverity::kTrace),
  kDebug = static_cast<int>(turbo::LogSeverity::kDebug),
  kInfo = static_cast<int>(turbo::LogSeverity::kInfo),
  kWarning = static_cast<int>(turbo::LogSeverity::kWarning),
  kError = static_cast<int>(turbo::LogSeverity::kError),
  kFatal = static_cast<int>(turbo::LogSeverity::kFatal),
  kInfinity = 1000,
};

std::ostream& operator<<(std::ostream& os, turbo::LogSeverityAtLeast s);

// Enums representing an upper bound for LogSeverity. APIs that only operate on
// messages of at most a certain level (for example, buffer all messages at or
// below a certain level) use this type to specify that level.
// turbo::LogSeverityAtMost::kNegativeInfinity is a level below all threshold
// levels and therefore will exclude all log messages.
enum class LogSeverityAtMost : int {
  kNegativeInfinity = -1000,
  kTrace = static_cast<int>(turbo::LogSeverity::kTrace),
  kDebug = static_cast<int>(turbo::LogSeverity::kDebug),
  kInfo = static_cast<int>(turbo::LogSeverity::kInfo),
  kWarning = static_cast<int>(turbo::LogSeverity::kWarning),
  kError = static_cast<int>(turbo::LogSeverity::kError),
  kFatal = static_cast<int>(turbo::LogSeverity::kFatal),
};

std::ostream& operator<<(std::ostream& os, turbo::LogSeverityAtMost s);

#define COMPOP(op1, op2, T)                                         \
  constexpr bool operator op1(turbo::T lhs, turbo::LogSeverity rhs) { \
    return static_cast<turbo::LogSeverity>(lhs) op1 rhs;             \
  }                                                                 \
  constexpr bool operator op2(turbo::LogSeverity lhs, turbo::T rhs) { \
    return lhs op2 static_cast<turbo::LogSeverity>(rhs);             \
  }

// Comparisons between `LogSeverity` and `LogSeverityAtLeast`/
// `LogSeverityAtMost` are only supported in one direction.
// Valid checks are:
//   LogSeverity >= LogSeverityAtLeast
//   LogSeverity < LogSeverityAtLeast
//   LogSeverity <= LogSeverityAtMost
//   LogSeverity > LogSeverityAtMost
COMPOP(>, <, LogSeverityAtLeast)
COMPOP(<=, >=, LogSeverityAtLeast)
COMPOP(<, >, LogSeverityAtMost)
COMPOP(>=, <=, LogSeverityAtMost)
#undef COMPOP


}  // namespace turbo

#endif  // TURBO_BASE_LOG_SEVERITY_H_
