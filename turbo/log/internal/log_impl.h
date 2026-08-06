// Copyright 2022 The Abseil Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// SKIP_TURBO_INLINE_NAMESPACE_CHECK

#pragma once


#include <turbo/log/turbo_vlog_is_on.h>
#include <turbo/log/internal/conditions.h>
#include <turbo/log/internal/log_message.h>
#include <turbo/log/internal/strip.h>

// TURBO_LOG()
#define TURBO_LOG_INTERNAL_LOG_IMPL(severity)             \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATELESS, true) \
      TURBO_LOG_INTERNAL_LOG##severity.internal_stream()

// TURBO_PLOG()
#define TURBO_LOG_INTERNAL_PLOG_IMPL(severity)            \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATELESS, true) \
      TURBO_LOG_INTERNAL_LOG##severity.internal_stream()   \
          .with_perror()

// TURBO_DLOG()
#ifndef NDEBUG
#define TURBO_LOG_INTERNAL_DLOG_IMPL(severity)            \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATELESS, true) \
      TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()
#else
#define TURBO_LOG_INTERNAL_DLOG_IMPL(severity)             \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATELESS, false) \
      TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()
#endif

// The `switch` ensures that this expansion is the beginning of a statement (as
// opposed to an expression). The use of both `case 0` and `default` is to
// suppress a compiler warning.
#define TURBO_LOG_INTERNAL_VLOG_IMPL(verbose_level)                     \
  switch (const int turbo_log_internal_verbose_level = (verbose_level)) \
  case 0:                                                              \
  default:                                                             \
    TURBO_LOG_INTERNAL_LOG_IF_IMPL(                                     \
        _INFO, TURBO_VLOG_IS_ON(turbo_log_internal_verbose_level))       \
        .with_verbosity(turbo_log_internal_verbose_level)

#ifndef NDEBUG
#define TURBO_LOG_INTERNAL_DVLOG_IMPL(verbose_level)                    \
  switch (const int turbo_log_internal_verbose_level = (verbose_level)) \
  case 0:                                                              \
  default:                                                             \
    TURBO_LOG_INTERNAL_DLOG_IF_IMPL(                                    \
        _INFO, TURBO_VLOG_IS_ON(turbo_log_internal_verbose_level))       \
        .with_verbosity(turbo_log_internal_verbose_level)
#else
#define TURBO_LOG_INTERNAL_DVLOG_IMPL(verbose_level)                       \
  switch (const int turbo_log_internal_verbose_level = (verbose_level))    \
  case 0:                                                                 \
  default:                                                                \
    TURBO_LOG_INTERNAL_DLOG_IF_IMPL(                                       \
        _INFO, false && TURBO_VLOG_IS_ON(turbo_log_internal_verbose_level)) \
        .with_verbosity(turbo_log_internal_verbose_level)
#endif

#define TURBO_LOG_INTERNAL_LOG_IF_IMPL(severity, condition)    \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATELESS, condition) \
      TURBO_LOG_INTERNAL_LOG##severity.internal_stream()
#define TURBO_LOG_INTERNAL_PLOG_IF_IMPL(severity, condition)   \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATELESS, condition) \
      TURBO_LOG_INTERNAL_LOG##severity.internal_stream()        \
          .with_perror()

#ifndef NDEBUG
#define TURBO_LOG_INTERNAL_DLOG_IF_IMPL(severity, condition)   \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATELESS, condition) \
      TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()
#else
#define TURBO_LOG_INTERNAL_DLOG_IF_IMPL(severity, condition)              \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATELESS, false && (condition)) \
      TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()
#endif

// TURBO_LOG_EVERY_N
#define TURBO_LOG_INTERNAL_LOG_EVERY_N_IMPL(severity, n)            \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, true)(EveryN, n) \
      TURBO_LOG_INTERNAL_LOG##severity.internal_stream()

// TURBO_LOG_FIRST_N
#define TURBO_LOG_INTERNAL_LOG_FIRST_N_IMPL(severity, n)            \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, true)(FirstN, n) \
      TURBO_LOG_INTERNAL_LOG##severity.internal_stream()

// TURBO_LOG_EVERY_POW_2
#define TURBO_LOG_INTERNAL_LOG_EVERY_POW_2_IMPL(severity)           \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, true)(EveryPow2) \
      TURBO_LOG_INTERNAL_LOG##severity.internal_stream()

// TURBO_LOG_EVERY_N_SEC
#define TURBO_LOG_INTERNAL_LOG_EVERY_N_SEC_IMPL(severity, n_seconds)           \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, true)(EveryNSec, n_seconds) \
      TURBO_LOG_INTERNAL_LOG##severity.internal_stream()

#define TURBO_LOG_INTERNAL_PLOG_EVERY_N_IMPL(severity, n)           \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, true)(EveryN, n) \
      TURBO_LOG_INTERNAL_LOG##severity.internal_stream()             \
          .with_perror()

#define TURBO_LOG_INTERNAL_PLOG_FIRST_N_IMPL(severity, n)           \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, true)(FirstN, n) \
      TURBO_LOG_INTERNAL_LOG##severity.internal_stream()             \
          .with_perror()

#define TURBO_LOG_INTERNAL_PLOG_EVERY_POW_2_IMPL(severity)          \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, true)(EveryPow2) \
      TURBO_LOG_INTERNAL_LOG##severity.internal_stream()             \
          .with_perror()

#define TURBO_LOG_INTERNAL_PLOG_EVERY_N_SEC_IMPL(severity, n_seconds)          \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, true)(EveryNSec, n_seconds) \
      TURBO_LOG_INTERNAL_LOG##severity.internal_stream()                        \
          .with_perror()

#ifndef NDEBUG
#define TURBO_LOG_INTERNAL_DLOG_EVERY_N_IMPL(severity, n) \
  TURBO_LOG_INTERNAL_CONDITION_INFO(STATEFUL, true)       \
  (EveryN, n) TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()

#define TURBO_LOG_INTERNAL_DLOG_FIRST_N_IMPL(severity, n) \
  TURBO_LOG_INTERNAL_CONDITION_INFO(STATEFUL, true)       \
  (FirstN, n) TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()

#define TURBO_LOG_INTERNAL_DLOG_EVERY_POW_2_IMPL(severity) \
  TURBO_LOG_INTERNAL_CONDITION_INFO(STATEFUL, true)        \
  (EveryPow2) TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()

#define TURBO_LOG_INTERNAL_DLOG_EVERY_N_SEC_IMPL(severity, n_seconds) \
  TURBO_LOG_INTERNAL_CONDITION_INFO(STATEFUL, true)                   \
  (EveryNSec, n_seconds) TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()

#else  // def NDEBUG
#define TURBO_LOG_INTERNAL_DLOG_EVERY_N_IMPL(severity, n) \
  TURBO_LOG_INTERNAL_CONDITION_INFO(STATEFUL, false)      \
  (EveryN, n) TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()

#define TURBO_LOG_INTERNAL_DLOG_FIRST_N_IMPL(severity, n) \
  TURBO_LOG_INTERNAL_CONDITION_INFO(STATEFUL, false)      \
  (FirstN, n) TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()

#define TURBO_LOG_INTERNAL_DLOG_EVERY_POW_2_IMPL(severity) \
  TURBO_LOG_INTERNAL_CONDITION_INFO(STATEFUL, false)       \
  (EveryPow2) TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()

#define TURBO_LOG_INTERNAL_DLOG_EVERY_N_SEC_IMPL(severity, n_seconds) \
  TURBO_LOG_INTERNAL_CONDITION_INFO(STATEFUL, false)                  \
  (EveryNSec, n_seconds) TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()
#endif  // def NDEBUG

#define TURBO_LOG_INTERNAL_VLOG_EVERY_N_IMPL(verbose_level, n)            \
  switch (const int turbo_log_internal_verbose_level = (verbose_level))   \
  case 0:                                                                \
  default:                                                               \
    TURBO_LOG_INTERNAL_CONDITION_INFO(                                    \
        STATEFUL, TURBO_VLOG_IS_ON(turbo_log_internal_verbose_level))      \
  (EveryN, n) TURBO_LOG_INTERNAL_LOG_INFO.internal_stream().with_verbosity( \
      turbo_log_internal_verbose_level)

#define TURBO_LOG_INTERNAL_VLOG_FIRST_N_IMPL(verbose_level, n)            \
  switch (const int turbo_log_internal_verbose_level = (verbose_level))   \
  case 0:                                                                \
  default:                                                               \
    TURBO_LOG_INTERNAL_CONDITION_INFO(                                    \
        STATEFUL, TURBO_VLOG_IS_ON(turbo_log_internal_verbose_level))      \
  (FirstN, n) TURBO_LOG_INTERNAL_LOG_INFO.internal_stream().with_verbosity( \
      turbo_log_internal_verbose_level)

#define TURBO_LOG_INTERNAL_VLOG_EVERY_POW_2_IMPL(verbose_level)           \
  switch (const int turbo_log_internal_verbose_level = (verbose_level))   \
  case 0:                                                                \
  default:                                                               \
    TURBO_LOG_INTERNAL_CONDITION_INFO(                                    \
        STATEFUL, TURBO_VLOG_IS_ON(turbo_log_internal_verbose_level))      \
  (EveryPow2) TURBO_LOG_INTERNAL_LOG_INFO.internal_stream().with_verbosity( \
      turbo_log_internal_verbose_level)

#define TURBO_LOG_INTERNAL_VLOG_EVERY_N_SEC_IMPL(verbose_level, n_seconds) \
  switch (const int turbo_log_internal_verbose_level = (verbose_level))    \
  case 0:                                                                 \
  default:                                                                \
    TURBO_LOG_INTERNAL_CONDITION_INFO(                                     \
        STATEFUL, TURBO_VLOG_IS_ON(turbo_log_internal_verbose_level))       \
  (EveryNSec, n_seconds) TURBO_LOG_INTERNAL_LOG_INFO.internal_stream()      \
      .with_verbosity(turbo_log_internal_verbose_level)

#define TURBO_LOG_INTERNAL_LOG_IF_EVERY_N_IMPL(severity, condition, n)   \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, condition)(EveryN, n) \
      TURBO_LOG_INTERNAL_LOG##severity.internal_stream()

#define TURBO_LOG_INTERNAL_LOG_IF_FIRST_N_IMPL(severity, condition, n)   \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, condition)(FirstN, n) \
      TURBO_LOG_INTERNAL_LOG##severity.internal_stream()

#define TURBO_LOG_INTERNAL_LOG_IF_EVERY_POW_2_IMPL(severity, condition)  \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, condition)(EveryPow2) \
      TURBO_LOG_INTERNAL_LOG##severity.internal_stream()

#define TURBO_LOG_INTERNAL_LOG_IF_EVERY_N_SEC_IMPL(severity, condition, \
    n_seconds)           \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, condition)(          \
      EveryNSec, n_seconds) TURBO_LOG_INTERNAL_LOG##severity.internal_stream()

#define TURBO_LOG_INTERNAL_PLOG_IF_EVERY_N_IMPL(severity, condition, n)  \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, condition)(EveryN, n) \
      TURBO_LOG_INTERNAL_LOG##severity.internal_stream()                  \
          .with_perror()

#define TURBO_LOG_INTERNAL_PLOG_IF_FIRST_N_IMPL(severity, condition, n)  \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, condition)(FirstN, n) \
      TURBO_LOG_INTERNAL_LOG##severity.internal_stream()                  \
          .with_perror()

#define TURBO_LOG_INTERNAL_PLOG_IF_EVERY_POW_2_IMPL(severity, condition) \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, condition)(EveryPow2) \
      TURBO_LOG_INTERNAL_LOG##severity.internal_stream()                  \
          .with_perror()

#define TURBO_LOG_INTERNAL_PLOG_IF_EVERY_N_SEC_IMPL(severity, condition,      \
    n_seconds)                \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, condition)(                \
      EveryNSec, n_seconds) TURBO_LOG_INTERNAL_LOG##severity.internal_stream() \
      .with_perror()

#ifndef NDEBUG
#define TURBO_LOG_INTERNAL_DLOG_IF_EVERY_N_IMPL(severity, condition, n)  \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, condition)(EveryN, n) \
      TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()

#define TURBO_LOG_INTERNAL_DLOG_IF_FIRST_N_IMPL(severity, condition, n)  \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, condition)(FirstN, n) \
      TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()

#define TURBO_LOG_INTERNAL_DLOG_IF_EVERY_POW_2_IMPL(severity, condition) \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, condition)(EveryPow2) \
      TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()

#define TURBO_LOG_INTERNAL_DLOG_IF_EVERY_N_SEC_IMPL(severity, condition, \
    n_seconds)           \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, condition)(           \
      EveryNSec, n_seconds) TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()

#else  // def NDEBUG
#define TURBO_LOG_INTERNAL_DLOG_IF_EVERY_N_IMPL(severity, condition, n)   \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, false && (condition))( \
      EveryN, n) TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()

#define TURBO_LOG_INTERNAL_DLOG_IF_FIRST_N_IMPL(severity, condition, n)   \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, false && (condition))( \
      FirstN, n) TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()

#define TURBO_LOG_INTERNAL_DLOG_IF_EVERY_POW_2_IMPL(severity, condition)  \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, false && (condition))( \
      EveryPow2) TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()

#define TURBO_LOG_INTERNAL_DLOG_IF_EVERY_N_SEC_IMPL(severity, condition,  \
                                                   n_seconds)            \
  TURBO_LOG_INTERNAL_CONDITION##severity(STATEFUL, false && (condition))( \
      EveryNSec, n_seconds) TURBO_LOG_INTERNAL_DLOG##severity.internal_stream()
#endif  // def NDEBUG

