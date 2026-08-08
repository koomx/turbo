// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <turbo/macros/config.h>

#include <cstdint>
#include <thread>

#include <gtest/gtest.h>

namespace {

TEST(ConfigTest, Endianness) {
  union {
    uint32_t value;
    uint8_t data[sizeof(uint32_t)];
  } number;
  number.data[0] = 0x00;
  number.data[1] = 0x01;
  number.data[2] = 0x02;
  number.data[3] = 0x03;
#if KUMO_ENDIAN_LITTLE && KUMO_ENDIAN_BIG
#error Both KUMO_ENDIAN_LITTLE and KUMO_ENDIAN_BIG are defined
#elif KUMO_ENDIAN_LITTLE
  EXPECT_EQ(UINT32_C(0x03020100), number.value);
#elif KUMO_ENDIAN_BIG
  EXPECT_EQ(UINT32_C(0x00010203), number.value);
#else
#error Unknown endianness
#endif
}

#if KUMO_HAVE_THREAD_LOCAL
TEST(ConfigTest, ThreadLocal) {
  static thread_local int mine_mine_mine = 16;
  EXPECT_EQ(16, mine_mine_mine);
  {
    std::thread t([] {
      EXPECT_EQ(16, mine_mine_mine);
      mine_mine_mine = 32;
      EXPECT_EQ(32, mine_mine_mine);
    });
    t.join();
  }
  EXPECT_EQ(16, mine_mine_mine);
}
#endif

}  // namespace
