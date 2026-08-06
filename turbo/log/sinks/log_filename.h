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
#include <string>
#include <string_view>

#include <turbo/time/time.h>

namespace turbo {

    struct BaseFilename {
        explicit BaseFilename(std::string_view filename);

        std::string directory;
        std::string basename;
        std::string extension;  // includes leading '.' when present
    };

    // Active file: dir/basename.ext
    // Rotated:     dir/basename_0001.ext ...
    std::string sequential_log_path(const BaseFilename &base, int64_t seq);

    // dir/basename_YYYY-MM-DD.ext
    std::string daily_log_path(const BaseFilename &base, turbo::Time tp,
                               bool utc);

    // dir/basename_YYYY-MM-DD-HH.ext
    std::string hourly_log_path(const BaseFilename &base, turbo::Time tp,
                                bool utc);

}  // namespace turbo
