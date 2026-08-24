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
#include <turbo/hash/xxh/hash64.h>

namespace turbo {

    void XXH64_state_t::reset(uint64_t seed) {
        memset(this, 0, sizeof(XXH64_state_t));
        init_accs(seed);
    }

    void XXH64_state_t::update(KUMO_ATTRIBUTE_NOESCAPE const uint8_t* input, size_t len) {
        if (input == nullptr) {
            KUMO_DASSERT(len == 0);
            return;
        }

        total_len += len;

        KUMO_DASSERT(bufferedSize <= sizeof(buffer));
        if (len < sizeof(buffer) - bufferedSize) { /* fill in tmp buffer */
            memcpy(buffer + bufferedSize, input, len);
            bufferedSize += (uint32_t)len;
            return;
        }

        {
            const uint8_t* xinput = (const uint8_t*)input;
            const uint8_t* const bEnd = xinput + len;

            if (bufferedSize) { /* non-empty buffer => complete first */
                memcpy(buffer + bufferedSize, xinput, sizeof(buffer) - bufferedSize);
                xinput += sizeof(buffer) - bufferedSize;
                /* and process one round */
                (void)consume_long( buffer, sizeof(buffer));
                bufferedSize = 0;
            }

            KUMO_DASSERT(xinput <= bEnd);
            if ((size_t)(bEnd - xinput) >= sizeof(buffer)) {
                /* Process the remaining data */
                xinput = consume_long( xinput, (size_t)(bEnd - xinput));
            }

            if (xinput < bEnd) {
                /* Copy the leftover to the tmp buffer */
                memcpy(buffer, xinput, (size_t)(bEnd - xinput));
                bufferedSize = (unsigned)(bEnd - xinput);
            }
        }
    }

    KUMO_FORCE_INLINE const uint8_t*XXH64_state_t::consume_long(uint8_t const* KUMO_RESTRICT input,size_t len) {
        const uint8_t* const bEnd = input + len;
        const uint8_t* const limit = bEnd - 31;
        KUMO_DASSERT(input != nullptr);
        KUMO_DASSERT(len >= 32);
        do {
            /* reroll on 32-bit */
            if (sizeof(void*) < sizeof(uint64_t)) {
                size_t i;
                for (i = 0; i < 4; i++) {
                    acc[i] = acc_round(acc[i], little_endian::Load64(input));
                    input += 8;
                }
            } else {
                acc[0] = acc_round(acc[0], little_endian::Load64(input));
                input += 8;
                acc[1] = acc_round(acc[1], little_endian::Load64(input));
                input += 8;
                acc[2] = acc_round(acc[2], little_endian::Load64(input));
                input += 8;
                acc[3] = acc_round(acc[3], little_endian::Load64(input));
                input += 8;
            }
        } while (input < limit);

        return input;
    }

    KUMO_ATTRIBUTE_PURE_FUNCTION uint64_t
    XXH64_state_t::finalize(uint64_t hash, const uint8_t* ptr, size_t len) {
        if (ptr == nullptr)
            KUMO_DASSERT(len == 0);
        len &= 31;
        while (len >= 8) {
            uint64_t const k1 = acc_round(0, little_endian::Load64(ptr));
            ptr += 8;
            hash ^= k1;
            hash = turbo::rotl(hash, 27) * XXH_PRIME64_1 + XXH_PRIME64_4;
            len -= 8;
        }
        if (len >= 4) {
            hash ^= (uint64_t)(turbo::little_endian::Load32(ptr)) * XXH_PRIME64_1;
            ptr += 4;
            hash = turbo::rotl(hash, 23) * XXH_PRIME64_2 + XXH_PRIME64_3;
            len -= 4;
        }
        while (len > 0) {
            hash ^= (*ptr++) * XXH_PRIME64_5;
            hash = turbo::rotl(hash, 11) * XXH_PRIME64_1;
            --len;
        }
        return xxh64_avalanche(hash);
    }

#undef XXH_PROCESS1_64
#undef XXH_PROCESS4_64
#undef XXH_PROCESS8_64


    uint64_t XXH64_state_t::digest() {
        uint64_t h64;

        if (total_len >= 32) {
            h64 = merge_accs();
        } else {
            h64 = acc[2] /*seed*/ + XXH_PRIME64_5;
        }

        h64 += (uint64_t)total_len;

        return finalize(h64, buffer, (size_t)total_len);
    }


}  // namespace turbo
