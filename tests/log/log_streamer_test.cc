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

#include <turbo/log/log_streamer.h>

#include <ios>
#include <iostream>
#include <utility>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <turbo/macros/config.h>
#include <turbo/base/internal/sysinfo.h>
#include <turbo/base/log_severity.h>
#include <tests/log/internal/test_actions.h>
#include <tests/log/internal/test_helpers.h>
#include <tests/log/internal/test_matchers.h>
#include <turbo/log/klog.h>
#include <tests/log/scoped_mock_log.h>
#include <string_view>
#include <turbo/types/source_location.h>

namespace {
using ::turbo::log_internal::DeathTestExpectedLogging;
using ::turbo::log_internal::DeathTestUnexpectedLogging;
using ::turbo::log_internal::DeathTestValidateExpectations;
#if GTEST_HAS_DEATH_TEST
using ::turbo::log_internal::DiedOfFatal;
#endif
using ::turbo::log_internal::InMatchWindow;
using ::turbo::log_internal::LogSeverity;
using ::turbo::log_internal::Prefix;
using ::turbo::log_internal::SourceFilename;
using ::turbo::log_internal::SourceLine;
using ::turbo::log_internal::Stacktrace;
using ::turbo::log_internal::TextMessage;
using ::turbo::log_internal::ThreadID;
using ::turbo::log_internal::Timestamp;
using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::IsEmpty;
using ::testing::IsTrue;

auto* test_env KUMO_ATTRIBUTE_UNUSED = ::testing::AddGlobalTestEnvironment(
    new turbo::log_internal::LogTestEnvironment);

void WriteToStream(std::string_view data, std::ostream* os) {
  *os << "WriteToStream: " << data;
}
void WriteToStreamRef(std::string_view data, std::ostream& os) {
  os << "WriteToStreamRef: " << data;
}

TEST(LogStreamerTest, log_info_streamer) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  EXPECT_CALL(
      test_sink,
      send(
          AllOf(SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
                Prefix(IsTrue()), LogSeverity(Eq(turbo::LogSeverity::kInfo)),
                Timestamp(InMatchWindow()),
                ThreadID(Eq(turbo::base_internal::GetTID())),
                TextMessage(Eq("WriteToStream: foo")),
                ENCODED_MESSAGE(MatchesEvent(
                    Eq("path/file.cc"), Eq(1234), InMatchWindow(),
                    Eq(logging::proto::INFO), Eq(turbo::base_internal::GetTID()),
                    ElementsAre(ValueWithStr(Eq("WriteToStream: foo"))))),
                Stacktrace(IsEmpty()))));

  test_sink.StartCapturingLogs();
  WriteToStream("foo", &turbo::log_info_streamer("path/file.cc", 1234).stream());
}

TEST(LogStreamerTest, log_warning_streamer) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  EXPECT_CALL(
      test_sink,
      send(AllOf(
          SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
          Prefix(IsTrue()), LogSeverity(Eq(turbo::LogSeverity::kWarning)),
          Timestamp(InMatchWindow()),
          ThreadID(Eq(turbo::base_internal::GetTID())),
          TextMessage(Eq("WriteToStream: foo")),
          ENCODED_MESSAGE(MatchesEvent(
              Eq("path/file.cc"), Eq(1234), InMatchWindow(),
              Eq(logging::proto::WARNING), Eq(turbo::base_internal::GetTID()),
              ElementsAre(ValueWithStr(Eq("WriteToStream: foo"))))),
          Stacktrace(IsEmpty()))));

  test_sink.StartCapturingLogs();
  WriteToStream("foo",
                &turbo::log_warning_streamer("path/file.cc", 1234).stream());
}

TEST(LogStreamerTest, log_error_streamer) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  EXPECT_CALL(
      test_sink,
      send(AllOf(
          SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
          Prefix(IsTrue()), LogSeverity(Eq(turbo::LogSeverity::kError)),
          Timestamp(InMatchWindow()),
          ThreadID(Eq(turbo::base_internal::GetTID())),
          TextMessage(Eq("WriteToStream: foo")),
          ENCODED_MESSAGE(MatchesEvent(
              Eq("path/file.cc"), Eq(1234), InMatchWindow(),
              Eq(logging::proto::ERROR), Eq(turbo::base_internal::GetTID()),
              ElementsAre(ValueWithStr(Eq("WriteToStream: foo"))))),
          Stacktrace(IsEmpty()))));

  test_sink.StartCapturingLogs();
  WriteToStream("foo", &turbo::log_error_streamer("path/file.cc", 1234).stream());
}

#if GTEST_HAS_DEATH_TEST
TEST(LogStreamerDeathTest, log_fatal_streamer) {
  EXPECT_EXIT(
      {
        turbo::ScopedMockLog test_sink;
        EXPECT_CALL(test_sink, send).Times(0);

        EXPECT_CALL(test_sink, send)
            .Times(AnyNumber())
            .WillRepeatedly(DeathTestUnexpectedLogging());

        EXPECT_CALL(
            test_sink,
            send(AllOf(
                SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
                Prefix(IsTrue()), LogSeverity(Eq(turbo::LogSeverity::kFatal)),
                Timestamp(InMatchWindow()),
                ThreadID(Eq(turbo::base_internal::GetTID())),
                TextMessage(Eq("WriteToStream: foo")),
                ENCODED_MESSAGE(MatchesEvent(
                    Eq("path/file.cc"), Eq(1234), InMatchWindow(),
                    Eq(logging::proto::FATAL),
                    Eq(turbo::base_internal::GetTID()),
                    ElementsAre(ValueWithStr(Eq("WriteToStream: foo"))))))))
            .WillOnce(DeathTestExpectedLogging());

        test_sink.StartCapturingLogs();
        WriteToStream("foo",
                      &turbo::log_fatal_streamer("path/file.cc", 1234).stream());
      },
      DiedOfFatal, DeathTestValidateExpectations());
}
#endif

#ifdef NDEBUG
TEST(LogStreamerTest, log_debug_fatal_streamer) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  EXPECT_CALL(
      test_sink,
      send(AllOf(
          SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
          Prefix(IsTrue()), LogSeverity(Eq(turbo::LogSeverity::kError)),
          Timestamp(InMatchWindow()),
          ThreadID(Eq(turbo::base_internal::GetTID())),
          TextMessage(Eq("WriteToStream: foo")),
          ENCODED_MESSAGE(MatchesEvent(
              Eq("path/file.cc"), Eq(1234), InMatchWindow(),
              Eq(logging::proto::ERROR), Eq(turbo::base_internal::GetTID()),
              ElementsAre(ValueWithStr(Eq("WriteToStream: foo"))))),
          Stacktrace(IsEmpty()))));

  test_sink.StartCapturingLogs();
  WriteToStream("foo",
                &turbo::log_debug_fatal_streamer("path/file.cc", 1234).stream());
}
#elif GTEST_HAS_DEATH_TEST
TEST(LogStreamerDeathTest, log_debug_fatal_streamer) {
  EXPECT_EXIT(
      {
        turbo::ScopedMockLog test_sink;
        EXPECT_CALL(test_sink, send).Times(0);

        EXPECT_CALL(test_sink, send)
            .Times(AnyNumber())
            .WillRepeatedly(DeathTestUnexpectedLogging());

        EXPECT_CALL(
            test_sink,
            send(AllOf(
                SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
                Prefix(IsTrue()), LogSeverity(Eq(turbo::LogSeverity::kFatal)),
                Timestamp(InMatchWindow()),
                ThreadID(Eq(turbo::base_internal::GetTID())),
                TextMessage(Eq("WriteToStream: foo")),
                ENCODED_MESSAGE(MatchesEvent(
                    Eq("path/file.cc"), Eq(1234), InMatchWindow(),
                    Eq(logging::proto::FATAL),
                    Eq(turbo::base_internal::GetTID()),
                    ElementsAre(ValueWithStr(Eq("WriteToStream: foo"))))))))
            .WillOnce(DeathTestExpectedLogging());

        test_sink.StartCapturingLogs();
        WriteToStream(
            "foo", &turbo::log_debug_fatal_streamer("path/file.cc", 1234).stream());
      },
      DiedOfFatal, DeathTestValidateExpectations());
}
#endif

TEST(LogStreamerTest, LogStreamer) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  EXPECT_CALL(
      test_sink,
      send(AllOf(
          SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
          Prefix(IsTrue()), LogSeverity(Eq(turbo::LogSeverity::kError)),
          Timestamp(InMatchWindow()),
          ThreadID(Eq(turbo::base_internal::GetTID())),
          TextMessage(Eq("WriteToStream: foo")),
          ENCODED_MESSAGE(MatchesEvent(
              Eq("path/file.cc"), Eq(1234), InMatchWindow(),
              Eq(logging::proto::ERROR), Eq(turbo::base_internal::GetTID()),
              ElementsAre(ValueWithStr(Eq("WriteToStream: foo"))))),
          Stacktrace(IsEmpty()))));

  test_sink.StartCapturingLogs();
  WriteToStream(
      "foo", &turbo::LogStreamer(turbo::LogSeverity::kError, "path/file.cc", 1234)
                  .stream());
}

#if GTEST_HAS_DEATH_TEST
TEST(LogStreamerDeathTest, LogStreamer) {
  EXPECT_EXIT(
      {
        turbo::ScopedMockLog test_sink;
        EXPECT_CALL(test_sink, send).Times(0);

        EXPECT_CALL(test_sink, send)
            .Times(AnyNumber())
            .WillRepeatedly(DeathTestUnexpectedLogging());

        EXPECT_CALL(
            test_sink,
            send(AllOf(
                SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
                Prefix(IsTrue()), LogSeverity(Eq(turbo::LogSeverity::kFatal)),
                Timestamp(InMatchWindow()),
                ThreadID(Eq(turbo::base_internal::GetTID())),
                TextMessage(Eq("WriteToStream: foo")),
                ENCODED_MESSAGE(MatchesEvent(
                    Eq("path/file.cc"), Eq(1234), InMatchWindow(),
                    Eq(logging::proto::FATAL),
                    Eq(turbo::base_internal::GetTID()),
                    ElementsAre(ValueWithStr(Eq("WriteToStream: foo"))))))))
            .WillOnce(DeathTestExpectedLogging());

        test_sink.StartCapturingLogs();
        WriteToStream("foo", &turbo::LogStreamer(turbo::LogSeverity::kFatal,
                                                "path/file.cc", 1234)
                                  .stream());
      },
      DiedOfFatal, DeathTestValidateExpectations());
}
#endif

TEST(LogStreamerTest, PassedByReference) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  EXPECT_CALL(
      test_sink,
      send(AllOf(SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
                 TextMessage(Eq("WriteToStreamRef: foo")),
                 ENCODED_MESSAGE(MatchesEvent(
                     Eq("path/file.cc"), Eq(1234), _, _, _,
                     ElementsAre(ValueWithStr(Eq("WriteToStreamRef: foo"))))),
                 Stacktrace(IsEmpty()))));

  test_sink.StartCapturingLogs();
  WriteToStreamRef("foo", turbo::log_info_streamer("path/file.cc", 1234).stream());
}

TEST(LogStreamerTest, StoredAsLocal) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  auto streamer = turbo::log_info_streamer("path/file.cc", 1234);
  WriteToStream("foo", &streamer.stream());
  streamer.stream() << " ";
  WriteToStreamRef("bar", streamer.stream());

  // The call should happen when `streamer` goes out of scope; if it
  // happened before this `EXPECT_CALL` the call would be unexpected and the
  // test would fail.
  EXPECT_CALL(
      test_sink,
      send(AllOf(SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
                 TextMessage(Eq("WriteToStream: foo WriteToStreamRef: bar")),
                 ENCODED_MESSAGE(MatchesEvent(
                     Eq("path/file.cc"), Eq(1234), _, _, _,
                     ElementsAre(ValueWithStr(
                         Eq("WriteToStream: foo WriteToStreamRef: bar"))))),
                 Stacktrace(IsEmpty()))));

  test_sink.StartCapturingLogs();
}

#if GTEST_HAS_DEATH_TEST
TEST(LogStreamerDeathTest, StoredAsLocal) {
  EXPECT_EXIT(
      {
        // This is fatal when it goes out of scope, but not until then:
        auto streamer = turbo::log_fatal_streamer("path/file.cc", 1234);
        std::cerr << "I'm still alive" << std::endl;
        WriteToStream("foo", &streamer.stream());
      },
      DiedOfFatal, HasSubstr("I'm still alive"));
}
#endif

TEST(LogStreamerTest, LogsEmptyLine) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  EXPECT_CALL(test_sink, send(AllOf(SourceFilename(Eq("path/file.cc")),
                                    SourceLine(Eq(1234)), TextMessage(Eq("")),
                                    ENCODED_MESSAGE(MatchesEvent(
                                        Eq("path/file.cc"), Eq(1234), _, _, _,
                                        ElementsAre(ValueWithStr(Eq(""))))),
                                    Stacktrace(IsEmpty()))));

  test_sink.StartCapturingLogs();
  turbo::log_info_streamer("path/file.cc", 1234);
}

#if GTEST_HAS_DEATH_TEST
TEST(LogStreamerDeathTest, LogsEmptyLine) {
  EXPECT_EXIT(
      {
        turbo::ScopedMockLog test_sink;
        EXPECT_CALL(test_sink, send)
            .Times(AnyNumber())
            .WillRepeatedly(DeathTestUnexpectedLogging());

        EXPECT_CALL(
            test_sink,
            send(AllOf(SourceFilename(Eq("path/file.cc")), TextMessage(Eq("")),
                       ENCODED_MESSAGE(
                           MatchesEvent(Eq("path/file.cc"), _, _, _, _,
                                        ElementsAre(ValueWithStr(Eq(""))))))))
            .WillOnce(DeathTestExpectedLogging());

        test_sink.StartCapturingLogs();
        // This is fatal even though it's never used:
        auto streamer = turbo::log_fatal_streamer("path/file.cc", 1234);
      },
      DiedOfFatal, DeathTestValidateExpectations());
}
#endif

TEST(LogStreamerTest, MoveConstruction) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  EXPECT_CALL(
      test_sink,
      send(
          AllOf(SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
                LogSeverity(Eq(turbo::LogSeverity::kInfo)),
                TextMessage(Eq("hello 0x10 world 0x10")),
                ENCODED_MESSAGE(MatchesEvent(
                    Eq("path/file.cc"), Eq(1234), _, Eq(logging::proto::INFO),
                    _, ElementsAre(ValueWithStr(Eq("hello 0x10 world 0x10"))))),
                Stacktrace(IsEmpty()))));

  test_sink.StartCapturingLogs();
  auto streamer1 = turbo::log_info_streamer("path/file.cc", 1234);
  streamer1.stream() << "hello " << std::hex << 16;
  turbo::LogStreamer streamer2(std::move(streamer1));
  streamer2.stream() << " world " << 16;
}

TEST(LogStreamerTest, MoveAssignment) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  testing::InSequence seq;
  EXPECT_CALL(
      test_sink,
      send(AllOf(
          SourceFilename(Eq("path/file2.cc")), SourceLine(Eq(5678)),
          LogSeverity(Eq(turbo::LogSeverity::kWarning)),
          TextMessage(Eq("something else")),
          ENCODED_MESSAGE(MatchesEvent(
              Eq("path/file2.cc"), Eq(5678), _, Eq(logging::proto::WARNING), _,
              ElementsAre(ValueWithStr(Eq("something else"))))),
          Stacktrace(IsEmpty()))));
  EXPECT_CALL(
      test_sink,
      send(
          AllOf(SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
                LogSeverity(Eq(turbo::LogSeverity::kInfo)),
                TextMessage(Eq("hello 0x10 world 0x10")),
                ENCODED_MESSAGE(MatchesEvent(
                    Eq("path/file.cc"), Eq(1234), _, Eq(logging::proto::INFO),
                    _, ElementsAre(ValueWithStr(Eq("hello 0x10 world 0x10"))))),
                Stacktrace(IsEmpty()))));

  test_sink.StartCapturingLogs();
  auto streamer1 = turbo::log_info_streamer("path/file.cc", 1234);
  streamer1.stream() << "hello " << std::hex << 16;
  auto streamer2 = turbo::log_warning_streamer("path/file2.cc", 5678);
  streamer2.stream() << "something else";
  streamer2 = std::move(streamer1);
  streamer2.stream() << " world " << 16;
}

TEST(LogStreamerTest, CorrectDefaultFlags) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  // The `boolalpha` and `showbase` flags should be set by default, to match
  // `LOG`.
  EXPECT_CALL(test_sink, send(AllOf(TextMessage(Eq("false0xdeadbeef")))))
      .Times(2);

  test_sink.StartCapturingLogs();
  turbo::log_info_streamer("path/file.cc", 1234).stream()
      << false << std::hex << 0xdeadbeef;
  KLOG(INFO) << false << std::hex << 0xdeadbeef;
}

TEST(LogStreamerTest, AtSourceLocation) {
  const int log_line = __LINE__ + 2;
  auto do_log = [] {
    WriteToStream("foo", &turbo::log_info_streamer().stream());  //
  };
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  EXPECT_CALL(test_sink,
              send(AllOf(SourceFilename(
                             Eq(turbo::SourceLocation::current().file_name())),
                         SourceLine(Eq(log_line)))));

  test_sink.StartCapturingLogs();
  do_log();
}

}  // namespace
