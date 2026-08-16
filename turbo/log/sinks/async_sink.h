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
#include <condition_variable>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <turbo/log/log_sink.h>
#include <turbo/time/time.h>

namespace turbo {

    class AsyncSink : public LogSink {
    public:
        static constexpr size_t kBufferSize = 16 * 1024 * 1024;

        explicit AsyncSink(int check_interval_s = 60);
        ~AsyncSink() override;

        void send(const turbo::LogEntry &entry) override;
        void flush() override;
        bool is_async() const override;

        void start();
        void stop();

    protected:
        struct LogEntryView {
            size_t offset;
            size_t length;
            turbo::Time timestamp;
        };
        struct QueueData {
            std::string data;
            std::vector<LogEntryView> entries;
        };

        static void send_sync(AsyncSink *self, const turbo::LogEntry &entry);
        static void send_async(AsyncSink *self, const turbo::LogEntry &entry);
        using send_func = void (*)(AsyncSink *, const turbo::LogEntry &entry);

        virtual void emit(std::string_view text, turbo::Time timestamp) = 0;
        virtual void emit_flush() = 0;
        virtual bool need_rewind(size_t addition_size, turbo::Time timestamp) = 0;
        virtual void reopen() = 0;

        send_func _send_func{&AsyncSink::send_sync};
        QueueData produce_;
        QueueData consume_;
        std::mutex buffer_mutex_;
        std::condition_variable buffer_cond_;
        std::thread worker_;
        bool stop_{false};
        int _check_interval_s;
        turbo::Time _next_check;

    private:
        void worker_loop();
        void consume_buffer();
    };

}  // namespace turbo
