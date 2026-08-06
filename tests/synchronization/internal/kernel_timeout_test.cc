// Copyright 2023 The Abseil Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <turbo/synchronization/internal/kernel_timeout.h>

#include <ctime>
#include <chrono>  // NOLINT(build/c++11)
#include <limits>

#include <turbo/macros/config.h>
#include <turbo/random/random.h>
#include <turbo/time/clock.h>
#include <turbo/time/time.h>
#include <gtest/gtest.h>

#if 0  // All supported platforms currently have steady clocks.
#define TURBO_INTERNAL_KERNEL_TIMEOUT_SUPPORTS_STEADY_CLOCK 0
#else
#define TURBO_INTERNAL_KERNEL_TIMEOUT_SUPPORTS_STEADY_CLOCK 1
#endif

static_assert(
    turbo::synchronization_internal::KernelTimeout::SupportsSteadyClock() ==
    static_cast<bool>(TURBO_INTERNAL_KERNEL_TIMEOUT_SUPPORTS_STEADY_CLOCK));

// Randomizing the value of clock_gettime() for CLOCK_MONOTONIC.
// This works by overriding a weak symbol in glibc.
// We should be resistant to this randomization when !SupportsSteadyClock().
#if !TURBO_INTERNAL_KERNEL_TIMEOUT_SUPPORTS_STEADY_CLOCK && \
    !KUMO_HAVE_ADDRESS_SANITIZER &&               \
    !KUMO_HAVE_MEMORY_SANITIZER &&                \
    !KUMO_HAVE_THREAD_SANITIZER
extern "C" int __clock_gettime(clockid_t c, struct timespec* ts);

extern "C" int clock_gettime(clockid_t c, struct timespec* ts) {
  if (c == CLOCK_MONOTONIC &&
      !turbo::synchronization_internal::KernelTimeout::SupportsSteadyClock()) {
    thread_local turbo::BitGen gen;  // NOLINT
    ts->tv_sec = turbo::Uniform(gen, 0, 1'000'000'000);
    ts->tv_nsec = turbo::Uniform(gen, 0, 1'000'000'000);
    return 0;
  }
  return __clock_gettime(c, ts);
}
#endif

namespace {

#if KUMO_HAVE_ADDRESS_SANITIZER ||                        \
    KUMO_HAVE_MEMORY_SANITIZER ||                         \
    KUMO_HAVE_THREAD_SANITIZER || defined(__ANDROID__) || \
    defined(__APPLE__) || defined(_WIN32) || defined(_WIN64)
constexpr turbo::Duration kTimingBound = turbo::Milliseconds(5);
#else
constexpr turbo::Duration kTimingBound = turbo::Microseconds(250);
#endif

using turbo::synchronization_internal::KernelTimeout;

// TODO(b/348224897): re-enabled when the flakiness is fixed.
TEST(KernelTimeout, DISABLED_FiniteTimes) {
  constexpr turbo::Duration kDurationsToTest[] = {
    turbo::ZeroDuration(),
    turbo::Nanoseconds(1),
    turbo::Microseconds(1),
    turbo::Milliseconds(1),
    turbo::Seconds(1),
    turbo::Minutes(1),
    turbo::Hours(1),
    turbo::Hours(1000),
    -turbo::Nanoseconds(1),
    -turbo::Microseconds(1),
    -turbo::Milliseconds(1),
    -turbo::Seconds(1),
    -turbo::Minutes(1),
    -turbo::Hours(1),
    -turbo::Hours(1000),
  };

  for (auto duration : kDurationsToTest) {
    const turbo::Time now = turbo::Now();
    const turbo::Time when = now + duration;
    SCOPED_TRACE(duration);
    KernelTimeout t(when);
    EXPECT_TRUE(t.has_timeout());
    EXPECT_TRUE(t.is_absolute_timeout());
    EXPECT_FALSE(t.is_relative_timeout());
    EXPECT_EQ(turbo::TimeFromTimespec(t.MakeAbsTimespec()), when);
#ifndef _WIN32
    EXPECT_LE(
        turbo::AbsDuration(turbo::Now() + duration -
                          turbo::TimeFromTimespec(
                              t.MakeClockAbsoluteTimespec(CLOCK_REALTIME))),
        turbo::Milliseconds(10));
#endif
    EXPECT_LE(
        turbo::AbsDuration(turbo::DurationFromTimespec(t.MakeRelativeTimespec()) -
                          std::max(duration, turbo::ZeroDuration())),
        kTimingBound);
    EXPECT_EQ(turbo::FromUnixNanos(t.MakeAbsNanos()), when);
    EXPECT_LE(turbo::AbsDuration(turbo::Milliseconds(t.InMillisecondsFromNow()) -
                                std::max(duration, turbo::ZeroDuration())),
              turbo::Milliseconds(5));
    EXPECT_LE(turbo::AbsDuration(turbo::FromChrono(t.ToChronoTimePoint()) - when),
              turbo::Microseconds(1));
    EXPECT_LE(turbo::AbsDuration(turbo::FromChrono(t.ToChronoDuration()) -
                                std::max(duration, turbo::ZeroDuration())),
              kTimingBound);
  }
}

TEST(KernelTimeout, InfiniteFuture) {
  KernelTimeout t(turbo::InfiniteFuture());
  EXPECT_FALSE(t.has_timeout());
  // Callers are expected to check has_timeout() instead of using the methods
  // below, but we do try to do something reasonable if they don't. We may not
  // be able to round-trip back to turbo::InfiniteDuration() or
  // turbo::InfiniteFuture(), but we should return a very large value.
  EXPECT_GT(turbo::TimeFromTimespec(t.MakeAbsTimespec()),
            turbo::Now() + turbo::Hours(100000));
#ifndef _WIN32
  EXPECT_GT(turbo::TimeFromTimespec(t.MakeClockAbsoluteTimespec(CLOCK_REALTIME)),
            turbo::Now() + turbo::Hours(100000));
#endif
  EXPECT_GT(turbo::DurationFromTimespec(t.MakeRelativeTimespec()),
            turbo::Hours(100000));
  EXPECT_GT(turbo::FromUnixNanos(t.MakeAbsNanos()),
            turbo::Now() + turbo::Hours(100000));
  EXPECT_EQ(t.InMillisecondsFromNow(),
            std::numeric_limits<KernelTimeout::DWord>::max());
  EXPECT_EQ(t.ToChronoTimePoint(),
            std::chrono::time_point<std::chrono::system_clock>::max());
  EXPECT_GE(t.ToChronoDuration(), std::chrono::nanoseconds::max());
}

TEST(KernelTimeout, DefaultConstructor) {
  // The default constructor is equivalent to turbo::InfiniteFuture().
  KernelTimeout t;
  EXPECT_FALSE(t.has_timeout());
  // Callers are expected to check has_timeout() instead of using the methods
  // below, but we do try to do something reasonable if they don't. We may not
  // be able to round-trip back to turbo::InfiniteDuration() or
  // turbo::InfiniteFuture(), but we should return a very large value.
  EXPECT_GT(turbo::TimeFromTimespec(t.MakeAbsTimespec()),
            turbo::Now() + turbo::Hours(100000));
#ifndef _WIN32
  EXPECT_GT(turbo::TimeFromTimespec(t.MakeClockAbsoluteTimespec(CLOCK_REALTIME)),
            turbo::Now() + turbo::Hours(100000));
#endif
  EXPECT_GT(turbo::DurationFromTimespec(t.MakeRelativeTimespec()),
            turbo::Hours(100000));
  EXPECT_GT(turbo::FromUnixNanos(t.MakeAbsNanos()),
            turbo::Now() + turbo::Hours(100000));
  EXPECT_EQ(t.InMillisecondsFromNow(),
            std::numeric_limits<KernelTimeout::DWord>::max());
  EXPECT_EQ(t.ToChronoTimePoint(),
            std::chrono::time_point<std::chrono::system_clock>::max());
  EXPECT_GE(t.ToChronoDuration(), std::chrono::nanoseconds::max());
}

TEST(KernelTimeout, TimeMaxNanos) {
  // Time >= kMaxNanos should behave as no timeout.
  KernelTimeout t(turbo::FromUnixNanos(std::numeric_limits<int64_t>::max()));
  EXPECT_FALSE(t.has_timeout());
  // Callers are expected to check has_timeout() instead of using the methods
  // below, but we do try to do something reasonable if they don't. We may not
  // be able to round-trip back to turbo::InfiniteDuration() or
  // turbo::InfiniteFuture(), but we should return a very large value.
  EXPECT_GT(turbo::TimeFromTimespec(t.MakeAbsTimespec()),
            turbo::Now() + turbo::Hours(100000));
#ifndef _WIN32
  EXPECT_GT(turbo::TimeFromTimespec(t.MakeClockAbsoluteTimespec(CLOCK_REALTIME)),
            turbo::Now() + turbo::Hours(100000));
#endif
  EXPECT_GT(turbo::DurationFromTimespec(t.MakeRelativeTimespec()),
            turbo::Hours(100000));
  EXPECT_GT(turbo::FromUnixNanos(t.MakeAbsNanos()),
            turbo::Now() + turbo::Hours(100000));
  EXPECT_EQ(t.InMillisecondsFromNow(),
            std::numeric_limits<KernelTimeout::DWord>::max());
  EXPECT_EQ(t.ToChronoTimePoint(),
            std::chrono::time_point<std::chrono::system_clock>::max());
  EXPECT_GE(t.ToChronoDuration(), std::chrono::nanoseconds::max());
}

TEST(KernelTimeout, Never) {
  // KernelTimeout::Never() is equivalent to turbo::InfiniteFuture().
  KernelTimeout t = KernelTimeout::Never();
  EXPECT_FALSE(t.has_timeout());
  // Callers are expected to check has_timeout() instead of using the methods
  // below, but we do try to do something reasonable if they don't. We may not
  // be able to round-trip back to turbo::InfiniteDuration() or
  // turbo::InfiniteFuture(), but we should return a very large value.
  EXPECT_GT(turbo::TimeFromTimespec(t.MakeAbsTimespec()),
            turbo::Now() + turbo::Hours(100000));
#ifndef _WIN32
  EXPECT_GT(turbo::TimeFromTimespec(t.MakeClockAbsoluteTimespec(CLOCK_REALTIME)),
            turbo::Now() + turbo::Hours(100000));
#endif
  EXPECT_GT(turbo::DurationFromTimespec(t.MakeRelativeTimespec()),
            turbo::Hours(100000));
  EXPECT_GT(turbo::FromUnixNanos(t.MakeAbsNanos()),
            turbo::Now() + turbo::Hours(100000));
  EXPECT_EQ(t.InMillisecondsFromNow(),
            std::numeric_limits<KernelTimeout::DWord>::max());
  EXPECT_EQ(t.ToChronoTimePoint(),
            std::chrono::time_point<std::chrono::system_clock>::max());
  EXPECT_GE(t.ToChronoDuration(), std::chrono::nanoseconds::max());
}

TEST(KernelTimeout, InfinitePast) {
  KernelTimeout t(turbo::InfinitePast());
  EXPECT_TRUE(t.has_timeout());
  EXPECT_TRUE(t.is_absolute_timeout());
  EXPECT_FALSE(t.is_relative_timeout());
  EXPECT_LE(turbo::TimeFromTimespec(t.MakeAbsTimespec()),
            turbo::FromUnixNanos(1));
#ifndef _WIN32
  EXPECT_LE(turbo::TimeFromTimespec(t.MakeClockAbsoluteTimespec(CLOCK_REALTIME)),
            turbo::FromUnixSeconds(1));
#endif
  EXPECT_EQ(turbo::DurationFromTimespec(t.MakeRelativeTimespec()),
            turbo::ZeroDuration());
  EXPECT_LE(turbo::FromUnixNanos(t.MakeAbsNanos()), turbo::FromUnixNanos(1));
  EXPECT_EQ(t.InMillisecondsFromNow(), KernelTimeout::DWord{0});
  EXPECT_LT(t.ToChronoTimePoint(), std::chrono::system_clock::from_time_t(0) +
                                       std::chrono::seconds(1));
  EXPECT_EQ(t.ToChronoDuration(), std::chrono::nanoseconds(0));
}

// TODO(b/348224897): re-enabled when the flakiness is fixed.
TEST(KernelTimeout, DISABLED_FiniteDurations) {
  constexpr turbo::Duration kDurationsToTest[] = {
    turbo::ZeroDuration(),
    turbo::Nanoseconds(1),
    turbo::Microseconds(1),
    turbo::Milliseconds(1),
    turbo::Seconds(1),
    turbo::Minutes(1),
    turbo::Hours(1),
    turbo::Hours(1000),
  };

  for (auto duration : kDurationsToTest) {
    SCOPED_TRACE(duration);
    KernelTimeout t(duration);
    EXPECT_TRUE(t.has_timeout());
    EXPECT_FALSE(t.is_absolute_timeout());
    EXPECT_TRUE(t.is_relative_timeout());
    EXPECT_LE(turbo::AbsDuration(turbo::Now() + duration -
                                turbo::TimeFromTimespec(t.MakeAbsTimespec())),
              turbo::Milliseconds(5));
#ifndef _WIN32
    EXPECT_LE(
        turbo::AbsDuration(turbo::Now() + duration -
                          turbo::TimeFromTimespec(
                              t.MakeClockAbsoluteTimespec(CLOCK_REALTIME))),
        turbo::Milliseconds(5));
#endif
    EXPECT_LE(
        turbo::AbsDuration(turbo::DurationFromTimespec(t.MakeRelativeTimespec()) -
                          duration),
        kTimingBound);
    EXPECT_LE(turbo::AbsDuration(turbo::Now() + duration -
                                turbo::FromUnixNanos(t.MakeAbsNanos())),
              turbo::Milliseconds(5));
    EXPECT_LE(turbo::Milliseconds(t.InMillisecondsFromNow()) - duration,
              turbo::Milliseconds(5));
    EXPECT_LE(turbo::AbsDuration(turbo::Now() + duration -
                                turbo::FromChrono(t.ToChronoTimePoint())),
              kTimingBound);
    EXPECT_LE(
        turbo::AbsDuration(turbo::FromChrono(t.ToChronoDuration()) - duration),
        kTimingBound);
  }
}

// TODO(b/348224897): re-enabled when the flakiness is fixed.
TEST(KernelTimeout, DISABLED_NegativeDurations) {
  constexpr turbo::Duration kDurationsToTest[] = {
    -turbo::ZeroDuration(),
    -turbo::Nanoseconds(1),
    -turbo::Microseconds(1),
    -turbo::Milliseconds(1),
    -turbo::Seconds(1),
    -turbo::Minutes(1),
    -turbo::Hours(1),
    -turbo::Hours(1000),
    -turbo::InfiniteDuration(),
  };

  for (auto duration : kDurationsToTest) {
    // Negative durations should all be converted to zero durations or "now".
    SCOPED_TRACE(duration);
    KernelTimeout t(duration);
    EXPECT_TRUE(t.has_timeout());
    EXPECT_FALSE(t.is_absolute_timeout());
    EXPECT_TRUE(t.is_relative_timeout());
    EXPECT_LE(turbo::AbsDuration(turbo::Now() -
                                turbo::TimeFromTimespec(t.MakeAbsTimespec())),
              turbo::Milliseconds(5));
#ifndef _WIN32
    EXPECT_LE(turbo::AbsDuration(turbo::Now() - turbo::TimeFromTimespec(
                                                  t.MakeClockAbsoluteTimespec(
                                                      CLOCK_REALTIME))),
              turbo::Milliseconds(5));
#endif
    EXPECT_EQ(turbo::DurationFromTimespec(t.MakeRelativeTimespec()),
              turbo::ZeroDuration());
    EXPECT_LE(
        turbo::AbsDuration(turbo::Now() - turbo::FromUnixNanos(t.MakeAbsNanos())),
        turbo::Milliseconds(5));
    EXPECT_EQ(t.InMillisecondsFromNow(), KernelTimeout::DWord{0});
    EXPECT_LE(turbo::AbsDuration(turbo::Now() -
                                turbo::FromChrono(t.ToChronoTimePoint())),
              turbo::Milliseconds(5));
    EXPECT_EQ(t.ToChronoDuration(), std::chrono::nanoseconds(0));
  }
}

TEST(KernelTimeout, InfiniteDuration) {
  KernelTimeout t(turbo::InfiniteDuration());
  EXPECT_FALSE(t.has_timeout());
  // Callers are expected to check has_timeout() instead of using the methods
  // below, but we do try to do something reasonable if they don't. We may not
  // be able to round-trip back to turbo::InfiniteDuration() or
  // turbo::InfiniteFuture(), but we should return a very large value.
  EXPECT_GT(turbo::TimeFromTimespec(t.MakeAbsTimespec()),
            turbo::Now() + turbo::Hours(100000));
#ifndef _WIN32
  EXPECT_GT(turbo::TimeFromTimespec(t.MakeClockAbsoluteTimespec(CLOCK_REALTIME)),
            turbo::Now() + turbo::Hours(100000));
#endif
  EXPECT_GT(turbo::DurationFromTimespec(t.MakeRelativeTimespec()),
            turbo::Hours(100000));
  EXPECT_GT(turbo::FromUnixNanos(t.MakeAbsNanos()),
            turbo::Now() + turbo::Hours(100000));
  EXPECT_EQ(t.InMillisecondsFromNow(),
            std::numeric_limits<KernelTimeout::DWord>::max());
  EXPECT_EQ(t.ToChronoTimePoint(),
            std::chrono::time_point<std::chrono::system_clock>::max());
  EXPECT_GE(t.ToChronoDuration(), std::chrono::nanoseconds::max());
}

TEST(KernelTimeout, DurationMaxNanos) {
  // Duration >= kMaxNanos should behave as no timeout.
  KernelTimeout t(turbo::Nanoseconds(std::numeric_limits<int64_t>::max()));
  EXPECT_FALSE(t.has_timeout());
  // Callers are expected to check has_timeout() instead of using the methods
  // below, but we do try to do something reasonable if they don't. We may not
  // be able to round-trip back to turbo::InfiniteDuration() or
  // turbo::InfiniteFuture(), but we should return a very large value.
  EXPECT_GT(turbo::TimeFromTimespec(t.MakeAbsTimespec()),
            turbo::Now() + turbo::Hours(100000));
#ifndef _WIN32
  EXPECT_GT(turbo::TimeFromTimespec(t.MakeClockAbsoluteTimespec(CLOCK_REALTIME)),
            turbo::Now() + turbo::Hours(100000));
#endif
  EXPECT_GT(turbo::DurationFromTimespec(t.MakeRelativeTimespec()),
            turbo::Hours(100000));
  EXPECT_GT(turbo::FromUnixNanos(t.MakeAbsNanos()),
            turbo::Now() + turbo::Hours(100000));
  EXPECT_EQ(t.InMillisecondsFromNow(),
            std::numeric_limits<KernelTimeout::DWord>::max());
  EXPECT_EQ(t.ToChronoTimePoint(),
            std::chrono::time_point<std::chrono::system_clock>::max());
  EXPECT_GE(t.ToChronoDuration(), std::chrono::nanoseconds::max());
}

TEST(KernelTimeout, OverflowNanos) {
  // Test what happens when KernelTimeout is constructed with an turbo::Duration
  // that would overflow now_nanos + duration.
  int64_t now_nanos = turbo::ToUnixNanos(turbo::Now());
  int64_t limit = std::numeric_limits<int64_t>::max() - now_nanos;
  turbo::Duration duration = turbo::Nanoseconds(limit) + turbo::Seconds(1);
  KernelTimeout t(duration);
  // Timeouts should still be far in the future.
  EXPECT_GT(turbo::TimeFromTimespec(t.MakeAbsTimespec()),
            turbo::Now() + turbo::Hours(100000));
#ifndef _WIN32
  EXPECT_GT(turbo::TimeFromTimespec(t.MakeClockAbsoluteTimespec(CLOCK_REALTIME)),
            turbo::Now() + turbo::Hours(100000));
#endif
  EXPECT_GT(turbo::DurationFromTimespec(t.MakeRelativeTimespec()),
            turbo::Hours(100000));
  EXPECT_GT(turbo::FromUnixNanos(t.MakeAbsNanos()),
            turbo::Now() + turbo::Hours(100000));
  EXPECT_LE(turbo::Milliseconds(t.InMillisecondsFromNow()) - duration,
            turbo::Milliseconds(5));
  EXPECT_GT(t.ToChronoTimePoint(),
            std::chrono::system_clock::now() + std::chrono::hours(100000));
  EXPECT_GT(t.ToChronoDuration(), std::chrono::hours(100000));
}

}  // namespace
