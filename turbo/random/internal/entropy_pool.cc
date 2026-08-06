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

#include <turbo/random/internal/entropy_pool.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <iterator>

#include <turbo/algorithm/container.h>
#include <turbo/macros/config.h>
#include <turbo/base/call_once.h>
#include <turbo/base/internal/spinlock.h>
#include <turbo/base/thread_annotations.h>
#include <turbo/random/internal/randen.h>
#include <turbo/random/internal/randen_traits.h>
#include <turbo/random/internal/seed_material.h>
#include <turbo/random/seed_gen_exception.h>
#include <turbo/types/span.h>

using turbo::base_internal::SpinLock;
using turbo::base_internal::SpinLockHolder;

namespace turbo {

namespace random_internal {
namespace {

// RandenPoolEntry is a thread-safe pseudorandom bit generator, implementing a
// single generator within a RandenPool<T>. It is an internal implementation
// detail, and does not aim to conform to [rand.req.urng].
//
// At least 32-byte alignment is required for the state_ array on some ARM
// platforms.  We also want this aligned to a cacheline to eliminate false
// sharing.
class alignas(std::max(size_t{KUMO_CACHELINE_SIZE}, size_t{32}))
    RandenPoolEntry {
 public:
  static constexpr size_t kState = RandenTraits::kStateBytes / sizeof(uint32_t);
  static constexpr size_t kCapacity =
      RandenTraits::kCapacityBytes / sizeof(uint32_t);

  void Init(turbo::Span<const uint32_t> data) {
    SpinLockHolder l(mu_);  // Always uncontested.
    turbo::c_copy(data, std::begin(state_));
    next_ = kState;
  }

  // Copy bytes into out.
  void Fill(uint8_t* out, size_t bytes) TURBO_LOCKS_EXCLUDED(mu_);

  void MaybeRefill() TURBO_EXCLUSIVE_LOCKS_REQUIRED(mu_) {
    if (next_ >= kState) {
      next_ = kCapacity;
      impl_.Generate(state_);
    }
  }

  size_t available() const TURBO_SHARED_LOCKS_REQUIRED(mu_) {
    return kState - next_;
  }

 private:
  // Randen URBG state.
  // At least 32-byte alignment is required by ARM platform code.
  alignas(32) uint32_t state_[kState] TURBO_GUARDED_BY(mu_);
  SpinLock mu_;
  const Randen impl_;
  size_t next_ TURBO_GUARDED_BY(mu_);
};

void RandenPoolEntry::Fill(uint8_t* out, size_t bytes) {
  SpinLockHolder l(mu_);
  while (bytes > 0) {
    MaybeRefill();
    size_t remaining = available() * sizeof(state_[0]);
    size_t to_copy = std::min(bytes, remaining);
    std::memcpy(out, &state_[next_], to_copy);
    out += to_copy;
    bytes -= to_copy;
    next_ += (to_copy + sizeof(state_[0]) - 1) / sizeof(state_[0]);
  }
}

// Number of pooled urbg entries.
static constexpr size_t kPoolSize = 8;

// Shared pool entries.
static turbo::once_flag pool_once;
KUMO_CACHELINE_ALIGNED static RandenPoolEntry* shared_pools[kPoolSize];

// Returns an id in the range [0 ... kPoolSize), which indexes into the
// pool of random engines.
//
// Each thread to access the pool is assigned a sequential ID (without reuse)
// from the pool-id space; the id is cached in a thread_local variable.
// This id is assigned based on the arrival-order of the thread to the
// GetPoolID call; this has no binary, CL, or runtime stability because
// on subsequent runs the order within the same program may be significantly
// different. However, as other thread IDs are not assigned sequentially,
// this is not expected to matter.
size_t GetPoolID() {
  static_assert(kPoolSize >= 1,
                "At least one urbg instance is required for PoolURBG");

  KUMO_CONST_INIT static std::atomic<uint64_t> sequence{0};

#if KUMO_HAVE_THREAD_LOCAL
  static thread_local size_t my_pool_id = kPoolSize;
  if (KUMO_UNLIKELY(my_pool_id == kPoolSize)) {
    my_pool_id = (sequence++ % kPoolSize);
  }
  return my_pool_id;
#else
  static pthread_key_t tid_key = [] {
    pthread_key_t tmp_key;
    int err = pthread_key_create(&tmp_key, nullptr);
    if (err) {
      TURBO_RAW_LOG(FATAL, "pthread_key_create failed with %d", err);
    }
    return tmp_key;
  }();

  // Store the value in the pthread_{get/set}specific. However an uninitialized
  // value is 0, so add +1 to distinguish from the null value.
  uintptr_t my_pool_id =
      reinterpret_cast<uintptr_t>(pthread_getspecific(tid_key));
  if (KUMO_UNLIKELY(my_pool_id == 0)) {
    // No allocated ID, allocate the next value, cache it, and return.
    my_pool_id = (sequence++ % kPoolSize) + 1;
    int err = pthread_setspecific(tid_key, reinterpret_cast<void*>(my_pool_id));
    if (err) {
      TURBO_RAW_LOG(FATAL, "pthread_setspecific failed with %d", err);
    }
  }
  return my_pool_id - 1;
#endif
}

// Allocate and initialize kPoolSize objects of type RandenPoolEntry.
void InitPoolURBG() {
  static constexpr size_t kSeedSize =
      RandenTraits::kStateBytes / sizeof(uint32_t);
  // Read OS entropy once, and use it to initialize each pool entry.
  uint32_t seed_material[kPoolSize * kSeedSize];
  if (!ReadSeedMaterialFromOSEntropy(turbo::MakeSpan(seed_material))) {
    ThrowSeedGenException();
  }
  for (size_t i = 0; i < kPoolSize; i++) {
    shared_pools[i] = new RandenPoolEntry();
    shared_pools[i]->Init(
        turbo::MakeSpan(&seed_material[i * kSeedSize], kSeedSize));
  }
}

// Returns the pool entry for the current thread.
RandenPoolEntry* GetPoolForCurrentThread() {
  turbo::call_once(pool_once, InitPoolURBG);
  return shared_pools[GetPoolID()];
}

}  // namespace

void GetEntropyFromRandenPool(void* dest, size_t bytes) {
  auto* pool = GetPoolForCurrentThread();
  pool->Fill(reinterpret_cast<uint8_t*>(dest), bytes);
}

}  // namespace random_internal

}  // namespace turbo
