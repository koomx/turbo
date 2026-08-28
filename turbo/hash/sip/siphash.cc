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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <turbo/hash/sip/siphash.h>
#include <turbo/hash/sip/config.h>
#include <turbo/bits/endian.h>

namespace turbo {

#define ROTL(x, b) (uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))

#define U32TO8_LE(p, v)            \
    (p)[0] = (uint8_t)((v));       \
    (p)[1] = (uint8_t)((v) >> 8);  \
    (p)[2] = (uint8_t)((v) >> 16); \
    (p)[3] = (uint8_t)((v) >> 24);

#define U64TO8_LE(p, v)              \
    U32TO8_LE((p), (uint32_t)((v))); \
    U32TO8_LE((p) + 4, (uint32_t)((v) >> 32));

#define U8TO64_LE(p) \
    (((uint64_t)((p)[0])) | ((uint64_t)((p)[1]) << 8) | ((uint64_t)((p)[2]) << 16) | ((uint64_t)((p)[3]) << 24) | ((uint64_t)((p)[4]) << 32) | ((uint64_t)((p)[5]) << 40) | ((uint64_t)((p)[6]) << 48) | ((uint64_t)((p)[7]) << 56))

#define SIPROUND64           \
    do {                   \
        v0 += v1;          \
        v1 = ROTL(v1, 13); \
        v1 ^= v0;          \
        v0 = ROTL(v0, 32); \
        v2 += v3;          \
        v3 = ROTL(v3, 16); \
        v3 ^= v2;          \
        v0 += v3;          \
        v3 = ROTL(v3, 21); \
        v3 ^= v0;          \
        v2 += v1;          \
        v1 = ROTL(v1, 17); \
        v1 ^= v2;          \
        v2 = ROTL(v2, 32); \
    } while (0)

    uint64_t siphash64(const uint8_t* in, const size_t inlen, SipHashKey k) {

        const unsigned char* ni = (const unsigned char*)in;

        assert((outlen == 8) || (outlen == 16));
        uint64_t v0 = UINT64_C(0x736f6d6570736575);
        uint64_t v1 = UINT64_C(0x646f72616e646f6d);
        uint64_t v2 = UINT64_C(0x6c7967656e657261);
        uint64_t v3 = UINT64_C(0x7465646279746573);
        uint64_t k0 = k.k0();
        uint64_t k1 = k.k1();
        uint64_t m;
        int i;
        const unsigned char* end = ni + inlen - (inlen % sizeof(uint64_t));
        const int left = inlen & 7;
        uint64_t b = ((uint64_t)inlen) << 56;
        v3 ^= k1;
        v2 ^= k0;
        v1 ^= k1;
        v0 ^= k0;

        for (; ni != end; ni += 8) {
            m = U8TO64_LE(ni);
            v3 ^= m;

            TRACE;
            for (i = 0; i < cROUNDS; ++i)
                SIPROUND64;

            v0 ^= m;
        }

        switch (left) {
        case 7:
            b |= ((uint64_t)ni[6]) << 48;
            /* FALLTHRU */
        case 6:
            b |= ((uint64_t)ni[5]) << 40;
            /* FALLTHRU */
        case 5:
            b |= ((uint64_t)ni[4]) << 32;
            /* FALLTHRU */
        case 4:
            b |= ((uint64_t)ni[3]) << 24;
            /* FALLTHRU */
        case 3:
            b |= ((uint64_t)ni[2]) << 16;
            /* FALLTHRU */
        case 2:
            b |= ((uint64_t)ni[1]) << 8;
            /* FALLTHRU */
        case 1:
            b |= ((uint64_t)ni[0]);
            break;
        case 0:
            break;
        }

        v3 ^= b;

        TRACE;
        for (i = 0; i < cROUNDS; ++i)
            SIPROUND64;

        v0 ^= b;
        v2 ^= 0xff;

        TRACE;
        for (i = 0; i < dROUNDS; ++i)
            SIPROUND64;

        b = v0 ^ v1 ^ v2 ^ v3;
        return  turbo::little_endian::from_host(b);
    }

    std::array<uint64_t, 2> siphash128(const uint8_t* in, const size_t inlen, SipHashKey k) {

        const unsigned char* ni = (const unsigned char*)in;

        assert((outlen == 8) || (outlen == 16));
        uint64_t v0 = UINT64_C(0x736f6d6570736575);
        uint64_t v1 = UINT64_C(0x646f72616e646f6d);
        uint64_t v2 = UINT64_C(0x6c7967656e657261);
        uint64_t v3 = UINT64_C(0x7465646279746573);
        uint64_t k0 = k.k0();
        uint64_t k1 = k.k1();
        uint64_t m;
        int i;
        const unsigned char* end = ni + inlen - (inlen % sizeof(uint64_t));
        const int left = inlen & 7;
        uint64_t b = ((uint64_t)inlen) << 56;
        v3 ^= k1;
        v2 ^= k0;
        v1 ^= k1;
        v0 ^= k0;

        v1 ^= 0xee;

        for (; ni != end; ni += 8) {
            m = U8TO64_LE(ni);
            v3 ^= m;

            TRACE;
            for (i = 0; i < cROUNDS; ++i)
                SIPROUND64;

            v0 ^= m;
        }

        switch (left) {
        case 7:
            b |= ((uint64_t)ni[6]) << 48;
            /* FALLTHRU */
        case 6:
            b |= ((uint64_t)ni[5]) << 40;
            /* FALLTHRU */
        case 5:
            b |= ((uint64_t)ni[4]) << 32;
            /* FALLTHRU */
        case 4:
            b |= ((uint64_t)ni[3]) << 24;
            /* FALLTHRU */
        case 3:
            b |= ((uint64_t)ni[2]) << 16;
            /* FALLTHRU */
        case 2:
            b |= ((uint64_t)ni[1]) << 8;
            /* FALLTHRU */
        case 1:
            b |= ((uint64_t)ni[0]);
            break;
        case 0:
            break;
        }

        v3 ^= b;

        TRACE;
        for (i = 0; i < cROUNDS; ++i)
            SIPROUND64;

        v0 ^= b;

        v2 ^= 0xee;


        TRACE;
        for (i = 0; i < dROUNDS; ++i)
            SIPROUND64;

        b = v0 ^ v1 ^ v2 ^ v3;
        auto r0 = turbo::little_endian::from_host(b);
        v1 ^= 0xdd;

        TRACE;
        for (i = 0; i < dROUNDS; ++i)
            SIPROUND64;

        b = v0 ^ v1 ^ v2 ^ v3;
        auto r1 =  turbo::little_endian::from_host(b);
        return {r0, r1};
    }
} // namespace turbo
