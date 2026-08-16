// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <turbo/base/call_once.h>

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include <gtest/gtest.h>
#include <turbo/macros/config.h>
#include <turbo/base/thread_annotations.h>

namespace turbo {

namespace {

turbo::once_flag once;

std::mutex counters_mu;
std::condition_variable counters_cv;

int running_thread_count TURBO_GUARDED_BY(counters_mu) = 0;
int call_once_invoke_count TURBO_GUARDED_BY(counters_mu) = 0;
int call_once_finished_count TURBO_GUARDED_BY(counters_mu) = 0;
int call_once_return_count TURBO_GUARDED_BY(counters_mu) = 0;
bool done_blocking TURBO_GUARDED_BY(counters_mu) = false;

// Function to be called from turbo::call_once.  Waits for a notification.
void WaitAndIncrement() {
  {
    std::lock_guard lock(counters_mu);
    ++call_once_invoke_count;
  }

  {
    std::unique_lock lock(counters_mu);
    counters_cv.wait(lock, [] { return done_blocking; });
    ++call_once_finished_count;
  }
}

void ThreadBody() {
  {
    std::lock_guard lock(counters_mu);
    ++running_thread_count;
  }
  counters_cv.notify_all();

  turbo::call_once(once, WaitAndIncrement);

  {
    std::lock_guard lock(counters_mu);
    ++call_once_return_count;
  }
}

TEST(CallOnceTest, ExecutionCount) {
  std::vector<std::thread> threads;

  // Start 10 threads all calling call_once on the same once_flag.
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back(ThreadBody);
  }

  // Wait until all ten threads have started, and WaitAndIncrement has been
  // invoked.
  {
    std::unique_lock lock(counters_mu);
    counters_cv.wait(lock, [] {
      return running_thread_count == 10 && call_once_invoke_count == 1;
    });

    // WaitAndIncrement should have been invoked by exactly one call_once()
    // instance.  That thread should be blocking on a notification, and all
    // other call_once instances should be blocking as well.
    EXPECT_EQ(call_once_invoke_count, 1);
    EXPECT_EQ(call_once_finished_count, 0);
    EXPECT_EQ(call_once_return_count, 0);

    // Allow WaitAndIncrement to finish executing.  Once it does, the other
    // call_once waiters will be unblocked.
    done_blocking = true;
  }
  counters_cv.notify_all();

  for (std::thread& thread : threads) {
    thread.join();
  }

  {
    std::lock_guard lock(counters_mu);
    EXPECT_EQ(call_once_invoke_count, 1);
    EXPECT_EQ(call_once_finished_count, 1);
    EXPECT_EQ(call_once_return_count, 10);
  }
}

}  // namespace

}  // namespace turbo
