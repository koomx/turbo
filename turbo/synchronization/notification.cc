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

#include <turbo/synchronization/notification.h>

#include <atomic>

#include <turbo/base/internal/raw_logging.h>
#include <turbo/base/internal/tracing.h>
#include <turbo/synchronization/mutex.h>
#include <turbo/time/time.h>

namespace turbo {


void Notification::Notify() {
  base_internal::TraceSignal(this, TraceObjectKind());
  MutexLock l(this->mutex_);

#ifndef NDEBUG
  if (KUMO_UNLIKELY(notified_yet_.load(std::memory_order_relaxed))) {
    TURBO_RAW_LOG(
        FATAL,
        "Notify() method called more than once for Notification object %p",
        static_cast<void *>(this));
  }
#endif

  notified_yet_.store(true, std::memory_order_release);
}

Notification::~Notification() {
  // Make sure that the thread running Notify() exits before the object is
  // destructed.
  MutexLock l(this->mutex_);
}

void Notification::WaitForNotification() const {
  base_internal::TraceWait(this, TraceObjectKind());
  if (!HasBeenNotifiedInternal(&this->notified_yet_)) {
    this->mutex_.LockWhen(
        Condition(&HasBeenNotifiedInternal, &this->notified_yet_));
    this->mutex_.unlock();
  }
  base_internal::TraceContinue(this, TraceObjectKind());
}

bool Notification::WaitForNotificationWithTimeout(
    turbo::Duration timeout) const {
  base_internal::TraceWait(this, TraceObjectKind());
  bool notified = HasBeenNotifiedInternal(&this->notified_yet_);
  if (!notified) {
    notified = this->mutex_.LockWhenWithTimeout(
        Condition(&HasBeenNotifiedInternal, &this->notified_yet_), timeout);
    this->mutex_.unlock();
  }
  base_internal::TraceContinue(notified ? this : nullptr, TraceObjectKind());
  return notified;
}

bool Notification::WaitForNotificationWithDeadline(turbo::Time deadline) const {
  base_internal::TraceWait(this, TraceObjectKind());
  bool notified = HasBeenNotifiedInternal(&this->notified_yet_);
  if (!notified) {
    notified = this->mutex_.LockWhenWithDeadline(
        Condition(&HasBeenNotifiedInternal, &this->notified_yet_), deadline);
    this->mutex_.unlock();
  }
  base_internal::TraceContinue(notified ? this : nullptr, TraceObjectKind());
  return notified;
}


}  // namespace turbo
