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
#include <string>
#include <string_view>

namespace turbo {
namespace log_internal {

    bool path_exists(std::string_view path);
    bool create_directories(std::string_view dir);
    bool rename_path(std::string_view from, std::string_view to);
    bool remove_path(std::string_view path);
    std::string parent_dir(std::string_view path);
    size_t file_size_of(FILE *fp);
    bool is_color_terminal(FILE *fp);

}  // namespace log_internal
}  // namespace turbo
