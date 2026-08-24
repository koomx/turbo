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

#include <cstdlib>
#include <turbo/hash/xxh/hash32.h>

namespace turbo {

    void XXH32_state_t::reset(uint32_t seed) {
        ::memset(this, 0, sizeof(*this));
        init_accs(seed);
    }

    void XXH32_state_t::update(const uint8_t* input, size_t len) {
        if (input == nullptr) {
            KUMO_DASSERT(len == 0);
            return;
        }

        total_len_32 += (uint32_t)len;
        large_len |= (uint32_t)((len >= 16) | (total_len_32 >= 16));

        KUMO_DASSERT(bufferedSize < sizeof(buffer));
        if (len < sizeof(buffer) - bufferedSize) { /* fill in tmp buffer */
            memcpy(buffer + bufferedSize, input, len);
            bufferedSize += (uint32_t)len;
            return;
        }

        {
            const uint8_t* xinput = (const uint8_t*)input;
            const uint8_t* const bEnd = xinput + len;

            if (bufferedSize) { /* non-empty buffer: complete first */
                memcpy(buffer + bufferedSize, xinput, sizeof(buffer) - bufferedSize);
                xinput += sizeof(buffer) - bufferedSize;
                /* then process one round */
                (void)consume_long(buffer, sizeof(buffer));
                bufferedSize = 0;
            }

            KUMO_DASSERT(xinput <= bEnd);
            if ((size_t)(bEnd - xinput) >= sizeof(buffer)) {
                /// Process the remaining data
                xinput = consume_long(xinput, (size_t)(bEnd - xinput));
            }

            if (xinput < bEnd) {
                /* Copy the leftover to the tmp buffer */
                memcpy(buffer, xinput, (size_t)(bEnd - xinput));
                bufferedSize = (unsigned)(bEnd - xinput);
            }
        }
    }

    uint32_t XXH32_state_t::digest() {
        uint32_t h32;

        if (large_len) {
            h32 = merge_accs();
        } else {
            h32 = acc[2] + XXH_PRIME32_5;
        }

        h32 += total_len_32;

        return finalize(h32, buffer, bufferedSize);
    }

    KUMO_ATTRIBUTE_PURE_FUNCTION uint32_t XXH32_state_t::merge_accs() const {
        return turbo::rotl(acc[0], 1) + turbo::rotl(acc[1], 7)
            + turbo::rotl(acc[2], 12) + turbo::rotl(acc[3], 18);
    }

    KUMO_ATTRIBUTE_PURE_FUNCTION uint32_t
    XXH32_state_t::finalize(uint32_t hash, const uint8_t* ptr, size_t len) {
#define XXH_PROCESS1                                  \
    do {                                              \
        hash += (*ptr++) * XXH_PRIME32_5;             \
        hash = turbo::rotl(hash, 11) * XXH_PRIME32_1; \
    } while (0)

#define XXH_PROCESS4                                               \
    do {                                                           \
        hash += turbo::little_endian::Load32(ptr) * XXH_PRIME32_3; \
        ptr += 4;                                                  \
        hash = turbo::rotl(hash, 17) * XXH_PRIME32_4;              \
    } while (0)

        KUMO_DASSERT(ptr == nullptr ? len == 0 : true);

        switch (len & 15) /* or switch(bEnd - p) */ {
        case 12:
            XXH_PROCESS4;
            KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
        case 8:
            XXH_PROCESS4;
            KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
        case 4:
            XXH_PROCESS4;
            return avalanche(hash);

        case 13:
            XXH_PROCESS4;
            KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
        case 9:
            XXH_PROCESS4;
            KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
        case 5:
            XXH_PROCESS4;
            XXH_PROCESS1;
            return avalanche(hash);

        case 14:
            XXH_PROCESS4;
            KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
        case 10:
            XXH_PROCESS4;
            KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
        case 6:
            XXH_PROCESS4;
            XXH_PROCESS1;
            XXH_PROCESS1;
            return avalanche(hash);

        case 15:
            XXH_PROCESS4;
            KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
        case 11:
            XXH_PROCESS4;
            KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
        case 7:
            XXH_PROCESS4;
            KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
        case 3:
            XXH_PROCESS1;
            KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
        case 2:
            XXH_PROCESS1;
            KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
        case 1:
            XXH_PROCESS1;
            KUMO_FALLTHROUGH_INTENDED; /* fallthrough */
        case 0:
            return avalanche(hash);
        }
        KUMO_UNREACHABLE();
    }

#undef XXH_PROCESS1
#undef XXH_PROCESS4

} // namespace turbo
