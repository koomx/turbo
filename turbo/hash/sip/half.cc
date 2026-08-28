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

#include <stddef.h>
#include <stdint.h>


#include <turbo/hash/sip/siphash.h>
#include <turbo/hash/sip/config.h>
#include <turbo/bits/endian.h>

namespace turbo {

#define ROTL32(x, b) (uint32_t)(((x) << (b)) | ((x) >> (32 - (b))))

#define U32TO8_LE(p, v)            \
    (p)[0] = (uint8_t)((v));       \
    (p)[1] = (uint8_t)((v) >> 8);  \
    (p)[2] = (uint8_t)((v) >> 16); \
    (p)[3] = (uint8_t)((v) >> 24);

#define U8TO32_LE(p) \
    (((uint32_t)((p)[0])) | ((uint32_t)((p)[1]) << 8) | ((uint32_t)((p)[2]) << 16) | ((uint32_t)((p)[3]) << 24))

#define SIPROUND32           \
    do {                   \
        v0 += v1;          \
        v1 = ROTL32(v1, 5);  \
        v1 ^= v0;          \
        v0 = ROTL32(v0, 16); \
        v2 += v3;          \
        v3 = ROTL32(v3, 8);  \
        v3 ^= v2;          \
        v0 += v3;          \
        v3 = ROTL32(v3, 7);  \
        v3 ^= v0;          \
        v2 += v1;          \
        v1 = ROTL32(v1, 13); \
        v1 ^= v2;          \
        v2 = ROTL32(v2, 16); \
    } while (0)


   uint32_t half_siphash32(const uint8_t* in, const size_t inlen,SipHashKey32 k) {

        const unsigned char* ni = (const unsigned char*)in;

        uint32_t v0 = 0;
        uint32_t v1 = 0;
        uint32_t v2 = UINT32_C(0x6c796765);
        uint32_t v3 = UINT32_C(0x74656462);
        uint32_t k0 = k.k0();
        uint32_t k1 = k.k1();
        uint32_t m;
        int i;
        const unsigned char* end = ni + inlen - (inlen % sizeof(uint32_t));
        const int left = inlen & 3;
        uint32_t b = ((uint32_t)inlen) << 24;
        v3 ^= k1;
        v2 ^= k0;
        v1 ^= k1;
        v0 ^= k0;

        for (; ni != end; ni += 4) {
            m = U8TO32_LE(ni);
            v3 ^= m;

            TRACE;
            for (i = 0; i < cROUNDS; ++i)
                SIPROUND32;

            v0 ^= m;
        }

        switch (left) {
        case 3:
            b |= ((uint32_t)ni[2]) << 16;
            /// FALLTHRU
        case 2:
            b |= ((uint32_t)ni[1]) << 8;
            /// FALLTHRU
        case 1:
            b |= ((uint32_t)ni[0]);
            break;
        case 0:
            break;
        }

        v3 ^= b;

        TRACE;
        for (i = 0; i < cROUNDS; ++i)
            SIPROUND32;

        v0 ^= b;

        v2 ^= 0xff;

        TRACE;
        for (i = 0; i < dROUNDS; ++i)
            SIPROUND32;

        b = v1 ^ v3;
        return turbo::little_endian::from_host(b);
    }


    uint64_t half_siphash64(const uint8_t* in, const size_t inlen,SipHashKey32 k) {

        const unsigned char* ni = (const unsigned char*)in;

        uint32_t v0 = 0;
        uint32_t v1 = 0;
        uint32_t v2 = UINT32_C(0x6c796765);
        uint32_t v3 = UINT32_C(0x74656462);
        uint32_t k0 = k.k0();
        uint32_t k1 = k.k1();
        uint32_t m;
        int i;
        const unsigned char* end = ni + inlen - (inlen % sizeof(uint32_t));
        const int left = inlen & 3;
        uint32_t b = ((uint32_t)inlen) << 24;
        v3 ^= k1;
        v2 ^= k0;
        v1 ^= k1;
        v0 ^= k0;

        v1 ^= 0xee;

        for (; ni != end; ni += 4) {
            m = U8TO32_LE(ni);
            v3 ^= m;

            TRACE;
            for (i = 0; i < cROUNDS; ++i)
                SIPROUND32;

            v0 ^= m;
        }

        switch (left) {
        case 3:
            b |= ((uint32_t)ni[2]) << 16;
            /* FALLTHRU */
        case 2:
            b |= ((uint32_t)ni[1]) << 8;
            /* FALLTHRU */
        case 1:
            b |= ((uint32_t)ni[0]);
            break;
        case 0:
            break;
        }

        v3 ^= b;

        TRACE;
        for (i = 0; i < cROUNDS; ++i)
            SIPROUND32;

        v0 ^= b;

        v2 ^= 0xee;

        TRACE;
        for (i = 0; i < dROUNDS; ++i)
            SIPROUND32;

        uint32_t lo = v1 ^ v3;

        v1 ^= 0xdd;

        TRACE;
        for (i = 0; i < dROUNDS; ++i)
            SIPROUND32;

        uint32_t hi = v1 ^ v3;
        return static_cast<uint64_t>(lo) | (static_cast<uint64_t>(hi) << 32);
    }

} // namespace turbo
