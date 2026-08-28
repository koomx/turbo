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

#include "vectors.h"

#include <turbo/hash/sip/siphash.h>

#include <cstdint>
#include <cstring>

#include <gtest/gtest.h>

namespace {

void StoreLe32(uint8_t* p, uint32_t v) {
    p[0] = static_cast<uint8_t>(v);
    p[1] = static_cast<uint8_t>(v >> 8);
    p[2] = static_cast<uint8_t>(v >> 16);
    p[3] = static_cast<uint8_t>(v >> 24);
}

void StoreLe64(uint8_t* p, uint64_t v) {
    StoreLe32(p, static_cast<uint32_t>(v));
    StoreLe32(p + 4, static_cast<uint32_t>(v >> 32));
}

uint32_t LoadLe32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8)
        | (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

uint64_t LoadLe64(const uint8_t* p) {
    return static_cast<uint64_t>(LoadLe32(p))
        | (static_cast<uint64_t>(LoadLe32(p + 4)) << 32);
}

TEST(SipHash, ReferenceVectors) {
    uint8_t in[64];
    uint8_t out[16];
    uint8_t k[16];
    for (int i = 0; i < 16; ++i)
        k[i] = static_cast<uint8_t>(i);

    turbo::SipHashKey sip_key(LoadLe64(k), LoadLe64(k + 8),
        turbo::AlreadyLittleTag {});
    turbo::SipHashKey32 half_key(LoadLe32(k), LoadLe32(k + 4),
        turbo::AlreadyLittleTag {});

    for (int i = 0; i < 64; ++i) {
        in[i] = static_cast<uint8_t>(i);

        StoreLe64(out, turbo::siphash64(in, static_cast<size_t>(i), sip_key));
        EXPECT_EQ(0, std::memcmp(out, vectors_sip64[i], 8)) << "SipHash-2-4-64 bytes=" << i;

        auto h128 = turbo::siphash128(in, static_cast<size_t>(i), sip_key);
        StoreLe64(out, h128[0]);
        StoreLe64(out + 8, h128[1]);
        EXPECT_EQ(0, std::memcmp(out, vectors_sip128[i], 16)) << "SipHash-2-4-128 bytes=" << i;

        StoreLe32(out, turbo::half_siphash32(in, static_cast<size_t>(i), half_key));
        EXPECT_EQ(0, std::memcmp(out, vectors_hsip32[i], 4)) << "HalfSipHash-2-4-32 bytes=" << i;

        StoreLe64(out, turbo::half_siphash64(in, static_cast<size_t>(i), half_key));
        EXPECT_EQ(0, std::memcmp(out, vectors_hsip64[i], 8)) << "HalfSipHash-2-4-64 bytes=" << i;
    }
}

} // namespace
