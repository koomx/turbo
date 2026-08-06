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
// File: log/globals.h
// -----------------------------------------------------------------------------
//
// This header declares global logging library configuration knobs.

#ifndef TURBO_LOG_GLOBALS_H_
#define TURBO_LOG_GLOBALS_H_

#include <turbo/macros/config.h>
#include <turbo/base/log_severity.h>
#include <turbo/log/internal/vlog_config.h>
#include <string_view>

namespace turbo {
    //------------------------------------------------------------------------------
    //  Minimum Log Level
    //------------------------------------------------------------------------------
    //
    // Messages logged at or above this severity are directed to all registered log
    // sinks or skipped otherwise.
    // See turbo/base/log_severity.h for descriptions of severity levels.

    // min_log_level()
    //
    // Returns the value of the Minimum Log Level parameter.
    // This function is async-signal-safe.
    [[nodiscard]] turbo::LogSeverityAtLeast min_log_level();

    // set_min_log_level()
    //
    // Updates the value of Minimum Log Level parameter.
    // This function is async-signal-safe.
    void set_min_log_level(turbo::LogSeverityAtLeast severity);

    namespace log_internal {
        // ScopedMinLogLevel
        //
        // RAII type used to temporarily update the Min Log Level parameter.
        class ScopedMinLogLevel final {
        public:
            explicit ScopedMinLogLevel(turbo::LogSeverityAtLeast severity);

            ScopedMinLogLevel(const ScopedMinLogLevel &) = delete;

            ScopedMinLogLevel &operator=(const ScopedMinLogLevel &) = delete;

            ~ScopedMinLogLevel();

        private:
            turbo::LogSeverityAtLeast saved_severity_;
        };
    } // namespace log_internal

    //------------------------------------------------------------------------------
    // Stderr Threshold
    //------------------------------------------------------------------------------
    //
    // Messages logged at or above this level are directed to stderr in
    // addition to other registered log sinks.
    // See turbo/base/log_severity.h for descriptions of severity levels.

    // stderr_threshold()
    //
    // Returns the value of the Stderr Threshold parameter.
    // This function is async-signal-safe.
    [[nodiscard]] turbo::LogSeverityAtLeast stderr_threshold();

    // set_stderr_threshold()
    //
    // Updates the Stderr Threshold parameter.
    // This function is async-signal-safe.
    void set_stderr_threshold(turbo::LogSeverityAtLeast severity);

    inline void set_stderr_threshold(turbo::LogSeverity severity) {
        turbo::set_stderr_threshold(static_cast<turbo::LogSeverityAtLeast>(severity));
    }

    // ScopedStderrThreshold
    //
    // RAII type used to temporarily update the Stderr Threshold parameter.
    class ScopedStderrThreshold final {
    public:
        explicit ScopedStderrThreshold(turbo::LogSeverityAtLeast severity);

        ScopedStderrThreshold(const ScopedStderrThreshold &) = delete;

        ScopedStderrThreshold &operator=(const ScopedStderrThreshold &) = delete;

        ~ScopedStderrThreshold();

    private:
        turbo::LogSeverityAtLeast saved_severity_;
    };

    //------------------------------------------------------------------------------
    // Log Backtrace At
    //------------------------------------------------------------------------------
    //
    // Users can request an existing `LOG` statement, specified by file and line
    // number, to also include a backtrace when logged.

    // should_log_backtrace_at()
    //
    // Returns true if we should log a backtrace at the specified location.
    namespace log_internal {
        [[nodiscard]] bool should_log_backtrace_at(std::string_view file, int line);
    } // namespace log_internal

    // set_log_backtrace_location()
    //
    // Sets the location the backtrace should be logged at.  If the specified
    // location isn't a `LOG` statement, the effect will be the same as
    // `clear_log_backtrace_location` (but less efficient).
    void set_log_backtrace_location(std::string_view file, int line);

    // clear_log_backtrace_location()
    //
    // Clears the set location so that backtraces will no longer be logged at it.
    void clear_log_backtrace_location();

    //------------------------------------------------------------------------------
    // Prepend Log Prefix
    //------------------------------------------------------------------------------
    //
    // This option tells the logging library that every logged message
    // should include the prefix (severity, date, time, PID, etc.)
    //
    // should_prepend_log_prefix()
    //
    // Returns the value of the Prepend Log Prefix option.
    // This function is async-signal-safe.
    [[nodiscard]] bool should_prepend_log_prefix();

    // enable_log_prefix()
    //
    // Updates the value of the Prepend Log Prefix option.
    // This function is async-signal-safe.
    void enable_log_prefix(bool on_off);

    //------------------------------------------------------------------------------
    // `VKLOG` Configuration
    //------------------------------------------------------------------------------
    //
    // These methods set the `(TURBO_)VKLOG(_IS_ON)` threshold.  They allow
    // programmatic control of global and per-module verbosity.
    //
    // Only `VKLOG`s with a severity level LESS THAN OR EQUAL TO the threshold will
    // be evaluated.
    //
    // For example, if the threshold is 2, then:
    //
    //   VKLOG(2) << "This message will be logged.";
    //   VKLOG(3) << "This message will NOT be logged.";
    //
    // The default threshold is 0. Since `VKLOG` levels must not be negative, a
    // negative threshold value will turn off all VLOGs.

    // set_global_vlog_level()
    //
    // Sets the global `VKLOG` level to threshold. Returns the previous global
    // threshold.
    inline int set_global_vlog_level(int threshold) {
        return turbo::log_internal::update_global_vlog_level(threshold);
    }

    // set_vlog_level()
    //
    // Sets the `VKLOG` threshold for all files that match `module_pattern`,
    // overwriting any prior value. Files that don't match aren't affected.
    // Returns the threshold that previously applied to `module_pattern`.
    inline int set_vlog_level(std::string_view module_pattern, int threshold) {
        return turbo::log_internal::prepend_vmodule(module_pattern, threshold);
    }

    //------------------------------------------------------------------------------
    // Configure Android Native Log Tag
    //------------------------------------------------------------------------------
    //
    // The logging library forwards to the Android system log API when built for
    // Android.  That API takes a string "tag" value in addition to a message and
    // severity level.  The tag is used to identify the source of messages and to
    // filter them.  This library uses the tag "native" by default.

    // set_android_native_tag()
    //
    // Stores a copy of the string pointed to by `tag` and uses it as the Android
    // logging tag thereafter. `tag` must not be null.
    // This function must not be called more than once!
    void set_android_native_tag(const char *tag);

    namespace log_internal {
        // get_android_native_tag()
        //
        // Returns the configured Android logging tag.
        const char *get_android_native_tag();
    } // namespace log_internal

} // namespace turbo

#endif  // TURBO_LOG_GLOBALS_H_
