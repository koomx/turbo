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

#include <turbo/base/internal/thread_identity.h>

#include <thread>  // NOLINT(build/c++11)
#include <vector>

#include <gtest/gtest.h>
#include <turbo/macros/config.h>
#include <turbo/base/internal/spinlock.h>
#include <turbo/base/thread_annotations.h>

namespace turbo {

namespace base_internal {
namespace {

KUMO_CONST_INIT static turbo::base_internal::SpinLock map_lock(
    base_internal::SCHEDULE_KERNEL_ONLY);
KUMO_CONST_INIT static int num_threads_seen TURBO_GUARDED_BY(map_lock);

static void ExerciseThreadIdentityPath() {
  // Without the removed synchronization module, identities are not allocated
  // by GetOrCreateCurrentThreadIdentity. CurrentThreadIdentityIfPresent may
  // still be null; the call must remain safe.
  (void)CurrentThreadIdentityIfPresent();

  turbo::base_internal::SpinLockHolder l(map_lock);
  num_threads_seen++;
}

TEST(ThreadIdentityTest, BasicIdentityWorks) {
  ExerciseThreadIdentityPath();
}

TEST(ThreadIdentityTest, BasicIdentityWorksThreaded) {
  static const int kNumLoops = 3;
  static const int kNumThreads = 32;
  for (int iter = 0; iter < kNumLoops; iter++) {
    std::vector<std::thread> threads;
    for (int i = 0; i < kNumThreads; ++i) {
      threads.push_back(std::thread(ExerciseThreadIdentityPath));
    }
    for (auto& thread : threads) {
      thread.join();
    }
  }

  turbo::base_internal::SpinLockHolder l(map_lock);
  EXPECT_LT(kNumThreads, num_threads_seen);
}

}  // namespace
}  // namespace base_internal

}  // namespace turbo
