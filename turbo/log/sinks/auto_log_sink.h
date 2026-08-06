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

#include <memory>
#include <type_traits>

#include <turbo/log/log_sink.h>
#include <turbo/log/log_sink_registry.h>

namespace turbo {
namespace log_internal {

    // is_system_managed_sink
    //
    // Detects sinks that are auto-registered and owned by the logging library
    // (currently StderrLogSink, AndroidLogSink and WindowsDebuggerLogSink).
    //
    // Such sinks carry the `turbo_is_system_managed` nested type and must not be
    // managed through `AutoLogSink` (registering them again would FATAL).  Any
    // user-defined sink that does not opt into the marker is allowed by default.
    template <typename T, typename = void>
    struct is_system_managed_sink : std::false_type {};

    template <typename T>
    struct is_system_managed_sink<
        T, std::void_t<typename T::turbo_is_system_managed>> : std::true_type {};

}  // namespace log_internal

    // AutoLogSink
    //
    // RAII wrapper that takes ownership of a `LogSink` (typically created by a
    // `create_*_sink` factory function), registers it with the logging library on
    // construction and removes it on destruction.
    //
    // Usage:
    //   {
    //     turbo::AutoLogSink<turbo::DailyFileSink> sink(
    //         turbo::create_daily_file_sink("logs/app.log"));
    //     KLOG(INFO) << "hello";  // routed to app.log
    //   }  // sink removed and destroyed here
    //
    // System-managed sinks (StderrLogSink, AndroidLogSink,
    // WindowsDebuggerLogSink) are rejected at compile time.
    template <typename SinkT>
    class AutoLogSink {
        static_assert(
            !log_internal::is_system_managed_sink<SinkT>::value,
            "AutoLogSink cannot manage system-managed sinks "
            "(StderrLogSink/AndroidLogSink/WindowsDebuggerLogSink); they are "
            "auto-registered and owned by the logging library.");

    public:
        explicit AutoLogSink(std::unique_ptr<SinkT> sink)
            : sink_(std::move(sink)) {
            turbo::add_log_sink(sink_.get());
        }

        AutoLogSink(const AutoLogSink &) = delete;
        AutoLogSink &operator=(const AutoLogSink &) = delete;

        ~AutoLogSink() {
            if (sink_) {
                turbo::remove_log_sink(sink_.get());
            }
        }

        SinkT *get() const { return sink_.get(); }
        SinkT &operator*() const { return *sink_; }
        SinkT *operator->() const { return sink_.get(); }

    private:
        std::unique_ptr<SinkT> sink_;
    };

}  // namespace turbo
