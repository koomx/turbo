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

#include <turbo/log/sinks/internal/fs_helper.h>

#include <filesystem>
#include <system_error>

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

namespace turbo {
namespace log_internal {
namespace {
    std::filesystem::path ToPath(std::string_view p) {
        return std::filesystem::path(std::string(p));
    }
}  // namespace

    bool path_exists(std::string_view path) {
        std::error_code ec;
        return std::filesystem::exists(ToPath(path), ec);
    }

    bool create_directories(std::string_view dir) {
        if (dir.empty()) {
            return true;
        }
        std::error_code ec;
        std::filesystem::create_directories(ToPath(dir), ec);
        return !ec;
    }

    bool rename_path(std::string_view from, std::string_view to) {
        std::error_code ec;
        std::filesystem::rename(ToPath(from), ToPath(to), ec);
        return !ec;
    }

    bool remove_path(std::string_view path) {
        std::error_code ec;
        std::filesystem::remove(ToPath(path), ec);
        return !ec;
    }

    std::string parent_dir(std::string_view path) {
        return ToPath(path).parent_path().string();
    }

    size_t file_size_of(FILE *fp) {
        if (fp == nullptr) {
            return 0;
        }
        const long cur = std::ftell(fp);
        if (cur < 0) {
            return 0;
        }
        if (std::fseek(fp, 0, SEEK_END) != 0) {
            return 0;
        }
        const long end = std::ftell(fp);
        std::fseek(fp, cur, SEEK_SET);
        return end > 0 ? static_cast<size_t>(end) : 0;
    }

    bool is_color_terminal(FILE *fp) {
        if (fp == nullptr) {
            return false;
        }
#if defined(_WIN32)
        return _isatty(_fileno(fp)) != 0;
#else
        return ::isatty(::fileno(fp)) != 0;
#endif
    }

}  // namespace log_internal
}  // namespace turbo
