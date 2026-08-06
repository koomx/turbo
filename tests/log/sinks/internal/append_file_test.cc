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

#include <turbo/log/sinks/internal/append_file.h>

#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

namespace {

class AppendFileTest : public ::testing::Test {
protected:
    void SetUp() override {
        tmpdir_ = std::filesystem::temp_directory_path() /
                  "turbo_append_file_test";
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

    std::string TestPath() const {
        return (tmpdir_ / "test.log").string();
    }

    std::string SubDirPath() const {
        return (tmpdir_ / "sub" / "test.log").string();
    }

    std::filesystem::path tmpdir_;
};

TEST_F(AppendFileTest, InitializeCreatesFile) {
    turbo::log_internal::AppendFile f;
    auto path = TestPath();
    EXPECT_EQ(f.initialize(path), 0);
    EXPECT_TRUE(std::filesystem::exists(path));
}

TEST_F(AppendFileTest, InitializeCreatesParentDirs) {
    turbo::log_internal::AppendFile f;
    auto path = SubDirPath();
    EXPECT_EQ(f.initialize(path), 0);
    EXPECT_TRUE(std::filesystem::exists(path));
}

TEST_F(AppendFileTest, WriteReturnsByteCount) {
    turbo::log_internal::AppendFile f;
    auto path = TestPath();
    ASSERT_EQ(f.initialize(path), 0);

    size_t written = f.write("hello");
    EXPECT_EQ(written, 5u);

    f.flush();
    auto content = ReadFileContents(path);
    EXPECT_EQ(content, "hello");
}

TEST_F(AppendFileTest, WriteMultipleMessages) {
    turbo::log_internal::AppendFile f;
    auto path = TestPath();
    ASSERT_EQ(f.initialize(path), 0);

    f.write("first ");
    f.write("second ");
    f.write("third");
    f.flush();

    auto content = ReadFileContents(path);
    EXPECT_EQ(content, "first second third");
}

TEST_F(AppendFileTest, WriteEmptyMessageReturnsZero) {
    turbo::log_internal::AppendFile f;
    auto path = TestPath();
    ASSERT_EQ(f.initialize(path), 0);

    size_t written = f.write("");
    EXPECT_EQ(written, 0u);
}

TEST_F(AppendFileTest, WriteToClosedFileReturnsZero) {
    turbo::log_internal::AppendFile f;
    auto path = TestPath();
    ASSERT_EQ(f.initialize(path), 0);
    f.close();

    size_t written = f.write("should not write");
    EXPECT_EQ(written, 0u);
}

TEST_F(AppendFileTest, FileSizeTracksWrittenBytes) {
    turbo::log_internal::AppendFile f;
    auto path = TestPath();
    ASSERT_EQ(f.initialize(path), 0);

    EXPECT_EQ(f.file_size(), 0u);

    f.write("hello");
    EXPECT_EQ(f.file_size(), 5u);

    f.write(" world");
    EXPECT_EQ(f.file_size(), 11u);
}

TEST_F(AppendFileTest, CloseAndReopen) {
    turbo::log_internal::AppendFile f;
    auto path = TestPath();
    ASSERT_EQ(f.initialize(path), 0);
    f.write("first ");
    f.close();

    EXPECT_EQ(f.reopen(), 0);
    f.write("second");
    f.flush();

    auto content = ReadFileContents(path);
    EXPECT_EQ(content, "first second");
}

TEST_F(AppendFileTest, FilePathReturnsCorrectPath) {
    turbo::log_internal::AppendFile f;
    auto path = TestPath();
    ASSERT_EQ(f.initialize(path), 0);

    EXPECT_EQ(f.file_path(), path);
}

TEST_F(AppendFileTest, DestructorClosesFile) {
    auto path = TestPath();
    {
        turbo::log_internal::AppendFile f;
        ASSERT_EQ(f.initialize(path), 0);
        f.write("data");
    }
    // File should still exist and be intact after destructor
    EXPECT_TRUE(std::filesystem::exists(path));
    auto content = ReadFileContents(path);
    EXPECT_EQ(content, "data");
}

TEST_F(AppendFileTest, FlushAfterWrite) {
    turbo::log_internal::AppendFile f;
    auto path = TestPath();
    ASSERT_EQ(f.initialize(path), 0);

    f.write("before flush");
    f.flush();

    // Data should be readable immediately after flush
    auto content = ReadFileContents(path);
    EXPECT_EQ(content, "before flush");
}

}  // namespace
