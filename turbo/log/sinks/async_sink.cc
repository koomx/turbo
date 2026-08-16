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

#include <turbo/log/sinks/async_sink.h>

#include <turbo/time/clock.h>
#include <turbo/types/latch.h>

#include <utility>

namespace turbo {

    AsyncSink::AsyncSink(int check_interval_s)
        : stop_(false),
          _check_interval_s(check_interval_s),
          _next_check(turbo::Now() + turbo::Seconds(check_interval_s)) {
        produce_.data.reserve(kBufferSize);
        consume_.data.reserve(kBufferSize);
    }

    AsyncSink::~AsyncSink() { stop(); }

    void AsyncSink::send(const turbo::LogEntry &entry) {
        _send_func(this, entry);
    }

    void AsyncSink::flush() {
        if (_send_func == &AsyncSink::send_sync) {
            emit_flush();
            return;
        }
        {
            std::unique_lock<std::mutex> lock(buffer_mutex_);
            buffer_cond_.notify_one();
        }
    }

    bool AsyncSink::is_async() const {
        return _send_func == &AsyncSink::send_async;
    }

    void AsyncSink::start() {
        {
            std::lock_guard<std::mutex> lock(buffer_mutex_);
            if (_send_func == &AsyncSink::send_async) {
                return;
            }
            stop_ = false;
            _send_func = &AsyncSink::send_async;
        }
        turbo::latch ready(1);
        worker_ = std::thread([this, &ready] {
            ready.count_down();
            worker_loop();
        });
        ready.wait();
    }

    void AsyncSink::stop() {
        {
            std::lock_guard<std::mutex> lock(buffer_mutex_);
            if (_send_func != &AsyncSink::send_async && !worker_.joinable()) {
                return;
            }
            stop_ = true;
            _send_func = &AsyncSink::send_sync;
            buffer_cond_.notify_one();
        }
        if (worker_.joinable()) {
            worker_.join();
        }
        emit_flush();
    }

    void AsyncSink::send_sync(AsyncSink *self, const turbo::LogEntry &entry) {
        const std::string_view text =
            entry.text_message_with_prefix_and_newline();
        const turbo::Time ts = entry.timestamp();
        if (ts >= self->_next_check) {
            self->reopen();
            self->_next_check = ts + turbo::Seconds(self->_check_interval_s);
        }
        self->emit(text, ts);
    }

    void AsyncSink::send_async(AsyncSink *self, const turbo::LogEntry &entry) {
        const std::string_view text =
            entry.text_message_with_prefix_and_newline();
        std::lock_guard<std::mutex> lock(self->buffer_mutex_);
        if (text.size() > kBufferSize ||
            self->produce_.data.size() + text.size() > kBufferSize) {
            return;
        }
        const size_t offset = self->produce_.data.size();
        self->produce_.data.append(text.data(), text.size());
        self->produce_.entries.push_back(
            LogEntryView{offset, text.size(), entry.timestamp()});
        self->buffer_cond_.notify_one();
    }

    void AsyncSink::worker_loop() {
        for (;;) {
            consume_.data.clear();
            consume_.entries.clear();
            {
                std::unique_lock<std::mutex> lock(buffer_mutex_);
                buffer_cond_.wait(lock, [this] {
                    return stop_ || !produce_.entries.empty();
                });
                std::swap(produce_, consume_);
            }
            consume_buffer();
            if (stop_) {
                break;
            }
        }
        consume_.data.clear();
        consume_.entries.clear();
        {
            std::unique_lock<std::mutex> lock(buffer_mutex_);
            std::swap(produce_, consume_);
        }
        consume_buffer();
    }

    void AsyncSink::consume_buffer() {
        if (consume_.entries.empty()) {
            return;
        }
        const turbo::Time ts = consume_.entries.back().timestamp;
        if (ts >= _next_check) {
            reopen();
            _next_check = ts + turbo::Seconds(_check_interval_s);
        }
        if (!need_rewind(consume_.data.size(), ts)) {
            emit(consume_.data, ts);
            return;
        }
        const char *base = consume_.data.data();
        for (const LogEntryView &item : consume_.entries) {
            emit(std::string_view(base + item.offset, item.length),
                 item.timestamp);
        }
    }

}  // namespace turbo
