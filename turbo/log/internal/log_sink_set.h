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
// File: log/internal/log_sink_set.h
// -----------------------------------------------------------------------------

#pragma once


#include <turbo/macros/config.h>
#include <turbo/log/log_entry.h>
#include <turbo/log/log_sink.h>
#include <turbo/types/span.h>

namespace turbo {
    namespace log_internal {
        // Returns true if a globally-registered `LogSink`'s `send()` is currently
        // being invoked on this thread.
        bool thread_is_logging_to_log_sink();

        // This function may log to two sets of sinks:
        //
        // * If `extra_sinks_only` is true, it will dispatch only to `extra_sinks`.
        //   `LogMessage::to_sink_also` and `LogMessage::to_sink_only` are used to attach
        //    extra sinks to the entry.
        // * Otherwise it will also log to the global sinks set. This set is managed
        //   by `turbo::add_log_sink` and `turbo::remove_log_sink`.
        void log_to_sinks(const turbo::LogEntry &entry,
                          turbo::Span<turbo::LogSink *> extra_sinks, bool extra_sinks_only);

        // Implementation for operations with log sink set.
        void add_log_sink(turbo::LogSink *sink);

        void remove_log_sink(turbo::LogSink *sink);

        void flush_log_sinks();
    } // namespace log_internal
} // namespace turbo
