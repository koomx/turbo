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

class DailyFileSinkTest : public ::testing::Test {
protected:
    void SetUp() override {
        tmpdir_ = std::filesystem::temp_directory_path() /
                  "turbo_daily_file_sink_test";
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

    std::string BaseLogPath() const {
        return (tmpdir_ / "test.log").string();
    }

    std::filesystem::path tmpdir_;
};

TEST_F(DailyFileSinkTest, CreatesLogFileOnConstruction) {
    turbo::DailyFileSink sink(BaseLogPath(), 7, 600, true);
    // The daily file is created with a date suffix (app_YYYY-MM-DD.log), so the
    // base path itself must not exist.
    EXPECT_FALSE(std::filesystem::exists(BaseLogPath()));
    EXPECT_EQ(std::distance(std::filesystem::directory_iterator(tmpdir_),
                            std::filesystem::directory_iterator{}),
              1);
}

TEST_F(DailyFileSinkTest, SendWritesToFile) {
    turbo::DailyFileSink sink(BaseLogPath(), 7, 600, true);
    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

    EXPECT_CALL(test_sink, Log(_, _, "hello daily"));
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(&sink) << "hello daily";
    test_sink.StopCapturingLogs();

    sink.flush();

    // Find the daily file in the temp directory
    bool found = false;
    for (const auto& entry : std::filesystem::directory_iterator(tmpdir_)) {
        auto content = ReadFileContents(entry.path().string());
        if (content.find("hello daily") != std::string::npos) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(DailyFileSinkTest, MultipleMessages) {
    turbo::DailyFileSink sink(BaseLogPath(), 7, 600, true);
    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

    EXPECT_CALL(test_sink, Log(_, _, "first"))
        .Times(1);
    EXPECT_CALL(test_sink, Log(_, _, "second"))
        .Times(1);
    EXPECT_CALL(test_sink, Log(_, _, "third"))
        .Times(1);
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(&sink) << "first";
    KLOG(INFO).to_sink_also(&sink) << "second";
    KLOG(INFO).to_sink_also(&sink) << "third";
    test_sink.StopCapturingLogs();

    sink.flush();

    bool found_first = false;
    bool found_second = false;
    bool found_third = false;
    for (const auto& entry : std::filesystem::directory_iterator(tmpdir_)) {
        auto content = ReadFileContents(entry.path().string());
        if (content.find("first") != std::string::npos) found_first = true;
        if (content.find("second") != std::string::npos) found_second = true;
        if (content.find("third") != std::string::npos) found_third = true;
    }
    EXPECT_TRUE(found_first);
    EXPECT_TRUE(found_second);
    EXPECT_TRUE(found_third);
}

TEST_F(DailyFileSinkTest, FlushWriteThenFlush) {
    turbo::DailyFileSink sink(BaseLogPath(), 7, 600, true);

    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    EXPECT_CALL(test_sink, Log(_, _, "flush test"));
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(&sink) << "flush test";
    test_sink.StopCapturingLogs();

    EXPECT_NO_FATAL_FAILURE(sink.flush());
}

TEST_F(DailyFileSinkTest, MaxFilesZeroKeepsAll) {
    auto path = BaseLogPath();
    turbo::DailyFileSink sink(path, 0, 600, true);

    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    EXPECT_CALL(test_sink, Log(_, _, _)).Times(::testing::AnyNumber());
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(&sink) << "message 1";
    KLOG(INFO).to_sink_also(&sink) << "message 2";
    test_sink.StopCapturingLogs();
    sink.flush();

    // Should not crash with max_files=0
    SUCCEED();
}

}  // namespace
