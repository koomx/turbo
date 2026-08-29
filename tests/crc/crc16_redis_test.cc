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

#include <turbo/crc/crc16_redis.h>

#include <cstdint>
#include <string>

#include <gtest/gtest.h>

TEST(Crc16RedisTest, XmodemCheck) {
    const char* s = "123456789";
    EXPECT_EQ(0x31C3u, turbo::crc16_redis(reinterpret_cast<const uint8_t*>(s), 9));
    EXPECT_EQ(0x31C3u, turbo::crc16_redis(std::string_view(s, 9)));
    EXPECT_EQ(0x31C3u, turbo::crc16_redis(std::string(s)));
}

TEST(Crc16RedisTest, ExtendMatchesOneShot) {
    const uint8_t* hello_space_world = reinterpret_cast<const uint8_t*>("hello world");
    const uint8_t* hello_space = reinterpret_cast<const uint8_t*>("hello ");
    const uint8_t* world = reinterpret_cast<const uint8_t*>("world");

    EXPECT_EQ(turbo::crc16_redis(hello_space_world, 11),
        turbo::crc16_redis_extend(turbo::crc16_redis(hello_space, 6), world, 5));
}

TEST(Crc16RedisTest, SlotIsMasked) {
    std::string_view key = "foo";
    const uint16_t crc = turbo::crc16_redis(key);
    EXPECT_EQ(static_cast<uint16_t>(crc & 0x3FFFu), turbo::crc16_redis_slot(key));
    EXPECT_LT(turbo::crc16_redis_slot(key), 16384u);
}
