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

#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>

#include <turbo/log/sinks/ansi_color_sink.h>
#include <turbo/log/sinks/daily_file_sink.h>
#include <turbo/log/sinks/hourly_file_sink.h>
#include <turbo/log/sinks/null_sink.h>
#include <turbo/log/sinks/rotating_file_sink.h>

namespace turbo {

    // Convenience factory functions for creating log sinks.
    //
    // These functions only construct a sink and return its ownership to the
    // caller; they do not register it.  Pair them with `turbo::AutoLogSink` to
    // automatically register on construction and remove on destruction.

    std::unique_ptr<NullSink> create_null_sink();

    std::unique_ptr<AnsiColorSink> create_ansi_color_sink(std::FILE *file);

    std::unique_ptr<DailyFileSink> create_daily_file_sink(
        std::string_view base_filename, uint16_t max_files = 7,
        int check_interval_s = 60, bool truncate = false, bool utc = false);

    std::unique_ptr<HourlyFileSink> create_hourly_file_sink(
        std::string_view base_filename, uint16_t max_files = 84,
        int check_interval_s = 60, bool truncate = false, bool utc = false);

    std::unique_ptr<RotatingFileSink> create_rotating_file_sink(
        std::string_view base_filename, size_t max_size_bytes,
        size_t max_files = 100, int check_interval_s = 60);

}  // namespace turbo
