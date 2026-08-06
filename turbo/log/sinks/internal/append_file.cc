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

#include <turbo/log/sinks/internal/append_file.h>
#include <turbo/log/sinks/internal/fs_helper.h>

#include <cerrno>
#include <cstring>

#if !defined(_WIN32)
#include <stdio.h>  // setvbuf / setbuffer
#endif

namespace turbo {
namespace log_internal {

    AppendFile::~AppendFile() { close(); }

    int AppendFile::initialize(std::string_view path) {
        close();
        _path.assign(path.data(), path.size());
        create_directories(parent_dir(_path));
        _file = std::fopen(_path.c_str(), "ab");
        if (_file == nullptr) {
            return errno;
        }
        _written = file_size_of(_file);
#if defined(_WIN32)
        std::setvbuf(_file, _buffer, _IOFBF, sizeof(_buffer));
#else
        ::setvbuf(_file, _buffer, _IOFBF, sizeof(_buffer));
#endif
        return 0;
    }

    int AppendFile::reopen() {
        if (_file != nullptr) {
            std::fclose(_file);
            _file = nullptr;
        }
        create_directories(parent_dir(_path));
        _file = std::fopen(_path.c_str(), "ab");
        if (_file == nullptr) {
            return errno;
        }
        _written = file_size_of(_file);
#if defined(_WIN32)
        std::setvbuf(_file, _buffer, _IOFBF, sizeof(_buffer));
#else
        ::setvbuf(_file, _buffer, _IOFBF, sizeof(_buffer));
#endif
        return 0;
    }

    size_t AppendFile::write(std::string_view message) {
        if (_file == nullptr || message.empty()) {
            return 0;
        }
        size_t written = 0;
        while (written < message.size()) {
            const size_t n = std::fwrite(message.data() + written, 1,
                                         message.size() - written, _file);
            if (n == 0) {
                if (std::ferror(_file)) {
                    break;
                }
            }
            written += n;
        }
        if (written > 0) {
            _written += written;
        }
        return written;
    }

    void AppendFile::flush() {
        if (_file != nullptr) {
            std::fflush(_file);
        }
    }

    void AppendFile::close() {
        if (_file != nullptr) {
            std::fclose(_file);
            _file = nullptr;
        }
    }

}  // namespace log_internal
}  // namespace turbo
