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

#include <turbo/log/sinks/internal/fs_helper.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

namespace {

class FsHelperTest : public ::testing::Test {
protected:
    void SetUp() override {
        tmpdir_ = std::filesystem::temp_directory_path() /
                  "turbo_fs_helper_test";
        std::filesystem::create_directories(tmpdir_);
    }

    void TearDown() override {
        std::filesystem::remove_all(tmpdir_);
    }

    std::string TestPath(const std::string& name) const {
        return (tmpdir_ / name).string();
    }

    std::filesystem::path tmpdir_;
};

TEST_F(FsHelperTest, PathExistsReturnsTrueForExistingPath) {
    auto path = TestPath("exists.txt");
    std::ofstream(path) << "data";
    EXPECT_TRUE(turbo::log_internal::path_exists(path));
}

TEST_F(FsHelperTest, PathExistsReturnsFalseForNonexistentPath) {
    auto path = TestPath("nonexistent.txt");
    EXPECT_FALSE(turbo::log_internal::path_exists(path));
}

TEST_F(FsHelperTest, CreateDirectoriesCreatesNewDir) {
    auto dir = TestPath("newdir");
    EXPECT_TRUE(turbo::log_internal::create_directories(dir));
    EXPECT_TRUE(std::filesystem::exists(dir));
    EXPECT_TRUE(std::filesystem::is_directory(dir));
}

TEST_F(FsHelperTest, CreateDirectoriesCreatesNestedDirs) {
    auto dir = TestPath("a/b/c");
    EXPECT_TRUE(turbo::log_internal::create_directories(dir));
    EXPECT_TRUE(std::filesystem::exists(dir));
    EXPECT_TRUE(std::filesystem::is_directory(dir));
}

TEST_F(FsHelperTest, CreateDirectoriesEmptyPathReturnsTrue) {
    EXPECT_TRUE(turbo::log_internal::create_directories(""));
}

TEST_F(FsHelperTest, RenamePathRenamesFile) {
    auto from = TestPath("original.txt");
    auto to = TestPath("renamed.txt");
    std::ofstream(from) << "content";

    EXPECT_TRUE(turbo::log_internal::rename_path(from, to));
    EXPECT_FALSE(std::filesystem::exists(from));
    EXPECT_TRUE(std::filesystem::exists(to));
}

TEST_F(FsHelperTest, RenamePathFailsForNonexistentSource) {
    auto from = TestPath("noexist.txt");
    auto to = TestPath("dest.txt");
    EXPECT_FALSE(turbo::log_internal::rename_path(from, to));
}

TEST_F(FsHelperTest, RemovePathDeletesFile) {
    auto path = TestPath("to_delete.txt");
    std::ofstream(path) << "data";

    EXPECT_TRUE(turbo::log_internal::remove_path(path));
    EXPECT_FALSE(std::filesystem::exists(path));
}

TEST_F(FsHelperTest, RemovePathSucceedsForNonexistentPath) {
    auto path = TestPath("nonexistent_to_delete.txt");
    // remove_path uses error_code and should return true even for non-existent
    EXPECT_TRUE(turbo::log_internal::remove_path(path));
}

TEST_F(FsHelperTest, ParentDirReturnsDirectory) {
    EXPECT_EQ(turbo::log_internal::parent_dir("/var/log/app.log"), "/var/log");
    EXPECT_EQ(turbo::log_internal::parent_dir("/var/log/"), "/var/log");
    EXPECT_EQ(turbo::log_internal::parent_dir("app.log"), "");
}

TEST_F(FsHelperTest, IsColorTerminalNullFileReturnsFalse) {
    EXPECT_FALSE(turbo::log_internal::is_color_terminal(nullptr));
}

TEST_F(FsHelperTest, IsColorTerminalRegularFileReturnsFalse) {
    auto path = TestPath("regular.txt");
    std::ofstream(path) << "data";
    FILE* f = std::fopen(path.c_str(), "r");
    ASSERT_NE(f, nullptr);
    EXPECT_FALSE(turbo::log_internal::is_color_terminal(f));
    std::fclose(f);
}

TEST_F(FsHelperTest, FileSizeOfNullFileReturnsZero) {
    EXPECT_EQ(turbo::log_internal::file_size_of(nullptr), 0u);
}

TEST_F(FsHelperTest, FileSizeOfReturnsCorrectSize) {
    auto path = TestPath("size_test.txt");
    {
        std::ofstream out(path);
        out << "1234567890";  // 10 bytes
    }
    FILE* f = std::fopen(path.c_str(), "r");
    ASSERT_NE(f, nullptr);
    EXPECT_EQ(turbo::log_internal::file_size_of(f), 10u);
    std::fclose(f);
}

}  // namespace
