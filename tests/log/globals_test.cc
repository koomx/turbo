//
// Copyright 2022 The Abseil Authors.
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

#include <turbo/log/globals.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <turbo/macros/config.h>
#include <turbo/base/log_severity.h>
#include <turbo/log/internal/globals.h>
#include <tests/log/internal/test_helpers.h>
#include <turbo/log/klog.h>
#include <tests/log/scoped_mock_log.h>

namespace {
using ::testing::_;
using ::testing::StrEq;

auto* test_env KUMO_ATTRIBUTE_UNUSED = ::testing::AddGlobalTestEnvironment(
    new turbo::log_internal::LogTestEnvironment);

constexpr static turbo::LogSeverityAtLeast DefaultMinLogLevel() {
  return turbo::LogSeverityAtLeast::kInfo;
}
constexpr static turbo::LogSeverityAtLeast DefaultStderrThreshold() {
  return turbo::LogSeverityAtLeast::kError;
}

TEST(TestGlobals, min_log_level) {
  EXPECT_EQ(turbo::min_log_level(), DefaultMinLogLevel());
  turbo::set_min_log_level(turbo::LogSeverityAtLeast::kError);
  EXPECT_EQ(turbo::min_log_level(), turbo::LogSeverityAtLeast::kError);
  turbo::set_min_log_level(DefaultMinLogLevel());
}

TEST(TestGlobals, ScopedMinLogLevel) {
  EXPECT_EQ(turbo::min_log_level(), DefaultMinLogLevel());
  {
    turbo::log_internal::ScopedMinLogLevel scoped_stderr_threshold(
        turbo::LogSeverityAtLeast::kError);
    EXPECT_EQ(turbo::min_log_level(), turbo::LogSeverityAtLeast::kError);
  }
  EXPECT_EQ(turbo::min_log_level(), DefaultMinLogLevel());
}

TEST(TestGlobals, stderr_threshold) {
  EXPECT_EQ(turbo::stderr_threshold(), DefaultStderrThreshold());
  turbo::set_stderr_threshold(turbo::LogSeverityAtLeast::kError);
  EXPECT_EQ(turbo::stderr_threshold(), turbo::LogSeverityAtLeast::kError);
  turbo::set_stderr_threshold(DefaultStderrThreshold());
}

TEST(TestGlobals, ScopedStderrThreshold) {
  EXPECT_EQ(turbo::stderr_threshold(), DefaultStderrThreshold());
  {
    turbo::ScopedStderrThreshold scoped_stderr_threshold(
        turbo::LogSeverityAtLeast::kError);
    EXPECT_EQ(turbo::stderr_threshold(), turbo::LogSeverityAtLeast::kError);
  }
  EXPECT_EQ(turbo::stderr_threshold(), DefaultStderrThreshold());
}

TEST(TestGlobals, LogBacktraceAt) {
  EXPECT_FALSE(turbo::log_internal::should_log_backtrace_at("some_file.cc", 111));
  turbo::set_log_backtrace_location("some_file.cc", 111);
  EXPECT_TRUE(turbo::log_internal::should_log_backtrace_at("some_file.cc", 111));
  EXPECT_FALSE(
      turbo::log_internal::should_log_backtrace_at("another_file.cc", 222));
}

TEST(TestGlobals, LogPrefix) {
  EXPECT_TRUE(turbo::should_prepend_log_prefix());
  turbo::enable_log_prefix(false);
  EXPECT_FALSE(turbo::should_prepend_log_prefix());
  turbo::enable_log_prefix(true);
  EXPECT_TRUE(turbo::should_prepend_log_prefix());
}

TEST(TestGlobals, set_global_vlog_level) {
  EXPECT_EQ(turbo::set_global_vlog_level(42), 0);
  EXPECT_EQ(turbo::set_global_vlog_level(1337), 42);
  // Restore the value since it affects the default unset module value for
  // `set_vlog_level()`.
  EXPECT_EQ(turbo::set_global_vlog_level(0), 1337);
}

TEST(TestGlobals, set_vlog_level) {
  EXPECT_EQ(turbo::set_vlog_level("setvloglevel", 42), 0);
  EXPECT_EQ(turbo::set_vlog_level("setvloglevel", 1337), 42);
  EXPECT_EQ(turbo::set_vlog_level("othersetvloglevel", 50), 0);

  EXPECT_EQ(turbo::set_vlog_level("*pattern*", 1), 0);
  EXPECT_EQ(turbo::set_vlog_level("*less_generic_pattern*", 2), 1);
  // "pattern_match" matches the pattern "*pattern*". Therefore, the previous
  // level must be 1.
  EXPECT_EQ(turbo::set_vlog_level("pattern_match", 3), 1);
  // "less_generic_pattern_match" matches the pattern "*pattern*". Therefore,
  // the previous level must be 2.
  EXPECT_EQ(turbo::set_vlog_level("less_generic_pattern_match", 4), 2);
}

TEST(TestGlobals, AndroidLogTag) {
  // Verify invalid tags result in a check failure.
  EXPECT_DEATH_IF_SUPPORTED(turbo::set_android_native_tag(nullptr), ".*");

  // Verify valid tags applied.
  EXPECT_THAT(turbo::log_internal::get_android_native_tag(), StrEq("native"));
  turbo::set_android_native_tag("test_tag");
  EXPECT_THAT(turbo::log_internal::get_android_native_tag(), StrEq("test_tag"));

  // Verify that additional calls (more than 1) result in a check failure.
  EXPECT_DEATH_IF_SUPPORTED(turbo::set_android_native_tag("test_tag_fail"), ".*");
}

TEST(TestExitOnDFatal, OffTest) {
  // Turn off...
  turbo::log_internal::set_exit_on_dfatal(false);
  EXPECT_FALSE(turbo::log_internal::exit_on_dfatal());

  // We don't die.
  {
    turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

    // KLOG(DFATAL) has severity FATAL if debugging, but is
    // downgraded to ERROR if not debugging.
    EXPECT_CALL(log, Log(turbo::kLogDebugFatal, _, "This should not be fatal"));

    log.StartCapturingLogs();
    KLOG(DFATAL) << "This should not be fatal";
  }
}

#if GTEST_HAS_DEATH_TEST
TEST(TestDeathWhileExitOnDFatal, OnTest) {
  turbo::log_internal::set_exit_on_dfatal(true);
  EXPECT_TRUE(turbo::log_internal::exit_on_dfatal());

  // Death comes on little cats' feet.
  EXPECT_DEBUG_DEATH({ KLOG(DFATAL) << "This should be fatal in debug mode"; },
                     "This should be fatal in debug mode");
}
#endif

}  // namespace
