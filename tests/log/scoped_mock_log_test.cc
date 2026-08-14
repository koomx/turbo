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

#include <tests/log/scoped_mock_log.h>

#include <memory>
#include <thread>  // NOLINT(build/c++11)

#include <gmock/gmock.h>
#include "gtest/gtest-spi.h"
#include <gtest/gtest.h>
#include <turbo/macros/config.h>
#include <turbo/types/barrier.h>
#include <turbo/types/latch.h>
#include <turbo/base/log_severity.h>
#include <turbo/log/globals.h>
#include <tests/log/internal/test_helpers.h>
#include <tests/log/internal/test_matchers.h>
#include <turbo/log/klog.h>
#include <turbo/memory/memory.h>
#include <turbo/strings/match.h>
#include <string_view>

namespace {

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::InSequence;
using ::testing::Lt;
using ::testing::Truly;
using turbo::log_internal::SourceBasename;
using turbo::log_internal::SourceFilename;
using turbo::log_internal::SourceLine;
using turbo::log_internal::TextMessageWithPrefix;
using turbo::log_internal::ThreadID;

auto* test_env KUMO_ATTRIBUTE_UNUSED = ::testing::AddGlobalTestEnvironment(
    new turbo::log_internal::LogTestEnvironment);

#if GTEST_HAS_DEATH_TEST
TEST(ScopedMockLogDeathTest,
     StartCapturingLogsCannotBeCalledWhenAlreadyCapturing) {
  EXPECT_DEATH(
      {
        turbo::ScopedMockLog log;
        log.StartCapturingLogs();
        log.StartCapturingLogs();
      },
      "StartCapturingLogs");
}

TEST(ScopedMockLogDeathTest, StopCapturingLogsCannotBeCalledWhenNotCapturing) {
  EXPECT_DEATH(
      {
        turbo::ScopedMockLog log;
        log.StopCapturingLogs();
      },
      "StopCapturingLogs");
}

TEST(ScopedMockLogDeathTest, FailsCheckIfStartCapturingLogsIsNeverCalled) {
  EXPECT_DEATH({ turbo::ScopedMockLog log; },
               "Did you forget to call StartCapturingLogs");
}
#endif

// Tests that ScopedMockLog intercepts KLOG()s when it's alive.
TEST(ScopedMockLogTest, LogMockCatchAndMatchStrictExpectations) {
  turbo::ScopedMockLog log;

  // The following expectations must match in the order they appear.
  InSequence s;
  EXPECT_CALL(log,
              Log(turbo::LogSeverity::kWarning, HasSubstr(__FILE__), "Danger."));
  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, "Working...")).Times(2);
  EXPECT_CALL(log, Log(turbo::LogSeverity::kError, _, "Bad!!"));

  log.StartCapturingLogs();
  KLOG(WARNING) << "Danger.";
  KLOG(INFO) << "Working...";
  KLOG(INFO) << "Working...";
  KLOG(ERROR) << "Bad!!";
}

TEST(ScopedMockLogTest, LogMockCatchAndMatchSendExpectations) {
  turbo::ScopedMockLog log;

  EXPECT_CALL(
      log,
      send(AllOf(SourceFilename(Eq("/my/very/very/very_long_source_file.cc")),
                 SourceBasename(Eq("very_long_source_file.cc")),
                 SourceLine(Eq(777)), ThreadID(Eq(turbo::LogEntry::tid_t{1234})),
                 TextMessageWithPrefix(Truly([](std::string_view msg) {
                   return turbo::ends_with(
                       msg, " very_long_source_file.cc:777] Info message");
                 })))));

  log.StartCapturingLogs();
  KLOG(INFO)
          .at_location("/my/very/very/very_long_source_file.cc", 777)
          .with_thread_id(1234)
      << "Info message";
}

TEST(ScopedMockLogTest, ScopedMockLogCanBeNice) {
  turbo::ScopedMockLog log;

  InSequence s;
  EXPECT_CALL(log,
              Log(turbo::LogSeverity::kWarning, HasSubstr(__FILE__), "Danger."));
  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, "Working...")).Times(2);
  EXPECT_CALL(log, Log(turbo::LogSeverity::kError, _, "Bad!!"));

  log.StartCapturingLogs();

  // Any number of these are OK.
  KLOG(INFO) << "Info message.";
  // Any number of these are OK.
  KLOG(WARNING).at_location("SomeOtherFile.cc", 100) << "Danger ";

  KLOG(WARNING) << "Danger.";

  // Any number of these are OK.
  KLOG(INFO) << "Info message.";
  // Any number of these are OK.
  KLOG(WARNING).at_location("SomeOtherFile.cc", 100) << "Danger ";

  KLOG(INFO) << "Working...";

  // Any number of these are OK.
  KLOG(INFO) << "Info message.";
  // Any number of these are OK.
  KLOG(WARNING).at_location("SomeOtherFile.cc", 100) << "Danger ";

  KLOG(INFO) << "Working...";

  // Any number of these are OK.
  KLOG(INFO) << "Info message.";
  // Any number of these are OK.
  KLOG(WARNING).at_location("SomeOtherFile.cc", 100) << "Danger ";

  KLOG(ERROR) << "Bad!!";

  // Any number of these are OK.
  KLOG(INFO) << "Info message.";
  // Any number of these are OK.
  KLOG(WARNING).at_location("SomeOtherFile.cc", 100) << "Danger ";
}

// Tests that ScopedMockLog generates a test failure if a message is logged
// that is not expected (here, that means ERROR or FATAL).
TEST(ScopedMockLogTest, RejectsUnexpectedLogs) {
  EXPECT_NONFATAL_FAILURE(
      {
        turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);
        // Any INFO and WARNING messages are permitted.
        EXPECT_CALL(log, Log(Lt(turbo::LogSeverity::kError), _, _))
            .Times(AnyNumber());
        log.StartCapturingLogs();
        KLOG(INFO) << "Ignored";
        KLOG(WARNING) << "Ignored";
        KLOG(ERROR) << "Should not be ignored";
      },
      "Should not be ignored");
}

TEST(ScopedMockLogTest, CapturesLogsAfterStartCapturingLogs) {
  turbo::set_stderr_threshold(turbo::LogSeverityAtLeast::kInfinity);
  turbo::ScopedMockLog log;

  // The ScopedMockLog object shouldn't see these LOGs, as it hasn't
  // started capturing LOGs yet.
  KLOG(INFO) << "Ignored info";
  KLOG(WARNING) << "Ignored warning";
  KLOG(ERROR) << "Ignored error";

  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, "Expected info"));
  log.StartCapturingLogs();

  // Only this LOG will be seen by the ScopedMockLog.
  KLOG(INFO) << "Expected info";
}

TEST(ScopedMockLogTest, DoesNotCaptureLogsAfterStopCapturingLogs) {
  turbo::ScopedMockLog log;
  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, "Expected info"));

  log.StartCapturingLogs();

  // This LOG should be seen by the ScopedMockLog.
  KLOG(INFO) << "Expected info";

  log.StopCapturingLogs();

  // The ScopedMockLog object shouldn't see these LOGs, as it has
  // stopped capturing LOGs.
  KLOG(INFO) << "Ignored info";
  KLOG(WARNING) << "Ignored warning";
  KLOG(ERROR) << "Ignored error";
}

// Tests that all messages are intercepted regardless of issuing thread. The
// purpose of this test is NOT to exercise thread-safety.
TEST(ScopedMockLogTest, LogFromMultipleThreads) {
  turbo::ScopedMockLog log;

  // We don't establish an order to expectations here, since the threads may
  // execute their log statements in different order.
  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, __FILE__, "Thread 1"));
  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, __FILE__, "Thread 2"));

  log.StartCapturingLogs();

  turbo::barrier barrier(2);
  std::thread thread1([&barrier]() {
    barrier.arrive_and_wait();
    KLOG(INFO) << "Thread 1";
  });
  std::thread thread2([&barrier]() {
    barrier.arrive_and_wait();
    KLOG(INFO) << "Thread 2";
  });

  thread1.join();
  thread2.join();
}

// Tests that no sequence will be imposed on two LOG message expectations from
// different threads. This test would actually deadlock if replaced to two LOG
// statements from the same thread.
TEST(ScopedMockLogTest, NoSequenceWithMultipleThreads) {
  turbo::ScopedMockLog log;

  turbo::barrier barrier(2);
  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, _))
      .Times(2)
      .WillRepeatedly([&barrier]() { barrier.arrive_and_wait(); });

  log.StartCapturingLogs();

  std::thread thread1([]() { KLOG(INFO) << "Thread 1"; });
  std::thread thread2([]() { KLOG(INFO) << "Thread 2"; });

  thread1.join();
  thread2.join();
}

TEST(ScopedMockLogTsanTest,
     ScopedMockLogCanBeDeletedWhenAnotherThreadIsLogging) {
  auto log = std::make_unique<turbo::ScopedMockLog>();
  EXPECT_CALL(*log, Log(turbo::LogSeverity::kInfo, __FILE__, "Thread log"))
      .Times(AnyNumber());

  log->StartCapturingLogs();

  turbo::latch logging_started(1);

  std::thread thread([&logging_started]() {
    for (int i = 0; i < 100; ++i) {
      if (i == 50) {
        logging_started.count_down();
      }
      KLOG(INFO) << "Thread log";
    }
  });

  logging_started.wait();
  log.reset();
  thread.join();
}

TEST(ScopedMockLogTest, AsLocalSink) {
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, Log(_, _, "two"));
  EXPECT_CALL(log, Log(_, _, "three"));

  KLOG(INFO) << "one";
  KLOG(INFO).to_sink_only(&log.UseAsLocalSink()) << "two";
  KLOG(INFO).to_sink_also(&log.UseAsLocalSink()) << "three";
}

}  // namespace
