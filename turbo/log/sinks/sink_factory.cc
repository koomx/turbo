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

#include <turbo/log/sinks/sink_factory.h>

namespace turbo {

    std::unique_ptr<NullSink> create_null_sink() {
        return std::make_unique<NullSink>();
    }

    std::unique_ptr<AnsiColorSink> create_ansi_color_sink(FILE *file) {
        return std::make_unique<AnsiColorSink>(file);
    }

    std::unique_ptr<DailyFileSink> create_daily_file_sink(
        std::string_view base_filename, uint16_t max_files,
        int check_interval_s, bool truncate, bool utc) {
        return std::make_unique<DailyFileSink>(base_filename, max_files,
                                               check_interval_s, truncate, utc);
    }

    std::unique_ptr<HourlyFileSink> create_hourly_file_sink(
        std::string_view base_filename, uint16_t max_files,
        int check_interval_s, bool truncate, bool utc) {
        return std::make_unique<HourlyFileSink>(base_filename, max_files,
                                                check_interval_s, truncate, utc);
    }

    std::unique_ptr<RotatingFileSink> create_rotating_file_sink(
        std::string_view base_filename, size_t max_size_bytes, size_t max_files,
        int check_interval_s) {
        return std::make_unique<RotatingFileSink>(base_filename, max_size_bytes,
                                                  max_files, check_interval_s);
    }

}  // namespace turbo
