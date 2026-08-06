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
// File: log/internal/globals.h
// -----------------------------------------------------------------------------
//
// This header file contains various global objects and static helper routines
// use in logging implementation.

#pragma once

#include <turbo/macros/config.h>
#include <turbo/base/log_severity.h>
#include <string_view>
#include <turbo/time/time.h>

namespace turbo {
    namespace log_internal {
        // is_initialized returns true if the logging library is initialized.
        // This function is async-signal-safe
        bool is_initialized();

        // SetLoggingInitialized is called once after logging initialization is done.
        void set_initialized();

        // Unconditionally write a `message` to stderr. If `severity` exceeds kInfo
        // we also flush the stderr stream.
        void write_to_stderr(std::string_view message, turbo::LogSeverity severity);

        // Set the TimeZone used for human-friendly times (for example, the log message
        // prefix) printed by the logging library. This may only be called once.
        void SetTimeZone(turbo::TimeZone tz);

        // Returns the TimeZone used for human-friendly times (for example, the log
        // message prefix) printed by the logging library Returns nullptr prior to
        // initialization.
        const turbo::TimeZone *TimeZone();

        // Returns true if stack traces emitted by the logging library should be
        // symbolized. This function is async-signal-safe.
        bool should_symbolize_log_stack_trace();

        // Enables or disables symbolization of stack traces emitted by the
        // logging library. This function is async-signal-safe.
        void enable_symbolize_log_stack_trace(bool on_off);

        // Returns the maximum number of frames that appear in stack traces
        // emitted by the logging library. This function is async-signal-safe.
        int max_frames_in_log_stack_trace();

        // Sets the maximum number of frames that appear in stack traces emitted by
        // the logging library. This function is async-signal-safe.
        void set_max_frames_in_log_stack_trace(int max_num_frames);

        // Determines whether we exit the program for a KLOG(DFATAL) message in
        // debug mode.  It does this by skipping the call to Fail/fail_quietly.
        // This is intended for testing only.
        //
        // This can have some effects on KLOG(FATAL) as well. Failure messages
        // are always allocated (rather than sharing a buffer), the crash
        // reason is not recorded, the "gwq" status message is not updated,
        // and the stack trace is not recorded.  The KLOG(FATAL) *will* still
        // exit the program. Since this function is used only in testing,
        // these differences are acceptable.
        //
        // Additionally, KLOG(LEVEL(FATAL)) is indistinguishable from KLOG(DFATAL) and
        // will not terminate the program if set_exit_on_dfatal(false) has been called.
        bool exit_on_dfatal();

        // set_exit_on_dfatal() sets the exit_on_dfatal() status
        void set_exit_on_dfatal(bool on_off);

        // Determines if the logging library should suppress logging of stacktraces in
        // the `SIGABRT` handler, typically because we just logged a stacktrace as part
        // of `KLOG(FATAL)` and are about to send ourselves a `SIGABRT` to end the
        // program.
        bool suppress_sigabort_trace();

        // Sets the suppress_sigabort_trace() status and returns the previous state.
        bool set_suppress_sigabort_trace(bool on_off);
    } // namespace log_internal
} // namespace turbo

