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

#include <cstddef>
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

    // Size-based rotation: active file is `base_filename`;
    // rotated files are `basename_0001.ext` ... `basename_NNNN.ext`.
    // No per-sink mutex: LogSinkSet serializes send/flush.
    class RotatingFileSink : public LogSink {
    public:
        RotatingFileSink(std::string_view base_filename, size_t max_size_bytes,
                         size_t max_files = 100, int check_interval_s = 60);

        ~RotatingFileSink() override;

        void send(const turbo::LogEntry &entry) override;
        void flush() override;

    private:
        void maybe_rotate(turbo::Time now);

        std::string _active_path;
        BaseFilename _base;
        size_t _max_size;
        size_t _max_files;
        int _check_interval_s;
        turbo::Time _next_check{};
        std::unique_ptr<log_internal::AppendFile> _file;
    };

}  // namespace turbo
