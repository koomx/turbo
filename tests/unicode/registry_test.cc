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
//

#include <turbo/unicode/engine/isa_select.h>

#include <string_view>

#include <gtest/gtest.h>

namespace {

constexpr const char* kExpectedNames[] = {
    "arm64",
    "fallback",
    "icelake",
    "haswell",
    "westmere",
    "ppc64",
    "rvv",
    "lasx",
    "lsx",
};

TEST(UnicodeRegistry, AllInfosRegistered) {
    const auto all = turbo::UnicodeRegistry::get_all_isa_info();
    ASSERT_EQ(all.size(), 9u);
    for (size_t i = 0; i < all.size(); ++i) {
        EXPECT_STREQ(all[i].isa_name, kExpectedNames[i]);
        if (std::string_view(all[i].isa_name) == "fallback") {
            EXPECT_TRUE(all[i].failback);
        } else {
            EXPECT_FALSE(all[i].failback);
        }
    }
}

TEST(UnicodeRegistry, AvailBestIsArm64WithFallback) {
    const auto avail = turbo::UnicodeRegistry::get_avail_isa_info();
#if KUMO_ARCH_ARM
    ASSERT_EQ(avail.size(), 2u);
    EXPECT_STREQ(avail[0].isa_name, "arm64");
    EXPECT_TRUE(avail[0].compiled);
    EXPECT_NE(avail[0].engine, nullptr);
    EXPECT_EQ(avail[0].rank, 10100u);
    EXPECT_STREQ(avail[1].isa_name, "fallback");
    EXPECT_TRUE(avail[1].failback);
    EXPECT_TRUE(avail[1].compiled);
    EXPECT_NE(avail[1].engine, nullptr);
    EXPECT_EQ(avail[1].rank, 1u);

    auto* best = turbo::UnicodeRegistry::get_best_isa();
    ASSERT_NE(best, nullptr);
    EXPECT_EQ(best->name(), std::string_view("arm64"));
    auto* fallback = turbo::UnicodeRegistry::get_failback_isa();
    ASSERT_NE(fallback, nullptr);
    EXPECT_EQ(fallback->name(), std::string_view("fallback"));
#else
    bool saw_fallback = false;
    for (const auto& info : avail) {
        EXPECT_STRNE(info.isa_name, "arm64");
        if (std::string_view(info.isa_name) == "fallback") {
            saw_fallback = true;
        }
    }
    EXPECT_TRUE(saw_fallback);
#endif
}

TEST(UnicodeRegistry, Dump) {
    std::cerr<<turbo::UnicodeRegistry::dump()<<std::endl;
}
}  // namespace
