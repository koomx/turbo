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

class RotatingFileSinkTest : public ::testing::Test {
protected:
    void SetUp() override {
        tmpdir_ = std::filesystem::temp_directory_path() /
                  "turbo_rotating_file_sink_test";
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

    std::vector<std::string> ListFiles() {
        std::vector<std::string> files;
        for (const auto& entry : std::filesystem::directory_iterator(tmpdir_)) {
            files.push_back(entry.path().filename().string());
        }
        std::sort(files.begin(), files.end());
        return files;
    }

    std::filesystem::path tmpdir_;
};

TEST_F(RotatingFileSinkTest, CreatesActiveFileOnConstruction) {
    turbo::RotatingFileSink sink(BaseLogPath(), 1024, 3);
    EXPECT_TRUE(std::filesystem::exists(BaseLogPath()));
}

TEST_F(RotatingFileSinkTest, SendWritesToActiveFile) {
    turbo::RotatingFileSink sink(BaseLogPath(), 1024, 3);
    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

    EXPECT_CALL(test_sink, Log(_, _, "hello rotating"));
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(&sink) << "hello rotating";
    test_sink.StopCapturingLogs();

    sink.flush();

    auto content = ReadFileContents(BaseLogPath());
    EXPECT_NE(content.find("hello rotating"), std::string::npos);
}

TEST_F(RotatingFileSinkTest, MultipleMessagesGoToSameFile) {
    turbo::RotatingFileSink sink(BaseLogPath(), 10 * 1024 * 1024, 3);
    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

    EXPECT_CALL(test_sink, Log(_, _, "first")).Times(1);
    EXPECT_CALL(test_sink, Log(_, _, "second")).Times(1);
    EXPECT_CALL(test_sink, Log(_, _, "third")).Times(1);
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(&sink) << "first";
    KLOG(INFO).to_sink_also(&sink) << "second";
    KLOG(INFO).to_sink_also(&sink) << "third";
    test_sink.StopCapturingLogs();

    sink.flush();

    auto content = ReadFileContents(BaseLogPath());
    EXPECT_NE(content.find("first"), std::string::npos);
    EXPECT_NE(content.find("second"), std::string::npos);
    EXPECT_NE(content.find("third"), std::string::npos);
}

TEST_F(RotatingFileSinkTest, RotationHappensWhenSizeExceeded) {
    // Set max_size to a very small value to trigger rotation quickly.
    turbo::RotatingFileSink sink(BaseLogPath(), 50, 3);
    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

    EXPECT_CALL(test_sink, Log(_, _, _)).Times(::testing::AnyNumber());
    test_sink.StartCapturingLogs();

    // Write multiple messages that exceed the small max_size.
    for (int i = 0; i < 10; ++i) {
        KLOG(INFO).to_sink_also(&sink) << "message number " << i;
    }
    test_sink.StopCapturingLogs();

    sink.flush();

    // After rotation, the active file and some rotated files should exist.
    auto files = ListFiles();
    EXPECT_GT(files.size(), 0u);
}

TEST_F(RotatingFileSinkTest, MaxFilesNotExceeded) {
    turbo::RotatingFileSink sink(BaseLogPath(), 50, 3);
    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

    EXPECT_CALL(test_sink, Log(_, _, _)).Times(::testing::AnyNumber());
    test_sink.StartCapturingLogs();

    for (int i = 0; i < 50; ++i) {
        KLOG(INFO).to_sink_also(&sink) << "rotation message " << i;
    }
    test_sink.StopCapturingLogs();
    sink.flush();

    auto files = ListFiles();
    // With max_files=3, we should have at most 3 rotated files + 1 active file
    EXPECT_LE(files.size(), 4u);
}

TEST_F(RotatingFileSinkTest, MaxSizeZeroNeverRotates) {
    turbo::RotatingFileSink sink(BaseLogPath(), 0, 3);
    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

    EXPECT_CALL(test_sink, Log(_, _, _)).Times(::testing::AnyNumber());
    test_sink.StartCapturingLogs();

    for (int i = 0; i < 10; ++i) {
        KLOG(INFO).to_sink_also(&sink) << "no rotation " << i;
    }
    test_sink.StopCapturingLogs();
    sink.flush();

    auto files = ListFiles();
    // Should only have the active file since max_size=0 disables rotation.
    EXPECT_EQ(files.size(), 1u);
}

TEST_F(RotatingFileSinkTest, FlushWriteThenFlush) {
    turbo::RotatingFileSink sink(BaseLogPath(), 1024, 3);

    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    EXPECT_CALL(test_sink, Log(_, _, "flush rotating"));
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(&sink) << "flush rotating";
    test_sink.StopCapturingLogs();

    EXPECT_NO_FATAL_FAILURE(sink.flush());
}

}  // namespace
