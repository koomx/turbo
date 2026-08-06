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

#include <turbo/log/vlog_is_on.h>

#include <optional>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <turbo/base/log_severity.h>
#include <turbo/log/globals.h>
#include <turbo/log/klog.h>
#include <tests/log/scoped_mock_log.h>

namespace {

using ::testing::_;

std::optional<int> MaxLogVerbosity() {
#ifdef TURBO_MAX_VLOG_VERBOSITY
  return TURBO_MAX_VLOG_VERBOSITY;
#else
  return std::nullopt;
#endif
}

std::optional<int> min_log_level() {
#ifdef TURBO_MIN_LOG_LEVEL
  return static_cast<int>(TURBO_MIN_LOG_LEVEL);
#else
  return std::nullopt;
#endif
}

// This fixture is used to reset the VKLOG levels to their default values before
// each test.
class VLogIsOnTest : public ::testing::Test {
 protected:
  void SetUp() override { ResetVLogLevels(); }

 private:
  // Resets the VKLOG levels to their default values.
  // It is supposed to be called in the SetUp() method of the test fixture to
  // eliminate any side effects from other tests.
  static void ResetVLogLevels() {
    turbo::log_internal::update_vmodule("");
    turbo::set_global_vlog_level(0);
  }
};

TEST_F(VLogIsOnTest, GlobalWorksWithoutMaxVerbosityAndMinLogLevel) {
  if (MaxLogVerbosity().has_value() || min_log_level().has_value()) {
    GTEST_SKIP();
  }

  turbo::set_global_vlog_level(3);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, "important"));

  log.StartCapturingLogs();
  VKLOG(3) << "important";
  VKLOG(4) << "spam";
}

TEST_F(VLogIsOnTest, FileWorksWithoutMaxVerbosityAndMinLogLevel) {
  if (MaxLogVerbosity().has_value() || min_log_level().has_value()) {
    GTEST_SKIP();
  }

  turbo::set_vlog_level("vlog_is_on_test", 3);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, "important"));

  log.StartCapturingLogs();
  VKLOG(3) << "important";
  VKLOG(4) << "spam";
}

TEST_F(VLogIsOnTest, PatternWorksWithoutMaxVerbosityAndMinLogLevel) {
  if (MaxLogVerbosity().has_value() || min_log_level().has_value()) {
    GTEST_SKIP();
  }

  turbo::set_vlog_level("vlog_is_on*", 3);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, "important"));

  log.StartCapturingLogs();
  VKLOG(3) << "important";
  VKLOG(4) << "spam";
}

TEST_F(VLogIsOnTest,
       PatternOverridesLessGenericOneWithoutMaxVerbosityAndMinLogLevel) {
  if (MaxLogVerbosity().has_value() || min_log_level().has_value()) {
    GTEST_SKIP();
  }

  // This should disable logging in this file
  turbo::set_vlog_level("vlog_is_on*", -1);
  // This overrides the previous setting, because "vlog*" is more generic than
  // "vlog_is_on*". This should enable VKLOG level 3 in this file.
  turbo::set_vlog_level("vlog*", 3);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, "important"));

  log.StartCapturingLogs();
  VKLOG(3) << "important";
  VKLOG(4) << "spam";
}

TEST_F(VLogIsOnTest,
       PatternDoesNotOverridesMoreGenericOneWithoutMaxVerbosityAndMinLogLevel) {
  if (MaxLogVerbosity().has_value() || min_log_level().has_value()) {
    GTEST_SKIP();
  }

  // This should enable VKLOG level 3 in this file.
  turbo::set_vlog_level("vlog*", 3);
  // This should not change the VKLOG level in this file. The pattern does not
  // match this file and it is less generic than the previous patter "vlog*".
  // Therefore, it does not disable VKLOG level 3 in this file.
  turbo::set_vlog_level("vlog_is_on_some_other_test*", -1);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, "important"));

  log.StartCapturingLogs();
  VKLOG(3) << "important";
  VKLOG(5) << "spam";
}

TEST_F(VLogIsOnTest, GlobalDoesNotFilterBelowMaxVerbosity) {
  if (!MaxLogVerbosity().has_value() || *MaxLogVerbosity() < 2) {
    GTEST_SKIP();
  }

  // Set an arbitrary high value to avoid filtering VLOGs in tests by default.
  turbo::set_global_vlog_level(1000);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, "asdf"));

  log.StartCapturingLogs();
  VKLOG(2) << "asdf";
}

TEST_F(VLogIsOnTest, FileDoesNotFilterBelowMaxVerbosity) {
  if (!MaxLogVerbosity().has_value() || *MaxLogVerbosity() < 2) {
    GTEST_SKIP();
  }

  // Set an arbitrary high value to avoid filtering VLOGs in tests by default.
  turbo::set_vlog_level("vlog_is_on_test", 1000);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, "asdf"));

  log.StartCapturingLogs();
  VKLOG(2) << "asdf";
}

TEST_F(VLogIsOnTest, PatternDoesNotFilterBelowMaxVerbosity) {
  if (!MaxLogVerbosity().has_value() || *MaxLogVerbosity() < 2) {
    GTEST_SKIP();
  }

  // Set an arbitrary high value to avoid filtering VLOGs in tests by default.
  turbo::set_vlog_level("vlog_is_on*", 1000);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, "asdf"));

  log.StartCapturingLogs();
  VKLOG(2) << "asdf";
}

TEST_F(VLogIsOnTest, GlobalFiltersAboveMaxVerbosity) {
  if (!MaxLogVerbosity().has_value() || *MaxLogVerbosity() >= 4) {
    GTEST_SKIP();
  }

  // Set an arbitrary high value to avoid filtering VLOGs in tests by default.
  turbo::set_global_vlog_level(1000);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  log.StartCapturingLogs();
  VKLOG(4) << "dfgh";
}

TEST_F(VLogIsOnTest, FileFiltersAboveMaxVerbosity) {
  if (!MaxLogVerbosity().has_value() || *MaxLogVerbosity() >= 4) {
    GTEST_SKIP();
  }

  // Set an arbitrary high value to avoid filtering VLOGs in tests by default.
  turbo::set_vlog_level("vlog_is_on_test", 1000);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  log.StartCapturingLogs();
  VKLOG(4) << "dfgh";
}

TEST_F(VLogIsOnTest, PatternFiltersAboveMaxVerbosity) {
  if (!MaxLogVerbosity().has_value() || *MaxLogVerbosity() >= 4) {
    GTEST_SKIP();
  }

  // Set an arbitrary high value to avoid filtering VLOGs in tests by default.
  turbo::set_vlog_level("vlog_is_on*", 1000);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  log.StartCapturingLogs();
  VKLOG(4) << "dfgh";
}

}  // namespace
