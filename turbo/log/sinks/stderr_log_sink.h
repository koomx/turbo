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

#pragma once

#include <turbo/macros/config.h>
#include <turbo/base/call_once.h>
#include <turbo/base/log_severity.h>
#include <turbo/log/globals.h>
#include <turbo/log/internal/globals.h>
#include <turbo/log/log_sink.h>

namespace turbo {

    // Default stderr sink. Writes formatted text to stderr, respecting
    // stderr_threshold and initialization state. Handles stacktraces for
    // FATAL entries. No per-sink mutex: LogSinkSet serializes send.
    // System-managed: auto-registered by the logging library; must not be
    // managed through `AutoLogSink`.
    class StderrLogSink final : public LogSink {
    public:
        using turbo_is_system_managed = void;

        ~StderrLogSink() override = default;

        void send(const turbo::LogEntry &entry) override {
            if (entry.log_severity() < turbo::stderr_threshold() &&
                turbo::log_internal::is_initialized()) {
                return;
            }

            KUMO_CONST_INIT static turbo::once_flag warn_if_not_initialized;
            turbo::call_once(warn_if_not_initialized, []() {
                if (turbo::log_internal::is_initialized()) return;
                const char w[] =
                        "WARNING: All log messages before turbo::initialize_log() is called"
                        " are written to STDERR\n";
                turbo::log_internal::write_to_stderr(w, turbo::LogSeverity::kWarning);
            });

            if (!entry.stacktrace().empty()) {
                turbo::log_internal::write_to_stderr(entry.stacktrace(),
                                                     entry.log_severity());
            } else {
                turbo::log_internal::write_to_stderr(
                    entry.text_message_with_prefix_and_newline(), entry.log_severity());
            }
        }
    };

}  // namespace turbo
