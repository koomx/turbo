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

// -----------------------------------------------------------------------------
// File: log/klog.h
// -----------------------------------------------------------------------------
//
// This header declares a family of LOG macros.
//
// Basic invocation looks like this:
//
//   KLOG(INFO) << "Found " << num_cookies << " cookies";
//
// Most `LOG` macros take a severity level argument.  The severity levels are
// `TRACE`, `DEBUG`, `INFO`, `WARNING`, `ERROR`, and `FATAL`.  They are
// defined in turbo/base/log_severity.h.  `VKLOG` uses severity `INFO` with a
// separate verbosity level — it is not the same as `KLOG(DEBUG)`.
// * The `FATAL` severity level terminates the program with a stack trace after
//   logging its message.  Error handlers registered with `RunOnFailure`
//   (process_state.h) are run, but exit handlers registered with `atexit(3)`
//   are not.
// * The `QFATAL` pseudo-severity level is equivalent to `FATAL` but triggers
//   quieter termination messages, e.g. without a full stack trace, and skips
//   running registered error handlers.
// * The `DFATAL` pseudo-severity level is defined as `FATAL` in debug mode and
//   as `ERROR` otherwise.
// * The `DO_NOT_SUBMIT` pseudo-severity level is an alias for `ERROR`, and is
//   intended for debugging statements that won't be submitted.  The name is
//   chosen to be easy to spot in review and with tools in order to ensure that
//   such statements aren't inadvertently checked in.
//   The contract is that **it may not be checked in**, meaning that no
//   in-contract uses will be affected if we decide in the future to remove it
//   or change what it does.
// Some preprocessor shenanigans are used to ensure that e.g. `KLOG(INFO)` has
// the same meaning even if a local symbol or preprocessor macro named `INFO` is
// defined.  To specify a severity level using an expression instead of a
// literal, use `LEVEL(expr)`.
// Example:
//
//   KLOG(LEVEL(stale ? turbo::LogSeverity::kWarning : turbo::LogSeverity::kInfo))
//       << "Cookies are " << days << " days old";

// `LOG` macros evaluate to an unterminated statement.  The value at the end of
// the statement supports some chainable methods:
//
//   * .at_location(std::string_view file, int line)
//     .at_location(turbo::SourceLocation loc)
//     Overrides the location inferred from the callsite.  The string pointed to
//     by `file` must be valid until the end of the statement.
//   * .no_prefix()
//     Omits the prefix from this line.  The prefix includes metadata about the
//     logged data such as source code location and timestamp.
//   * .with_verbosity(int verbose_level)
//     Sets the verbosity field of the logged message as if it was logged by
//     `VKLOG(verbose_level)`.  Unlike `VKLOG`, this method does not affect
//     evaluation of the statement when the specified `verbose_level` has been
//     disabled.  The only effect is on `LogSink` implementations which make use
//     of the `turbo::LogSink::verbosity()` value.  The value
//     `turbo::LogEntry::kNoVerbosityLevel` can be specified to mark the message
//     not verbose.
//   * .with_timestamp(turbo::Time timestamp)
//     Uses the specified timestamp instead of one collected at the time of
//     execution.
//   * .with_thread_id(turbo::LogEntry::tid_t tid)
//     Uses the specified thread ID instead of one collected at the time of
//     execution.
//   * .with_metadata_from(const turbo::LogEntry &entry)
//     Copies all metadata (but no data) from the specified `turbo::LogEntry`.
//     This can be used to change the severity of a message, but it has some
//     limitations:
//     * `TURBO_MIN_LOG_LEVEL` is evaluated against the severity passed into
//       `LOG` (or the implicit `FATAL` level of `KCHECK`).
//     * `KLOG(FATAL)` and `KCHECK` terminate the process unconditionally, even if
//       the severity is changed later.
//     `.with_metadata_from(entry)` should almost always be used in combination
//     with `KLOG(LEVEL(entry.log_severity()))`.
//   * .with_perror()
//     Appends to the logged message a colon, a space, a textual description of
//     the current value of `errno` (as by `strerror(3)`), and the numerical
//     value of `errno`.
//   * .to_sink_also(turbo::LogSink* sink)
//     Sends this message to `*sink` in addition to whatever other sinks it
//     would otherwise have been sent to.  `sink` must not be null.
//   * .to_sink_only(turbo::LogSink* sink)
//     Sends this message to `*sink` and no others.  `sink` must not be null.
//
// No interfaces in this header are async-signal-safe; their use in signal
// handlers is unsupported and may deadlock your program or eat your lunch.
//
// Many logging statements are inherently conditional.  For example,
// `KLOG_IF(INFO, !foo)` does nothing if `foo` is true.  Even seemingly
// unconditional statements like `KLOG(INFO)` might be disabled at
// compile-time to minimize binary size or for security reasons.
//
// * Except for the condition in a `KCHECK` or `QKCHECK` statement, programs must
//   not rely on evaluation of expressions anywhere in logging statements for
//   correctness.  For example, this is ok:
//
//     KCHECK((fp = fopen("config.ini", "r")) != nullptr);
//
//   But this is probably not ok:
//
//     KLOG(INFO) << "Server status: " << StartServerAndReturnStatusString();
//
//   The example below is bad too; the `i++` in the `LOG_IF` condition might
//   not be evaluated, resulting in an infinite loop:
//
//     for (int i = 0; i < 1000000;)
//       KLOG_IF(INFO, i++ % 1000 == 0) << "Still working...";
//
// * Except where otherwise noted, conditions which cause a statement not to log
//   also cause expressions not to be evaluated.  Programs may rely on this for
//   performance reasons, e.g. by streaming the result of an expensive function
//   call into a `DLOG` or `KLOG_EVERY_N` statement.
// * Care has been taken to ensure that expressions are parsed by the compiler
//   even if they are never evaluated.  This means that syntax errors will be
//   caught and variables will be considered used for the purposes of
//   unused-variable diagnostics.  For example, this statement won't compile
//   even if `INFO`-level logging has been compiled out:
//
//     int number_of_cakes = 40;
//     KLOG(INFO) << "Number of cakes: " << number_of_cake;  // Note the typo!
//
//   Similarly, this won't produce unused-variable compiler diagnostics even
//   if `INFO`-level logging is compiled out:
//
//     {
//       char fox_line1[] = "Hatee-hatee-hatee-ho!";
//       KLOG_IF(ERROR, false) << "The fox says " << fox_line1;
//       char fox_line2[] = "A-oo-oo-oo-ooo!";
//       KLOG(INFO) << "The fox also says " << fox_line2;
//     }
//
//   This error-checking is not perfect; for example, symbols that have been
//   declared but not defined may not produce link errors if used in logging
//   statements that compile away.
//
// Expressions streamed into these macros are formatted using `operator<<` just
// as they would be if streamed into a `std::ostream`, however it should be
// noted that their actual type is unspecified.
//
// To implement a custom formatting operator for a type you own, there are two
// options: `turbo_stringify()` or `std::ostream& operator<<(std::ostream&, ...)`.
// It is recommended that users make their types loggable through
// `turbo_stringify()` as it is a universal stringification extension that also
// enables `turbo::str_sprintf` and `turbo::str_cat` support. If both
// `turbo_stringify()` and `std::ostream& operator<<(std::ostream&, ...)` are
// defined, `turbo_stringify()` will be used.
//
// To use the `turbo_stringify()` API, define a friend function template in your
// type's namespace with the following signature:
//
//   template <typename Sink>
//   void turbo_stringify(Sink& sink, const UserDefinedType& value);
//
// `Sink` has the same interface as `turbo::FormatSink`, but without
// `put_padded_string()`.
//
// Example:
//
//   struct Point {
//     template <typename Sink>
//     friend void turbo_stringify(Sink& sink, const Point& p) {
//       turbo::str_printf_to(&sink, "(%v, %v)", p.x, p.y);
//     }
//
//     int x;
//     int y;
//   };
//
// To use `std::ostream& operator<<(std::ostream&, ...)`, define
// `std::ostream& operator<<(std::ostream&, ...)` in your type's namespace (for
// ADL) just as you would to stream it to `std::cout`.
//
// Currently `turbo_stringify()` ignores output manipulators but this is not
// guaranteed behavior and may be subject to change in the future. If you would
// like guaranteed behavior regarding output manipulators, please use
// `std::ostream& operator<<(std::ostream&, ...)` to make custom types loggable
// instead.
//
// Those macros that support streaming honor output manipulators and `fmtflag`
// changes that output data (e.g. `std::ends`) or control formatting of data
// (e.g. `std::hex` and `std::fixed`), however flushing such a stream is
// ignored.  The message produced by a log statement is sent to registered
// `turbo::LogSink` instances at the end of the statement; those sinks are
// responsible for their own flushing (e.g. to disk) semantics.
//
// Flag settings are not carried over from one `LOG` statement to the next; this
// is a bit different than e.g. `std::cout`:
//
//   KLOG(INFO) << std::hex << 0xdeadbeef;  // logs "0xdeadbeef"
//   KLOG(INFO) << 0xdeadbeef;              // logs "3735928559"

// SKIP_TURBO_INLINE_NAMESPACE_CHECK

#ifndef TURBO_LOG_LOG_H_
#define TURBO_LOG_LOG_H_

#include <turbo/log/internal/log_impl.h>

// KLOG()
//
// `LOG` takes a single argument which is a severity level.  Data streamed in
// comprise the logged message.
// Example:
//
//   KLOG(INFO) << "Found " << num_cookies << " cookies";
#define KLOG(severity) TURBO_LOG_INTERNAL_LOG_IMPL(_##severity)

// PKLOG()
//
// `PLOG` behaves like `LOG` except that a description of the current state of
// `errno` is appended to the streamed message.
#define PKLOG(severity) TURBO_LOG_INTERNAL_PLOG_IMPL(_##severity)

// DKLOG()
//
// `DLOG` behaves like `LOG` in debug mode (i.e. `#ifndef NDEBUG`).  Otherwise
// it compiles away and does nothing.  Note that `DKLOG(FATAL)` does not
// terminate the program if `NDEBUG` is defined.
#define DKLOG(severity) TURBO_LOG_INTERNAL_DLOG_IMPL(_##severity)

// `VKLOG` uses numeric levels to provide verbose logging that can configured at
// runtime, including at a per-module level.  `VKLOG` statements are logged at
// `INFO` severity if they are logged at all; the numeric levels are on a
// different scale than the proper severity levels.  Positive levels are
// disabled by default.  Negative levels should not be used.
// Example:
//
//   VKLOG(1) << "I print when you run the program with --v=1 or higher";
//   VKLOG(2) << "I print when you run the program with --v=2 or higher";
//
// See vlog_is_on.h for further documentation, including the usage of the
// `set_vlog_level` to log at different levels in different source files.
//
// `VKLOG` does not produce any output when verbose logging is not enabled.
// However, simply testing whether verbose logging is enabled can be expensive.
// If you don't intend to enable verbose logging in non-debug builds, consider
// using `DVLOG` instead.
#define VKLOG(verbose_level) TURBO_LOG_INTERNAL_VLOG_IMPL(verbose_level)

// `DVLOG` behaves like `VKLOG` in debug mode (i.e. `#ifndef NDEBUG`).
// Otherwise, it compiles away and does nothing.
#define DVKLOG(verbose_level) TURBO_LOG_INTERNAL_DVLOG_IMPL(verbose_level)

// `LOG_IF` and friends add a second argument which specifies a condition.  If
// the condition is false, nothing is logged.
// Example:
//
//   KLOG_IF(INFO, num_cookies > 10) << "Got lots of cookies";
//
// There is no `VLOG_IF` because the order of evaluation of the arguments is
// ambiguous and the alternate spelling with an `if`-statement is trivial.
#define KLOG_IF(severity, condition) \
  TURBO_LOG_INTERNAL_LOG_IF_IMPL(_##severity, condition)
#define PKLOG_IF(severity, condition) \
  TURBO_LOG_INTERNAL_PLOG_IF_IMPL(_##severity, condition)
#define DKLOG_IF(severity, condition) \
  TURBO_LOG_INTERNAL_DLOG_IF_IMPL(_##severity, condition)

// KLOG_EVERY_N
// KLOG_FIRST_N
// KLOG_EVERY_POW_2
// KLOG_EVERY_N_SEC
//
// These "stateful" macros log conditionally based on a hidden counter or timer.
// When the condition is false and no logging is done, streamed operands aren't
// evaluated either.  Each instance has its own state (i.e. counter, timer)
// that's independent of other instances of the macros.  The macros in this
// family are thread-safe in the sense that they are meant to be called
// concurrently and will not invoke undefined behavior, however their
// implementation prioritizes efficiency over exactness and may occasionally log
// more or less often than specified.
//
//   * `KLOG_EVERY_N` logs the first time and once every `n` times thereafter.
//   * `KLOG_FIRST_N` logs the first `n` times and then stops.
//   * `KLOG_EVERY_POW_2` logs the first, second, fourth, eighth, etc. times.
//   * `KLOG_EVERY_N_SEC` logs the first time and no more than once every `n`
//     seconds thereafter.  `n` is passed as a floating point value.
//
// The `LOG_IF`... variations with an extra condition evaluate the specified
// condition first and short-circuit if it is false.  For example, an evaluation
// of `LOG_IF_FIRST_N` does not count against the first `n` if the specified
// condition is false.  Stateful `VKLOG`... variations likewise short-circuit
// if `VKLOG` is disabled.
//
// An approximate count of the number of times a particular instance's stateful
// condition has been evaluated (i.e. excluding those where a specified `LOG_IF`
// condition was false) can be included in the logged message by streaming the
// symbol `COUNTER`.
//
// The `n` parameter need not be a constant.  Conditional logging following a
// change to `n` isn't fully specified, but it should converge on the new value
// within roughly `max(old_n, new_n)` evaluations or seconds.
//
// Examples:
//
//   KLOG_EVERY_N(WARNING, 1000) << "Got a packet with a bad CRC (" << COUNTER
//                              << " total)";
//
//   KLOG_EVERY_N_SEC(INFO, 2.5) << "Got " << COUNTER << " cookies so far";
//
//   LOG_IF_EVERY_N(INFO, (size > 1024), 10) << "Got the " << COUNTER
//                                           << "th big cookie";
#define KLOG_EVERY_N(severity, n) \
  TURBO_LOG_INTERNAL_LOG_EVERY_N_IMPL(_##severity, n)
#define KLOG_FIRST_N(severity, n) \
  TURBO_LOG_INTERNAL_LOG_FIRST_N_IMPL(_##severity, n)
#define KLOG_EVERY_POW_2(severity) \
  TURBO_LOG_INTERNAL_LOG_EVERY_POW_2_IMPL(_##severity)
#define KLOG_EVERY_N_SEC(severity, n_seconds) \
  TURBO_LOG_INTERNAL_LOG_EVERY_N_SEC_IMPL(_##severity, n_seconds)

#define PKLOG_EVERY_N(severity, n) \
  TURBO_LOG_INTERNAL_PLOG_EVERY_N_IMPL(_##severity, n)
#define PKLOG_FIRST_N(severity, n) \
  TURBO_LOG_INTERNAL_PLOG_FIRST_N_IMPL(_##severity, n)
#define PKLOG_EVERY_POW_2(severity) \
  TURBO_LOG_INTERNAL_PLOG_EVERY_POW_2_IMPL(_##severity)
#define PKLOG_EVERY_N_SEC(severity, n_seconds) \
  TURBO_LOG_INTERNAL_PLOG_EVERY_N_SEC_IMPL(_##severity, n_seconds)

#define DKLOG_EVERY_N(severity, n) \
  TURBO_LOG_INTERNAL_DLOG_EVERY_N_IMPL(_##severity, n)
#define DKLOG_FIRST_N(severity, n) \
  TURBO_LOG_INTERNAL_DLOG_FIRST_N_IMPL(_##severity, n)
#define DKLOG_EVERY_POW_2(severity) \
  TURBO_LOG_INTERNAL_DLOG_EVERY_POW_2_IMPL(_##severity)
#define DKLOG_EVERY_N_SEC(severity, n_seconds) \
  TURBO_LOG_INTERNAL_DLOG_EVERY_N_SEC_IMPL(_##severity, n_seconds)

#define VKLOG_EVERY_N(severity, n) \
  TURBO_LOG_INTERNAL_VLOG_EVERY_N_IMPL(severity, n)
#define VKLOG_FIRST_N(severity, n) \
  TURBO_LOG_INTERNAL_VLOG_FIRST_N_IMPL(severity, n)
#define VKLOG_EVERY_POW_2(severity) \
  TURBO_LOG_INTERNAL_VLOG_EVERY_POW_2_IMPL(severity)
#define VKLOG_EVERY_N_SEC(severity, n_seconds) \
  TURBO_LOG_INTERNAL_VLOG_EVERY_N_SEC_IMPL(severity, n_seconds)

#define LOG_IF_EVERY_N(severity, condition, n) \
  TURBO_LOG_INTERNAL_LOG_IF_EVERY_N_IMPL(_##severity, condition, n)
#define LOG_IF_FIRST_N(severity, condition, n) \
  TURBO_LOG_INTERNAL_LOG_IF_FIRST_N_IMPL(_##severity, condition, n)
#define LOG_IF_EVERY_POW_2(severity, condition) \
  TURBO_LOG_INTERNAL_LOG_IF_EVERY_POW_2_IMPL(_##severity, condition)
#define LOG_IF_EVERY_N_SEC(severity, condition, n_seconds) \
  TURBO_LOG_INTERNAL_LOG_IF_EVERY_N_SEC_IMPL(_##severity, condition, n_seconds)

#define PLOG_IF_EVERY_N(severity, condition, n) \
  TURBO_LOG_INTERNAL_PLOG_IF_EVERY_N_IMPL(_##severity, condition, n)
#define PLOG_IF_FIRST_N(severity, condition, n) \
  TURBO_LOG_INTERNAL_PLOG_IF_FIRST_N_IMPL(_##severity, condition, n)
#define PLOG_IF_EVERY_POW_2(severity, condition) \
  TURBO_LOG_INTERNAL_PLOG_IF_EVERY_POW_2_IMPL(_##severity, condition)
#define PLOG_IF_EVERY_N_SEC(severity, condition, n_seconds) \
  TURBO_LOG_INTERNAL_PLOG_IF_EVERY_N_SEC_IMPL(_##severity, condition, n_seconds)

#define DLOG_IF_EVERY_N(severity, condition, n) \
  TURBO_LOG_INTERNAL_DLOG_IF_EVERY_N_IMPL(_##severity, condition, n)
#define DLOG_IF_FIRST_N(severity, condition, n) \
  TURBO_LOG_INTERNAL_DLOG_IF_FIRST_N_IMPL(_##severity, condition, n)
#define DLOG_IF_EVERY_POW_2(severity, condition) \
  TURBO_LOG_INTERNAL_DLOG_IF_EVERY_POW_2_IMPL(_##severity, condition)
#define DLOG_IF_EVERY_N_SEC(severity, condition, n_seconds) \
  TURBO_LOG_INTERNAL_DLOG_IF_EVERY_N_SEC_IMPL(_##severity, condition, n_seconds)

#endif  // TURBO_LOG_LOG_H_
