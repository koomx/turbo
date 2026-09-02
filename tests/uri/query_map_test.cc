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

#include <turbo/uri/query_map.h>

#include <deque>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

namespace {

    using StringMap = turbo::QueryMap<std::string, std::string>;
    using StringDequeMap = turbo::QueryMap<std::string, std::string, std::less<std::string>,
        std::deque<std::pair<std::string, std::string>>>;
    using StringInlinedMap = turbo::InlinedQueryMap<std::string, std::string, 32>;
    using ViewMap = turbo::QueryMap<std::string_view, std::string_view>;
    using ViewDequeMap = turbo::QueryMap<std::string_view, std::string_view, std::less<std::string_view>,
        std::deque<std::pair<std::string_view, std::string_view>>>;
    using ViewInlinedMap = turbo::InlinedQueryMap<std::string_view, std::string_view, 32>;

    template <typename T>
    class QueryMapTest : public testing::Test { };

    using QueryMapTypes = testing::Types<StringMap, StringDequeMap, StringInlinedMap, ViewMap, ViewDequeMap, ViewInlinedMap>;

    TYPED_TEST_SUITE(QueryMapTest, QueryMapTypes);

    TYPED_TEST(QueryMapTest, ConstructionAndAssignment) {
        TypeParam mm1;
        EXPECT_TRUE(mm1.empty());

        TypeParam mm2 = { { "1", "one" }, { "1", "uno" }, { "2", "two" } };
        EXPECT_EQ(mm2.size(), 3u);
        EXPECT_EQ(mm2.count("1"), 2u);

        TypeParam mm3(mm2);
        EXPECT_EQ(mm3, mm2);

        TypeParam mm4(std::move(mm2));
        EXPECT_EQ(mm4.size(), 3u);

        mm1 = mm4;
        EXPECT_EQ(mm1, mm4);
    }

    TYPED_TEST(QueryMapTest, InsertAndEmplace) {
        TypeParam mm;
        auto it1 = mm.insert(typename TypeParam::value_type { "1", "100" });
        EXPECT_EQ(it1->first, "1");
        auto it2 = mm.insert(typename TypeParam::value_type { "1", "101" });
        EXPECT_EQ(it2->first, "1");
        EXPECT_EQ(mm.size(), 2u);
        EXPECT_EQ(mm.count("1"), 2u);

        auto it3 = mm.emplace("2", "200");
        EXPECT_EQ(it3->first, "2");
        EXPECT_EQ(it3->second, "200");

        auto it4 = mm.emplace_hint(mm.end(), "2", "201");
        EXPECT_EQ(it4->first, "2");
        EXPECT_EQ(mm.count("2"), 2u);

        auto it5 = mm.insert(typename TypeParam::key_type { "3" });
        EXPECT_EQ(it5->first, "3");
        EXPECT_TRUE(it5->second.empty());
    }

    TYPED_TEST(QueryMapTest, FindAndContains) {
        TypeParam mm = { { "1", "one" }, { "1", "uno" }, { "2", "two" } };
        auto it = mm.find("1");
        ASSERT_NE(it, mm.end());
        EXPECT_EQ(it->first, "1");
        EXPECT_TRUE(mm.contains("1"));
        EXPECT_FALSE(mm.contains("3"));
    }

    TYPED_TEST(QueryMapTest, Erase) {
        TypeParam mm = { { "1", "one" }, { "1", "uno" }, { "2", "two" }, { "3", "three" } };
        size_t erased = mm.erase("1");
        EXPECT_EQ(erased, 2u);
        EXPECT_EQ(mm.size(), 2u);
        EXPECT_FALSE(mm.contains("1"));

        auto it = mm.find("2");
        it = mm.erase(it);
        EXPECT_EQ(mm.size(), 1u);
        EXPECT_EQ(it->first, "3");

        mm.erase(mm.begin(), mm.end());
        EXPECT_TRUE(mm.empty());
    }

    TYPED_TEST(QueryMapTest, EqualRangeAndCount) {
        TypeParam mm = { { "1", "one" }, { "1", "uno" }, { "1", "eins" }, { "2", "two" } };
        auto [first, last] = mm.equal_range("1");
        EXPECT_EQ(std::distance(first, last), 3);
        int count = 0;
        for (auto it = first; it != last; ++it) {
            EXPECT_EQ(it->first, "1");
            ++count;
        }
        EXPECT_EQ(count, 3);
        EXPECT_EQ(mm.count("1"), 3u);
        EXPECT_EQ(mm.count("2"), 1u);
        EXPECT_EQ(mm.count("3"), 0u);
    }

    TYPED_TEST(QueryMapTest, LowerUpperBound) {
        TypeParam mm = { { "1", "10" }, { "1", "11" }, { "2", "20" }, { "3", "30" } };
        auto lb = mm.lower_bound("1");
        EXPECT_EQ(lb->first, "1");
        auto ub = mm.upper_bound("1");
        EXPECT_EQ(ub->first, "2");
        EXPECT_EQ(std::distance(lb, ub), 2);

        lb = mm.lower_bound("5");
        EXPECT_EQ(lb, mm.end());
        ub = mm.upper_bound("5");
        EXPECT_EQ(ub, mm.end());
    }

    TYPED_TEST(QueryMapTest, IteratorsAndReverse) {
        TypeParam mm = { { "1", "10" }, { "2", "20" }, { "3", "30" } };
        std::vector<std::string> keys;
        for (auto it = mm.begin(); it != mm.end(); ++it)
            keys.emplace_back(it->first);
        EXPECT_EQ(keys, (std::vector<std::string> { "1", "2", "3" }));

        keys.clear();
        for (auto it = mm.rbegin(); it != mm.rend(); ++it)
            keys.emplace_back(it->first);
        EXPECT_EQ(keys, (std::vector<std::string> { "3", "2", "1" }));
    }

    TYPED_TEST(QueryMapTest, Swap) {
        TypeParam mm1 = { { "1", "one" }, { "2", "two" } };
        TypeParam mm2 = { { "3", "three" } };
        mm1.swap(mm2);
        EXPECT_EQ(mm1.size(), 1u);
        EXPECT_EQ(mm2.size(), 2u);
        EXPECT_TRUE(mm1.contains("3"));
        EXPECT_TRUE(mm2.contains("1"));
    }

    TEST(QueryMapHeterogeneousTest, StringLessTransparent) {
        turbo::QueryMap<std::string, std::string, std::less<>> mm = {
            { "apple", "1" }, { "apple", "2" }, { "banana", "3" }
        };
        std::string_view sv = "apple";
        auto range = mm.equal_range(sv);
        int count = 0;
        for (auto it = range.first; it != range.second; ++it) {
            EXPECT_EQ(it->first, "apple");
            ++count;
        }
        EXPECT_EQ(count, 2);
        EXPECT_EQ(mm.count(sv), 2u);
    }

}  // namespace
