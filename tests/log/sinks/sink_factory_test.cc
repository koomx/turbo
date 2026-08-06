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

#include <turbo/log/sinks/sink_factory.h>

#include <cstdio>
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

class SinkFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        tmpdir_ = std::filesystem::temp_directory_path() /
                  "turbo_sink_factory_test";
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

    std::filesystem::path tmpdir_;
};

TEST_F(SinkFactoryTest, CreateNullSink) {
    auto sink = turbo::create_null_sink();
    ASSERT_NE(sink, nullptr);
    EXPECT_NO_FATAL_FAILURE(sink->flush());
}

TEST_F(SinkFactoryTest, CreateNullSinkCanBeUsed) {
    auto sink = turbo::create_null_sink();
    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    EXPECT_CALL(test_sink, Log(_, _, "via null sink"));
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(sink.get()) << "via null sink";
    test_sink.StopCapturingLogs();
}

TEST_F(SinkFactoryTest, CreateAnsiColorSink) {
    auto path = (tmpdir_ / "console.log").string();
    FILE* f = std::fopen(path.c_str(), "w+");
    ASSERT_NE(f, nullptr);

    auto sink = turbo::create_ansi_color_sink(f);
    ASSERT_NE(sink, nullptr);

    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    EXPECT_CALL(test_sink, Log(_, _, "console message"));
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(sink.get()) << "console message";
    test_sink.StopCapturingLogs();

    sink->flush();
    std::fclose(f);

    auto content = ReadFileContents(path);
    EXPECT_NE(content.find("console message"), std::string::npos);
}

TEST_F(SinkFactoryTest, CreateDailyFileSink) {
    auto base = (tmpdir_ / "app.log").string();
    auto sink = turbo::create_daily_file_sink(base, 7, 600, true);
    ASSERT_NE(sink, nullptr);

    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    EXPECT_CALL(test_sink, Log(_, _, "daily factory"));
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(sink.get()) << "daily factory";
    test_sink.StopCapturingLogs();
    sink->flush();

    bool found = false;
    for (const auto& entry : std::filesystem::directory_iterator(tmpdir_)) {
        if (ReadFileContents(entry.path().string()).find("daily factory") !=
            std::string::npos) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(SinkFactoryTest, CreateHourlyFileSink) {
    auto base = (tmpdir_ / "app.log").string();
    auto sink = turbo::create_hourly_file_sink(base, 84, 600, true);
    ASSERT_NE(sink, nullptr);

    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    EXPECT_CALL(test_sink, Log(_, _, "hourly factory"));
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(sink.get()) << "hourly factory";
    test_sink.StopCapturingLogs();
    sink->flush();

    bool found = false;
    for (const auto& entry : std::filesystem::directory_iterator(tmpdir_)) {
        if (ReadFileContents(entry.path().string()).find("hourly factory") !=
            std::string::npos) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(SinkFactoryTest, CreateRotatingFileSink) {
    auto base = (tmpdir_ / "rot.log").string();
    auto sink = turbo::create_rotating_file_sink(base, 1024, 3);
    ASSERT_NE(sink, nullptr);

    turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);
    EXPECT_CALL(test_sink, Log(_, _, "rotating factory"));
    test_sink.StartCapturingLogs();
    KLOG(INFO).to_sink_also(sink.get()) << "rotating factory";
    test_sink.StopCapturingLogs();
    sink->flush();

    auto content = ReadFileContents(base);
    EXPECT_NE(content.find("rotating factory"), std::string::npos);
}

}  // namespace
