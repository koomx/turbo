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
// File: log/zcheck.h
// -----------------------------------------------------------------------------
//
// This header declares a family of `ZCHECK` macros — the printf-style
// counterparts of the `KCHECK` macros.  They behave exactly like `KCHECK` and
// friends (terminating the program with a fatal error when the check fails)
// but take a printf-style format string and arguments instead of streamed
// expressions.
//
// Basic invocation:
//
//   ZCHECK(fd >= 0, "open() failed for %s", path);
//
// On failure this terminates the program with a message like:
//
//   Check failed: fd >= 0 open() failed for /etc/passwd
//
// The format string uses the same type-safe printf syntax as
// `turbo::str_sprintf()` and is checked at compile time on GCC and Clang.
// See log/zlog.h for the format specifiers supported.
//
// Except for those whose names begin with `DZCHECK`, these macros are not
// controlled by `NDEBUG` (cf. `assert`), so the check will be executed
// regardless of compilation mode.  As with `KCHECK`, programs must not rely on
// evaluation of `condition` for correctness except in `ZCHECK` itself — the
// check is the point.
//
// Note that the printf-style arguments are only evaluated when the check
// fails; when the check passes they are compiled but not evaluated.

#ifndef TURBO_LOG_ZCHECK_H_
#define TURBO_LOG_ZCHECK_H_

#include <turbo/macros/config.h>
#include <turbo/log/internal/check_impl.h>
#include <turbo/log/internal/log_message.h>

// ---------------------------------------------------------------------------
// Internal ZCHECK implementation macros
// ---------------------------------------------------------------------------

// ZCHECK(condition, fmt, args...)
#define TURBO_LOG_INTERNAL_ZCHECK_IMPL(condition, condition_text, ...) \
  TURBO_LOG_INTERNAL_CONDITION_FATAL(STATELESS,                        \
                                     KUMO_UNLIKELY(!(condition)))       \
      TURBO_LOG_INTERNAL_CHECK(condition_text).printf(__VA_ARGS__)

// QZCHECK(condition, fmt, args...)
#define TURBO_LOG_INTERNAL_QZCHECK_IMPL(condition, condition_text, ...) \
  TURBO_LOG_INTERNAL_CONDITION_QFATAL(STATELESS,                        \
                                      KUMO_UNLIKELY(!(condition)))      \
      TURBO_LOG_INTERNAL_QCHECK(condition_text).printf(__VA_ARGS__)

// PZCHECK(condition, fmt, args...)
#define TURBO_LOG_INTERNAL_PZCHECK_IMPL(condition, condition_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_IMPL(condition, condition_text,             \
                                 __VA_ARGS__)                           \
      .with_perror()

// DZCHECK(condition, fmt, args...)
#ifndef NDEBUG
#define TURBO_LOG_INTERNAL_DZCHECK_IMPL(condition, condition_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_IMPL(condition, condition_text, __VA_ARGS__)
#else
#define TURBO_LOG_INTERNAL_DZCHECK_IMPL(condition, condition_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_IMPL(true || (condition), "true", __VA_ARGS__)
#endif

// ZCHECK_EQ/NE/LE/LT/GE/GT
#define TURBO_LOG_INTERNAL_ZCHECK_OP(name, op, val1, val1_text, val2,       \
    val2_text, ...)                                    \
  while (const char* turbo_nullable turbo_log_internal_zcheck_op_result      \
         [[maybe_unused]] = ::turbo::log_internal::name##Impl(               \
             ::turbo::log_internal::get_referenceable_value(val1),           \
             ::turbo::log_internal::get_referenceable_value(val2),           \
             TURBO_LOG_INTERNAL_STRIP_STRING_LITERAL(val1_text " " #op       \
                                                              " " val2_text))) \
    TURBO_LOG_INTERNAL_CONDITION_FATAL(STATELESS, true)                      \
  TURBO_LOG_INTERNAL_CHECK(::turbo::implicit_cast<const char* turbo_nonnull>( \
                              turbo_log_internal_zcheck_op_result))          \
      .printf(__VA_ARGS__)
#define TURBO_LOG_INTERNAL_QZCHECK_OP(name, op, val1, val1_text, val2,     \
    val2_text, ...)                                 \
  while (const char* turbo_nullable turbo_log_internal_qzcheck_op_result =   \
             ::turbo::log_internal::name##Impl(                             \
                 ::turbo::log_internal::get_referenceable_value(val1),      \
                 ::turbo::log_internal::get_referenceable_value(val2),      \
                 TURBO_LOG_INTERNAL_STRIP_STRING_LITERAL(val1_text " " #op  \
                                                                 " " val2_text))) \
    TURBO_LOG_INTERNAL_CONDITION_QFATAL(STATELESS, true)                    \
  TURBO_LOG_INTERNAL_QCHECK(::turbo::implicit_cast<const char* turbo_nonnull>( \
                               turbo_log_internal_qzcheck_op_result))       \
      .printf(__VA_ARGS__)

// ZCHECK_STREQ/STRNE/STRCASEEQ/STRCASENE
#define TURBO_LOG_INTERNAL_ZCHECK_STROP(func, op, expected, s1, s1_text, s2, \
    s2_text, ...)                           \
  while (const char* turbo_nullable turbo_log_internal_zcheck_strop_result = \
             ::turbo::log_internal::Check##func##expected##Impl(             \
                 (s1), (s2),                                                \
                 TURBO_LOG_INTERNAL_STRIP_STRING_LITERAL(s1_text " " #op     \
                                                                 " " s2_text))) \
    TURBO_LOG_INTERNAL_CONDITION_FATAL(STATELESS, true)                      \
  TURBO_LOG_INTERNAL_CHECK(::turbo::implicit_cast<const char* turbo_nonnull>( \
                              turbo_log_internal_zcheck_strop_result))       \
      .printf(__VA_ARGS__)
#define TURBO_LOG_INTERNAL_QZCHECK_STROP(func, op, expected, s1, s1_text, s2, \
    s2_text, ...)                             \
  while (const char* turbo_nullable turbo_log_internal_qzcheck_strop_result = \
             ::turbo::log_internal::Check##func##expected##Impl(             \
                 (s1), (s2),                                                \
                 TURBO_LOG_INTERNAL_STRIP_STRING_LITERAL(s1_text " " #op     \
                                                                 " " s2_text))) \
    TURBO_LOG_INTERNAL_CONDITION_QFATAL(STATELESS, true)                     \
  TURBO_LOG_INTERNAL_QCHECK(::turbo::implicit_cast<const char* turbo_nonnull>( \
                               turbo_log_internal_qzcheck_strop_result))     \
      .printf(__VA_ARGS__)

// ZCHECK_OK
#define TURBO_LOG_INTERNAL_ZCHECK_OK(val, val_text, ...)                        \
  for (::std::pair<const ::turbo::Status* turbo_nonnull,                        \
                   const char* turbo_nonnull>                                   \
           turbo_log_internal_zcheck_ok_goo;                                    \
       turbo_log_internal_zcheck_ok_goo.first =                                 \
           ::turbo::log_internal::AsStatus(val),                                \
       turbo_log_internal_zcheck_ok_goo.second =                                \
           KUMO_LIKELY(turbo_log_internal_zcheck_ok_goo.first->ok())            \
               ? "" /* Don't use nullptr, to keep the annotation happy */      \
               : ::turbo::status_internal::make_check_fail_string(              \
                     turbo_log_internal_zcheck_ok_goo.first,                    \
                     TURBO_LOG_INTERNAL_STRIP_STRING_LITERAL(val_text " is OK")), \
       !KUMO_LIKELY(turbo_log_internal_zcheck_ok_goo.first->ok());)             \
    TURBO_LOG_INTERNAL_CONDITION_FATAL(STATELESS, true)                         \
  TURBO_LOG_INTERNAL_CHECK(turbo_log_internal_zcheck_ok_goo.second)             \
      .printf(__VA_ARGS__)
#define TURBO_LOG_INTERNAL_QZCHECK_OK(val, val_text, ...)                      \
  for (::std::pair<const ::turbo::Status* turbo_nonnull,                       \
                   const char* turbo_nonnull>                                  \
           turbo_log_internal_qzcheck_ok_goo;                                  \
       turbo_log_internal_qzcheck_ok_goo.first =                               \
           ::turbo::log_internal::AsStatus(val),                               \
       turbo_log_internal_qzcheck_ok_goo.second =                              \
           KUMO_LIKELY(turbo_log_internal_qzcheck_ok_goo.first->ok())          \
               ? "" /* Don't use nullptr, to keep the annotation happy */     \
               : ::turbo::status_internal::make_check_fail_string(             \
                     turbo_log_internal_qzcheck_ok_goo.first,                  \
                     TURBO_LOG_INTERNAL_STRIP_STRING_LITERAL(val_text " is OK")), \
       !KUMO_LIKELY(turbo_log_internal_qzcheck_ok_goo.first->ok());)           \
    TURBO_LOG_INTERNAL_CONDITION_QFATAL(STATELESS, true)                       \
  TURBO_LOG_INTERNAL_QCHECK(turbo_log_internal_qzcheck_ok_goo.second)          \
      .printf(__VA_ARGS__)

// ZCHECK_EQ/NE/LE/LT/GE/GT wrappers
#define TURBO_LOG_INTERNAL_ZCHECK_EQ_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_OP(Check_EQ, ==, val1, val1_text, val2, val2_text,   \
                               __VA_ARGS__)
#define TURBO_LOG_INTERNAL_ZCHECK_NE_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_OP(Check_NE, !=, val1, val1_text, val2, val2_text,   \
                               __VA_ARGS__)
#define TURBO_LOG_INTERNAL_ZCHECK_LE_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_OP(Check_LE, <=, val1, val1_text, val2, val2_text,   \
                               __VA_ARGS__)
#define TURBO_LOG_INTERNAL_ZCHECK_LT_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_OP(Check_LT, <, val1, val1_text, val2, val2_text,    \
                               __VA_ARGS__)
#define TURBO_LOG_INTERNAL_ZCHECK_GE_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_OP(Check_GE, >=, val1, val1_text, val2, val2_text,   \
                               __VA_ARGS__)
#define TURBO_LOG_INTERNAL_ZCHECK_GT_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_OP(Check_GT, >, val1, val1_text, val2, val2_text,    \
                               __VA_ARGS__)
#define TURBO_LOG_INTERNAL_QZCHECK_EQ_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_OP(Check_EQ, ==, val1, val1_text, val2, val2_text,   \
                                __VA_ARGS__)
#define TURBO_LOG_INTERNAL_QZCHECK_NE_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_OP(Check_NE, !=, val1, val1_text, val2, val2_text,   \
                                __VA_ARGS__)
#define TURBO_LOG_INTERNAL_QZCHECK_LE_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_OP(Check_LE, <=, val1, val1_text, val2, val2_text,   \
                                __VA_ARGS__)
#define TURBO_LOG_INTERNAL_QZCHECK_LT_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_OP(Check_LT, <, val1, val1_text, val2, val2_text,    \
                                __VA_ARGS__)
#define TURBO_LOG_INTERNAL_QZCHECK_GE_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_OP(Check_GE, >=, val1, val1_text, val2, val2_text,   \
                                __VA_ARGS__)
#define TURBO_LOG_INTERNAL_QZCHECK_GT_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_OP(Check_GT, >, val1, val1_text, val2, val2_text,    \
                                __VA_ARGS__)
#ifndef NDEBUG
#define TURBO_LOG_INTERNAL_DZCHECK_EQ_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_EQ_IMPL(val1, val1_text, val2, val2_text, __VA_ARGS__)
#define TURBO_LOG_INTERNAL_DZCHECK_NE_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_NE_IMPL(val1, val1_text, val2, val2_text, __VA_ARGS__)
#define TURBO_LOG_INTERNAL_DZCHECK_LE_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_LE_IMPL(val1, val1_text, val2, val2_text, __VA_ARGS__)
#define TURBO_LOG_INTERNAL_DZCHECK_LT_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_LT_IMPL(val1, val1_text, val2, val2_text, __VA_ARGS__)
#define TURBO_LOG_INTERNAL_DZCHECK_GE_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_GE_IMPL(val1, val1_text, val2, val2_text, __VA_ARGS__)
#define TURBO_LOG_INTERNAL_DZCHECK_GT_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_GT_IMPL(val1, val1_text, val2, val2_text, __VA_ARGS__)
#else  // ndef NDEBUG
#define TURBO_LOG_INTERNAL_DZCHECK_EQ_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(val1, val2)
#define TURBO_LOG_INTERNAL_DZCHECK_NE_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(val1, val2)
#define TURBO_LOG_INTERNAL_DZCHECK_LE_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(val1, val2)
#define TURBO_LOG_INTERNAL_DZCHECK_LT_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(val1, val2)
#define TURBO_LOG_INTERNAL_DZCHECK_GE_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(val1, val2)
#define TURBO_LOG_INTERNAL_DZCHECK_GT_IMPL(val1, val1_text, val2, val2_text, ...) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(val1, val2)
#endif  // def NDEBUG

// ZCHECK_OK wrappers
#define TURBO_LOG_INTERNAL_ZCHECK_OK_IMPL(status, status_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_OK((status), status_text, __VA_ARGS__)
#define TURBO_LOG_INTERNAL_QZCHECK_OK_IMPL(status, status_text, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_OK((status), status_text, __VA_ARGS__)
#ifndef NDEBUG
#define TURBO_LOG_INTERNAL_DZCHECK_OK_IMPL(status, status_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_OK((status), status_text, __VA_ARGS__)
#else
#define TURBO_LOG_INTERNAL_DZCHECK_OK_IMPL(status, status_text, ...) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(status, nullptr)
#endif

// ZCHECK_STREQ/STRNE/STRCASEEQ/STRCASENE wrappers
#define TURBO_LOG_INTERNAL_ZCHECK_STREQ_IMPL(s1, s1_text, s2, s2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_STROP(strcmp, ==, true, s1, s1_text, s2,        \
                                  s2_text, __VA_ARGS__)
#define TURBO_LOG_INTERNAL_ZCHECK_STRNE_IMPL(s1, s1_text, s2, s2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_STROP(strcmp, !=, false, s1, s1_text, s2,       \
                                  s2_text, __VA_ARGS__)
#define TURBO_LOG_INTERNAL_ZCHECK_STRCASEEQ_IMPL(s1, s1_text, s2, s2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_STROP(strcasecmp, ==, true, s1, s1_text, s2,       \
                                  s2_text, __VA_ARGS__)
#define TURBO_LOG_INTERNAL_ZCHECK_STRCASENE_IMPL(s1, s1_text, s2, s2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_STROP(strcasecmp, !=, false, s1, s1_text, s2,      \
                                  s2_text, __VA_ARGS__)
#define TURBO_LOG_INTERNAL_QZCHECK_STREQ_IMPL(s1, s1_text, s2, s2_text, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_STROP(strcmp, ==, true, s1, s1_text, s2,       \
                                   s2_text, __VA_ARGS__)
#define TURBO_LOG_INTERNAL_QZCHECK_STRNE_IMPL(s1, s1_text, s2, s2_text, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_STROP(strcmp, !=, false, s1, s1_text, s2,      \
                                   s2_text, __VA_ARGS__)
#define TURBO_LOG_INTERNAL_QZCHECK_STRCASEEQ_IMPL(s1, s1_text, s2, s2_text, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_STROP(strcasecmp, ==, true, s1, s1_text, s2,       \
                                   s2_text, __VA_ARGS__)
#define TURBO_LOG_INTERNAL_QZCHECK_STRCASENE_IMPL(s1, s1_text, s2, s2_text, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_STROP(strcasecmp, !=, false, s1, s1_text, s2,      \
                                   s2_text, __VA_ARGS__)
#ifndef NDEBUG
#define TURBO_LOG_INTERNAL_DZCHECK_STREQ_IMPL(s1, s1_text, s2, s2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_STREQ_IMPL(s1, s1_text, s2, s2_text, __VA_ARGS__)
#define TURBO_LOG_INTERNAL_DZCHECK_STRNE_IMPL(s1, s1_text, s2, s2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_STRNE_IMPL(s1, s1_text, s2, s2_text, __VA_ARGS__)
#define TURBO_LOG_INTERNAL_DZCHECK_STRCASEEQ_IMPL(s1, s1_text, s2, s2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_STRCASEEQ_IMPL(s1, s1_text, s2, s2_text, __VA_ARGS__)
#define TURBO_LOG_INTERNAL_DZCHECK_STRCASENE_IMPL(s1, s1_text, s2, s2_text, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_STRCASENE_IMPL(s1, s1_text, s2, s2_text, __VA_ARGS__)
#else  // ndef NDEBUG
#define TURBO_LOG_INTERNAL_DZCHECK_STREQ_IMPL(s1, s1_text, s2, s2_text, ...) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(s1, s2)
#define TURBO_LOG_INTERNAL_DZCHECK_STRNE_IMPL(s1, s1_text, s2, s2_text, ...) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(s1, s2)
#define TURBO_LOG_INTERNAL_DZCHECK_STRCASEEQ_IMPL(s1, s1_text, s2, s2_text, ...) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(s1, s2)
#define TURBO_LOG_INTERNAL_DZCHECK_STRCASENE_IMPL(s1, s1_text, s2, s2_text, ...) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(s1, s2)
#endif  // def NDEBUG

// ---------------------------------------------------------------------------
// User-facing ZCHECK macros
// ---------------------------------------------------------------------------

// ZCHECK()
//
// `ZCHECK` enforces that `condition` is true, terminating the program with a
// fatal error otherwise.  The failure message is produced by printf-style
// formatting of `fmt` and the following arguments.
//
// Example:
//
//   ZCHECK(fd >= 0, "open(\"%s\") failed", path);
//
// Might produce a message like:
//
//   Check failed: fd >= 0 open("/etc/passwd") failed
#define ZCHECK(condition, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_IMPL((condition), #condition, __VA_ARGS__)

// QZCHECK()
//
// `QZCHECK` behaves like `ZCHECK` but does not print a full stack trace and
// does not run registered error handlers (as `QFATAL`).
#define QZCHECK(condition, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_IMPL((condition), #condition, __VA_ARGS__)

// PZCHECK()
//
// `PZCHECK` behaves like `ZCHECK` but appends a description of the current
// state of `errno` to the failure message.
//
// Example:
//
//   PZCHECK(fd != -1, "posix is difficult");
//
// Might produce a message like:
//
//   Check failed: fd != -1 posix is difficult: No such file or directory [2]
#define PZCHECK(condition, ...) \
  TURBO_LOG_INTERNAL_PZCHECK_IMPL((condition), #condition, __VA_ARGS__)

// DZCHECK()
//
// `DZCHECK` behaves like `ZCHECK` in debug mode and does nothing otherwise (as
// `DLOG`).  When `NDEBUG` is enabled, `DZCHECK` does not evaluate the
// condition.
#define DZCHECK(condition, ...) \
  TURBO_LOG_INTERNAL_DZCHECK_IMPL((condition), #condition, __VA_ARGS__)

// `ZCHECK_EQ` and friends are syntactic sugar for `ZCHECK(x == y, ...)` that
// automatically output the expression being tested and the evaluated values on
// either side.  The format string and arguments are appended to the failure
// message.
//
// Example:
//
//   int x = 3, y = 5;
//   ZCHECK_EQ(2 * x, y, "oops (%d)", 42);
//
// Might produce a message like:
//
//   Check failed: 2 * x == y (6 vs. 5) oops (42)
//
// WARNING: Passing `NULL` as an argument to `ZCHECK_EQ` and similar macros does
// not compile.  Use `nullptr` instead.
#define ZCHECK_EQ(val1, val2, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_EQ_IMPL((val1), #val1, (val2), #val2, __VA_ARGS__)
#define ZCHECK_NE(val1, val2, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_NE_IMPL((val1), #val1, (val2), #val2, __VA_ARGS__)
#define ZCHECK_LE(val1, val2, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_LE_IMPL((val1), #val1, (val2), #val2, __VA_ARGS__)
#define ZCHECK_LT(val1, val2, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_LT_IMPL((val1), #val1, (val2), #val2, __VA_ARGS__)
#define ZCHECK_GE(val1, val2, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_GE_IMPL((val1), #val1, (val2), #val2, __VA_ARGS__)
#define ZCHECK_GT(val1, val2, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_GT_IMPL((val1), #val1, (val2), #val2, __VA_ARGS__)
#define QZCHECK_EQ(val1, val2, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_EQ_IMPL((val1), #val1, (val2), #val2, __VA_ARGS__)
#define QZCHECK_NE(val1, val2, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_NE_IMPL((val1), #val1, (val2), #val2, __VA_ARGS__)
#define QZCHECK_LE(val1, val2, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_LE_IMPL((val1), #val1, (val2), #val2, __VA_ARGS__)
#define QZCHECK_LT(val1, val2, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_LT_IMPL((val1), #val1, (val2), #val2, __VA_ARGS__)
#define QZCHECK_GE(val1, val2, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_GE_IMPL((val1), #val1, (val2), #val2, __VA_ARGS__)
#define QZCHECK_GT(val1, val2, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_GT_IMPL((val1), #val1, (val2), #val2, __VA_ARGS__)
#define DZCHECK_EQ(val1, val2, ...) \
  TURBO_LOG_INTERNAL_DZCHECK_EQ_IMPL((val1), #val1, (val2), #val2, __VA_ARGS__)
#define DZCHECK_NE(val1, val2, ...) \
  TURBO_LOG_INTERNAL_DZCHECK_NE_IMPL((val1), #val1, (val2), #val2, __VA_ARGS__)
#define DZCHECK_LE(val1, val2, ...) \
  TURBO_LOG_INTERNAL_DZCHECK_LE_IMPL((val1), #val1, (val2), #val2, __VA_ARGS__)
#define DZCHECK_LT(val1, val2, ...) \
  TURBO_LOG_INTERNAL_DZCHECK_LT_IMPL((val1), #val1, (val2), #val2, __VA_ARGS__)
#define DZCHECK_GE(val1, val2, ...) \
  TURBO_LOG_INTERNAL_DZCHECK_GE_IMPL((val1), #val1, (val2), #val2, __VA_ARGS__)
#define DZCHECK_GT(val1, val2, ...) \
  TURBO_LOG_INTERNAL_DZCHECK_GT_IMPL((val1), #val1, (val2), #val2, __VA_ARGS__)

// `ZCHECK_OK` and friends validate that the provided `turbo::Status` or
// `turbo::StatusOr<T>` is OK.  If it isn't, they print a failure message that
// includes the actual status (plus the printf-formatted message) and terminate
// the program.
//
// Example:
//
//   ZCHECK_OK(FunctionReturnsStatus(x, y, z), "x=%d y=%d z=%d", x, y, z);
//
// Might produce a message like:
//
//   Check failed: FunctionReturnsStatus(x, y, z) is OK (ABORTED: timeout)
#define ZCHECK_OK(status, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_OK_IMPL((status), #status, __VA_ARGS__)
#define QZCHECK_OK(status, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_OK_IMPL((status), #status, __VA_ARGS__)
#define DZCHECK_OK(status, ...) \
  TURBO_LOG_INTERNAL_DZCHECK_OK_IMPL((status), #status, __VA_ARGS__)

// `ZCHECK_STREQ` and friends provide `ZCHECK_EQ` functionality for C strings,
// i.e., null-terminated char arrays.  The `CASE` versions are case-insensitive.
//
// Example:
//
//   ZCHECK_STREQ(argv[0], "./skynet", "bad program name");
//
// Note that both arguments may be temporary strings which are destroyed by the
// compiler at the end of the current full expression.
#define ZCHECK_STREQ(s1, s2, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_STREQ_IMPL((s1), #s1, (s2), #s2, __VA_ARGS__)
#define ZCHECK_STRNE(s1, s2, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_STRNE_IMPL((s1), #s1, (s2), #s2, __VA_ARGS__)
#define ZCHECK_STRCASEEQ(s1, s2, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_STRCASEEQ_IMPL((s1), #s1, (s2), #s2, __VA_ARGS__)
#define ZCHECK_STRCASENE(s1, s2, ...) \
  TURBO_LOG_INTERNAL_ZCHECK_STRCASENE_IMPL((s1), #s1, (s2), #s2, __VA_ARGS__)
#define QZCHECK_STREQ(s1, s2, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_STREQ_IMPL((s1), #s1, (s2), #s2, __VA_ARGS__)
#define QZCHECK_STRNE(s1, s2, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_STRNE_IMPL((s1), #s1, (s2), #s2, __VA_ARGS__)
#define QZCHECK_STRCASEEQ(s1, s2, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_STRCASEEQ_IMPL((s1), #s1, (s2), #s2, __VA_ARGS__)
#define QZCHECK_STRCASENE(s1, s2, ...) \
  TURBO_LOG_INTERNAL_QZCHECK_STRCASENE_IMPL((s1), #s1, (s2), #s2, __VA_ARGS__)
#define DZCHECK_STREQ(s1, s2, ...) \
  TURBO_LOG_INTERNAL_DZCHECK_STREQ_IMPL((s1), #s1, (s2), #s2, __VA_ARGS__)
#define DZCHECK_STRNE(s1, s2, ...) \
  TURBO_LOG_INTERNAL_DZCHECK_STRNE_IMPL((s1), #s1, (s2), #s2, __VA_ARGS__)
#define DZCHECK_STRCASEEQ(s1, s2, ...) \
  TURBO_LOG_INTERNAL_DZCHECK_STRCASEEQ_IMPL((s1), #s1, (s2), #s2, __VA_ARGS__)
#define DZCHECK_STRCASENE(s1, s2, ...) \
  TURBO_LOG_INTERNAL_DZCHECK_STRCASENE_IMPL((s1), #s1, (s2), #s2, __VA_ARGS__)

#endif  // TURBO_LOG_ZCHECK_H_
