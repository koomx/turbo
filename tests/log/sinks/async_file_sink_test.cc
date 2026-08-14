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

#include <turbo/log/sinks/daily_file_sink.h>
#include <turbo/log/sinks/hourly_file_sink.h>
#include <turbo/log/sinks/rotating_file_sink.h>

#include <filesystem>
#include <fstream>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <turbo/log/klog.h>
#include <tests/log/internal/test_helpers.h>
#include <tests/log/scoped_mock_log.h>

namespace {

using ::testing::_;

auto* test_env KUMO_ATTRIBUTE_UNUSED = ::testing::AddGlobalTestEnvironment(
    new turbo::log_internal::LogTestEnvironment);

class AsyncFileSinkTest : public ::testing::Test {
protected:
    void SetUp() override {
        tmpdir_ = std::filesystem::temp_directory_path() /
                  "turbo_async_file_sink_test";
        std::filesystem::remove_all(tmpdir_);
        std::filesystem::create_directories(tmpdir_);
    }

    void TearDown() override {
        std::filesystem::remove_all(tmpdir_);
    }

    std::string ReadFileContents(const std::string& filepath) {
        std::ifstream in(filepath);
        return std::string(std::istreambuf_iterator<char>(in),
                           std::istreambuf_iterator<char>());
    }

    bool DirContains(const std::string& needle) {
        for (const auto& entry : std::filesystem::directory_iterator(tmpdir_)) {
            if (ReadFileContents(entry.path().string()).find(needle) !=
                std::string::npos) {
                return true;
            }
        }
        return false;
    }

    std::string Path(const char* name) const {
        return (tmpdir_ / name).string();
    }

    std::filesystem::path tmpdir_;
};

TEST_F(AsyncFileSinkTest, StartStopTogglesAsync) {
    turbo::DailyFileSink sink(Path("daily.log"), 7, 600, true);
    EXPECT_FALSE(sink.is_async());
    sink.start();
    EXPECT_TRUE(sink.is_async());
    sink.stop();
    EXPECT_FALSE(sink.is_async());
}

TEST_F(AsyncFileSinkTest, DailyAsyncWritesAfterStop) {
    turbo::DailyFileSink sink(Path("daily.log"), 7, 600, true);
    sink.start();

    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    EXPECT_CALL(test_sink, Log(_, _, "async daily"));
    EXPECT_CALL(test_sink, Log(_, _, "async daily two"));
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(&sink) << "async daily";
    KLOG(INFO).to_sink_also(&sink) << "async daily two";
    test_sink.StopCapturingLogs();

    sink.stop();
    EXPECT_TRUE(DirContains("async daily"));
    EXPECT_TRUE(DirContains("async daily two"));
}

TEST_F(AsyncFileSinkTest, HourlyAsyncWritesAfterStop) {
    turbo::HourlyFileSink sink(Path("hourly.log"), 7, 600, true);
    sink.start();

    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    EXPECT_CALL(test_sink, Log(_, _, "async hourly"));
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(&sink) << "async hourly";
    test_sink.StopCapturingLogs();

    sink.stop();
    EXPECT_TRUE(DirContains("async hourly"));
}

TEST_F(AsyncFileSinkTest, RotatingAsyncWritesAfterStop) {
    turbo::RotatingFileSink sink(Path("rotate.log"), 10 * 1024 * 1024, 3);
    sink.start();

    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    EXPECT_CALL(test_sink, Log(_, _, "async rotating"));
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(&sink) << "async rotating";
    test_sink.StopCapturingLogs();

    sink.stop();
    EXPECT_NE(ReadFileContents(Path("rotate.log")).find("async rotating"),
              std::string::npos);
}

TEST_F(AsyncFileSinkTest, RotatingAsyncSizeRewind) {
    turbo::RotatingFileSink sink(Path("rotate.log"), 50, 3);
    sink.start();

    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    EXPECT_CALL(test_sink, Log(_, _, _)).Times(::testing::AnyNumber());
    test_sink.StartCapturingLogs();
    for (int i = 0; i < 10; ++i) {
        KLOG(INFO).to_sink_also(&sink) << "async rotate " << i;
    }
    test_sink.StopCapturingLogs();

    sink.stop();
    EXPECT_GT(std::distance(std::filesystem::directory_iterator(tmpdir_),
                            std::filesystem::directory_iterator{}),
              0);
    EXPECT_TRUE(DirContains("async rotate"));
}

TEST_F(AsyncFileSinkTest, AfterStopSendIsSync) {
    turbo::RotatingFileSink sink(Path("rotate.log"), 10 * 1024 * 1024, 3);
    sink.start();
    sink.stop();
    EXPECT_FALSE(sink.is_async());

    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    EXPECT_CALL(test_sink, Log(_, _, "sync after stop"));
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(&sink) << "sync after stop";
    test_sink.StopCapturingLogs();
    sink.flush();

    EXPECT_NE(ReadFileContents(Path("rotate.log")).find("sync after stop"),
              std::string::npos);
}

}  // namespace
