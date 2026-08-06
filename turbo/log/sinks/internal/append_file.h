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
#include <cstdio>
#include <string>
#include <string_view>

namespace turbo {
namespace log_internal {

    // Buffered append-only FILE writer.
    class AppendFile {
    public:
        AppendFile() = default;
        ~AppendFile();

        AppendFile(const AppendFile &) = delete;
        AppendFile &operator=(const AppendFile &) = delete;

        // Open path for append; creates parent dirs. Returns 0 or errno.
        int initialize(std::string_view path);
        int reopen();
        // Bytes written; 0 on failure / empty.
        size_t write(std::string_view message);
        void flush();
        void close();

        size_t file_size() const { return _written; }
        const std::string &file_path() const { return _path; }

    private:
        std::string _path;
        char _buffer[64 * 1024]{};
        size_t _written{0};
        FILE *_file{nullptr};
    };

}  // namespace log_internal
}  // namespace turbo
