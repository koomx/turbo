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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <turbo/base/log_severity.h>
#include <turbo/log/globals.h>
#include <turbo/log/klog.h>
#include <turbo/log/log_entry.h>
#include <turbo/log/vlog_is_on.h>
#include <tests/log/internal/test_helpers.h>
#include <tests/log/internal/test_matchers.h>
#include <tests/log/scoped_mock_log.h>

namespace {

using ::testing::_;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::turbo::log_internal::LogSeverity;
using ::turbo::log_internal::Prefix;
using ::turbo::log_internal::TextMessage;

auto* test_env KUMO_ATTRIBUTE_UNUSED = ::testing::AddGlobalTestEnvironment(
    new turbo::log_internal::LogTestEnvironment);

class MinLogLevelSeverityTest : public ::testing::Test {
 protected:
  void SetUp() override {
    turbo::set_min_log_level(turbo::LogSeverityAtLeast::kInfo);
    turbo::set_global_vlog_level(0);
  }

  void TearDown() override {
    turbo::set_min_log_level(turbo::LogSeverityAtLeast::kInfo);
    turbo::set_global_vlog_level(0);
  }
};

TEST_F(MinLogLevelSeverityTest, InfoFiltersTraceAndDebug) {
  turbo::set_min_log_level(turbo::LogSeverityAtLeast::kInfo);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, Eq("info ok")));

  log.StartCapturingLogs();
  KLOG(TRACE) << "trace hidden";
  KLOG(DEBUG) << "debug hidden";
  KLOG(INFO) << "info ok";
}

TEST_F(MinLogLevelSeverityTest, DebugAllowsDebugAndInfoNotTrace) {
  turbo::set_min_log_level(turbo::LogSeverityAtLeast::kDebug);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, Log(turbo::LogSeverity::kDebug, _, Eq("debug ok")));
  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, Eq("info ok")));

  log.StartCapturingLogs();
  KLOG(TRACE) << "trace hidden";
  KLOG(DEBUG) << "debug ok";
  KLOG(INFO) << "info ok";
}

TEST_F(MinLogLevelSeverityTest, TraceAllowsAllThree) {
  turbo::set_min_log_level(turbo::LogSeverityAtLeast::kTrace);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, Log(turbo::LogSeverity::kTrace, _, Eq("trace ok")));
  EXPECT_CALL(log, Log(turbo::LogSeverity::kDebug, _, Eq("debug ok")));
  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, Eq("info ok")));

  log.StartCapturingLogs();
  KLOG(TRACE) << "trace ok";
  KLOG(DEBUG) << "debug ok";
  KLOG(INFO) << "info ok";
}

TEST_F(MinLogLevelSeverityTest, PrefixLetters) {
  turbo::set_min_log_level(turbo::LogSeverityAtLeast::kTrace);
  turbo::set_stderr_threshold(turbo::LogSeverityAtLeast::kInfinity);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, send)
      .WillOnce([](const turbo::LogEntry& entry) {
        EXPECT_THAT(entry.text_message_with_prefix(), HasSubstr("T"));
        EXPECT_EQ(entry.log_severity(), turbo::LogSeverity::kTrace);
      })
      .WillOnce([](const turbo::LogEntry& entry) {
        EXPECT_THAT(entry.text_message_with_prefix(), HasSubstr("D"));
        EXPECT_EQ(entry.log_severity(), turbo::LogSeverity::kDebug);
      })
      .WillOnce([](const turbo::LogEntry& entry) {
        EXPECT_THAT(entry.text_message_with_prefix(), HasSubstr("I"));
        EXPECT_EQ(entry.log_severity(), turbo::LogSeverity::kInfo);
      });

  log.StartCapturingLogs();
  KLOG(TRACE) << "t";
  KLOG(DEBUG) << "d";
  KLOG(INFO) << "i";
}

TEST_F(MinLogLevelSeverityTest, VlogRemainsInfoWhenMinIsInfo) {
  turbo::set_min_log_level(turbo::LogSeverityAtLeast::kInfo);
  turbo::set_global_vlog_level(2);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, send).WillOnce([](const turbo::LogEntry& entry) {
    EXPECT_EQ(entry.log_severity(), turbo::LogSeverity::kInfo);
    EXPECT_EQ(entry.verbosity(), 1);
    EXPECT_THAT(entry.text_message(), Eq("vlog ok"));
  });

  log.StartCapturingLogs();
  KLOG(DEBUG) << "should not appear";
  VKLOG(1) << "vlog ok";
  VKLOG(3) << "too verbose";
}

TEST_F(MinLogLevelSeverityTest, WarningFiltersVlogEvenIfVerbose) {
  turbo::set_min_log_level(turbo::LogSeverityAtLeast::kWarning);
  turbo::set_global_vlog_level(100);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, Log).Times(0);

  log.StartCapturingLogs();
  VKLOG(0) << "should not appear";
  KLOG(INFO) << "should not appear";
  KLOG(DEBUG) << "should not appear";
}

}  // namespace
