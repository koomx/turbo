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

#include <turbo/log/sinks/null_sink.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <turbo/log/klog.h>
#include <turbo/log/log_sink_registry.h>
#include <tests/log/internal/test_helpers.h>
#include <tests/log/scoped_mock_log.h>

namespace {

using ::testing::_;

auto* test_env KUMO_ATTRIBUTE_UNUSED = ::testing::AddGlobalTestEnvironment(
    new turbo::log_internal::LogTestEnvironment);

TEST(NullSinkTest, SendAndFlushDoesNotCrash) {
    turbo::NullSink sink;
    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    EXPECT_CALL(test_sink, Log(_, _, "null sink test"));
    test_sink.StartCapturingLogs();
    turbo::add_log_sink(&sink);
    KLOG(INFO) << "null sink test";
    turbo::flush_log_sinks();
    turbo::remove_log_sink(&sink);
    test_sink.StopCapturingLogs();
}

TEST(NullSinkTest, CanRegisterAndUnregister) {
    turbo::NullSink sink;
    turbo::add_log_sink(&sink);
    turbo::remove_log_sink(&sink);
}

TEST(NullSinkTest, LogEntriesPassedToNullSink) {
    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    turbo::NullSink null_sink;

    EXPECT_CALL(test_sink, Log(_, _, "hello null sink"));

    test_sink.StartCapturingLogs();
    turbo::add_log_sink(&null_sink);
    KLOG(INFO) << "hello null sink";
    turbo::remove_log_sink(&null_sink);
    test_sink.StopCapturingLogs();
}

TEST(NullSinkTest, ToSinkAlsoWithNullSink) {
    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    turbo::NullSink null_sink;

    EXPECT_CALL(test_sink, Log(_, _, "to sink also"));

    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(&null_sink) << "to sink also";
    test_sink.StopCapturingLogs();
}

}  // namespace
