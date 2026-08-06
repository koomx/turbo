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
// File: log/zlog.h
// -----------------------------------------------------------------------------
//
// This header declares a family of ZLOG macros — printf-style formatted
// logging. ZLOG macros are the printf-style counterparts of the stream-based
// LOG macros.
//
// Basic invocation:
//
//   ZLOG(INFO, "Found %d cookies", num_cookies);
//
// Unlike `KLOG(INFO) << ...`, ZLOG uses type-safe printf format strings
// (the same as turbo::str_sprintf). The format string is checked at
// compile-time on GCC and Clang, at runtime on MSVC.
//
// Supported format specifiers: %c, %s, %d, %i, %o, %u, %x, %X, %f, %e, %g,
// %a, %p, %v, %n.

#pragma once

#include <turbo/log/turbo_vlog_is_on.h>
#include <turbo/log/internal/conditions.h>
#include <turbo/log/internal/log_message.h>
#include <turbo/log/internal/strip.h>

// ---------------------------------------------------------------------------
// Internal ZLOG implementation macros
// ---------------------------------------------------------------------------

// ZLOG(severity, fmt, args...)
#define TURBO_LOG_INTERNAL_ZLOG_IMPL(severity, ...)           \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATELESS, true)     \
      TURBO_LOG_INTERNAL_LOG##severity.printf(__VA_ARGS__)

// DZLOG(severity, fmt, args...)
#ifndef NDEBUG
#define TURBO_LOG_INTERNAL_DZLOG_IMPL(severity, ...)          \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATELESS, true)     \
      TURBO_LOG_INTERNAL_DLOG##severity.printf(__VA_ARGS__)
#else
#define TURBO_LOG_INTERNAL_DZLOG_IMPL(severity, ...)           \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATELESS, false)     \
      TURBO_LOG_INTERNAL_DLOG##severity.printf(__VA_ARGS__)
#endif

// ZLOG_IF(severity, condition, fmt, args...)
#define TURBO_LOG_INTERNAL_ZLOG_IF_IMPL(severity, condition, ...) \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATELESS, condition)    \
      TURBO_LOG_INTERNAL_LOG##severity.printf(__VA_ARGS__)

// DZLOG_IF(severity, condition, fmt, args...)
#ifndef NDEBUG
#define TURBO_LOG_INTERNAL_DZLOG_IF_IMPL(severity, condition, ...) \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATELESS, condition)     \
      TURBO_LOG_INTERNAL_DLOG##severity.printf(__VA_ARGS__)
#else
#define TURBO_LOG_INTERNAL_DZLOG_IF_IMPL(severity, condition, ...)              \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATELESS, false && (condition))       \
      TURBO_LOG_INTERNAL_DLOG##severity.printf(__VA_ARGS__)
#endif

// VZLOG(verbose_level, fmt, args...)
#define TURBO_LOG_INTERNAL_VZLOG_IMPL(verbose_level, ...)                    \
  switch (const int turbo_log_internal_verbose_level = (verbose_level))      \
  case 0:                                                                    \
  default:                                                                   \
    TURBO_LOG_INTERNAL_CONDITION_INFO(STATELESS,                              \
        TURBO_VLOG_IS_ON(turbo_log_internal_verbose_level))                    \
        TURBO_LOG_INTERNAL_LOG_INFO                                          \
            .with_verbosity(turbo_log_internal_verbose_level)                  \
            .printf(__VA_ARGS__)

// DVZLOG(verbose_level, fmt, args...)
#ifndef NDEBUG
#define TURBO_LOG_INTERNAL_DVZLOG_IMPL(verbose_level, ...)                   \
  switch (const int turbo_log_internal_verbose_level = (verbose_level))      \
  case 0:                                                                    \
  default:                                                                   \
    TURBO_LOG_INTERNAL_CONDITION_INFO(STATELESS,                              \
        TURBO_VLOG_IS_ON(turbo_log_internal_verbose_level))                    \
        TURBO_LOG_INTERNAL_LOG_INFO                                          \
            .with_verbosity(turbo_log_internal_verbose_level)                  \
            .printf(__VA_ARGS__)
#else
#define TURBO_LOG_INTERNAL_DVZLOG_IMPL(verbose_level, ...)                       \
  switch (const int turbo_log_internal_verbose_level = (verbose_level))          \
  case 0:                                                                        \
  default:                                                                       \
    TURBO_LOG_INTERNAL_CONDITION_INFO(STATELESS,                                  \
        false && TURBO_VLOG_IS_ON(turbo_log_internal_verbose_level))              \
        TURBO_LOG_INTERNAL_LOG_INFO                                              \
            .with_verbosity(turbo_log_internal_verbose_level)                      \
            .printf(__VA_ARGS__)
#endif

// ZLOG_EVERY_N(severity, n, fmt, args...)
#define TURBO_LOG_INTERNAL_ZLOG_EVERY_N_IMPL(severity, n, ...)          \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, true)(EveryN, n)     \
      TURBO_LOG_INTERNAL_LOG##severity.printf(__VA_ARGS__)

// ZLOG_FIRST_N(severity, n, fmt, args...)
#define TURBO_LOG_INTERNAL_ZLOG_FIRST_N_IMPL(severity, n, ...)          \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, true)(FirstN, n)     \
      TURBO_LOG_INTERNAL_LOG##severity.printf(__VA_ARGS__)

// ZLOG_EVERY_POW_2(severity, fmt, args...)
#define TURBO_LOG_INTERNAL_ZLOG_EVERY_POW_2_IMPL(severity, ...)         \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, true)(EveryPow2)     \
      TURBO_LOG_INTERNAL_LOG##severity.printf(__VA_ARGS__)

// ZLOG_EVERY_N_SEC(severity, n_seconds, fmt, args...)
#define TURBO_LOG_INTERNAL_ZLOG_EVERY_N_SEC_IMPL(severity, n_seconds, ...)     \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, true)(EveryNSec, n_seconds) \
      TURBO_LOG_INTERNAL_LOG##severity.printf(__VA_ARGS__)

// DZLOG_EVERY_N
#ifndef NDEBUG
#define TURBO_LOG_INTERNAL_DZLOG_EVERY_N_IMPL(severity, n, ...) \
  TURBO_LOG_INTERNAL_CONDITION_INFO(STATEFUL, true)             \
  (EveryN, n) TURBO_LOG_INTERNAL_DLOG##severity.printf(__VA_ARGS__)
#else
#define TURBO_LOG_INTERNAL_DZLOG_EVERY_N_IMPL(severity, n, ...) \
  TURBO_LOG_INTERNAL_CONDITION_INFO(STATEFUL, false)            \
  (EveryN, n) TURBO_LOG_INTERNAL_DLOG##severity.printf(__VA_ARGS__)
#endif

// ZLOG_IF_EVERY_N(severity, condition, n, fmt, args...)
#define TURBO_LOG_INTERNAL_ZLOG_IF_EVERY_N_IMPL(severity, condition, n, ...) \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, condition)(EveryN, n)     \
      TURBO_LOG_INTERNAL_LOG##severity.printf(__VA_ARGS__)

// VZLOG_EVERY_N(verbose_level, n, fmt, args...)
#define TURBO_LOG_INTERNAL_VZLOG_EVERY_N_IMPL(verbose_level, n, ...)          \
  switch (const int turbo_log_internal_verbose_level = (verbose_level))       \
  case 0:                                                                    \
  default:                                                                   \
    TURBO_LOG_INTERNAL_CONDITION_INFO(                                        \
        STATEFUL, TURBO_VLOG_IS_ON(turbo_log_internal_verbose_level))          \
  (EveryN, n) TURBO_LOG_INTERNAL_LOG_INFO                                    \
      .with_verbosity(turbo_log_internal_verbose_level)                        \
      .printf(__VA_ARGS__)

// ---------------------------------------------------------------------------
// User-facing ZLOG macros
// ---------------------------------------------------------------------------

// ZLOG()
//
// `ZLOG` takes a severity level and a printf-style format string followed by
// zero or more arguments. The format string uses the same syntax as
// `turbo::str_sprintf()`.
//
// Example:
//
//   ZLOG(INFO, "Found %d cookies", num_cookies);
#define ZLOG(severity, ...) TURBO_LOG_INTERNAL_ZLOG_IMPL(_##severity, __VA_ARGS__)

// DZLOG()
//
// `DZLOG` behaves like `ZLOG` in debug mode (`#ifndef NDEBUG`). Otherwise it
// compiles away and does nothing.
#define DZLOG(severity, ...) TURBO_LOG_INTERNAL_DZLOG_IMPL(_##severity, __VA_ARGS__)

// VZLOG()
//
// `VZLOG` uses numeric levels for verbose printf logging, logged at `INFO`
// severity. Aligns with `VKLOG` behavior but uses printf format.
#define VZLOG(verbose_level, ...) \
  TURBO_LOG_INTERNAL_VZLOG_IMPL(verbose_level, __VA_ARGS__)

// DVZLOG()
//
// `DVZLOG` behaves like `VZLOG` in debug mode.
#define DVZLOG(verbose_level, ...) \
  TURBO_LOG_INTERNAL_DVZLOG_IMPL(verbose_level, __VA_ARGS__)

// ZLOG_IF()
//
// `ZLOG_IF` adds a condition. If the condition is false, nothing is logged.
#define ZLOG_IF(severity, condition, ...) \
  TURBO_LOG_INTERNAL_ZLOG_IF_IMPL(_##severity, condition, __VA_ARGS__)

// DZLOG_IF()
#define DZLOG_IF(severity, condition, ...) \
  TURBO_LOG_INTERNAL_DZLOG_IF_IMPL(_##severity, condition, __VA_ARGS__)

// ZLOG_EVERY_N
// ZLOG_FIRST_N
// ZLOG_EVERY_POW_2
// ZLOG_EVERY_N_SEC
//
// Stateful printf-style logging macros. Same semantics as the LOG_* variants,
// but using printf format strings.
#define ZLOG_EVERY_N(severity, n, ...) \
  TURBO_LOG_INTERNAL_ZLOG_EVERY_N_IMPL(_##severity, n, __VA_ARGS__)
#define ZLOG_FIRST_N(severity, n, ...) \
  TURBO_LOG_INTERNAL_ZLOG_FIRST_N_IMPL(_##severity, n, __VA_ARGS__)
#define ZLOG_EVERY_POW_2(severity, ...) \
  TURBO_LOG_INTERNAL_ZLOG_EVERY_POW_2_IMPL(_##severity, __VA_ARGS__)
#define ZLOG_EVERY_N_SEC(severity, n_seconds, ...) \
  TURBO_LOG_INTERNAL_ZLOG_EVERY_N_SEC_IMPL(_##severity, n_seconds, __VA_ARGS__)

#define DZLOG_EVERY_N(severity, n, ...) \
  TURBO_LOG_INTERNAL_DZLOG_EVERY_N_IMPL(_##severity, n, __VA_ARGS__)

#define ZLOG_IF_EVERY_N(severity, condition, n, ...) \
  TURBO_LOG_INTERNAL_ZLOG_IF_EVERY_N_IMPL(_##severity, condition, n, __VA_ARGS__)

#define VZLOG_EVERY_N(verbose_level, n, ...) \
  TURBO_LOG_INTERNAL_VZLOG_EVERY_N_IMPL(verbose_level, n, __VA_ARGS__)
