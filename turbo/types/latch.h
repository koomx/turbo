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
//

#pragma once

#include <cstddef>
#include <limits>

#if defined(__cpp_lib_latch) && __cpp_lib_latch >= 201907L
#include <latch>

namespace turbo {
    using latch = std::latch;
} // namespace turbo

#else

#include <condition_variable>
#include <mutex>

namespace turbo {

    // C++17-compatible subset of std::latch.
    class latch {
    public:
        static constexpr std::ptrdiff_t max() noexcept {
            return std::numeric_limits<std::ptrdiff_t>::max();
        }

        explicit latch(std::ptrdiff_t expected)
            : count_(expected) { }

        latch(const latch&) = delete;
        latch& operator=(const latch&) = delete;

        void count_down(std::ptrdiff_t n = 1) {
            std::lock_guard<std::mutex> lock(mu_);
            count_ -= n;
            if (count_ <= 0) {
                cv_.notify_all();
            }
        }

        bool try_wait() const {
            std::lock_guard<std::mutex> lock(mu_);
            return count_ <= 0;
        }

        void wait() const {
            std::unique_lock<std::mutex> lock(mu_);
            cv_.wait(lock, [this] { return count_ <= 0; });
        }

        void arrive_and_wait(std::ptrdiff_t n = 1) {
            count_down(n);
            wait();
        }

    private:
        mutable std::mutex mu_;
        mutable std::condition_variable cv_;
        std::ptrdiff_t count_;
    };

} // namespace turbo

#endif
