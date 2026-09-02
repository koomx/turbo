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
#include <string_view>

#include <turbo/base/log_severity.h>
#include <turbo/log/log_sink.h>

namespace turbo {

    // No per-sink mutex: LogSinkSet serializes send/flush.
    class AnsiColorSink : public LogSink {
    public:
        explicit AnsiColorSink(std::FILE *file);

        void send(const turbo::LogEntry &entry) override;
        void flush() override;

        static constexpr std::string_view reset = "\033[m";
        static constexpr std::string_view green = "\033[32m";
        static constexpr std::string_view yellow_bold = "\033[33m\033[1m";
        static constexpr std::string_view red_bold = "\033[31m\033[1m";
        static constexpr std::string_view bold_on_red = "\033[1m\033[41m";
        static constexpr std::string_view cyan = "\033[36m";
        static constexpr std::string_view white = "\033[37m";

    private:
        std::FILE *_file{nullptr};
        bool _color_active{false};
    };

}  // namespace turbo
