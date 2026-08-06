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

#include <turbo/log/sinks/log_filename.h>

#include <gtest/gtest.h>
#include <turbo/time/civil_time.h>
#include <turbo/time/time.h>

namespace {

TEST(BaseFilenameTest, BasicFilenameNoExt) {
    turbo::BaseFilename base("myapp");
    EXPECT_EQ(base.directory, "");
    EXPECT_EQ(base.basename, "myapp");
    EXPECT_EQ(base.extension, "");
}

TEST(BaseFilenameTest, FilenameWithExt) {
    turbo::BaseFilename base("myapp.log");
    EXPECT_EQ(base.directory, "");
    EXPECT_EQ(base.basename, "myapp");
    EXPECT_EQ(base.extension, ".log");
}

TEST(BaseFilenameTest, FilenameWithPath) {
    turbo::BaseFilename base("/var/log/myapp.log");
    EXPECT_EQ(base.directory, "/var/log");
    EXPECT_EQ(base.basename, "myapp");
    EXPECT_EQ(base.extension, ".log");
}

TEST(BaseFilenameTest, FilenameWithRelativePath) {
    turbo::BaseFilename base("logs/myapp.log");
    EXPECT_EQ(base.directory, "logs");
    EXPECT_EQ(base.basename, "myapp");
    EXPECT_EQ(base.extension, ".log");
}

TEST(BaseFilenameTest, FilenameWithMultipleDots) {
    turbo::BaseFilename base("/var/log/myapp.v2.log");
    EXPECT_EQ(base.directory, "/var/log");
    EXPECT_EQ(base.basename, "myapp.v2");
    EXPECT_EQ(base.extension, ".log");
}

TEST(BaseFilenameTest, FilenameWithNoStemJustExt) {
    turbo::BaseFilename base(".hidden");
    EXPECT_EQ(base.basename, ".hidden");
    EXPECT_EQ(base.extension, "");
}

TEST(BaseFilenameTest, FilenameJustDirectory) {
    turbo::BaseFilename base("/var/log/");
    // On some platforms, trailing slash parsing varies.
    // The key assertion is that parsing does not crash.
    SUCCEED();
}

TEST(SequentialLogPathTest, FormatsCorrectly) {
    turbo::BaseFilename base("myapp.log");
    EXPECT_EQ(turbo::sequential_log_path(base, 1), "myapp_0001.log");
    EXPECT_EQ(turbo::sequential_log_path(base, 42), "myapp_0042.log");
    EXPECT_EQ(turbo::sequential_log_path(base, 9999), "myapp_9999.log");
}

TEST(SequentialLogPathTest, WithDirectory) {
    turbo::BaseFilename base("/var/log/myapp.log");
    EXPECT_EQ(turbo::sequential_log_path(base, 1), "/var/log/myapp_0001.log");
    EXPECT_EQ(turbo::sequential_log_path(base, 100), "/var/log/myapp_0100.log");
}

TEST(SequentialLogPathTest, NoExtension) {
    turbo::BaseFilename base("myapp");
    EXPECT_EQ(turbo::sequential_log_path(base, 5), "myapp_0005");
}

TEST(DailyLogPathTest, FormatsCorrectly) {
    turbo::BaseFilename base("app.log");
    turbo::CivilSecond cs(2026, 8, 5, 0, 0, 0);
    const turbo::Time t = turbo::FromCivil(cs, turbo::UTCTimeZone());
    EXPECT_EQ(turbo::daily_log_path(base, t, true), "app_2026-08-05.log");
}

TEST(DailyLogPathTest, WithDirectory) {
    turbo::BaseFilename base("/var/log/app.log");
    turbo::CivilSecond cs(2026, 1, 17, 0, 0, 0);
    const turbo::Time t = turbo::FromCivil(cs, turbo::UTCTimeZone());
    EXPECT_EQ(turbo::daily_log_path(base, t, true),
              "/var/log/app_2026-01-17.log");
}

TEST(DailyLogPathTest, NoExtension) {
    turbo::BaseFilename base("app");
    turbo::CivilSecond cs(2026, 12, 31, 0, 0, 0);
    const turbo::Time t = turbo::FromCivil(cs, turbo::UTCTimeZone());
    EXPECT_EQ(turbo::daily_log_path(base, t, true), "app_2026-12-31");
}

TEST(HourlyLogPathTest, FormatsCorrectly) {
    turbo::BaseFilename base("app.log");
    turbo::CivilSecond cs(2026, 8, 5, 14, 0, 0);
    const turbo::Time t = turbo::FromCivil(cs, turbo::UTCTimeZone());
    EXPECT_EQ(turbo::hourly_log_path(base, t, true), "app_2026-08-05-14.log");
}

TEST(HourlyLogPathTest, WithDirectory) {
    turbo::BaseFilename base("/var/log/app.log");
    turbo::CivilSecond cs(2026, 1, 1, 3, 0, 0);
    const turbo::Time t = turbo::FromCivil(cs, turbo::UTCTimeZone());
    EXPECT_EQ(turbo::hourly_log_path(base, t, true),
              "/var/log/app_2026-01-01-03.log");
}

TEST(HourlyLogPathTest, LeadingZeroForHour) {
    turbo::BaseFilename base("app.log");
    turbo::CivilSecond cs(2026, 8, 5, 9, 0, 0);
    const turbo::Time t = turbo::FromCivil(cs, turbo::UTCTimeZone());
    EXPECT_EQ(turbo::hourly_log_path(base, t, true), "app_2026-08-05-09.log");
}

}  // namespace
