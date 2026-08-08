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

#include <errno.h>

#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <tests/log/internal/test_actions.h>
#include <tests/log/internal/test_helpers.h>
#include <tests/log/internal/test_matchers.h>
#include <turbo/log/klog.h>
#include <turbo/log/log_sink.h>
#include <tests/log/scoped_mock_log.h>
#include <turbo/strings/match.h>
#include <string_view>
#include <turbo/time/time.h>
#include <source_location>

namespace {
#if GTEST_HAS_DEATH_TEST
using ::turbo::log_internal::DeathTestExpectedLogging;
using ::turbo::log_internal::DeathTestUnexpectedLogging;
using ::turbo::log_internal::DeathTestValidateExpectations;
using ::turbo::log_internal::DiedOfQFatal;
#endif
using ::turbo::log_internal::LogSeverity;
using ::turbo::log_internal::Prefix;
using ::turbo::log_internal::SourceBasename;
using ::turbo::log_internal::SourceFilename;
using ::turbo::log_internal::SourceLine;
using ::turbo::log_internal::Stacktrace;
using ::turbo::log_internal::TextMessage;
using ::turbo::log_internal::TextMessageWithPrefix;
using ::turbo::log_internal::TextMessageWithPrefixAndNewline;
using ::turbo::log_internal::TextPrefix;
using ::turbo::log_internal::ThreadID;
using ::turbo::log_internal::Timestamp;
using ::turbo::log_internal::Verbosity;

using ::testing::AllOf;
using ::testing::AnyNumber;
using ::testing::AnyOf;
using ::testing::Eq;
using ::testing::IsEmpty;
using ::testing::IsFalse;
using ::testing::Truly;

TEST(TailCallsModifiesTest, AtLocationFileLine) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  EXPECT_CALL(
      test_sink,
      send(AllOf(
          // The metadata should change:
          SourceFilename(Eq("/my/very/very/very_long_source_file.cc")),
          SourceBasename(Eq("very_long_source_file.cc")), SourceLine(Eq(777)),
          // The logged line should change too, even though the prefix must
          // grow to fit the new metadata.
          TextMessageWithPrefix(Truly([](std::string_view msg) {
            return turbo::EndsWith(msg,
                                  " very_long_source_file.cc:777] hello world");
          })))));

  test_sink.StartCapturingLogs();
  KLOG(INFO).at_location("/my/very/very/very_long_source_file.cc", 777)
      << "hello world";
}

TEST(TailCallsModifiesTest, AtLocationFileLineLifetime) {
  // The macro takes care to not use this temporary after its lifetime.
  // The only salient expectation is "no sanitizer diagnostics".
  KLOG(INFO).at_location(std::string("/my/very/very/very_long_source_file.cc"),
                       777)
      << "hello world";
}

TEST(TailCallsModifiesTest, AtLocationSourceLocation) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  const int log_line = __LINE__ + 1;
  constexpr std::source_location loc = std::source_location::current();
  auto do_log = [loc] { KLOG(INFO).at_location(loc) << "hello world"; };

  EXPECT_CALL(test_sink,
              send(AllOf(SourceFilename(Eq(__FILE__)),
                         SourceBasename(Eq("log_modifier_methods_test.cc")),
                         SourceLine(Eq(log_line)))));

  test_sink.StartCapturingLogs();
  do_log();
}

TEST(TailCallsModifiesTest, no_prefix) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  EXPECT_CALL(test_sink, send(AllOf(Prefix(IsFalse()), TextPrefix(IsEmpty()),
                                    TextMessageWithPrefix(Eq("hello world")))));

  test_sink.StartCapturingLogs();
  KLOG(INFO).no_prefix() << "hello world";
}

TEST(TailCallsModifiesTest, NoPrefixNoMessageNoShirtNoShoesNoService) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  EXPECT_CALL(test_sink,
              send(AllOf(Prefix(IsFalse()), TextPrefix(IsEmpty()),
                         TextMessageWithPrefix(IsEmpty()),
                         TextMessageWithPrefixAndNewline(Eq("\n")))));
  test_sink.StartCapturingLogs();
  KLOG(INFO).no_prefix();
}

TEST(TailCallsModifiesTest, with_verbosity) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  EXPECT_CALL(test_sink, send(Verbosity(Eq(2))));

  test_sink.StartCapturingLogs();
  KLOG(INFO).with_verbosity(2) << "hello world";
}

TEST(TailCallsModifiesTest, WithVerbosityNoVerbosity) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  EXPECT_CALL(test_sink,
              send(Verbosity(Eq(turbo::LogEntry::kNoVerbosityLevel))));

  test_sink.StartCapturingLogs();
  KLOG(INFO).with_verbosity(2).with_verbosity(turbo::LogEntry::kNoVerbosityLevel)
      << "hello world";
}

TEST(TailCallsModifiesTest, with_timestamp) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  EXPECT_CALL(test_sink, send(Timestamp(Eq(turbo::UnixEpoch()))));

  test_sink.StartCapturingLogs();
  KLOG(INFO).with_timestamp(turbo::UnixEpoch()) << "hello world";
}

TEST(TailCallsModifiesTest, with_thread_id) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  EXPECT_CALL(test_sink,
              send(AllOf(ThreadID(Eq(turbo::LogEntry::tid_t{1234})))));

  test_sink.StartCapturingLogs();
  KLOG(INFO).with_thread_id(1234) << "hello world";
}

TEST(TailCallsModifiesTest, with_metadata_from) {
  class ForwardingLogSink : public turbo::LogSink {
   public:
    void send(const turbo::LogEntry &entry) override {
      KLOG(LEVEL(entry.log_severity())).with_metadata_from(entry)
          << "forwarded: " << entry.text_message();
    }
  } forwarding_sink;

  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  EXPECT_CALL(
      test_sink,
      send(AllOf(SourceFilename(Eq("fake/file")), SourceBasename(Eq("file")),
                 SourceLine(Eq(123)), Prefix(IsFalse()),
                 LogSeverity(Eq(turbo::LogSeverity::kWarning)),
                 Timestamp(Eq(turbo::UnixEpoch())),
                 ThreadID(Eq(turbo::LogEntry::tid_t{456})),
                 TextMessage(Eq("forwarded: hello world")), Verbosity(Eq(7)),
                 ENCODED_MESSAGE(MatchesEvent(
                     Eq("fake/file"), Eq(123), Eq(turbo::UnixEpoch()),
                     Eq(logging::proto::WARNING), Eq(456),
                     ElementsAre(ValueWithLiteral(Eq("forwarded: ")),
                                 ValueWithStr(Eq("hello world"))))))));

  test_sink.StartCapturingLogs();
  KLOG(WARNING)
          .at_location("fake/file", 123)
          .no_prefix()
          .with_timestamp(turbo::UnixEpoch())
          .with_thread_id(456)
          .with_verbosity(7)
          .to_sink_only(&forwarding_sink)
      << "hello world";
}

TEST(TailCallsModifiesTest, with_perror) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
  EXPECT_CALL(test_sink, send).Times(0);

  EXPECT_CALL(
      test_sink,
      send(AllOf(
          TextMessage(AnyOf(Eq("hello world: Bad file number [9]"),
                            Eq("hello world: Bad file descriptor [9]"),
                            Eq("hello world: Bad file descriptor [8]"))),
          ENCODED_MESSAGE(HasValues(ElementsAre(
              ValueWithLiteral(Eq("hello world")), ValueWithLiteral(Eq(": ")),
              AnyOf(ValueWithStr(Eq("Bad file number")),
                    ValueWithStr(Eq("Bad file descriptor"))),
              ValueWithLiteral(Eq(" [")),
              AnyOf(ValueWithStr(Eq("8")), ValueWithStr(Eq("9"))),
              ValueWithLiteral(Eq("]"))))))));

  test_sink.StartCapturingLogs();
  errno = EBADF;
  KLOG(INFO).with_perror() << "hello world";
}

#if GTEST_HAS_DEATH_TEST
TEST(ModifierMethodDeathTest, ToSinkOnlyQFatal) {
  EXPECT_EXIT(
      {
        turbo::ScopedMockLog test_sink(
            turbo::MockLogDefault::kDisallowUnexpected);
        EXPECT_CALL(test_sink, send).Times(0);

        auto do_log = [&test_sink] {
          KLOG(QFATAL).to_sink_only(&test_sink.UseAsLocalSink()) << "hello world";
        };

        EXPECT_CALL(test_sink, send)
            .Times(AnyNumber())
            .WillRepeatedly(DeathTestUnexpectedLogging());

        EXPECT_CALL(test_sink, send(AllOf(TextMessage(Eq("hello world")),
                                          Stacktrace(IsEmpty()))))
            .WillOnce(DeathTestExpectedLogging());

        test_sink.StartCapturingLogs();
        do_log();
      },
      DiedOfQFatal, DeathTestValidateExpectations());
}
#endif

}  // namespace
