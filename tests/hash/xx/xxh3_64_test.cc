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

#include <tests/hash/xx/testdata_64b.h>
#include <turbo/hash/xx/xxh3.h>

#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

TEST(Xxh3_64, Vectors) {
    std::vector<uint8_t> buf(4096 + 64 + 1);
    turbo::xxtest::fill_buffer(buf.data(), buf.size());
    for (auto const& t : turbo::xxtest::xxhash_testdata_64b) {
        EXPECT_EQ(turbo::xxhash_64bits_with_seed(buf.data(), t.len, t.seed), t.nresult) << t.len << ' ' << t.seed;
        turbo::XxHashState64 st(t.seed);
        st.update(buf.data(), t.len);
        EXPECT_EQ(st.digest(), t.nresult) << t.len << ' ' << t.seed;
    }
}
