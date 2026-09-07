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

#include <string>

#include <gtest/gtest.h>
#include <turbo/uri/wpt/parser.h>

TEST(from_file_tests, basics) {
    for (std::string path :
        {"", "fsfds", "C:\\\\blabala\\fdfds\\back.txt", "/home/user/txt.txt",
            "/%2e.bar", "/foo/%2e%2", "/foo/..bar", "foo\t%91"}) {
        const std::string href = turbo::href_from_file(path);
        ASSERT_TRUE(href.rfind("file://", 0) == 0) << href;
        ASSERT_FALSE(href.empty());
    }
}
