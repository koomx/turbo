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
// File: log/kcheck.h
// -----------------------------------------------------------------------------
//
// This header declares a family of `KCHECK` macros.
//
// `KCHECK` macros terminate the program with a fatal error if the specified
// condition is not true.
//
// Except for those whose names begin with `DKCHECK`, these macros are not
// controlled by `NDEBUG` (cf. `assert`), so the check will be executed
// regardless of compilation mode. `KCHECK` and friends are thus useful for
// confirming invariants in situations where continuing to run would be worse
// than terminating, e.g., due to risk of data corruption or security
// compromise.  It is also more robust and portable to deliberately terminate
// at a particular place with a useful message and backtrace than to assume some
// ultimately unspecified and unreliable crashing behavior (such as a
// "segmentation fault").

#ifndef TURBO_LOG_CHECK_H_
#define TURBO_LOG_CHECK_H_

#include <turbo/log/internal/check_impl.h>
#include <turbo/log/internal/check_op.h>     // IWYU pragma: export
#include <turbo/log/internal/conditions.h>   // IWYU pragma: export
#include <turbo/log/internal/log_message.h>  // IWYU pragma: export
#include <turbo/log/internal/strip.h>        // IWYU pragma: export

// KCHECK()
//
// `KCHECK` enforces that the `condition` is true. If the condition is false,
// the program is terminated with a fatal error.
//
// The message may include additional information such as stack traces, when
// available.
//
// Example:
//
//   KCHECK(!cheese.empty()) << "Out of Cheese";
//
// Might produce a message like:
//
//   Check failed: !cheese.empty() Out of Cheese
#define KCHECK(condition) TURBO_LOG_INTERNAL_CHECK_IMPL((condition), #condition)

// QKCHECK()
//
// `QKCHECK` behaves like `KCHECK` but does not print a full stack trace and does
// not run registered error handlers (as `QFATAL`).  It is useful when the
// problem is definitely unrelated to program flow, e.g. when validating user
// input.
#define QKCHECK(condition) TURBO_LOG_INTERNAL_QCHECK_IMPL((condition), #condition)

// PKCHECK()
//
// `PCHECK` behaves like `KCHECK` but appends a description of the current state
// of `errno` to the failure message.
//
// Example:
//
//   int fd = open("/var/empty/missing", O_RDONLY);
//   PKCHECK(fd != -1) << "posix is difficult";
//
// Might produce a message like:
//
//   Check failed: fd != -1 posix is difficult: No such file or directory [2]
#define PKCHECK(condition) TURBO_LOG_INTERNAL_PCHECK_IMPL((condition), #condition)

// DKCHECK()
//
// `DKCHECK` behaves like `KCHECK` in debug mode and does nothing otherwise (as
// `DLOG`).  Unlike with `KCHECK` (but as with `assert`), it is not safe to rely
// on evaluation of `condition`: when `NDEBUG` is enabled, DKCHECK does not
// evaluate the condition.
#define DKCHECK(condition) TURBO_LOG_INTERNAL_DCHECK_IMPL((condition), #condition)

// `KCHECK_EQ` and friends are syntactic sugar for `KCHECK(x == y)` that
// automatically output the expression being tested and the evaluated values on
// either side.
//
// Example:
//
//   int x = 3, y = 5;
//   KCHECK_EQ(2 * x, y) << "oops!";
//
// Might produce a message like:
//
//   Check failed: 2 * x == y (6 vs. 5) oops!
//
// The values must implement the appropriate comparison operator as well as
// either `operator<<(std::ostream&, ...)` or `turbo_stringify`.  Care is taken to
// ensure that each argument is evaluated exactly once, and that anything which
// is legal to pass as a function argument is legal here.  In particular, the
// arguments may be temporary expressions which will end up being destroyed at
// the end of the statement,
//
// Example:
//
//   KCHECK_EQ(std::string("abc")[1], 'b');
//
// WARNING: Passing `NULL` as an argument to `KCHECK_EQ` and similar macros does
// not compile.  Use `nullptr` instead.
#define KCHECK_EQ(val1, val2) \
  TURBO_LOG_INTERNAL_CHECK_EQ_IMPL((val1), #val1, (val2), #val2)
#define KCHECK_NE(val1, val2) \
  TURBO_LOG_INTERNAL_CHECK_NE_IMPL((val1), #val1, (val2), #val2)
#define KCHECK_LE(val1, val2) \
  TURBO_LOG_INTERNAL_CHECK_LE_IMPL((val1), #val1, (val2), #val2)
#define KCHECK_LT(val1, val2) \
  TURBO_LOG_INTERNAL_CHECK_LT_IMPL((val1), #val1, (val2), #val2)
#define KCHECK_GE(val1, val2) \
  TURBO_LOG_INTERNAL_CHECK_GE_IMPL((val1), #val1, (val2), #val2)
#define KCHECK_GT(val1, val2) \
  TURBO_LOG_INTERNAL_CHECK_GT_IMPL((val1), #val1, (val2), #val2)
#define QKCHECK_EQ(val1, val2) \
  TURBO_LOG_INTERNAL_QCHECK_EQ_IMPL((val1), #val1, (val2), #val2)
#define QKCHECK_NE(val1, val2) \
  TURBO_LOG_INTERNAL_QCHECK_NE_IMPL((val1), #val1, (val2), #val2)
#define QKCHECK_LE(val1, val2) \
  TURBO_LOG_INTERNAL_QCHECK_LE_IMPL((val1), #val1, (val2), #val2)
#define QKCHECK_LT(val1, val2) \
  TURBO_LOG_INTERNAL_QCHECK_LT_IMPL((val1), #val1, (val2), #val2)
#define QKCHECK_GE(val1, val2) \
  TURBO_LOG_INTERNAL_QCHECK_GE_IMPL((val1), #val1, (val2), #val2)
#define QKCHECK_GT(val1, val2) \
  TURBO_LOG_INTERNAL_QCHECK_GT_IMPL((val1), #val1, (val2), #val2)
#define DKCHECK_EQ(val1, val2) \
  TURBO_LOG_INTERNAL_DCHECK_EQ_IMPL((val1), #val1, (val2), #val2)
#define DKCHECK_NE(val1, val2) \
  TURBO_LOG_INTERNAL_DCHECK_NE_IMPL((val1), #val1, (val2), #val2)
#define DKCHECK_LE(val1, val2) \
  TURBO_LOG_INTERNAL_DCHECK_LE_IMPL((val1), #val1, (val2), #val2)
#define DKCHECK_LT(val1, val2) \
  TURBO_LOG_INTERNAL_DCHECK_LT_IMPL((val1), #val1, (val2), #val2)
#define DKCHECK_GE(val1, val2) \
  TURBO_LOG_INTERNAL_DCHECK_GE_IMPL((val1), #val1, (val2), #val2)
#define DKCHECK_GT(val1, val2) \
  TURBO_LOG_INTERNAL_DCHECK_GT_IMPL((val1), #val1, (val2), #val2)

// `KCHECK_OK` and friends validate that the provided `turbo::Status` or
// `turbo::StatusOr<T>` is OK.  If it isn't, they print a failure message that
// includes the actual status and terminate the program.
//
// As with all `DKCHECK` variants, `DKCHECK` has no effect (not even
// evaluating its argument) if `NDEBUG` is enabled.
//
// Example:
//
//   KCHECK_OK(FunctionReturnsStatus(x, y, z)) << "oops!";
//
// Might produce a message like:
//
//   Check failed: FunctionReturnsStatus(x, y, z) is OK (ABORTED: timeout) oops!
#define KCHECK_OK(status) TURBO_LOG_INTERNAL_CHECK_OK_IMPL((status), #status)
#define QKCHECK_OK(status) TURBO_LOG_INTERNAL_QCHECK_OK_IMPL((status), #status)
#define DKCHECK_OK(status) TURBO_LOG_INTERNAL_DCHECK_OK_IMPL((status), #status)

// `KCHECK_STREQ` and friends provide `KCHECK_EQ` functionality for C strings,
// i.e., null-terminated char arrays.  The `CASE` versions are case-insensitive.
//
// Example:
//
//   KCHECK_STREQ(argv[0], "./skynet");
//
// Note that both arguments may be temporary strings which are destroyed by the
// compiler at the end of the current full expression.
//
// Example:
//
//   KCHECK_STREQ(Foo().c_str(), Bar().c_str());
#define KCHECK_STREQ(s1, s2) \
  TURBO_LOG_INTERNAL_CHECK_STREQ_IMPL((s1), #s1, (s2), #s2)
#define KCHECK_STRNE(s1, s2) \
  TURBO_LOG_INTERNAL_CHECK_STRNE_IMPL((s1), #s1, (s2), #s2)
#define KCHECK_STRCASEEQ(s1, s2) \
  TURBO_LOG_INTERNAL_CHECK_STRCASEEQ_IMPL((s1), #s1, (s2), #s2)
#define KCHECK_STRCASENE(s1, s2) \
  TURBO_LOG_INTERNAL_CHECK_STRCASENE_IMPL((s1), #s1, (s2), #s2)
#define QKCHECK_STREQ(s1, s2) \
  TURBO_LOG_INTERNAL_QCHECK_STREQ_IMPL((s1), #s1, (s2), #s2)
#define QKCHECK_STRNE(s1, s2) \
  TURBO_LOG_INTERNAL_QCHECK_STRNE_IMPL((s1), #s1, (s2), #s2)
#define QKCHECK_STRCASEEQ(s1, s2) \
  TURBO_LOG_INTERNAL_QCHECK_STRCASEEQ_IMPL((s1), #s1, (s2), #s2)
#define QKCHECK_STRCASENE(s1, s2) \
  TURBO_LOG_INTERNAL_QCHECK_STRCASENE_IMPL((s1), #s1, (s2), #s2)
#define DKCHECK_STREQ(s1, s2) \
  TURBO_LOG_INTERNAL_DCHECK_STREQ_IMPL((s1), #s1, (s2), #s2)
#define DKCHECK_STRNE(s1, s2) \
  TURBO_LOG_INTERNAL_DCHECK_STRNE_IMPL((s1), #s1, (s2), #s2)
#define DKCHECK_STRCASEEQ(s1, s2) \
  TURBO_LOG_INTERNAL_DCHECK_STRCASEEQ_IMPL((s1), #s1, (s2), #s2)
#define DKCHECK_STRCASENE(s1, s2) \
  TURBO_LOG_INTERNAL_DCHECK_STRCASENE_IMPL((s1), #s1, (s2), #s2)

#endif  // TURBO_LOG_CHECK_H_
