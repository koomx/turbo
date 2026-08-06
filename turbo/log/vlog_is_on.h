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
// File: log/vlog_is_on.h
// -----------------------------------------------------------------------------
//
// This header defines the `VKLOG_IS_ON()` macro that controls the
// variable-verbosity conditional logging.
//
// It's used by `VKLOG` in log.h, or it can also be used directly like this:
//
//   if (VKLOG_IS_ON(2)) {
//     foo_server.RecomputeStatisticsExpensive();
//     KLOG(INFO) << foo_server.LastStatisticsAsString();
//   }
//
// Each source file has an effective verbosity level that's a non-negative
// integer set via `turbo::set_global_vlog_level` and `turbo::set_vlog_level`.
// `VKLOG_IS_ON(n)` is true, and `VKLOG(n)` logs, if that effective verbosity
// level is greater than or equal to `n`.
//
// `set_vlog_level(module_pattern, level)` takes a glob pattern matched against
// filenames.  '?' and '*' are single-character and zero-or-more-character
// wildcards.  Patterns including a slash match full pathnames; otherwise the
// basename is matched.  One suffix (the last . and everything after it) is
// stripped from each filename prior to matching, as is the special suffix "-inl".
//
// Example: turbo::set_vlog_level("module_a", 1);
//
// Module patterns are matched in order; the first match determines the
// verbosity level.  Files that match none use the global VKLOG level (default 0).
// Newer `set_vlog_level` entries are prepended and thus take priority.

#ifndef TURBO_LOG_VLOG_IS_ON_H_
#define TURBO_LOG_VLOG_IS_ON_H_

#include <turbo/log/turbo_vlog_is_on.h>  // IWYU pragma: export

// Each VLOG_IS_ON call site gets its own VLogSite that registers with the
// global linked list of sites to asynchronously update its verbosity level on
// changes to the global/module VKLOG levels. The verbosity can also be set by manually
// calling set_vlog_level.
//
// VLOG_IS_ON is not async signal safe, but it is guaranteed not to allocate
// new memory.
#define VKLOG_IS_ON(verbose_level) TURBO_VLOG_IS_ON(verbose_level)
#define VZLOG_IS_ON(verbose_level) TURBO_VLOG_IS_ON(verbose_level)

#endif  // TURBO_LOG_VLOG_IS_ON_H_
