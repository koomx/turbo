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

#include <turbo/log/sinks/ansi_color_sink.h>
#include <turbo/log/sinks/internal/fs_helper.h>

namespace turbo {
namespace {

    std::string_view ColorFor(turbo::LogSeverity sev) {
        switch (sev) {
            case turbo::LogSeverity::kTrace:
                return AnsiColorSink::white;
            case turbo::LogSeverity::kDebug:
                return AnsiColorSink::cyan;
            case turbo::LogSeverity::kInfo:
                return AnsiColorSink::green;
            case turbo::LogSeverity::kWarning:
                return AnsiColorSink::yellow_bold;
            case turbo::LogSeverity::kError:
                return AnsiColorSink::red_bold;
            case turbo::LogSeverity::kFatal:
                return AnsiColorSink::bold_on_red;
        }
        return AnsiColorSink::reset;
    }

}  // namespace

    AnsiColorSink::AnsiColorSink(FILE *file) : _file(file) {
        _color_active = log_internal::is_color_terminal(file);
    }

    void AnsiColorSink::send(const turbo::LogEntry &entry) {
        // Called under LogSinkSet mutex — no sink-level lock.
        if (_file == nullptr) {
            return;
        }
        const std::string_view data =
            entry.text_message_with_prefix_and_newline();
        if (!_color_active) {
            std::fwrite(data.data(), 1, data.size(), _file);
            return;
        }
        const auto color = ColorFor(entry.log_severity());
        std::fwrite(color.data(), 1, color.size(), _file);
        std::fwrite(data.data(), 1, data.size(), _file);
        std::fwrite(reset.data(), 1, reset.size(), _file);
    }

    void AnsiColorSink::flush() {
        if (_file != nullptr) {
            std::fflush(_file);
        }
    }

}  // namespace turbo
