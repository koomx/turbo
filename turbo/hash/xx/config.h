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

#include <turbo/macros/macros.h>
#include <turbo/bits/prefetch.h>

#ifndef XXHASH_STREAM_USE_STACK
#if !defined(__clang__)
#define XXHASH_STREAM_USE_STACK 1
#else
#define XXHASH_STREAM_USE_STACK 0
#endif
#endif


///////////////////////////////////////////////////////////
/// Controls the alignment of the accumulator,
/// for compatibility with aligned vector loads, which are usually faster.
///

namespace turbo::xxhash {

#ifdef __clang__
    static constexpr size_t kDefaultXxhPrefetchDist = 320;
#else
    static constexpr size_t kDefaultXxhPrefetchDist = 384;
#endif

    static constexpr size_t kXxh3SecretSizeMin = 136;
    /// minimum kXxh3SecretSizeMin
    static constexpr size_t kXxhSecretDefaultSize = 192;
    static constexpr size_t kXxh3InternalBufferSize = 256;
    static constexpr size_t kXxh3SecretDefaultSize = 192;
    static constexpr size_t kXxh3MidsizeMax = 240;
    static constexpr size_t kXxhSecretMergeAccsStart = 11;
    static constexpr size_t kXxhStripeLen = 64;
    /// nb of secret bytes consumed at each accumulation
    static constexpr size_t kXxhSecretConsumeRate = 8;
    static constexpr size_t kXxhAccNb = kXxhStripeLen / sizeof(uint64_t);

    static_assert(kXxhSecretDefaultSize >= kXxh3SecretSizeMin, "default keyset is not large enough");


    /// 0b10011110001101110111100110110001
    static constexpr uint32_t kXxhPrime32_1 = 0x9E3779B1U;
    /// 0b10000101111010111100101001110111
    static constexpr uint32_t kXxhPrime32_2 = 0x85EBCA77U;
    /// 0b11000010101100101010111000111101
    static constexpr uint32_t kXxhPrime32_3 = 0xC2B2AE3DU;
    /// 0b00100111110101001110101100101111
    static constexpr uint32_t kXxhPrime32_4 = 0x27D4EB2FU;
    /// 0b00010110010101100110011110110001
    static constexpr uint32_t kXxhPrime32_5 = 0x165667B1U;

    /// 0b1001111000110111011110011011000110000101111010111100101010000111
    static constexpr uint64_t kXxhPrime64_1 = 0x9E3779B185EBCA87ULL;
    /// 0b1100001010110010101011100011110100100111110101001110101101001111
    static constexpr uint64_t kXxhPrime64_2 = 0xC2B2AE3D27D4EB4FULL;
    /// 0b0001011001010110011001111011000110011110001101110111100111111001
    static constexpr uint64_t kXxhPrime64_3 = 0x165667B19E3779F9ULL;
    /// 0b1000010111101011110010100111011111000010101100101010111001100011
    static constexpr uint64_t kXxhPrime64_4 = 0x85EBCA77C2B2AE63ULL;
    /// 0b0010011111010100111010110010111100010110010101100110011111000101
    static constexpr uint64_t kXxhPrime64_5 = 0x27D4EB2F165667C5ULL;

    /// 0b0001011001010110011001111001000110011110001101110111100111111001
    static const uint64_t kXxhPrimeMX1 = 0x165667919E3779F9ULL;
    /// 0b1001111110110010000111000110010100011110100110001101111100100101
    static const uint64_t kXxhPrimeMX2 = 0x9FB21C651E98DF25ULL;

    /// not aligned on 8, last secret is different from acc & scrambler
    static constexpr size_t kXxhashSecretLastAccStart = 7;

    static size_t constexpr kXxhashMidSizeStartOffset = 3;
    static size_t constexpr kXxhashMidSizeLastOffset = 17;
} // namespace turbo::xxhash
