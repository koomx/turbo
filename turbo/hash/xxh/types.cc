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

#include <turbo/hash/xxh/types.h>
#include <turbo/bits/bits.h>

namespace turbo {

    KUMO_DLL void XXH32_canonicalFromHash(XXH32_canonical_t* dst, uint32_t hash) {
        static_assert(sizeof(XXH32_canonical_t) == sizeof(uint32_t), "(sizeof(XXH32_canonical_t) == sizeof(uint32_t)");
        if constexpr (KUMO_ENDIAN_LITTLE)
            hash = turbo::byteswap(hash);
        memcpy(dst, &hash, sizeof(*dst));
    }

    KUMO_DLL uint32_t XXH32_hashFromCanonical(const XXH32_canonical_t* src) {
        return turbo::big_endian::Load32(src);
    }

    KUMO_DLL void XXH64_canonicalFromHash(KUMO_ATTRIBUTE_NOESCAPE XXH64_canonical_t* dst, uint64_t hash) {
        static_assert(sizeof(XXH64_canonical_t) == sizeof(uint64_t), "sizeof(XXH64_canonical_t) == sizeof(uint64_t)");
        if (KUMO_ENDIAN_LITTLE)
            hash = turbo::byteswap(hash);
        memcpy(dst, &hash, sizeof(*dst));
    }

    KUMO_DLL uint64_t XXH64_hashFromCanonical(KUMO_ATTRIBUTE_NOESCAPE const XXH64_canonical_t* src) {
        return turbo::big_endian::Load64(src);
    }

    bool XXH128_hash_t::operator==(XXH128_hash_t rhs) const {
        return !(memcmp(this, &rhs, sizeof(rhs)));
    }

    int XXH128_hash_t::compare(XXH128_hash_t rhs) const {
        int const hcmp = (high64 > rhs.high64) - (rhs.high64 > high64);
        /// note : bets that, in most cases, hash values are different
        if (hcmp)
            return hcmp;
        return (low64 > rhs.low64) - (rhs.low64 > low64);
    }

    KUMO_DLL int XXH128_cmp(KUMO_ATTRIBUTE_NOESCAPE const void* h128_1, KUMO_ATTRIBUTE_NOESCAPE const void* h128_2) {
        XXH128_hash_t const h1 = *(const XXH128_hash_t*)h128_1;
        XXH128_hash_t const h2 = *(const XXH128_hash_t*)h128_2;
        int const hcmp = (h1.high64 > h2.high64) - (h2.high64 > h1.high64);
        /* note : bets that, in most cases, hash values are different */
        if (hcmp)
            return hcmp;
        return (h1.low64 > h2.low64) - (h2.low64 > h1.low64);
    }


    KUMO_DLL void
  XXH128_canonicalFromHash(KUMO_ATTRIBUTE_NOESCAPE XXH128_canonical_t* dst, XXH128_hash_t hash) {
        static_assert(sizeof(XXH128_canonical_t) == sizeof(XXH128_hash_t), "sizeof(XXH128_canonical_t) == sizeof(XXH128_hash_t)");
        if constexpr (KUMO_ENDIAN_LITTLE) {
            hash.high64 = turbo::byteswap(hash.high64);
            hash.low64 = turbo::byteswap(hash.low64);
        }
        memcpy(dst, &hash.high64, sizeof(hash.high64));
        memcpy((char*)dst + sizeof(hash.high64), &hash.low64, sizeof(hash.low64));
    }

    KUMO_DLL XXH128_hash_t
    XXH128_hashFromCanonical(KUMO_ATTRIBUTE_NOESCAPE const XXH128_canonical_t* src) {
        XXH128_hash_t h;
        h.high64 = turbo::big_endian::Load64(src);
        h.low64 = turbo::big_endian::Load64(src->digest + 8);
        return h;
    }

}  // namespace turbo
