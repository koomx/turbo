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

#include <tests/hash/xx/testdata_secret.h>
#include <turbo/hash/xx/xxh3.h>

#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

TEST(XxhashGenerateSecret, Vectors) {
    std::vector<uint8_t> seed_buf(4096 + 64 + 1);
    turbo::xxtest::fill_buffer(seed_buf.data(), seed_buf.size());
    static constexpr int kSample[] = { 0, 62, 131, 191, 241 };
    for (auto const& t : turbo::xxtest::xxhash_testdata_secret) {
        std::vector<uint8_t> secret(9867, 0);
        turbo::xxhash_generate_secret(secret.data(), t.secretLen, seed_buf.data(), t.seedLen);
        for (int i = 0; i < 5; ++i) {
            EXPECT_EQ(secret[kSample[i]], t.byte[i]) << t.seedLen << ' ' << t.secretLen << ' ' << i;
        }
    }
}
