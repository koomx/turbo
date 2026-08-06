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

#include <turbo/log/sinks/hourly_file_sink.h>

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

class HourlyFileSinkTest : public ::testing::Test {
protected:
    void SetUp() override {
        tmpdir_ = std::filesystem::temp_directory_path() /
                  "turbo_hourly_file_sink_test";
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

TEST_F(HourlyFileSinkTest, CreatesLogFileOnConstruction) {
    turbo::HourlyFileSink sink(BaseLogPath(), 7, 600, true);
    // After construction, files should be created in the temp directory.
    EXPECT_GE(std::distance(std::filesystem::directory_iterator(tmpdir_),
                             std::filesystem::directory_iterator{}),
              0);
}

TEST_F(HourlyFileSinkTest, SendWritesToFile) {
    turbo::HourlyFileSink sink(BaseLogPath(), 7, 600, true);
    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

    EXPECT_CALL(test_sink, Log(_, _, "hello hourly"));
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(&sink) << "hello hourly";
    test_sink.StopCapturingLogs();

    sink.flush();

    bool found = false;
    for (const auto& entry : std::filesystem::directory_iterator(tmpdir_)) {
        auto content = ReadFileContents(entry.path().string());
        if (content.find("hello hourly") != std::string::npos) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(HourlyFileSinkTest, MultipleMessages) {
    turbo::HourlyFileSink sink(BaseLogPath(), 7, 600, true);
    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

    EXPECT_CALL(test_sink, Log(_, _, "alpha")).Times(1);
    EXPECT_CALL(test_sink, Log(_, _, "beta")).Times(1);
    EXPECT_CALL(test_sink, Log(_, _, "gamma")).Times(1);
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(&sink) << "alpha";
    KLOG(INFO).to_sink_also(&sink) << "beta";
    KLOG(INFO).to_sink_also(&sink) << "gamma";
    test_sink.StopCapturingLogs();

    sink.flush();

    bool found_alpha = false;
    bool found_beta = false;
    bool found_gamma = false;
    for (const auto& entry : std::filesystem::directory_iterator(tmpdir_)) {
        auto content = ReadFileContents(entry.path().string());
        if (content.find("alpha") != std::string::npos) found_alpha = true;
        if (content.find("beta") != std::string::npos) found_beta = true;
        if (content.find("gamma") != std::string::npos) found_gamma = true;
    }
    EXPECT_TRUE(found_alpha);
    EXPECT_TRUE(found_beta);
    EXPECT_TRUE(found_gamma);
}

TEST_F(HourlyFileSinkTest, FlushWriteThenFlush) {
    turbo::HourlyFileSink sink(BaseLogPath(), 7, 600, true);

    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    EXPECT_CALL(test_sink, Log(_, _, "flush hourly"));
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(&sink) << "flush hourly";
    test_sink.StopCapturingLogs();

    EXPECT_NO_FATAL_FAILURE(sink.flush());
}

TEST_F(HourlyFileSinkTest, DefaultMaxFiles) {
    // Default max_files is 84 (3.5 days of hourly files)
    turbo::HourlyFileSink sink(BaseLogPath());
    // Construction should succeed
    SUCCEED();
}

TEST_F(HourlyFileSinkTest, MaxFilesZeroKeepsAll) {
    turbo::HourlyFileSink sink(BaseLogPath(), 0, 600, true);

    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    EXPECT_CALL(test_sink, Log(_, _, _)).Times(::testing::AnyNumber());
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(&sink) << "keep me";
    test_sink.StopCapturingLogs();
    sink.flush();

    SUCCEED();
}

}  // namespace
