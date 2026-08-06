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
//
// -----------------------------------------------------------------------------
// File: log/internal/conditions.h
// -----------------------------------------------------------------------------
//
// This file contains implementation of conditional log statements, like LOG_IF
// including all the TURBO_LOG_INTERNAL_..._CONDITION_... macros and
// various condition classes like LogEveryNState.

#ifndef TURBO_LOG_INTERNAL_CONDITIONS_H_
#define TURBO_LOG_INTERNAL_CONDITIONS_H_

#if defined(_WIN32) || defined(__hexagon__)
#include <cstdlib>
#else
#include <unistd.h>
#endif
#include <stdlib.h>

#include <atomic>
#include <cstdint>
#include <turbo/macros/config.h>
#include <turbo/log/internal/voidify.h>

// `TURBO_LOG_INTERNAL_CONDITION` prefixes another macro that expands to a
// temporary `LogMessage` instantiation followed by zero or more streamed
// expressions.  This definition is tricky to read correctly.  It evaluates to
// either
//
//   (void)0;
//
// or
//
//   ::turbo::log_internal::Voidify() &&
//       ::turbo::log_internal::LogMessage(...) << "the user's message";
//
// If the condition is evaluable at compile time, as is often the case, it
// compiles away to just one side or the other.
//
// Although this is not used anywhere a statement (e.g. `if`) could not go,
// the ternary expression does a better job avoiding spurious diagnostics
// (dangling else, missing switch case) and preserving noreturn semantics (e.g.
// on `KLOG(FATAL)`) without requiring braces.
//
// The `switch` ensures that this expansion is the beginning of a statement (as
// opposed to an expression) and prevents shenanigans like
// `AFunction(KLOG(INFO))` and `decltype(KLOG(INFO))`.  The apparently-redundant
// `default` case makes the condition more amenable to Clang dataflow analysis.
#define TURBO_LOG_INTERNAL_STATELESS_CONDITION(condition) \
  switch (0)                                             \
  case 0:                                                \
  default:                                               \
    !(condition) ? (void)0 : ::turbo::log_internal::Voidify() &&

// `TURBO_LOG_INTERNAL_STATEFUL_CONDITION` applies a condition like
// `TURBO_LOG_INTERNAL_STATELESS_CONDITION` but adds to that a series of variable
// declarations, including a local static object which stores the state needed
// to implement the stateful macros like `KLOG_EVERY_N`.
//
// `for`-loops are used to declare scoped variables without braces (to permit
// streaming into the macro's expansion) and without the dangling-`else`
// problems/diagnostics that come with `if`.
//
// Two more variables are declared in separate `for`-loops:
//
// * `COUNTER` implements a streamable token whose value when streamed is the
//   number of times execution has passed through the macro.
// * A boolean flag is used to prevent any of the `for`-loops from ever actually
//   looping.
#define TURBO_LOG_INTERNAL_STATEFUL_CONDITION(condition)             \
  for (bool turbo_log_internal_stateful_condition_do_log(condition); \
       turbo_log_internal_stateful_condition_do_log;                 \
       turbo_log_internal_stateful_condition_do_log = false)         \
  TURBO_LOG_INTERNAL_STATEFUL_CONDITION_IMPL
#define TURBO_LOG_INTERNAL_STATEFUL_CONDITION_IMPL(kind, ...)              \
  for (static ::turbo::log_internal::Log##kind##State                      \
           turbo_log_internal_stateful_condition_state;                    \
       turbo_log_internal_stateful_condition_do_log &&                     \
       turbo_log_internal_stateful_condition_state.should_log(__VA_ARGS__); \
       turbo_log_internal_stateful_condition_do_log = false)               \
    for (const uint32_t COUNTER KUMO_ATTRIBUTE_UNUSED =                   \
             turbo_log_internal_stateful_condition_state.counter();        \
         turbo_log_internal_stateful_condition_do_log;                     \
         turbo_log_internal_stateful_condition_do_log = false)             \
  ::turbo::log_internal::Voidify() &&

// `TURBO_LOG_INTERNAL_CONDITION_*` serve to combine any conditions from the
// macro (e.g. `LOG_IF` or `VKLOG`) with inherent conditions (e.g.
// `TURBO_MIN_LOG_LEVEL`) into a single boolean expression.  We could chain
// ternary operators instead, however some versions of Clang sometimes issue
// spurious diagnostics after such expressions due to a control flow analysis
// bug.
#ifdef TURBO_MIN_LOG_LEVEL
#define TURBO_LOG_INTERNAL_CONDITION_TRACE(type, condition) \
  TURBO_LOG_INTERNAL_##type##_CONDITION(                    \
      (condition) &&                                       \
      ::turbo::LogSeverity::kTrace >=                       \
          static_cast<::turbo::LogSeverityAtLeast>(TURBO_MIN_LOG_LEVEL))
#define TURBO_LOG_INTERNAL_CONDITION_DEBUG(type, condition) \
  TURBO_LOG_INTERNAL_##type##_CONDITION(                    \
      (condition) &&                                       \
      ::turbo::LogSeverity::kDebug >=                       \
          static_cast<::turbo::LogSeverityAtLeast>(TURBO_MIN_LOG_LEVEL))
#define TURBO_LOG_INTERNAL_CONDITION_INFO(type, condition) \
  TURBO_LOG_INTERNAL_##type##_CONDITION(                   \
      (condition) &&                                      \
      ::turbo::LogSeverity::kInfo >=                       \
          static_cast<::turbo::LogSeverityAtLeast>(TURBO_MIN_LOG_LEVEL))
#define TURBO_LOG_INTERNAL_CONDITION_WARNING(type, condition) \
  TURBO_LOG_INTERNAL_##type##_CONDITION(                      \
      (condition) &&                                         \
      ::turbo::LogSeverity::kWarning >=                       \
          static_cast<::turbo::LogSeverityAtLeast>(TURBO_MIN_LOG_LEVEL))
#define TURBO_LOG_INTERNAL_CONDITION_ERROR(type, condition) \
  TURBO_LOG_INTERNAL_##type##_CONDITION(                    \
      (condition) &&                                       \
      ::turbo::LogSeverity::kError >=                       \
          static_cast<::turbo::LogSeverityAtLeast>(TURBO_MIN_LOG_LEVEL))
#define TURBO_LOG_INTERNAL_CONDITION_DO_NOT_SUBMIT(type, condition) \
  TURBO_LOG_INTERNAL_CONDITION_ERROR(type, condition)
// NOTE: Use ternary operators instead of short-circuiting to mitigate
// https://bugs.llvm.org/show_bug.cgi?id=51928.
#define TURBO_LOG_INTERNAL_CONDITION_FATAL(type, condition)                 \
  TURBO_LOG_INTERNAL_##type##_CONDITION(                                    \
      ((condition) ? (::turbo::LogSeverity::kFatal >=                       \
                              static_cast<::turbo::LogSeverityAtLeast>(     \
                                  TURBO_MIN_LOG_LEVEL)                      \
                          ? true                                           \
                          : (::turbo::log_internal::abort_quietly(), false)) \
                   : false))
// NOTE: Use ternary operators instead of short-circuiting to mitigate
// https://bugs.llvm.org/show_bug.cgi?id=51928.
#define TURBO_LOG_INTERNAL_CONDITION_QFATAL(type, condition)               \
  TURBO_LOG_INTERNAL_##type##_CONDITION(                                   \
      ((condition) ? (::turbo::LogSeverity::kFatal >=                      \
                              static_cast<::turbo::LogSeverityAtLeast>(    \
                                  TURBO_MIN_LOG_LEVEL)                     \
                          ? true                                          \
                          : (::turbo::log_internal::exit_quietly(), false)) \
                   : false))
#define TURBO_LOG_INTERNAL_CONDITION_DFATAL(type, condition)                    \
  TURBO_LOG_INTERNAL_##type##_CONDITION(                                        \
      (KUMO_ASSUME(turbo::kLogDebugFatal == turbo::LogSeverity::kError ||        \
                   turbo::kLogDebugFatal == turbo::LogSeverity::kFatal),         \
       (condition) &&                                                          \
           (::turbo::kLogDebugFatal >=                                          \
                static_cast<::turbo::LogSeverityAtLeast>(TURBO_MIN_LOG_LEVEL) || \
            (::turbo::kLogDebugFatal == ::turbo::LogSeverity::kFatal &&          \
             (::turbo::log_internal::abort_quietly(), false)))))

#define TURBO_LOG_INTERNAL_CONDITION_LEVEL(severity)                            \
  for (int turbo_log_internal_severity_loop = 1;                                \
       turbo_log_internal_severity_loop; turbo_log_internal_severity_loop = 0)   \
    for (const turbo::LogSeverity turbo_log_internal_severity =                  \
             ::turbo::NormalizeLogSeverity(severity);                           \
         turbo_log_internal_severity_loop; turbo_log_internal_severity_loop = 0) \
  TURBO_LOG_INTERNAL_CONDITION_LEVEL_IMPL
#define TURBO_LOG_INTERNAL_CONDITION_LEVEL_IMPL(type, condition)            \
  TURBO_LOG_INTERNAL_##type##_CONDITION(                                    \
      ((condition) &&                                                      \
       (turbo_log_internal_severity >=                                      \
            static_cast<::turbo::LogSeverityAtLeast>(TURBO_MIN_LOG_LEVEL) || \
        (turbo_log_internal_severity == ::turbo::LogSeverity::kFatal &&      \
         (::turbo::log_internal::abort_quietly(), false)))))
#else  // ndef TURBO_MIN_LOG_LEVEL
#define TURBO_LOG_INTERNAL_CONDITION_TRACE(type, condition) \
  TURBO_LOG_INTERNAL_##type##_CONDITION(condition)
#define TURBO_LOG_INTERNAL_CONDITION_DEBUG(type, condition) \
  TURBO_LOG_INTERNAL_##type##_CONDITION(condition)
#define TURBO_LOG_INTERNAL_CONDITION_INFO(type, condition) \
  TURBO_LOG_INTERNAL_##type##_CONDITION(condition)
#define TURBO_LOG_INTERNAL_CONDITION_WARNING(type, condition) \
  TURBO_LOG_INTERNAL_##type##_CONDITION(condition)
#define TURBO_LOG_INTERNAL_CONDITION_ERROR(type, condition) \
  TURBO_LOG_INTERNAL_##type##_CONDITION(condition)
#define TURBO_LOG_INTERNAL_CONDITION_DO_NOT_SUBMIT(type, condition) \
  TURBO_LOG_INTERNAL_CONDITION_ERROR(type, condition)
#define TURBO_LOG_INTERNAL_CONDITION_FATAL(type, condition) \
  TURBO_LOG_INTERNAL_##type##_CONDITION(condition)
#define TURBO_LOG_INTERNAL_CONDITION_QFATAL(type, condition) \
  TURBO_LOG_INTERNAL_##type##_CONDITION(condition)
#define TURBO_LOG_INTERNAL_CONDITION_DFATAL(type, condition) \
  TURBO_LOG_INTERNAL_##type##_CONDITION(condition)
#define TURBO_LOG_INTERNAL_CONDITION_LEVEL(severity)                            \
  for (int turbo_log_internal_severity_loop = 1;                                \
       turbo_log_internal_severity_loop; turbo_log_internal_severity_loop = 0)   \
    for (const turbo::LogSeverity turbo_log_internal_severity =                  \
             ::turbo::NormalizeLogSeverity(severity);                           \
         turbo_log_internal_severity_loop; turbo_log_internal_severity_loop = 0) \
  TURBO_LOG_INTERNAL_CONDITION_LEVEL_IMPL
#define TURBO_LOG_INTERNAL_CONDITION_LEVEL_IMPL(type, condition) \
  TURBO_LOG_INTERNAL_##type##_CONDITION(condition)
#endif  // ndef TURBO_MIN_LOG_LEVEL

namespace turbo {
    namespace log_internal {
        // Stateful condition class name should be "Log" + name + "State".
        class LogEveryNState final {
        public:
            bool should_log(int n);

            uint32_t counter() { return counter_.load(std::memory_order_relaxed); }

        private:
            std::atomic<uint32_t> counter_{0};
        };

        class LogFirstNState final {
        public:
            bool should_log(int n);

            uint32_t counter() { return counter_.load(std::memory_order_relaxed); }

        private:
            std::atomic<uint32_t> counter_{0};
        };

        class LogEveryPow2State final {
        public:
            bool should_log();

            uint32_t counter() { return counter_.load(std::memory_order_relaxed); }

        private:
            std::atomic<uint32_t> counter_{0};
        };

        class LogEveryNSecState final {
        public:
            bool should_log(double seconds);

            uint32_t counter() { return counter_.load(std::memory_order_relaxed); }

        private:
            std::atomic<uint32_t> counter_{0};
            // Cycle count according to CycleClock that we should next log at.
            std::atomic<int64_t> next_log_time_cycles_{0};
        };

        // Helper routines to abort the application quietly

        [[noreturn]] inline void abort_quietly() { abort(); }
        [[noreturn]] inline void exit_quietly() { _exit(1); }
    } // namespace log_internal
} // namespace turbo

#endif  // TURBO_LOG_INTERNAL_CONDITIONS_H_
