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

#if defined(__cpp_lib_barrier) && __cpp_lib_barrier >= 201907L
#include <barrier>

namespace turbo {
    using barrier = std::barrier<>;
} // namespace turbo

#else

#include <condition_variable>
#include <mutex>

namespace turbo {

    // C++17-compatible subset of std::barrier<> (arrive_and_wait only).
    class barrier {
    public:
        explicit barrier(std::ptrdiff_t expected)
            : expected_(expected)
            , count_(expected)
            , generation_(0) { }

        barrier(const barrier&) = delete;
        barrier& operator=(const barrier&) = delete;

        void arrive_and_wait() {
            std::unique_lock<std::mutex> lock(mu_);
            const std::ptrdiff_t gen = generation_;
            if (--count_ == 0) {
                ++generation_;
                count_ = expected_;
                cv_.notify_all();
            } else {
                cv_.wait(lock, [this, gen] { return gen != generation_; });
            }
        }

    private:
        std::mutex mu_;
        std::condition_variable cv_;
        const std::ptrdiff_t expected_;
        std::ptrdiff_t count_;
        std::ptrdiff_t generation_;
    };

} // namespace turbo

#endif
