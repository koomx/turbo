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

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <turbo/numeric/int128.h>

namespace turbo {

    class murmur3_32 {
    public:
        explicit murmur3_32(uint32_t seed = 0)
            : h1_(seed)
            , total_len_(0)
            , tail_len_(0) { }
        void update(const void* data, size_t len) {
            auto p = static_cast<const uint8_t*>(data);
            if (tail_len_ > 0) {
                size_t append = (len < (4 - tail_len_)) ? len : (4 - tail_len_);
                ::memcpy(tail_ + tail_len_, p, append);
                total_len_ += append;
                tail_len_ += append;
                p += append;
                len -= append;
                if (tail_len_ == 4) {
                    uint32_t k1 = *reinterpret_cast<const uint32_t*>(tail_);
                    k1 *= c1_;
                    k1 = rotl32(k1, 15);
                    k1 *= c2_;
                    h1_ ^= k1;
                    h1_ = rotl32(h1_, 13);
                    h1_ = h1_ * 5 + 0xe6546b64;
                    tail_len_ = 0;
                }
            }
            int nblocks = len / 4;
            auto blocks = reinterpret_cast<const uint32_t*>(p + nblocks * 4);
            for (int i = -nblocks; i; ++i) {
                uint32_t k1 = blocks[i];
                k1 *= c1_;
                k1 = rotl32(k1, 15);
                k1 *= c2_;
                h1_ ^= k1;
                h1_ = rotl32(h1_, 13);
                h1_ = h1_ * 5 + 0xe6546b64;
            }
            int rem = len & 3;
            if (rem > 0) {
                memcpy(tail_, p + nblocks * 4, rem);
                tail_len_ = rem;
            }
            total_len_ += len;
        }
        uint32_t final() {
            uint32_t k1 = 0;
            switch (tail_len_) {
            case 3:
                k1 ^= tail_[2] << 16;
            case 2:
                k1 ^= tail_[1] << 8;
            case 1:
                k1 ^= tail_[0];
                k1 *= c1_;
                k1 = rotl32(k1, 15);
                k1 *= c2_;
                h1_ ^= k1;
            }
            h1_ ^= total_len_;
            h1_ = fmix32(h1_);
            return h1_;
        }
        static uint32_t hash(const void* data, size_t len, uint32_t seed = 0) {
            murmur3_32 h(seed);
            h.update(data, len);
            return h.final();
        }

    private:
        static uint32_t rotl32(uint32_t x, int8_t r) {
            return (x << r) | (x >> (32 - r));
        }
        static uint32_t fmix32(uint32_t h) {
            h ^= h >> 16;
            h *= 0x85ebca6b;
            h ^= h >> 13;
            h *= 0xc2b2ae35;
            h ^= h >> 16;
            return h;
        }
        static const uint32_t c1_ = 0xcc9e2d51;
        static const uint32_t c2_ = 0x1b873593;
        uint32_t h1_;
        int total_len_;
        int tail_len_;
        uint8_t tail_[4];
    };
    class murmur3_64 {
    public:
        explicit murmur3_64(uint32_t seed = 0)
            : h1_(seed)
            , h2_(seed)
            , total_len_(0)
            , tail_len_(0) { }
        void update(const void* data, size_t len) {
            auto p = static_cast<const uint8_t*>(data);
            if (tail_len_ > 0) {
                size_t append = (len < (16 - tail_len_)) ? len : (16 - tail_len_);
                memcpy(tail_ + tail_len_, p, append);
                total_len_ += append;
                tail_len_ += append;
                p += append;
                len -= append;
                if (tail_len_ == 16) {
                    auto b = reinterpret_cast<const uint64_t*>(tail_);
                    uint64_t k1 = b[0];
                    uint64_t k2 = b[1];
                    k1 *= c1_;
                    k1 = rotl64(k1, 31);
                    k1 *= c2_;
                    h1_ ^= k1;
                    h1_ = rotl64(h1_, 27);
                    h1_ += h2_;
                    h1_ = h1_ * 5 + 0x52dce729;
                    k2 *= c2_;
                    k2 = rotl64(k2, 33);
                    k2 *= c1_;
                    h2_ ^= k2;
                    h2_ = rotl64(h2_, 31);
                    h2_ += h1_;
                    h2_ = h2_ * 5 + 0x38495ab5;
                    tail_len_ = 0;
                }
            }
            int nblocks = len / 16;
            auto blocks = reinterpret_cast<const uint64_t*>(p);
            for (int i = 0; i < nblocks; ++i) {
                uint64_t k1 = blocks[i * 2 + 0];
                uint64_t k2 = blocks[i * 2 + 1];
                k1 *= c1_;
                k1 = rotl64(k1, 31);
                k1 *= c2_;
                h1_ ^= k1;
                h1_ = rotl64(h1_, 27);
                h1_ += h2_;
                h1_ = h1_ * 5 + 0x52dce729;
                k2 *= c2_;
                k2 = rotl64(k2, 33);
                k2 *= c1_;
                h2_ ^= k2;
                h2_ = rotl64(h2_, 31);
                h2_ += h1_;
                h2_ = h2_ * 5 + 0x38495ab5;
            }
            int rem = len & 15;
            if (rem > 0) {
                memcpy(tail_, p + nblocks * 16, rem);
                tail_len_ = rem;
            }
            total_len_ += len;
        }
        uint64_t final() {
            uint64_t k1 = 0;
            uint64_t k2 = 0;
            auto tail = tail_;
            switch (tail_len_) {
            case 15:
                k2 ^= uint64_t(tail[14]) << 48;
            case 14:
                k2 ^= uint64_t(tail[13]) << 40;
            case 13:
                k2 ^= uint64_t(tail[12]) << 32;
            case 12:
                k2 ^= uint64_t(tail[11]) << 24;
            case 11:
                k2 ^= uint64_t(tail[10]) << 16;
            case 10:
                k2 ^= uint64_t(tail[9]) << 8;
            case 9:
                k2 ^= uint64_t(tail[8]) << 0;
                k2 *= c2_;
                k2 = rotl64(k2, 33);
                k2 *= c1_;
                h2_ ^= k2;
            case 8:
                k1 ^= uint64_t(tail[7]) << 56;
            case 7:
                k1 ^= uint64_t(tail[6]) << 48;
            case 6:
                k1 ^= uint64_t(tail[5]) << 40;
            case 5:
                k1 ^= uint64_t(tail[4]) << 32;
            case 4:
                k1 ^= uint64_t(tail[3]) << 24;
            case 3:
                k1 ^= uint64_t(tail[2]) << 16;
            case 2:
                k1 ^= uint64_t(tail[1]) << 8;
            case 1:
                k1 ^= uint64_t(tail[0]) << 0;
                k1 *= c1_;
                k1 = rotl64(k1, 31);
                k1 *= c2_;
                h1_ ^= k1;
            }
            h1_ ^= total_len_;
            h2_ ^= total_len_;
            h1_ += h2_;
            h2_ += h1_;
            h1_ = fmix64(h1_);
            h2_ = fmix64(h2_);
            h1_ += h2_;
            h2_ += h1_;
            return h1_ * 101 + h2_;
        }
        static uint64_t hash(const void* data, size_t len, uint32_t seed = 0) {
            murmur3_64 h(seed);
            h.update(data, len);
            return h.final();
        }

    private:
        static uint64_t rotl64(uint64_t x, int8_t r) {
            return (x << r) | (x >> (64 - r));
        }
        static uint64_t fmix64(uint64_t k) {
            k ^= k >> 33;
            k *= 0xff51afd7ed558ccdull;
            k ^= k >> 33;
            k *= 0xc4ceb9fe1a85ec53ull;
            k ^= k >> 33;
            return k;
        }
        static const uint64_t c1_ = 0x87c37b91114253d5ull;
        static const uint64_t c2_ = 0x4cf5ad432745937full;
        uint64_t h1_;
        uint64_t h2_;
        uint64_t total_len_;
        int tail_len_;
        uint8_t tail_[16];
    };
    class murmur3_128 {
    public:
        explicit murmur3_128(uint32_t seed = 0)
            : h1_(seed)
            , h2_(seed)
            , total_len_(0)
            , tail_len_(0) { }
        void update(const void* data, size_t len) {
            auto p = static_cast<const uint8_t*>(data);
            if (tail_len_ > 0) {
                size_t append = (len < (16 - tail_len_)) ? len : (16 - tail_len_);
                memcpy(tail_ + tail_len_, p, append);
                total_len_ += append;
                tail_len_ += append;
                p += append;
                len -= append;
                if (tail_len_ == 16) {
                    auto b = reinterpret_cast<const uint64_t*>(tail_);
                    uint64_t k1 = b[0];
                    uint64_t k2 = b[1];
                    k1 *= c1_;
                    k1 = rotl64(k1, 31);
                    k1 *= c2_;
                    h1_ ^= k1;
                    h1_ = rotl64(h1_, 27);
                    h1_ += h2_;
                    h1_ = h1_ * 5 + 0x52dce729;
                    k2 *= c2_;
                    k2 = rotl64(k2, 33);
                    k2 *= c1_;
                    h2_ ^= k2;
                    h2_ = rotl64(h2_, 31);
                    h2_ += h1_;
                    h2_ = h2_ * 5 + 0x38495ab5;
                    tail_len_ = 0;
                }
            }
            int nblocks = len / 16;
            auto blocks = reinterpret_cast<const uint64_t*>(p);
            for (int i = 0; i < nblocks; ++i) {
                uint64_t k1 = blocks[i * 2 + 0];
                uint64_t k2 = blocks[i * 2 + 1];
                k1 *= c1_;
                k1 = rotl64(k1, 31);
                k1 *= c2_;
                h1_ ^= k1;
                h1_ = rotl64(h1_, 27);
                h1_ += h2_;
                h1_ = h1_ * 5 + 0x52dce729;
                k2 *= c2_;
                k2 = rotl64(k2, 33);
                k2 *= c1_;
                h2_ ^= k2;
                h2_ = rotl64(h2_, 31);
                h2_ += h1_;
                h2_ = h2_ * 5 + 0x38495ab5;
            }
            int rem = len & 15;
            if (rem > 0) {
                memcpy(tail_, p + nblocks * 16, rem);
                tail_len_ = rem;
            }
            total_len_ += len;
        }
        turbo::uint128 final() {
            uint64_t k1 = 0;
            uint64_t k2 = 0;
            auto tail = tail_;
            switch (tail_len_) {
            case 15:
                k2 ^= uint64_t(tail[14]) << 48;
            case 14:
                k2 ^= uint64_t(tail[13]) << 40;
            case 13:
                k2 ^= uint64_t(tail[12]) << 32;
            case 12:
                k2 ^= uint64_t(tail[11]) << 24;
            case 11:
                k2 ^= uint64_t(tail[10]) << 16;
            case 10:
                k2 ^= uint64_t(tail[9]) << 8;
            case 9:
                k2 ^= uint64_t(tail[8]) << 0;
                k2 *= c2_;
                k2 = rotl64(k2, 33);
                k2 *= c1_;
                h2_ ^= k2;
            case 8:
                k1 ^= uint64_t(tail[7]) << 56;
            case 7:
                k1 ^= uint64_t(tail[6]) << 48;
            case 6:
                k1 ^= uint64_t(tail[5]) << 40;
            case 5:
                k1 ^= uint64_t(tail[4]) << 32;
            case 4:
                k1 ^= uint64_t(tail[3]) << 24;
            case 3:
                k1 ^= uint64_t(tail[2]) << 16;
            case 2:
                k1 ^= uint64_t(tail[1]) << 8;
            case 1:
                k1 ^= uint64_t(tail[0]) << 0;
                k1 *= c1_;
                k1 = rotl64(k1, 31);
                k1 *= c2_;
                h1_ ^= k1;
            }
            h1_ ^= total_len_;
            h2_ ^= total_len_;
            h1_ += h2_;
            h2_ += h1_;
            h1_ = fmix64(h1_);
            h2_ = fmix64(h2_);
            h1_ += h2_;
            h2_ += h1_;
            turbo::uint128 out = turbo::MakeUint128(h1_,h2_);
            return out;
        }
        static turbo::uint128 hash(const void* data, size_t len, uint32_t seed = 0) {
            murmur3_128 h(seed);
            h.update(data, len);
            return h.final();
        }

    private:
        static uint64_t rotl64(uint64_t x, int8_t r) {
            return (x << r) | (x >> (64 - r));
        }
        static uint64_t fmix64(uint64_t k) {
            k ^= k >> 33;
            k *= 0xff51afd7ed558ccdull;
            k ^= k >> 33;
            k *= 0xc4ceb9fe1a85ec53ull;
            k ^= k >> 33;
            return k;
        }
        static const uint64_t c1_ = 0x87c37b91114253d5ull;
        static const uint64_t c2_ = 0x4cf5ad432745937full;
        uint64_t h1_;
        uint64_t h2_;
        uint64_t total_len_;
        int tail_len_;
        uint8_t tail_[16];
    };

}  // namespace turbo
