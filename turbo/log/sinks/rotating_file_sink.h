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

#include <turbo/log/sinks/async_sink.h>
#include <turbo/log/sinks/log_filename.h>
#include <turbo/time/time.h>

namespace turbo {
namespace log_internal {
    class AppendFile;
}  // namespace log_internal

    class RotatingFileSink : public AsyncSink {
    public:
        RotatingFileSink(std::string_view base_filename, size_t max_size_bytes,
                         size_t max_files = 100, int check_interval_s = 60);

        ~RotatingFileSink() override;

    protected:
        void emit(std::string_view text, turbo::Time timestamp) override;
        void emit_flush() override;
        bool need_rewind(size_t addition_size, turbo::Time timestamp) override;
        void reopen() override;

    private:
        void maybe_rotate();

        std::string _active_path;
        BaseFilename _base;
        size_t _max_size;
        size_t _max_files;
        std::unique_ptr<log_internal::AppendFile> _file;
    };

}  // namespace turbo
