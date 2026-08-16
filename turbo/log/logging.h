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
// File: log/logging.h
// -----------------------------------------------------------------------------
//
// This header is the single entry point for the turbo logging library.  It
// aggregates and documents the complete public API:
//
//   * stream-style logging macros  (KLOG family)         -- turbo/log/klog.h
//   * printf-style logging macros  (ZLOG family)         -- turbo/log/zlog.h
//   * fatal checks                 (KCHECK family)       -- turbo/log/kcheck.h
//   * printf-style fatal checks    (ZCHECK family)       -- turbo/log/zcheck.h
//   * verbosity control            (VKLOG_IS_ON)         -- turbo/log/vlog_is_on.h
//   * global configuration knobs   (min_log_level, etc.) -- turbo/log/globals.h
//   * library initialization       (initialize_log)      -- turbo/log/initialize.h
//   * sink registration            (add/remove/flush)    -- turbo/log/log_sink_registry.h
//   * built-in sinks and factories                       -- turbo/log/sinks/
//
// Quick start:
//
//   #include <turbo/log/logging.h>
//
//   int main() {
//     turbo::initialize_log();
//     KLOG(INFO) << "Hello, turbo!";
//   }
//
// -----------------------------------------------------------------------------
// Severity levels
// -----------------------------------------------------------------------------
//
// Six severities are defined in turbo/base/log_severity.h (least to most
// severe): TRACE, DEBUG, INFO, WARNING, ERROR, FATAL.
//
//   * `FATAL` logs the message and then terminates the program with a stack
//     trace.  Error handlers registered with `RunOnFailure` are run, but
//     `atexit(3)` handlers are not.
//   * `QFATAL` is like `FATAL` but produces quieter termination messages (e.g.
//     no full stack trace) and skips registered error handlers.
//   * `DFATAL` is `FATAL` in debug mode and `ERROR` otherwise.
//   * `DO_NOT_SUBMIT` is an alias for `ERROR`, for debugging statements that
//     must not be checked in.
//
// A severity level may be given as an expression with `LEVEL(expr)`:
//
//   KLOG(LEVEL(stale ? turbo::LogSeverity::kWarning : turbo::LogSeverity::kInfo))
//       << "Cookies are " << days << " days old";
//
// -----------------------------------------------------------------------------
// KLOG: stream-style logging
// -----------------------------------------------------------------------------
//
// `KLOG(severity)` logs whatever is streamed into it with `operator<<`:
//
//   KLOG(INFO) << "Found " << num_cookies << " cookies";
//   KLOG(WARNING) << "disk usage: " << disk_usage << "%";
//   KLOG(ERROR) << "failed to load config: " << errno_message;
//
// `PKLOG(severity)` behaves like `KLOG` but appends a description of the
// current `errno`:
//
//   int fd = open("/etc/passwd", O_RDONLY);
//   PKLOG(INFO) << "opened /etc/passwd";
//
// `DKLOG(severity)` behaves like `KLOG` in debug mode (`#ifndef NDEBUG`) and
// compiles away otherwise.  Note that `DKLOG(FATAL)` does not terminate the
// program when `NDEBUG` is defined.
//
// Chainable methods.  A log statement evaluates to an unterminated expression
// that supports the following methods:
//
//   KLOG(INFO).at_location("my_file.cc", 42) << "custom location";
//   KLOG(INFO).no_prefix() << "omits the prefix (severity/time/...)";
//   KLOG(INFO).with_verbosity(2) << "sets the verbosity field";
//   KLOG(INFO).with_timestamp(now) << "uses a custom timestamp";
//   KLOG(INFO).with_thread_id(tid) << "uses a custom thread id";
//   KLOG(INFO).with_metadata_from(entry) << "copies metadata from an entry";
//   KLOG(INFO).with_perror() << "appends errno description";
//   KLOG(INFO).to_sink_also(sink) << "sent to `sink` in addition to others";
//   KLOG(INFO).to_sink_only(sink) << "sent to `sink` and no others";
//
// Custom types are made loggable either via `turbo_stringify()` (recommended)
// or `std::ostream& operator<<(std::ostream&, ...)`:
//
//   struct Point {
//     template <typename Sink>
//     friend void turbo_stringify(Sink& sink, const Point& p) {
//       turbo::str_printf_to(&sink, "(%v, %v)", p.x, p.y);
//     }
//     int x;
//     int y;
//   };
//
// -----------------------------------------------------------------------------
// Conditional logging
// -----------------------------------------------------------------------------
//
// `KLOG_IF(severity, condition)` logs only when `condition` is true:
//
//   KLOG_IF(INFO, num_cookies > 10) << "Got lots of cookies";
//   PKLOG_IF(WARNING, fd < 0) << "open failed";
//   DKLOG_IF(INFO, debug_mode) << "debug only";
//
// There is no `VLOG_IF`; use an `if`-statement around `VKLOG` instead.
//
// -----------------------------------------------------------------------------
// Stateful logging
// -----------------------------------------------------------------------------
//
// These macros log conditionally based on a hidden per-instance counter or
// timer.  When they do not log, streamed operands are not evaluated.
// `COUNTER` streams the approximate number of times the condition was
// evaluated:
//
//   KLOG_EVERY_N(WARNING, 1000) << "Got a packet with a bad CRC ("
//                               << COUNTER << " total)";   // 1st, then every 1000th
//   KLOG_FIRST_N(INFO, 3) << "only the first 3 times";     // logs 3 times then stops
//   KLOG_EVERY_POW_2(INFO) << "1st, 2nd, 4th, 8th, ...";   // powers of two
//   KLOG_EVERY_N_SEC(INFO, 2.5) << "at most once per 2.5s";
//
// `IF` variants evaluate the extra condition first and short-circuit:
//
//   KLOG_IF_EVERY_N(INFO, (size > 1024), 10) << "Got the " << COUNTER
//                                            << "th big cookie";
//
// P and D variants exist for each of the above (e.g. `PKLOG_EVERY_N`,
// `DKLOG_EVERY_N`, ...).  See klog.h for the full list.
//
// -----------------------------------------------------------------------------
// VKLOG: verbose logging
// -----------------------------------------------------------------------------
//
// `VKLOG(verbose_level)` logs at `INFO` severity when the effective verbosity
// threshold (default 0) is >= `verbose_level`.  Positive levels are disabled by
// default.  Negative levels should not be used:
//
//   VKLOG(1) << "I print when you run the program with --v=1 or higher";
//   VKLOG(2) << "I print when you run the program with --v=2 or higher";
//
// `DVKLOG(verbose_level)` behaves like `VKLOG` in debug mode and compiles away
// otherwise.
//
// `VKLOG_IS_ON(verbose_level)` tests the threshold without logging:
//
//   if (VKLOG_IS_ON(2)) {
//     foo_server.RecomputeStatisticsExpensive();
//     KLOG(INFO) << foo_server.LastStatisticsAsString();
//   }
//
// Verbosity can be configured globally or per module:
//
//   turbo::set_global_vlog_level(1);           // all files, threshold 1
//   turbo::set_vlog_level("module_a", 3);      // files matching "module_a"
//
// `set_vlog_level` takes a glob pattern.  '?' and '*' are wildcards; patterns
// with a slash match full pathnames, otherwise the basename is matched.
//
// -----------------------------------------------------------------------------
// ZLOG: printf-style logging
// -----------------------------------------------------------------------------
//
// `ZLOG(severity, fmt, args...)` is the printf-style counterpart of `KLOG`.
// The format string uses the same type-safe syntax as `turbo::str_sprintf()`
// and is checked at compile time on GCC/Clang:
//
//   ZLOG(INFO, "Found %d cookies", num_cookies);
//   ZLOG(WARNING, "temp %.2f out of range", temp);
//
// Supported specifiers: %c %s %d %i %o %u %x %X %f %e %g %a %p %v %n.
//
// The ZLOG family mirrors the KLOG family:
//
//   ZLOG_IF(ERROR, !config_ok, "bad config: %s", name);
//   ZLOG_EVERY_N(WARNING, 1000, "bad crc on %d", pkt_id);
//   VZLOG(1, "user %s logged in", user);       // verbose printf logging
//   DZLOG(INFO, "debug only: %d", x);          // debug-mode only
//   DVZLOG(1, "debug verbose: %d", x);
//   ZLOG_FIRST_N(INFO, 3, "first %d times", n);
//   ZLOG_EVERY_POW_2(INFO, "pow2 %d", n);
//   ZLOG_EVERY_N_SEC(INFO, 2.5, "per %d sec", n);
//
// -----------------------------------------------------------------------------
// KCHECK: fatal checks
// -----------------------------------------------------------------------------
//
// `KCHECK(condition)` terminates the program with a fatal error when the
// condition is false.  Unlike `assert`, it is not controlled by `NDEBUG`:
//
//   KCHECK(!cheese.empty()) << "Out of Cheese";
//
// Outputs something like: "Check failed: !cheese.empty() Out of Cheese".
//
// Comparison checks print both operands' evaluated values:
//
//   int x = 3, y = 5;
//   KCHECK_EQ(2 * x, y) << "oops!";   // "Check failed: 2 * x == y (6 vs. 5)"
//   KCHECK_NE(a, b);
//   KCHECK_LE(a, b);
//   KCHECK_LT(a, b);
//   KCHECK_GE(a, b);
//   KCHECK_GT(a, b);
//
// C-string and Status variants:
//
//   KCHECK_STREQ(argv[0], "./skynet");
//   KCHECK_STRNE(s1, s2);
//   KCHECK_STRCASEEQ(s1, s2);   // case-insensitive
//   KCHECK_STRCASENE(s1, s2);
//   KCHECK_OK(FunctionReturnsStatus(x, y, z)) << "oops!";
//
// Other members of the family:
//
//   QKCHECK(cond);      // like KCHECK, no stack trace, skips error handlers
//   PKCHECK(cond);      // like KCHECK, appends errno description
//   DKCHECK(cond);      // debug-mode only (like assert)
//   QKCHECK_EQ/NE/LE/LT/GE/GT, QKCHECK_OK, QKCHECK_STREQ/STRNE/STRCASEEQ/STRCASENE
//   DKCHECK_EQ/NE/LE/LT/GE/GT, DKCHECK_OK, DKCHECK_STREQ/STRNE/STRCASEEQ/STRCASENE
//
// Passing `NULL` to `KCHECK_EQ` and friends does not compile; use `nullptr`.
//
// -----------------------------------------------------------------------------
// ZCHECK: printf-style fatal checks
// -----------------------------------------------------------------------------
//
// `ZCHECK(condition, fmt, args...)` behaves exactly like `KCHECK` but takes a
// printf-style format string (the same type-safe syntax as `turbo::str_sprintf`)
// instead of streamed expressions.  On failure it terminates the program with
// a fatal error:
//
//   ZCHECK(fd >= 0, "open(\"%s\") failed", path);
//
// Outputs something like: "Check failed: fd >= 0 open("/etc/passwd") failed".
// The format arguments are only evaluated when the check fails.
//
// The full ZCHECK family mirrors KCHECK:
//
//   QZCHECK(cond, "fmt %d", x);   // no stack trace, skips error handlers
//   PZCHECK(cond, "fmt");         // appends errno description
//   DZCHECK(cond, "fmt %d", x);   // debug-mode only (like assert)
//
//   ZCHECK_EQ(2 * x, y, "oops (%d)", 42);   // prints "(6 vs. 5)" then fmt
//   ZCHECK_NE(a, b, "fmt");
//   ZCHECK_LE(a, b, "fmt");
//   ZCHECK_LT(a, b, "fmt");
//   ZCHECK_GE(a, b, "fmt");
//   ZCHECK_GT(a, b, "fmt");
//
//   ZCHECK_OK(status, "fmt");               // validates a turbo::Status/Result
//   ZCHECK_STREQ(argv[0], "./skynet", "fmt");
//   ZCHECK_STRNE(s1, s2, "fmt");
//   ZCHECK_STRCASEEQ(s1, s2, "fmt");        // case-insensitive
//   ZCHECK_STRCASENE(s1, s2, "fmt");
//
// QZCHECK_* and DZCHECK_* variants exist for each of the above.
// Passing `NULL` to `ZCHECK_EQ` and friends does not compile; use `nullptr`.
//
// -----------------------------------------------------------------------------
// Global configuration
// -----------------------------------------------------------------------------
//
// Minimum log level -- messages below this severity are skipped:
//
//   turbo::set_min_log_level(turbo::LogSeverityAtLeast::kWarning);
//   turbo::LogSeverityAtLeast level = turbo::min_log_level();
//
// Stderr threshold -- messages at or above this level are also sent to stderr:
//
//   turbo::set_stderr_threshold(turbo::LogSeverityAtLeast::kError);
//   turbo::LogSeverityAtLeast threshold = turbo::stderr_threshold();
//
// RAII scoped variants temporarily change a knob and restore it on scope exit:
//
//   {
//     turbo::log_internal::ScopedMinLogLevel s(turbo::LogSeverityAtLeast::kTrace);
//     KLOG(TRACE) << "trace is visible here";
//   }  // previous minimum level restored
//
// Backtrace-on-demand for a specific statement:
//
//   turbo::set_log_backtrace_location("my_file.cc", 42);
//   turbo::clear_log_backtrace_location();
//
// Log prefix (severity/date/time/PID ...) on every message:
//
//   bool on = turbo::should_prepend_log_prefix();
//   turbo::enable_log_prefix(false);
//
// Verbose logging control (see the VKLOG section above):
//
//   turbo::set_global_vlog_level(2);
//   turbo::set_vlog_level("module_a", 1);
//
// -----------------------------------------------------------------------------
// Library initialization
// -----------------------------------------------------------------------------
//
//   turbo::initialize_log();
//
// Before this is called, all log messages are directed only to stderr.  After
// initialization, messages are sent to all registered `LogSink`s.  It is an
// error to call this function twice, and there is no corresponding shutdown
// function.
//
// `add_log_sink` is independent of `initialize_log`: sinks registered before
// initialization receive messages even before `initialize_log` is called.
//
// -----------------------------------------------------------------------------
// initialize_log() and AutoLogSink: relationship and ordering
// -----------------------------------------------------------------------------
//
// `initialize_log()` and `AutoLogSink<SinkT>` are independent of each other;
// neither calls the other:
//
//   * `AutoLogSink<SinkT>` registers its sink the moment it is constructed
//     (via `add_log_sink`) and removes it when destroyed (via
//     `remove_log_sink`).  It never calls `initialize_log()`.
//   * `initialize_log()` only flips an internal "initialized" flag and
//     installs the local time zone; it does not touch the sink registry.
//
// As a consequence, the relative order between them does not matter for sink
// routing:
//
//   * A sink registered before `initialize_log()` receives every message,
//     both before and after initialization.
//   * The only thing `initialize_log()` changes is the stderr channel.  Before
//     it is called, every message is also written to stderr (along with a
//     one-time warning about messages before initialization).  After it is
//     called, the built-in stderr sink applies `stderr_threshold()` and drops
//     messages below that threshold.
//
// Complete example with `main()` (the routing of each statement is annotated
// in the comments):
//
//   #include <cstdio>
//   #include <memory>
//   #include <turbo/log/logging.h>
//
//   // A sink that prefixes every message it receives, so routing is visible
//   // in the output below.
//   class TaggedSink final : public turbo::LogSink {
//    public:
//     explicit TaggedSink(const char *name) : name_(name) {}
//     void send(const turbo::LogEntry &entry) override {
//       std::fprintf(stderr, "[%s] %s", name_,
//                    entry.text_message_with_prefix_and_newline_c_str());
//     }
//    private:
//     const char *name_;
//   };
//
//   int main() {
//     // --- Order A: AutoLogSink first, initialize_log() second ---
//     {
//       turbo::AutoLogSink<TaggedSink> auto_sink(
//           std::make_unique<TaggedSink>("daily"));
//
//       // The sink is registered right now, so this message reaches `daily`
//       // AND stderr (stderr also prints a one-time pre-init warning).
//       KLOG(INFO) << "before init: `daily` + stderr";
//
//       turbo::initialize_log();   // only flips the "initialized" flag
//
//       // `daily` still receives this; stderr now drops messages below
//       // stderr_threshold() (INFO is below the default ERROR threshold).
//       KLOG(INFO) << "after init: `daily` only";
//       KLOG(ERROR) << "after init: `daily` + stderr (>= threshold)";
//
//       // Access the raw sink for .to_sink_only() / .to_sink_also().
//       TaggedSink *raw = auto_sink.get();
//       KLOG(WARNING).to_sink_only(raw) << "only `daily` sees this";
//     }   // <-- destructor runs here: remove_log_sink(&auto_sink)
//
//     // The sink is gone; only the built-in stderr sink remains.  (ERROR is
//     // used here because stderr only shows messages >= stderr_threshold().
//     // A WARNING would be silently dropped now that the sink is removed.)
//     KLOG(ERROR) << "sink gone: stderr only";
//
//     // --- Order B: initialize_log() first, AutoLogSink second ---
//     // The outcome is identical: the sink receives messages either way.
//     {
//       turbo::AutoLogSink<TaggedSink> auto_sink2(
//           std::make_unique<TaggedSink>("rotating"));
//       KLOG(INFO) << "init already done: `rotating`; stderr still gated";
//     }
//
//     return 0;
//   }
//
// -----------------------------------------------------------------------------
// Log sinks
// -----------------------------------------------------------------------------
//
// A `turbo::LogSink` receives every message (when registered) or only specific
// messages (via `.to_sink_only()` / `.to_sink_also()`).  It has two methods:
//
//   class MySink : public turbo::LogSink {
//    public:
//     void send(const turbo::LogEntry &entry) override { ... }
//     void flush() override { ... }
//   };
//
// Registering and removing sinks:
//
//   MySink sink;
//   turbo::add_log_sink(&sink);       // sink now receives all messages
//   turbo::remove_log_sink(&sink);    // sink no longer receives messages
//   turbo::flush_log_sinks();         // flush() on every registered sink
//
// These are thread-safe.  It is an error to add an already-registered sink or
// remove one that isn't registered.  Do not call these (or `flush_log_sinks`)
// from inside `LogSink::send`.
//
// -----------------------------------------------------------------------------
// Built-in sinks and the factory API
// -----------------------------------------------------------------------------
//
// The library ships several ready-made sinks.  `create_*_sink` factory
// functions only construct a sink and return ownership; they do NOT register
// it:
//
//   auto null = turbo::create_null_sink();
//   auto color = turbo::create_ansi_color_sink(stderr);
//   auto daily = turbo::create_daily_file_sink("logs/app.log",
//                                              /*max_files=*/7,
//                                              /*check_interval_s=*/60);
//   auto hourly = turbo::create_hourly_file_sink("logs/app.log",
//                                                /*max_files=*/84);
//   auto rotating = turbo::create_rotating_file_sink("logs/app.log",
//                                                    /*max_size_bytes=*/64 << 20,
//                                                    /*max_files=*/100);
//
// `AutoLogSink<SinkT>` is an RAII wrapper that takes a `std::unique_ptr<SinkT>`
// (typically from a factory), registers it on construction and removes it on
// destruction:
//
//   {
//     turbo::AutoLogSink<turbo::DailyFileSink> sink(
//         turbo::create_daily_file_sink("logs/app.log"));
//     KLOG(INFO) << "hello";          // routed to logs/app.log
//     KLOG(ERROR).to_sink_only(sink.get()) << "only to app.log";
//   }   // sink removed and destroyed here
//
// System-managed sinks (StderrLogSink, AndroidLogSink, WindowsDebuggerLogSink)
// are auto-registered and owned by the logging library; wrapping them in
// `AutoLogSink` is rejected at compile time.

#pragma once

#include <turbo/log/klog.h>
#include <turbo/log/kcheck.h>
#include <turbo/log/zcheck.h>
#include <turbo/log/vlog_is_on.h>
#include <turbo/log/zlog.h>
#include <turbo/log/zcheck.h>
#include <turbo/log/globals.h>
#include <turbo/log/initialize.h>
#include <turbo/log/log_sink_registry.h>
#include <turbo/log/sinks/auto_log_sink.h>
#include <turbo/log/sinks/sink_factory.h>

namespace turbo {

}  // namespace turbo
