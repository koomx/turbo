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

#include <turbo/log/sinks/ansi_color_sink.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <turbo/log/globals.h>
#include <turbo/log/klog.h>
#include <tests/log/internal/test_helpers.h>
#include <tests/log/scoped_mock_log.h>

namespace {

using ::testing::_;

auto* test_env KUMO_ATTRIBUTE_UNUSED = ::testing::AddGlobalTestEnvironment(
    new turbo::log_internal::LogTestEnvironment);

class AnsiColorSinkTest : public ::testing::Test {
protected:
    void SetUp() override {
        tmpdir_ = std::filesystem::temp_directory_path() /
                  "turbo_ansi_color_sink_test";
        std::filesystem::create_directories(tmpdir_);
        tmpfile_ = tmpdir_ / "test.log";
    }

    void TearDown() override {
        if (_fp) {
            std::fclose(_fp);
            _fp = nullptr;
        }
        std::filesystem::remove_all(tmpdir_);
    }

    FILE* OpenFile() {
        _fp = std::fopen(tmpfile_.c_str(), "w+");
        return _fp;
    }

    std::string ReadFileContents() {
        if (_fp) {
            std::fflush(_fp);
        }
        std::ifstream in(tmpfile_);
        return std::string(std::istreambuf_iterator<char>(in),
                           std::istreambuf_iterator<char>());
    }

    std::filesystem::path tmpdir_;
    std::filesystem::path tmpfile_;
    FILE* _fp = nullptr;
};

TEST_F(AnsiColorSinkTest, NullFileReturnsEarly) {
    turbo::AnsiColorSink sink(nullptr);
    // Should not crash
    EXPECT_NO_FATAL_FAILURE(sink.flush());
}

TEST_F(AnsiColorSinkTest, WriteToFileNoAnsiCodes) {
    FILE* f = OpenFile();
    ASSERT_NE(f, nullptr);
    turbo::AnsiColorSink sink(f);

    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    EXPECT_CALL(test_sink, Log(_, _, "test message"));
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(&sink) << "test message";
    test_sink.StopCapturingLogs();

    std::string content = ReadFileContents();
    EXPECT_NE(content.find("test message"), std::string::npos);
    // Regular files are not terminals, so no ANSI codes
    EXPECT_EQ(content.find("\033["), std::string::npos);
}

TEST_F(AnsiColorSinkTest, FlushCallsFflush) {
    FILE* f = OpenFile();
    ASSERT_NE(f, nullptr);
    turbo::AnsiColorSink sink(f);
    // Should not crash
    EXPECT_NO_FATAL_FAILURE(sink.flush());
}

TEST_F(AnsiColorSinkTest, AllSeverityLevels) {
    FILE* f = OpenFile();
    ASSERT_NE(f, nullptr);
    turbo::AnsiColorSink sink(f);

    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    EXPECT_CALL(test_sink, Log(_, _, "trace msg"));
    EXPECT_CALL(test_sink, Log(_, _, "debug msg"));
    EXPECT_CALL(test_sink, Log(_, _, "info msg"));
    EXPECT_CALL(test_sink, Log(_, _, "warning msg"));
    EXPECT_CALL(test_sink, Log(_, _, "error msg"));
    test_sink.StartCapturingLogs();

    // TRACE/DEBUG are gated by min_log_level, which defaults to kInfo.
    const auto saved_min = turbo::min_log_level();
    turbo::set_min_log_level(turbo::LogSeverityAtLeast::kTrace);

    KLOG(TRACE).to_sink_also(&sink) << "trace msg";
    KLOG(DEBUG).to_sink_also(&sink) << "debug msg";
    KLOG(INFO).to_sink_also(&sink) << "info msg";
    KLOG(WARNING).to_sink_also(&sink) << "warning msg";
    KLOG(ERROR).to_sink_also(&sink) << "error msg";

    turbo::set_min_log_level(saved_min);
    test_sink.StopCapturingLogs();

    std::string content = ReadFileContents();
    EXPECT_NE(content.find("trace msg"), std::string::npos);
    EXPECT_NE(content.find("debug msg"), std::string::npos);
    EXPECT_NE(content.find("info msg"), std::string::npos);
    EXPECT_NE(content.find("warning msg"), std::string::npos);
    EXPECT_NE(content.find("error msg"), std::string::npos);
}

}  // namespace
