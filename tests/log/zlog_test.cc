// Copyright 2024 The Abseil Authors.
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

#include <turbo/log/zlog.h>

#include <string>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <turbo/log/klog.h>
#include <tests/log/scoped_mock_log.h>
#include <turbo/base/log_severity.h>

namespace {

using ::testing::_;
using ::testing::HasSubstr;
using ::turbo::LogSeverity;

TEST(ZlogTest, BasicFormat) {
    turbo::ScopedMockLog log;
    EXPECT_CALL(log, Log(LogSeverity::kInfo, _, HasSubstr("hello 42 world")))
        .Times(1);
    log.StartCapturingLogs();

    ZLOG(INFO, "%s %d %s", "hello", 42, "world");
}

TEST(ZlogTest, IntegerFormats) {
    turbo::ScopedMockLog log;
    EXPECT_CALL(log, Log(LogSeverity::kInfo, _, HasSubstr("x=42 y=-7 u=100")))
        .Times(1);
    log.StartCapturingLogs();

    ZLOG(INFO, "x=%d y=%i u=%u", 42, -7, 100u);
}

TEST(ZlogTest, FloatFormats) {
    turbo::ScopedMockLog log;
    EXPECT_CALL(log, Log(LogSeverity::kInfo, _, HasSubstr("pi=3.14")))
        .Times(1);
    log.StartCapturingLogs();

    ZLOG(INFO, "pi=%.2f", 3.14);
}

TEST(ZlogTest, StringLiteral) {
    turbo::ScopedMockLog log;
    EXPECT_CALL(log, Log(LogSeverity::kWarning, _, HasSubstr("name=turbo")))
        .Times(1);
    log.StartCapturingLogs();

    ZLOG(WARNING, "name=%s", "turbo");
}

TEST(ZlogTest, ZlogIfTrue) {
    turbo::ScopedMockLog log;
    EXPECT_CALL(log, Log(LogSeverity::kInfo, _, HasSubstr("enabled")))
        .Times(1);
    log.StartCapturingLogs();

    ZLOG_IF(INFO, true, "%s", "enabled");
}

TEST(ZlogTest, ZlogIfFalse) {
    turbo::ScopedMockLog log;
    EXPECT_CALL(log, Log(LogSeverity::kInfo, _, _)).Times(0);
    log.StartCapturingLogs();

    ZLOG_IF(INFO, false, "%s", "should not appear");
}

TEST(ZlogTest, ZlogEveryN) {
    turbo::ScopedMockLog log;
    EXPECT_CALL(log, Log(LogSeverity::kInfo, _, HasSubstr("tick")))
        .Times(2);
    log.StartCapturingLogs();

    for (int i = 0; i < 6; ++i) {
        ZLOG_EVERY_N(INFO, 3, "tick %d", i);
    }
}

TEST(ZlogTest, ZlogFirstN) {
    turbo::ScopedMockLog log;
    EXPECT_CALL(log, Log(LogSeverity::kInfo, _, HasSubstr("first")))
        .Times(2);
    log.StartCapturingLogs();

    for (int i = 0; i < 5; ++i) {
        ZLOG_FIRST_N(INFO, 2, "first %d", i);
    }
}

TEST(ZlogTest, MultipleArgs) {
    turbo::ScopedMockLog log;
    EXPECT_CALL(log, Log(LogSeverity::kError, _,
                HasSubstr("a=1 b=2 c=3 d=4 e=5")))
        .Times(1);
    log.StartCapturingLogs();

    ZLOG(ERROR, "a=%d b=%d c=%d d=%d e=%d", 1, 2, 3, 4, 5);
}

TEST(ZlogTest, HexFormat) {
    turbo::ScopedMockLog log;
    EXPECT_CALL(log, Log(LogSeverity::kInfo, _, HasSubstr("0xdeadbeef")))
        .Times(1);
    log.StartCapturingLogs();

    ZLOG(INFO, "%#x", 0xdeadbeefu);
}

TEST(ZlogTest, PointerFormat) {
    turbo::ScopedMockLog log;
    EXPECT_CALL(log, Log(LogSeverity::kInfo, _, _))
        .Times(1);
    log.StartCapturingLogs();

    int x = 42;
    ZLOG(INFO, "ptr=%p", static_cast<void*>(&x));
}

TEST(ZlogTest, CharFormat) {
    turbo::ScopedMockLog log;
    EXPECT_CALL(log, Log(LogSeverity::kInfo, _, HasSubstr("A B")))
        .Times(1);
    log.StartCapturingLogs();

    ZLOG(INFO, "%c %c", 'A', 'B');
}

}  // namespace
