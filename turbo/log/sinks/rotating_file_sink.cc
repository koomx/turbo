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

#include <turbo/log/sinks/rotating_file_sink.h>
#include <turbo/log/sinks/internal/append_file.h>
#include <turbo/log/sinks/internal/fs_helper.h>
#include <turbo/time/clock.h>

namespace turbo {

    RotatingFileSink::RotatingFileSink(std::string_view base_filename,
                                       size_t max_size_bytes, size_t max_files,
                                       int check_interval_s)
        : AsyncSink(check_interval_s),
          _active_path(base_filename),
          _base(base_filename),
          _max_size(max_size_bytes),
          _max_files(max_files),
          _file(std::make_unique<log_internal::AppendFile>()) {
        _file->initialize(_active_path);
    }

    RotatingFileSink::~RotatingFileSink() {
        stop();
        if (_file) {
            _file->close();
        }
    }

    void RotatingFileSink::emit(std::string_view text, turbo::Time) {
        maybe_rotate();
        if (!_file) {
            return;
        }
        _file->write(text);
    }

    void RotatingFileSink::emit_flush() {
        if (_file) {
            _file->flush();
        }
    }

    bool RotatingFileSink::need_rewind(size_t addition_size, turbo::Time) {
        if (!_file || _max_size == 0) {
            return false;
        }
        return _file->file_size() + addition_size >= _max_size;
    }

    void RotatingFileSink::reopen() {
        if (_file) {
            _file->reopen();
        }
    }

    void RotatingFileSink::maybe_rotate() {
        if (!_file) {
            return;
        }
        if (_max_size == 0 || _file->file_size() < _max_size) {
            return;
        }
        _file->close();
        if (_max_files > 0) {
            const auto oldest = sequential_log_path(
                _base, static_cast<int64_t>(_max_files));
            log_internal::remove_path(oldest);
            for (size_t i = _max_files; i > 1; --i) {
                const auto src =
                    sequential_log_path(_base, static_cast<int64_t>(i - 1));
                const auto dst =
                    sequential_log_path(_base, static_cast<int64_t>(i));
                if (log_internal::path_exists(src)) {
                    log_internal::remove_path(dst);
                    log_internal::rename_path(src, dst);
                }
            }
            const auto first = sequential_log_path(_base, 1);
            log_internal::remove_path(first);
            log_internal::rename_path(_active_path, first);
        } else {
            log_internal::remove_path(_active_path);
        }
        _file->initialize(_active_path);
    }

}  // namespace turbo
