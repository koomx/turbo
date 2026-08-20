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
    ASSERT_EQ(all.size(), 8u);
    for (size_t i = 0; i < all.size(); ++i) {
        EXPECT_STREQ(all[i].isa_name, kExpectedNames[i]);
        EXPECT_FALSE(all[i].failback);
    }
}

TEST(UnicodeRegistry, AvailIsArm64Only) {
    const auto avail = turbo::UnicodeRegistry::get_avail_isa_info();
#if KUMO_ARCH_ARM
    ASSERT_EQ(avail.size(), 1u);
    EXPECT_STREQ(avail[0].isa_name, "arm64");
    EXPECT_TRUE(avail[0].compiled);
    EXPECT_NE(avail[0].engine, nullptr);
    EXPECT_GT(avail[0].rank, 0u);

    auto* best = turbo::UnicodeRegistry::get_best_isa();
    ASSERT_NE(best, nullptr);
    EXPECT_EQ(best->name(), std::string_view("arm64"));
#else
    for (const auto& info : avail) {
        EXPECT_STRNE(info.isa_name, "arm64");
    }
#endif
}

}  // namespace
