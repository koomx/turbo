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
// File: raw_log.h
// -----------------------------------------------------------------------------
//
// Async-signal-safe, non-allocating raw logging to stderr.  Designed for
// low-level code (memory allocators, synchronization primitives, signal
// handlers) that cannot use a full-featured logger.
//
// Printf-style only.  No formatting library, no sinks, no locks.
//
// Usage:
//   KUMO_RAW_LOG(KUMO_LOG_LEVEL_ERROR, "failed: %s", msg);
//   KUMO_RAW_CHECK(ptr != nullptr, "ptr is null");
//   KUMO_RAW_DLOG(KUMO_LOG_LEVEL_WARNING, "skip: %d", n);
//
// Output can be redirected at runtime:
//   KUMO_RAW_LOG_SET_OUTPUT(my_write_fn);  // default = stderr

#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <turbo/macros/attributes/attributes.h>
#include <turbo/macros/optimization/optimization.h>

// ===========================================================================
// Log levels (simple integer constants)
// ===========================================================================

#define KUMO_LOG_LEVEL_INFO    0
#define KUMO_LOG_LEVEL_WARNING 1
#define KUMO_LOG_LEVEL_ERROR   2
#define KUMO_LOG_LEVEL_FATAL   3

// ===========================================================================
// KUMO_RAW_LOG
// ===========================================================================

#ifdef __cplusplus
#define KUMO_RAW_LOG(severity, ...)                                           \
  do {                                                                        \
    constexpr const char* kumo_raw_log_basename_ =                            \
        ::kumo::raw_log_internal::Basename(__FILE__, sizeof(__FILE__) - 1);   \
    ::kumo::raw_log_internal::RawLog(severity, kumo_raw_log_basename_,       \
                                     __LINE__, __VA_ARGS__);                  \
    if (severity == KUMO_LOG_LEVEL_FATAL) {                                   \
      KUMO_INTERNAL_UNREACHABLE_IMPL();                                       \
    }                                                                         \
  } while (0)
#else
#define KUMO_RAW_LOG(severity, ...)                                           \
  do {                                                                        \
    kumo_raw_log_internal_RawLog(severity, __FILE__, __LINE__,                \
                                 __VA_ARGS__);                                \
    if (severity == KUMO_LOG_LEVEL_FATAL) {                                   \
      KUMO_INTERNAL_UNREACHABLE_IMPL();                                       \
    }                                                                         \
  } while (0)
#endif

// ===========================================================================
// KUMO_RAW_CHECK
// ===========================================================================

#if defined(__GNUC__) || defined(__clang__)
#define KUMO_RAW_CHECK(condition, message)                                    \
  do {                                                                        \
    if (__builtin_expect(!(condition), 0)) {                                  \
      KUMO_RAW_LOG(KUMO_LOG_LEVEL_FATAL, "Check %s failed: %s",              \
                   #condition, message);                                      \
    }                                                                         \
  } while (0)
#else
#define KUMO_RAW_CHECK(condition, message)                                    \
  do {                                                                        \
    if (!(condition)) {                                                       \
      KUMO_RAW_LOG(KUMO_LOG_LEVEL_FATAL, "Check %s failed: %s",              \
                   #condition, message);                                      \
    }                                                                         \
  } while (0)
#endif

// ===========================================================================
// KUMO_RAW_DLOG / KUMO_RAW_DCHECK (debug-only, stripped in NDEBUG)
// ===========================================================================

#ifndef NDEBUG
#define KUMO_RAW_DLOG(severity, ...)        KUMO_RAW_LOG(severity, __VA_ARGS__)
#define KUMO_RAW_DCHECK(condition, message) KUMO_RAW_CHECK(condition, message)
#else
#define KUMO_RAW_DLOG(severity, ...) \
  while (0) KUMO_RAW_LOG(severity, __VA_ARGS__)
#define KUMO_RAW_DCHECK(condition, message) \
  while (0) KUMO_RAW_CHECK(condition, message)
#endif

// ===========================================================================
// KUMO_RAW_LOG_SET_OUTPUT
// ===========================================================================

#ifdef __cplusplus

#define KUMO_RAW_LOG_SET_OUTPUT(fn)                                         \
  do {                                                                      \
    ::kumo::raw_log_internal::raw_log_output_fn = (fn);                     \
  } while (0)

// ===========================================================================
// C++ implementation
// ===========================================================================

namespace kumo {
namespace raw_log_internal {

// Default output: write to stderr.
inline void RawStderrWrite(const char* s, size_t len) {
  if (!len) return;
#ifdef _WIN32
  _write(2, s, (unsigned)len);
#else
  write(2, s, len);
#endif
}

// Output function pointer (default = stderr).
inline void (*raw_log_output_fn)(const char*, size_t) = RawStderrWrite;

enum { kRawBufSize = 3000 };

// Compile-time basename extraction.
constexpr const char* Basename(const char* fname, int offset) {
  return offset == 0 || fname[offset - 1] == '/' || fname[offset - 1] == '\\'
             ? fname + offset
             : Basename(fname, offset - 1);
}

KUMO_PRINTF_ATTRIBUTE(4, 0)
inline void RawLogVA(int severity, const char* file, int line,
                     const char* format, va_list ap) {
  char buf[kRawBufSize];
  char* p = buf;
  int remaining = (int)sizeof(buf);

  const char* sev = "UNKN";
  if (severity == KUMO_LOG_LEVEL_INFO)         sev = "INFO";
  else if (severity == KUMO_LOG_LEVEL_WARNING) sev = "WARN";
  else if (severity == KUMO_LOG_LEVEL_ERROR)   sev = "ERROR";
  else if (severity == KUMO_LOG_LEVEL_FATAL)   sev = "FATAL";

  int n = snprintf(p, (size_t)remaining, "%s %s:%d] RAW: ", sev, file, line);
  if (n > 0 && n < remaining) { p += n; remaining -= n; }

  n = vsnprintf(p, (size_t)remaining, format, ap);
  if (n < 0) return;
  if (n >= remaining) {
    n = remaining - 1;
    if (n > 0) {
      const char kTrunc[] = "...";
      size_t tlen = sizeof(kTrunc) - 1;
      if ((size_t)n > tlen) memcpy(p + n - tlen, kTrunc, tlen);
    }
  }
  p += n;
  remaining -= n;

  if (remaining > 1) { *p = '\n'; *(p + 1) = '\0'; }
  else { buf[sizeof(buf) - 1] = '\0'; }

  raw_log_output_fn(buf, strlen(buf));

  if (severity == KUMO_LOG_LEVEL_FATAL) {
    abort();
  }
}

KUMO_PRINTF_ATTRIBUTE(4, 5)
inline void RawLog(int severity, const char* file, int line,
                   const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  RawLogVA(severity, file, line, format, ap);
  va_end(ap);
}

}  // namespace raw_log_internal
}  // namespace kumo

#else  // !__cplusplus (C only)

// ===========================================================================
// C implementation (no namespace, no inline, no constexpr)
// ===========================================================================

#include <turbo/macros/optimization/optimization.h>

#define KUMO_RAW_LOG_SET_OUTPUT(fn)                                           \
  do {                                                                        \
    kumo_raw_log_output_fn_ = (fn);                                           \
  } while (0)

enum { kumo_kRawBufSize_ = 3000 };

static void (*kumo_raw_log_output_fn_)(const char*, size_t) = NULL;

static void kumo_raw_stderr_write_(const char* s, size_t len) {
  if (!len) return;
#ifdef _WIN32
  _write(2, s, (unsigned)len);
#else
  write(2, s, len);
#endif
}

static void kumo_raw_log_va_(int severity, const char* file, int line,
                             const char* format, va_list ap) {
  char buf[kumo_kRawBufSize_];
  char* p = buf;
  int remaining = (int)sizeof(buf);

  const char* sev = "UNKN";
  if (severity == KUMO_LOG_LEVEL_INFO)         sev = "INFO";
  else if (severity == KUMO_LOG_LEVEL_WARNING) sev = "WARN";
  else if (severity == KUMO_LOG_LEVEL_ERROR)   sev = "ERROR";
  else if (severity == KUMO_LOG_LEVEL_FATAL)   sev = "FATAL";

  int n = snprintf(p, (size_t)remaining, "%s %s:%d] RAW: ", sev, file, line);
  if (n > 0 && n < remaining) { p += n; remaining -= n; }

  n = vsnprintf(p, (size_t)remaining, format, ap);
  if (n < 0) return;
  if (n >= remaining) {
    n = remaining - 1;
    if (n > 0) {
      const char kTrunc[] = "...";
      size_t tlen = sizeof(kTrunc) - 1;
      if ((size_t)n > tlen) memcpy(p + n - tlen, kTrunc, tlen);
    }
  }
  p += n;
  remaining -= n;

  if (remaining > 1) { *p = '\n'; *(p + 1) = '\0'; }
  else { buf[sizeof(buf) - 1] = '\0'; }

  void (*out)(const char*, size_t) = kumo_raw_log_output_fn_;
  if (out) out(buf, strlen(buf));
  else kumo_raw_stderr_write_(buf, strlen(buf));

  if (severity == KUMO_LOG_LEVEL_FATAL) {
    abort();
  }
}

KUMO_PRINTF_ATTRIBUTE(4, 5)
static void kumo_raw_log_internal_RawLog(int severity, const char* file,
                                          int line, const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  kumo_raw_log_va_(severity, file, line, format, ap);
  va_end(ap);
}

#endif  // __cplusplus
