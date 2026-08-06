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

#include <turbo/log/log_sink.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <turbo/macros/config.h>
#include <tests/log/internal/test_actions.h>
#include <tests/log/internal/test_helpers.h>
#include <tests/log/internal/test_matchers.h>
#include <turbo/log/klog.h>
#include <turbo/log/log_sink_registry.h>
#include <tests/log/scoped_mock_log.h>
#include <string_view>

namespace {

using ::turbo::log_internal::DeathTestExpectedLogging;
using ::turbo::log_internal::DeathTestUnexpectedLogging;
using ::turbo::log_internal::DeathTestValidateExpectations;
using ::turbo::log_internal::DiedOfFatal;
using ::testing::_;
using ::testing::AnyNumber;
using ::testing::HasSubstr;
using ::testing::InSequence;

auto* test_env KUMO_ATTRIBUTE_UNUSED = ::testing::AddGlobalTestEnvironment(
    new turbo::log_internal::LogTestEnvironment);

// Tests for global log sink registration.
// ---------------------------------------

TEST(LogSinkRegistryTest, add_log_sink) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

  InSequence s;
  EXPECT_CALL(test_sink, Log(_, _, "hello world")).Times(0);
  EXPECT_CALL(test_sink, Log(turbo::LogSeverity::kInfo, __FILE__, "Test : 42"));
  EXPECT_CALL(test_sink,
              Log(turbo::LogSeverity::kWarning, __FILE__, "Danger ahead"));
  EXPECT_CALL(test_sink,
              Log(turbo::LogSeverity::kError, __FILE__, "This is an error"));

  KLOG(INFO) << "hello world";
  test_sink.StartCapturingLogs();

  KLOG(INFO) << "Test : " << 42;
  KLOG(WARNING) << "Danger" << ' ' << "ahead";
  KLOG(ERROR) << "This is an error";

  test_sink.StopCapturingLogs();
  KLOG(INFO) << "Goodby world";
}

TEST(LogSinkRegistryTest, MultipleLogSinks) {
  turbo::ScopedMockLog test_sink1(turbo::MockLogDefault::kDisallowUnexpected);
  turbo::ScopedMockLog test_sink2(turbo::MockLogDefault::kDisallowUnexpected);

  ::testing::InSequence seq;
  EXPECT_CALL(test_sink1, Log(turbo::LogSeverity::kInfo, _, "First")).Times(1);
  EXPECT_CALL(test_sink2, Log(turbo::LogSeverity::kInfo, _, "First")).Times(0);

  EXPECT_CALL(test_sink1, Log(turbo::LogSeverity::kInfo, _, "Second")).Times(1);
  EXPECT_CALL(test_sink2, Log(turbo::LogSeverity::kInfo, _, "Second")).Times(1);

  EXPECT_CALL(test_sink1, Log(turbo::LogSeverity::kInfo, _, "Third")).Times(0);
  EXPECT_CALL(test_sink2, Log(turbo::LogSeverity::kInfo, _, "Third")).Times(1);

  KLOG(INFO) << "Before first";

  test_sink1.StartCapturingLogs();
  KLOG(INFO) << "First";

  test_sink2.StartCapturingLogs();
  KLOG(INFO) << "Second";

  test_sink1.StopCapturingLogs();
  KLOG(INFO) << "Third";

  test_sink2.StopCapturingLogs();
  KLOG(INFO) << "Fourth";
}

TEST(LogSinkRegistrationDeathTest, DuplicateSinkRegistration) {
  ASSERT_DEATH_IF_SUPPORTED(
      {
        turbo::ScopedMockLog sink;
        sink.StartCapturingLogs();
        turbo::add_log_sink(&sink.UseAsLocalSink());
      },
      HasSubstr("Duplicate log sinks"));
}

TEST(LogSinkRegistrationDeathTest, MismatchSinkRemoval) {
  ASSERT_DEATH_IF_SUPPORTED(
      {
        turbo::ScopedMockLog sink;
        turbo::remove_log_sink(&sink.UseAsLocalSink());
      },
      HasSubstr("Mismatched log sink"));
}

// Tests for log sink semantic.
// ---------------------------------------

TEST(LogSinkTest, FlushSinks) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(test_sink, flush()).Times(2);

  test_sink.StartCapturingLogs();

  turbo::flush_log_sinks();
  turbo::flush_log_sinks();
}

TEST(LogSinkDeathTest, DeathInSend) {
  class FatalSendSink : public turbo::LogSink {
   public:
    void send(const turbo::LogEntry&) override { KLOG(FATAL) << "goodbye world"; }
  };

  FatalSendSink sink;
  EXPECT_EXIT({ KLOG(INFO).to_sink_also(&sink) << "hello world"; }, DiedOfFatal,
              _);
}

// Tests for explicit log sink redirection.
// ---------------------------------------

TEST(LogSinkTest, to_sink_also) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  turbo::ScopedMockLog another_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, Log(_, _, "hello world"));
  EXPECT_CALL(another_sink, Log(_, _, "hello world"));

  test_sink.StartCapturingLogs();
  KLOG(INFO).to_sink_also(&another_sink.UseAsLocalSink()) << "hello world";
}

TEST(LogSinkTest, to_sink_only) {
  turbo::ScopedMockLog another_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(another_sink, Log(_, _, "hello world"));
  KLOG(INFO).to_sink_only(&another_sink.UseAsLocalSink()) << "hello world";
}

TEST(LogSinkTest, ToManySinks) {
  turbo::ScopedMockLog sink1(turbo::MockLogDefault::kDisallowUnexpected);
  turbo::ScopedMockLog sink2(turbo::MockLogDefault::kDisallowUnexpected);
  turbo::ScopedMockLog sink3(turbo::MockLogDefault::kDisallowUnexpected);
  turbo::ScopedMockLog sink4(turbo::MockLogDefault::kDisallowUnexpected);
  turbo::ScopedMockLog sink5(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(sink3, Log(_, _, "hello world"));
  EXPECT_CALL(sink4, Log(_, _, "hello world"));
  EXPECT_CALL(sink5, Log(_, _, "hello world"));

  KLOG(INFO)
          .to_sink_also(&sink1.UseAsLocalSink())
          .to_sink_also(&sink2.UseAsLocalSink())
          .to_sink_only(&sink3.UseAsLocalSink())
          .to_sink_also(&sink4.UseAsLocalSink())
          .to_sink_also(&sink5.UseAsLocalSink())
      << "hello world";
}

class ReentrancyTest : public ::testing::Test {
 protected:
  ReentrancyTest() = default;
  enum class LogMode : int { kNormal, kToSinkAlso, kToSinkOnly };

  class ReentrantSendLogSink : public turbo::LogSink {
   public:
    explicit ReentrantSendLogSink(turbo::LogSeverity severity,
                                  turbo::LogSink* sink, LogMode mode)
        : severity_(severity), sink_(sink), mode_(mode) {}
    explicit ReentrantSendLogSink(turbo::LogSeverity severity)
        : ReentrantSendLogSink(severity, nullptr, LogMode::kNormal) {}

    void send(const turbo::LogEntry&) override {
      switch (mode_) {
        case LogMode::kNormal:
          KLOG(LEVEL(severity_)) << "The log is coming from *inside the sink*.";
          break;
        case LogMode::kToSinkAlso:
          KLOG(LEVEL(severity_)).to_sink_also(sink_)
              << "The log is coming from *inside the sink*.";
          break;
        case LogMode::kToSinkOnly:
          KLOG(LEVEL(severity_)).to_sink_only(sink_)
              << "The log is coming from *inside the sink*.";
          break;
        default:
          KLOG(FATAL) << "Invalid mode " << static_cast<int>(mode_);
      }
    }

   private:
    turbo::LogSeverity severity_;
    turbo::LogSink* sink_;
    LogMode mode_;
  };

  static std::string_view LogAndReturn(turbo::LogSeverity severity,
                                        std::string_view to_log,
                                        std::string_view to_return) {
    KLOG(LEVEL(severity)) << to_log;
    return to_return;
  }
};

TEST_F(ReentrancyTest, LogFunctionThatLogs) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

  InSequence seq;
  EXPECT_CALL(test_sink, Log(turbo::LogSeverity::kInfo, _, "hello"));
  EXPECT_CALL(test_sink, Log(turbo::LogSeverity::kInfo, _, "world"));
  EXPECT_CALL(test_sink, Log(turbo::LogSeverity::kWarning, _, "danger"));
  EXPECT_CALL(test_sink, Log(turbo::LogSeverity::kInfo, _, "here"));

  test_sink.StartCapturingLogs();
  KLOG(INFO) << LogAndReturn(turbo::LogSeverity::kInfo, "hello", "world");
  KLOG(INFO) << LogAndReturn(turbo::LogSeverity::kWarning, "danger", "here");
}

TEST_F(ReentrancyTest, RegisteredLogSinkThatLogsInSend) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  ReentrantSendLogSink renentrant_sink(turbo::LogSeverity::kInfo);
  EXPECT_CALL(test_sink, Log(_, _, "hello world"));

  test_sink.StartCapturingLogs();
  turbo::add_log_sink(&renentrant_sink);
  KLOG(INFO) << "hello world";
  turbo::remove_log_sink(&renentrant_sink);
}

TEST_F(ReentrancyTest, AlsoLogSinkThatLogsInSend) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  ReentrantSendLogSink reentrant_sink(turbo::LogSeverity::kInfo);
  EXPECT_CALL(test_sink, Log(_, _, "hello world"));
  EXPECT_CALL(test_sink,
              Log(_, _, "The log is coming from *inside the sink*."));

  test_sink.StartCapturingLogs();
  KLOG(INFO).to_sink_also(&reentrant_sink) << "hello world";
}

TEST_F(ReentrancyTest, RegisteredAlsoLogSinkThatLogsInSend) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  ReentrantSendLogSink reentrant_sink(turbo::LogSeverity::kInfo);
  EXPECT_CALL(test_sink, Log(_, _, "hello world"));
  // We only call into the test_log sink once with this message, since the
  // second time log statement is run we are in "ThreadIsLogging" mode and all
  // the log statements are redirected into stderr.
  EXPECT_CALL(test_sink,
              Log(_, _, "The log is coming from *inside the sink*."));

  test_sink.StartCapturingLogs();
  turbo::add_log_sink(&reentrant_sink);
  KLOG(INFO).to_sink_also(&reentrant_sink) << "hello world";
  turbo::remove_log_sink(&reentrant_sink);
}

TEST_F(ReentrancyTest, OnlyLogSinkThatLogsInSend) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  ReentrantSendLogSink reentrant_sink(turbo::LogSeverity::kInfo);
  EXPECT_CALL(test_sink,
              Log(_, _, "The log is coming from *inside the sink*."));

  test_sink.StartCapturingLogs();
  KLOG(INFO).to_sink_only(&reentrant_sink) << "hello world";
}

TEST_F(ReentrancyTest, RegisteredOnlyLogSinkThatLogsInSend) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  ReentrantSendLogSink reentrant_sink(turbo::LogSeverity::kInfo);
  EXPECT_CALL(test_sink,
              Log(_, _, "The log is coming from *inside the sink*."));

  test_sink.StartCapturingLogs();
  turbo::add_log_sink(&reentrant_sink);
  KLOG(INFO).to_sink_only(&reentrant_sink) << "hello world";
  turbo::remove_log_sink(&reentrant_sink);
}

using ReentrancyDeathTest = ReentrancyTest;

TEST_F(ReentrancyDeathTest, LogFunctionThatLogsFatal) {
  EXPECT_EXIT(
      {
        turbo::ScopedMockLog test_sink;

        EXPECT_CALL(test_sink, Log)
            .Times(AnyNumber())
            .WillRepeatedly(DeathTestUnexpectedLogging());
        EXPECT_CALL(test_sink, Log(_, _, "hello"))
            .WillOnce(DeathTestExpectedLogging());

        test_sink.StartCapturingLogs();
        KLOG(INFO) << LogAndReturn(turbo::LogSeverity::kFatal, "hello", "world");
      },
      DiedOfFatal, DeathTestValidateExpectations());
}

TEST_F(ReentrancyDeathTest, RegisteredLogSinkThatLogsFatalInSend) {
  EXPECT_EXIT(
      {
        turbo::ScopedMockLog test_sink;
        ReentrantSendLogSink reentrant_sink(turbo::LogSeverity::kFatal);
        EXPECT_CALL(test_sink, Log)
            .Times(AnyNumber())
            .WillRepeatedly(DeathTestUnexpectedLogging());
        EXPECT_CALL(test_sink, Log(_, _, "hello world"))
            .WillOnce(DeathTestExpectedLogging());

        test_sink.StartCapturingLogs();
        turbo::add_log_sink(&reentrant_sink);
        KLOG(INFO) << "hello world";
        // No need to call remove_log_sink - process is dead at this point.
      },
      DiedOfFatal, DeathTestValidateExpectations());
}

TEST_F(ReentrancyDeathTest, AlsoLogSinkThatLogsFatalInSend) {
  EXPECT_EXIT(
      {
        turbo::ScopedMockLog test_sink;
        ReentrantSendLogSink reentrant_sink(turbo::LogSeverity::kFatal);

        EXPECT_CALL(test_sink, Log)
            .Times(AnyNumber())
            .WillRepeatedly(DeathTestUnexpectedLogging());
        EXPECT_CALL(test_sink, Log(_, _, "hello world"))
            .WillOnce(DeathTestExpectedLogging());
        EXPECT_CALL(test_sink,
                    Log(_, _, "The log is coming from *inside the sink*."))
            .WillOnce(DeathTestExpectedLogging());

        test_sink.StartCapturingLogs();
        KLOG(INFO).to_sink_also(&reentrant_sink) << "hello world";
      },
      DiedOfFatal, DeathTestValidateExpectations());
}

TEST_F(ReentrancyDeathTest, RegisteredAlsoLogSinkThatLogsFatalInSend) {
  EXPECT_EXIT(
      {
        turbo::ScopedMockLog test_sink;
        ReentrantSendLogSink reentrant_sink(turbo::LogSeverity::kFatal);
        EXPECT_CALL(test_sink, Log)
            .Times(AnyNumber())
            .WillRepeatedly(DeathTestUnexpectedLogging());
        EXPECT_CALL(test_sink, Log(_, _, "hello world"))
            .WillOnce(DeathTestExpectedLogging());
        EXPECT_CALL(test_sink,
                    Log(_, _, "The log is coming from *inside the sink*."))
            .WillOnce(DeathTestExpectedLogging());

        test_sink.StartCapturingLogs();
        turbo::add_log_sink(&reentrant_sink);
        KLOG(INFO).to_sink_also(&reentrant_sink) << "hello world";
        // No need to call remove_log_sink - process is dead at this point.
      },
      DiedOfFatal, DeathTestValidateExpectations());
}

TEST_F(ReentrancyDeathTest, OnlyLogSinkThatLogsFatalInSend) {
  EXPECT_EXIT(
      {
        turbo::ScopedMockLog test_sink;
        ReentrantSendLogSink reentrant_sink(turbo::LogSeverity::kFatal);
        EXPECT_CALL(test_sink, Log)
            .Times(AnyNumber())
            .WillRepeatedly(DeathTestUnexpectedLogging());
        EXPECT_CALL(test_sink,
                    Log(_, _, "The log is coming from *inside the sink*."))
            .WillOnce(DeathTestExpectedLogging());

        test_sink.StartCapturingLogs();
        KLOG(INFO).to_sink_only(&reentrant_sink) << "hello world";
      },
      DiedOfFatal, DeathTestValidateExpectations());
}

TEST_F(ReentrancyDeathTest, RegisteredOnlyLogSinkThatLogsFatalInSend) {
  EXPECT_EXIT(
      {
        turbo::ScopedMockLog test_sink;
        ReentrantSendLogSink reentrant_sink(turbo::LogSeverity::kFatal);
        EXPECT_CALL(test_sink, Log)
            .Times(AnyNumber())
            .WillRepeatedly(DeathTestUnexpectedLogging());
        EXPECT_CALL(test_sink,
                    Log(_, _, "The log is coming from *inside the sink*."))
            .WillOnce(DeathTestExpectedLogging());

        test_sink.StartCapturingLogs();
        turbo::add_log_sink(&reentrant_sink);
        KLOG(INFO).to_sink_only(&reentrant_sink) << "hello world";
        // No need to call remove_log_sink - process is dead at this point.
      },
      DiedOfFatal, DeathTestValidateExpectations());
}

}  // namespace
