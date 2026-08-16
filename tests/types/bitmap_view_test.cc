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

#include <turbo/types/bitmap_view.h>

#include <climits>
#include <cstdint>
#include <initializer_list>
#include <vector>

#include <gtest/gtest.h>

namespace {

    using turbo::BitmapView;
    using turbo::GetFirstBit;
    using turbo::GetLastBit;

    template <bool OverflowAsFalse = true, typename WordType = uint64_t>
    struct BitmapHarness {
        std::vector<WordType> storage;
        BitmapView<OverflowAsFalse, WordType> view;

        explicit BitmapHarness(size_t bits, uint64_t init = 0) {
            storage.assign(BITSET_WORD_COUNT(bits, WordType), WordType { 0 });
            view.setup({ storage.data(), storage.size() }, bits);
            view.from_unsigned(init);
        }
    };

    template <typename UInt>
    void TestGetFirstBitImpl() {
        EXPECT_EQ(GetFirstBit(static_cast<UInt>(0)), sizeof(UInt) * CHAR_BIT);

        for (uint32_t i = 0; i < sizeof(UInt) * CHAR_BIT; ++i) {
            UInt x = (static_cast<UInt>(1) << i) | (static_cast<UInt>(1) << (sizeof(UInt) * CHAR_BIT - 1));
            EXPECT_EQ(GetFirstBit(x), i);
        }
    }

    template <typename UInt>
    void TestGetLastBitImpl() {
        EXPECT_EQ(GetLastBit(static_cast<UInt>(0)), sizeof(UInt) * CHAR_BIT);

        for (uint32_t i = 0; i < sizeof(UInt) * CHAR_BIT; ++i) {
            UInt x = (static_cast<UInt>(1) << i) | static_cast<UInt>(1);
            EXPECT_EQ(GetLastBit(x), i);
        }
    }

    template <typename WordType>
    void TestBitmapViewWithWord() {
        auto verifyToUint32Truncated = [](const auto& bs, uint32_t truncatedValue) {
            EXPECT_EQ(bs.to_uint32_no_assert_convertible(), truncatedValue);
        };
        auto verifyToUint64Truncated = [](const auto& bs, uint64_t truncatedValue) {
            EXPECT_EQ(bs.to_uint64_no_assert_convertible(), truncatedValue);
        };
        auto verifyToUlongTruncatedIf32bit = [](const auto& bs, unsigned long truncatedValue) {
            EXPECT_EQ(bs.to_ulong_no_assert_convertible(), truncatedValue);
        };

        {
            BitmapHarness<true, WordType> h0(0, 0x10101010);
            auto& b0 = h0.view;
            EXPECT_EQ(b0.count(), 0u);
            EXPECT_EQ(b0.to_ulong_assert_convertible(), 0x00000000ul);
            EXPECT_EQ(b0.to_uint32_assert_convertible(), 0x00000000u);
            EXPECT_EQ(b0.to_uint64_assert_convertible(), 0x00000000ull);
            EXPECT_EQ(b0.as_uint64(), 0x00000000ull);
            EXPECT_EQ(b0.template as_uint<uint64_t>(), 0x00000000ull);

            b0.flip();
            EXPECT_EQ(b0.count(), 0u);

            b0 <<= 1;
            EXPECT_EQ(b0.count(), 0u);

            BitmapHarness<true, WordType> h8(8, 0x10101010);
            auto& b8 = h8.view;
            EXPECT_EQ(b8.count(), 1u);
            EXPECT_EQ(b8.to_ulong_assert_convertible(), 0x00000010ul);
            EXPECT_EQ(b8.to_uint32_assert_convertible(), 0x00000010u);
            EXPECT_EQ(b8.to_uint64_assert_convertible(), 0x00000010ull);
            EXPECT_EQ(b8.as_uint64(), 0x00000010ull);

            b8.flip();
            EXPECT_EQ(b8.count(), 7u);
            EXPECT_EQ(b8.to_uint32_assert_convertible(), 0x000000efu);

            b8 <<= 1;
            EXPECT_EQ(b8.count(), 6u);
            EXPECT_EQ(b8.to_uint32_assert_convertible(), 0x000000deu);

            b8.reset();
            b8.flip();
            b8 >>= 33;
            EXPECT_EQ(b8.count(), 0u);

            BitmapHarness<true, WordType> h16(16, 0x10101010);
            auto& b16 = h16.view;
            EXPECT_EQ(b16.count(), 2u);
            EXPECT_EQ(b16.to_uint32_assert_convertible(), 0x00001010u);
            b16.flip();
            EXPECT_EQ(b16.count(), 14u);
            EXPECT_EQ(b16.to_uint32_assert_convertible(), 0x0000efefu);
            b16 <<= 1;
            EXPECT_EQ(b16.count(), 13u);
            EXPECT_EQ(b16.to_uint32_assert_convertible(), 0x0000dfdeu);

            BitmapHarness<true, WordType> h32(32, 0x10101010);
            auto& b32 = h32.view;
            EXPECT_EQ(b32.count(), 4u);
            EXPECT_EQ(b32.to_uint32_assert_convertible(), 0x10101010u);
            b32.flip();
            EXPECT_EQ(b32.count(), 28u);
            EXPECT_EQ(b32.to_uint32_assert_convertible(), 0xefefefefu);
            b32 <<= 1;
            EXPECT_EQ(b32.count(), 27u);
            EXPECT_EQ(b32.to_uint32_assert_convertible(), 0xdfdfdfdeu);

            BitmapHarness<true, WordType> h64(64, 0x10101010);
            auto& b64 = h64.view;
            EXPECT_EQ(b64.count(), 4u);
            EXPECT_EQ(b64.to_uint32_assert_convertible(), 0x10101010u);
            EXPECT_EQ(b64.to_uint64_assert_convertible(), 0x10101010ull);

            b64.flip();
            EXPECT_EQ(b64.count(), 60u);
            verifyToUlongTruncatedIf32bit(b64, static_cast<unsigned long>(0xffffffffefefefefull));
            verifyToUint32Truncated(b64, 0xefefefefu);
            EXPECT_EQ(b64.to_uint64_assert_convertible(), 0xffffffffefefefefull);
            EXPECT_EQ(b64.as_uint64(), 0xffffffffefefefefull);

            b64 <<= 1;
            EXPECT_EQ(b64.count(), 59u);
            verifyToUint32Truncated(b64, 0xdfdfdfdeu);
            EXPECT_EQ(b64.to_uint64_assert_convertible(), 0xffffffffdfdfdfdeull);

            b64.reset();
            EXPECT_EQ(b64.count(), 0u);
            b64.flip();
            EXPECT_EQ(b64.count(), 64u);
            b64 <<= 1;
            EXPECT_EQ(b64.count(), 63u);

            b64.reset();
            b64.flip();
            b64 >>= 33;
            EXPECT_EQ(b64.count(), 31u);

            b64.reset();
            b64.flip();
            b64 >>= 65;
            EXPECT_EQ(b64.count(), 0u);

            h64.view.from_unsigned(UINT64_C(0x1010101010101010));
            EXPECT_EQ(h64.view.to_uint64_assert_convertible(), UINT64_C(0x1010101010101010));
        }

        {
            BitmapHarness<true, WordType> h1(1);
            BitmapHarness<true, WordType> h1A(1, 1);
            auto& b1 = h1.view;
            auto& b1A = h1A.view;
            EXPECT_EQ(b1.size(), 1u);
            EXPECT_FALSE(b1.any());
            EXPECT_FALSE(b1.all());
            EXPECT_TRUE(b1.none());
            EXPECT_TRUE(b1A.any());
            EXPECT_TRUE(b1A.all());
            EXPECT_FALSE(b1A.none());
            EXPECT_EQ(b1A.to_ulong_assert_convertible(), 1ul);

            BitmapHarness<true, WordType> h33(33);
            BitmapHarness<true, WordType> h33A(33, 1);
            auto& b33 = h33.view;
            auto& b33A = h33A.view;
            EXPECT_EQ(b33.size(), 33u);
            EXPECT_TRUE(b33.none());
            EXPECT_TRUE(b33A.any());
            EXPECT_FALSE(b33A.all());

            BitmapHarness<true, WordType> h65(65);
            BitmapHarness<true, WordType> h65A(65, 1);
            auto& b65 = h65.view;
            auto& b65A = h65A.view;
            EXPECT_EQ(b65.size(), 65u);
            EXPECT_TRUE(b65.none());
            EXPECT_TRUE(b65A.any());
            EXPECT_FALSE(b65A.all());

            BitmapHarness<true, WordType> h129(129);
            BitmapHarness<true, WordType> h129A(129, 1);
            auto& b129 = h129.view;
            auto& b129A = h129A.view;
            EXPECT_EQ(b129.size(), 129u);
            EXPECT_TRUE(b129.none());
            EXPECT_TRUE(b129A.any());

            b1[0] = true;
            EXPECT_TRUE(b1.test(0));
            EXPECT_EQ(b1.count(), 1u);

            b33[0] = true;
            b33[32] = true;
            EXPECT_TRUE(b33.test(0));
            EXPECT_FALSE(b33.test(15));
            EXPECT_TRUE(b33.test(32));
            EXPECT_EQ(b33.count(), 2u);

            b65.set(0, true);
            b65.set(32, true);
            b65.set(64, true);
            EXPECT_TRUE(b65.test(64));
            EXPECT_EQ(b65.count(), 3u);

            b129.set(0, true);
            b129.set(32, true);
            b129.set(64, true);
            b129.set(128, true);
            EXPECT_TRUE(b129.test(128));
            EXPECT_EQ(b129.count(), 4u);

            EXPECT_NE(b1.data(), nullptr);
            EXPECT_NE(b129.data(), nullptr);

            b1.reset();
            b1.set();
            EXPECT_TRUE(b1.all());
            b1.flip();
            EXPECT_TRUE(b1.none());
            b1.set(0, true);
            EXPECT_TRUE(b1[0]);
            b1.reset(0);
            EXPECT_FALSE(b1[0]);
            b1.flip(0);
            EXPECT_TRUE(b1[0]);

            BitmapHarness<true, WordType> h1Not(1);
            h1Not.storage = h1.storage;
            h1Not.view.setup({ h1Not.storage.data(), h1Not.storage.size() }, 1);
            h1Not.view.flip();
            EXPECT_TRUE(b1[0]);
            EXPECT_FALSE(h1Not.view[0]);

            b33.reset();
            b33.set();
            EXPECT_TRUE(b33.all());
            EXPECT_EQ(b33.count(), b33.size());
            b33.flip();
            EXPECT_TRUE(b33.none());
            b33.set(0, true);
            b33.set(32, true);
            EXPECT_TRUE(b33[32]);
            b33.reset(0);
            b33.reset(32);
            b33.flip(0);
            b33.flip(32);
            EXPECT_TRUE(b33[0]);
            EXPECT_TRUE(b33[32]);

            b65.reset();
            b65.set();
            EXPECT_TRUE(b65.all());
            b65.flip();
            EXPECT_TRUE(b65.none());
            b65.set(0, true);
            b65.set(32, true);
            b65.set(64, true);
            EXPECT_TRUE(b65[64]);

            b129.reset();
            b129.set();
            EXPECT_EQ(b129.count(), b129.size());
            b129.flip();
            EXPECT_TRUE(b129.none());
            b129.set(0, true);
            b129.set(32, true);
            b129.set(64, true);
            b129.set(128, true);
            EXPECT_TRUE(b129[128]);

            b1.reset();
            b1[0] = true;
            b1 >>= 0;
            EXPECT_TRUE(b1[0]);
            b1 >>= 1;
            EXPECT_FALSE(b1[0]);
            b1[0] = true;
            b1 <<= 1;
            EXPECT_FALSE(b1[0]);

            b1.reset();
            b1.flip();
            b1 >>= 33;
            EXPECT_TRUE(b1.none());

            b33.reset();
            b33[0] = true;
            b33[32] = true;
            b33 >>= 10;
            EXPECT_TRUE(b33[22]);
            b33.reset();
            b33[0] = true;
            b33[32] = true;
            b33 <<= 10;
            EXPECT_TRUE(b33[10]);
            b33.reset();
            b33.flip();
            b33 >>= 33;
            EXPECT_TRUE(b33.none());
            b33.reset();
            b33.flip();
            b33 <<= 65;
            EXPECT_TRUE(b33.none());

            b65.reset();
            b65[0] = true;
            b65[32] = true;
            b65[64] = true;
            b65 >>= 10;
            EXPECT_TRUE(b65[22]);
            EXPECT_TRUE(b65[54]);
            b65.reset();
            b65[0] = true;
            b65[32] = true;
            b65[64] = true;
            b65 <<= 10;
            EXPECT_TRUE(b65[10]);
            EXPECT_TRUE(b65[42]);
            b65.reset();
            b65.flip();
            b65 >>= 33;
            EXPECT_EQ(b65.count(), 32u);
            b65.reset();
            b65.flip();
            b65 <<= 33;
            EXPECT_EQ(b65.count(), 32u);
            b65.reset();
            b65.flip();
            b65 >>= 65;
            EXPECT_EQ(b65.count(), 0u);

            b129.reset();
            b129[0] = true;
            b129[32] = true;
            b129[64] = true;
            b129[128] = true;
            b129 >>= 10;
            EXPECT_TRUE(b129[22]);
            EXPECT_TRUE(b129[54]);
            EXPECT_TRUE(b129[118]);
            b129.reset();
            b129.flip();
            b129 >>= 33;
            EXPECT_EQ(b129.count(), 96u);
            b129.reset();
            b129.flip();
            b129 <<= 65;
            EXPECT_EQ(b129.count(), 64u);

            b1.set();
            b1[0] = false;
            b1A[0] = true;
            b1 &= b1A;
            EXPECT_FALSE(b1[0]);
            b1 |= b1A;
            EXPECT_TRUE(b1[0]);
            b1 ^= b1A;
            EXPECT_FALSE(b1[0]);
            b1 |= b1A;
            EXPECT_TRUE(b1[0]);

            b33.set();
            b33[0] = false;
            b33[32] = false;
            b33A[0] = true;
            b33A[32] = true;
            b33 &= b33A;
            EXPECT_FALSE(b33[0]);
            EXPECT_FALSE(b33[32]);
            b33 |= b33A;
            EXPECT_TRUE(b33[0]);
            EXPECT_TRUE(b33[32]);
            b33 ^= b33A;
            EXPECT_FALSE(b33[0]);
            b33 |= b33A;
            EXPECT_TRUE(b33[32]);

            b65.set();
            b65[0] = false;
            b65[32] = false;
            b65[64] = false;
            b65A[0] = true;
            b65A[32] = true;
            b65A[64] = true;
            b65 &= b65A;
            EXPECT_FALSE(b65[64]);
            b65 |= b65A;
            EXPECT_TRUE(b65[64]);
            b65 ^= b65A;
            EXPECT_FALSE(b65[64]);

            b129.set();
            b129[0] = false;
            b129[32] = false;
            b129[64] = false;
            b129[128] = false;
            b129A[0] = true;
            b129A[32] = true;
            b129A[64] = true;
            b129A[128] = true;
            b129 &= b129A;
            EXPECT_FALSE(b129[128]);
            b129 |= b129A;
            EXPECT_TRUE(b129[128]);
            b129 ^= b129A;
            EXPECT_FALSE(b129[128]);
        }

        {
            BitmapHarness<true, WordType> h65(65);
            typename BitmapView<true, WordType>::Reference r = h65.view[33];
            r = true;
            EXPECT_TRUE(r);
        }

        auto check_find_first = [](size_t bits, std::initializer_list<size_t> set_bits) {
            BitmapHarness<true, WordType> h(bits);
            auto& b = h.view;
            EXPECT_EQ(b.find_first(), b.size());
            for (size_t bit : set_bits) {
                b.set(bit, true);
            }
            size_t i = 0;
            size_t j = b.find_first();
            for (size_t expected : set_bits) {
                EXPECT_EQ(j, expected);
                j = b.find_next(j);
                ++i;
            }
            EXPECT_EQ(j, b.size());
            EXPECT_EQ(i, set_bits.size());

            b.set();
            size_t n = 0;
            for (j = b.find_first(); j != b.size(); j = b.find_next(j)) {
                ++n;
            }
            EXPECT_EQ(n, bits);
        };

        check_find_first(1, { 0 });
        check_find_first(7, { 0, 5 });
        check_find_first(32, { 0, 27 });
        check_find_first(41, { 0, 27, 37 });
        check_find_first(64, { 0, 27, 37 });
        check_find_first(79, { 0, 27, 37 });
        check_find_first(128, { 0, 27, 37, 77 });
        check_find_first(137, { 0, 27, 37, 77, 99, 136 });

        auto check_find_last = [](size_t bits, std::initializer_list<size_t> set_bits) {
            BitmapHarness<true, WordType> h(bits);
            auto& b = h.view;
            EXPECT_EQ(b.find_last(), b.size());
            for (size_t bit : set_bits) {
                b.set(bit, true);
            }
            std::vector<size_t> expected(set_bits);
            size_t j = b.find_last();
            for (auto it = expected.rbegin(); it != expected.rend(); ++it) {
                EXPECT_EQ(j, *it);
                j = b.find_prev(j);
            }
            EXPECT_EQ(j, b.size());

            b.set();
            size_t n = 0;
            for (j = b.find_last(); j != b.size(); j = b.find_prev(j)) {
                ++n;
            }
            EXPECT_EQ(n, bits);
        };

        check_find_last(1, { 0 });
        check_find_last(7, { 0, 5 });
        check_find_last(32, { 0, 27 });
        check_find_last(41, { 0, 27, 37 });
        check_find_last(64, { 0, 27, 37 });
        check_find_last(79, { 0, 27, 37 });
        check_find_last(128, { 0, 27, 37, 77 });
        check_find_last(137, { 0, 27, 37, 77, 99, 136 });

        {
            BitmapHarness<true, WordType> h64(64);
            h64.view.set(0, true);
            h64.view.set(27, true);
            h64.view.set(37, true);
            verifyToUint32Truncated(h64.view, 0x08000001u);
        }
        {
            BitmapHarness<true, WordType> h79(79);
            h79.view.set(0, true);
            h79.view.set(27, true);
            h79.view.set(37, true);
            EXPECT_EQ(h79.view.to_uint64_assert_convertible(), 0x0000002008000001ull);
            h79.view.set();
            verifyToUint64Truncated(h79.view, 0xffffffffffffffffull);
        }
        {
            BitmapHarness<true, WordType> h99(99);
            h99.view.set(63);
            verifyToUint32Truncated(h99.view, 0x0u);
            EXPECT_EQ(h99.view.to_uint64_assert_convertible(), 0x8000000000000000ull);
        }

        TestGetFirstBitImpl<WordType>();
        TestGetLastBitImpl<WordType>();
    }

} // namespace

TEST(BitmapViewTest, Uint64Word) {
    TestBitmapViewWithWord<uint64_t>();
}

TEST(BitmapViewTest, GetFirstLastBitWidths) {
    TestGetFirstBitImpl<uint8_t>();
    TestGetFirstBitImpl<uint16_t>();
    TestGetFirstBitImpl<uint32_t>();
    TestGetFirstBitImpl<uint64_t>();
    TestGetLastBitImpl<uint8_t>();
    TestGetLastBitImpl<uint16_t>();
    TestGetLastBitImpl<uint32_t>();
    TestGetLastBitImpl<uint64_t>();
}

TEST(BitmapViewTest, WordCountMacro) {
    EXPECT_EQ(BITSET_WORD_COUNT(32, uint64_t), 1u);
    EXPECT_EQ(BITSET_WORD_COUNT(128, uint64_t), 2u);
    EXPECT_EQ(BITSET_WORD_COUNT(256, uint64_t), 4u);
    BitmapHarness<> h32(32);
    EXPECT_EQ(h32.view.word_size(), BITSET_WORD_COUNT(32, uint64_t));
    BitmapHarness<> h256(256);
    EXPECT_EQ(h256.view.word_size(), BITSET_WORD_COUNT(256, uint64_t));
}

TEST(BitmapViewTest, SetupSpanBits) {
    std::vector<uint64_t> words(2, 0);
    BitmapView<true, uint64_t> v;
    v.setup({ words.data(), words.size() });
    EXPECT_EQ(v.size(), 128u);
    EXPECT_EQ(v.word_size(), 2u);
}

TEST(BitmapViewTest, OverflowAsFalse) {
    BitmapHarness<true, uint64_t> h(8);
    EXPECT_FALSE(h.view.test(100));
    h.view.set(100, true);
    EXPECT_FALSE(h.view.test(0));
    EXPECT_EQ(h.view.count(), 0u);
}
