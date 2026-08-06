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

#include <turbo/log/sinks/auto_log_sink.h>

#include <memory>
#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <turbo/log/klog.h>
#include <turbo/log/sinks/stderr_log_sink.h>
#include <tests/log/internal/test_helpers.h>
#include <tests/log/scoped_mock_log.h>

namespace {

using ::testing::_;

auto* test_env KUMO_ATTRIBUTE_UNUSED = ::testing::AddGlobalTestEnvironment(
    new turbo::log_internal::LogTestEnvironment);

class CapturingSink : public turbo::LogSink {
public:
    void send(const turbo::LogEntry& entry) override {
        messages_.push_back(std::string(entry.text_message()));
    }
    void flush() override {}
    std::vector<std::string> messages_;
};

TEST(AutoLogSinkTest, RegistersOnConstructionAndCaptures) {
    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    auto capture = std::make_unique<CapturingSink>();
    CapturingSink* raw = capture.get();

    turbo::AutoLogSink<CapturingSink> guard(std::move(capture));

    EXPECT_CALL(test_sink, Log(_, _, "captured message"));
    test_sink.StartCapturingLogs();
    KLOG(INFO) << "captured message";
    test_sink.StopCapturingLogs();

    ASSERT_EQ(raw->messages_.size(), 1u);
    EXPECT_EQ(raw->messages_[0], "captured message");
}

TEST(AutoLogSinkTest, AccessorsExposeSink) {
    auto capture = std::make_unique<CapturingSink>();
    CapturingSink* raw = capture.get();

    turbo::AutoLogSink<CapturingSink> guard(std::move(capture));

    EXPECT_EQ(guard.get(), raw);
    EXPECT_EQ(&*guard, raw);
    EXPECT_EQ(guard.operator->(), raw);
}

TEST(AutoLogSinkTest, RemovesOnDestruction) {
    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    {
        turbo::AutoLogSink<CapturingSink> guard(
            std::make_unique<CapturingSink>());
        EXPECT_CALL(test_sink, Log(_, _, "inside"));
        test_sink.StartCapturingLogs();
        KLOG(INFO) << "inside";
        test_sink.StopCapturingLogs();
    }
    // Sink is removed and destroyed at scope exit; logging afterwards must not
    // dispatch to the destroyed sink.
    EXPECT_CALL(test_sink, Log(_, _, "after"));
    test_sink.StartCapturingLogs();
    KLOG(INFO) << "after";
    test_sink.StopCapturingLogs();
}

TEST(AutoLogSinkTest, TraitDetectsSystemManagedSinks) {
    EXPECT_TRUE(turbo::log_internal::is_system_managed_sink<
        turbo::StderrLogSink>::value);
    EXPECT_FALSE(turbo::log_internal::is_system_managed_sink<
        CapturingSink>::value);
}

// Negative compile-time checks are intentionally not tested: instantiating
// `turbo::AutoLogSink<turbo::StderrLogSink>` (and, on their respective
// platforms, `turbo::AndroidLogSink` / `turbo::WindowsDebuggerLogSink`) fails
// to compile because those system-managed sinks carry the
// `turbo_is_system_managed` marker.

}  // namespace
