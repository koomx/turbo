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

#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <string_view>

#include <turbo/log/log_sink.h>
#include <turbo/log/sinks/log_filename.h>
#include <turbo/time/time.h>

namespace turbo {
namespace log_internal {
    class AppendFile;
}  // namespace log_internal

    // No per-sink mutex: LogSinkSet serializes send/flush.
    class HourlyFileSink : public LogSink {
    public:
        HourlyFileSink(std::string_view base_filename, uint16_t max_files = 84,
                       int check_interval_s = 60, bool truncate = false,
                       bool utc = false);

        ~HourlyFileSink() override;

        void send(const turbo::LogEntry &entry) override;
        void flush() override;

    private:
        void rotate_file(turbo::Time stamp);

        BaseFilename _base;
        bool _truncate;
        bool _utc;
        uint16_t _max_files;
        int _check_interval_s;
        turbo::Time _next_check{};
        std::deque<std::string> _files;
        std::unique_ptr<log_internal::AppendFile> _file;
    };

}  // namespace turbo
