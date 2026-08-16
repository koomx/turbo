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

#include <turbo/log/sinks/daily_file_sink.h>
#include <turbo/log/sinks/internal/append_file.h>
#include <turbo/log/sinks/internal/fs_helper.h>
#include <turbo/time/clock.h>

namespace turbo {

    DailyFileSink::DailyFileSink(std::string_view base_filename,
                                 uint16_t max_files, int check_interval_s,
                                 bool truncate, bool utc)
        : AsyncSink(check_interval_s),
          _base(base_filename),
          _truncate(truncate),
          _utc(utc),
          _max_files(max_files),
          _file(std::make_unique<log_internal::AppendFile>()) {
        rotate_file(turbo::Now());
    }

    DailyFileSink::~DailyFileSink() {
        stop();
        if (_file) {
            _file->close();
        }
    }

    void DailyFileSink::emit(std::string_view text, turbo::Time timestamp) {
        rotate_file(timestamp);
        if (!_file) {
            return;
        }
        _file->write(text);
    }

    void DailyFileSink::emit_flush() {
        if (_file) {
            _file->flush();
        }
    }

    bool DailyFileSink::need_rewind(size_t, turbo::Time timestamp) {
        if (!_file) {
            return false;
        }
        return daily_log_path(_base, timestamp, _utc) != _file->file_path();
    }

    void DailyFileSink::reopen() {
        if (_file) {
            _file->reopen();
        }
    }

    void DailyFileSink::rotate_file(turbo::Time stamp) {
        const std::string path = daily_log_path(_base, stamp, _utc);
        if (_file && _file->file_path() == path) {
            return;
        }
        if (_truncate) {
            log_internal::remove_path(path);
        }
        if (_file) {
            _file->close();
        }
        _file = std::make_unique<log_internal::AppendFile>();
        _file->initialize(path);

        if (_max_files == 0) {
            return;
        }
        if (_files.size() >= _max_files) {
            log_internal::remove_path(_files.front());
            _files.pop_front();
        }
        _files.push_back(path);
    }

}  // namespace turbo
