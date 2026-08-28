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

#include <turbo/bits/endian.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace turbo {


    struct AlreadyLittleTag{};
    template<typename T>
    class SipHashKeyBase {
    public:
        static_assert(std::is_same_v<T, uint64_t>|| std::is_same_v<T, uint32_t>, "must be uint64 or uint32");
    public:
        SipHashKeyBase(T k0,T k1)
            :_k0(turbo::little_endian::from_host(k0)),_k1(turbo::little_endian::from_host(k1)) {}

        SipHashKeyBase(T k0,T k1,AlreadyLittleTag tag)
        :_k0(k0),_k1(k1) {}

        T k0() const {
            return _k0;
        }
        T k1() const {
            return _k1;
        }

        T host_k0() const {
            return turbo::little_endian::to_host(_k0);
        }
        T host_k1() const {
            return turbo::little_endian::to_host(_k1);
        }
    private:
       T _k0;
       T _k1;
    };

    using SipHashKey = SipHashKeyBase<uint64_t>;

    using SipHashKey64 = SipHashKeyBase<uint64_t>;

    using SipHashKey32 = SipHashKeyBase<uint32_t>;

    ////////////////////////////////////////////////////////////////////////////////
    /// c apis
    uint64_t siphash64(const uint8_t* in, const size_t inlen,SipHashKey k);

    std::array<uint64_t, 2> siphash128(const uint8_t* in, const size_t inlen, SipHashKey k);

    uint32_t half_siphash32(const uint8_t* in, const size_t inlen,SipHashKey32 k);

    uint64_t half_siphash64(const uint8_t* in, const size_t inlen,SipHashKey32 k);

    class SipHasher {
    public:
        SipHasher(uint64_t k0,uint64_t k1) : _key(k0,k1){}
        SipHasher(uint64_t k0,uint64_t k1, AlreadyLittleTag tag) : _key(k0,k1, tag){}

        SipHashKey key() const {
            return _key;
        }

        uint64_t hash64(const uint8_t* in, const size_t inlen) {
            return siphash64(in, inlen, _key);
        }

         std::array<uint64_t, 2> hash128(const uint8_t* in, const size_t inlen) {
            return siphash128(in, inlen, _key);
        }

    private:
      SipHashKey _key;
    };

    class SipHasher32 {
    public:
        SipHasher32(uint32_t k0,uint32_t k1) : _key(k0,k1){}
        SipHasher32(uint32_t k0,uint32_t k1, AlreadyLittleTag tag) : _key(k0,k1, tag){}

        SipHashKey32 key() const {
            return _key;
        }

        uint32_t hash32(const uint8_t* in, const size_t inlen) {
            return half_siphash32(in, inlen, _key);
        }

        uint64_t hash64(const uint8_t* in, const size_t inlen) {
            return half_siphash64(in, inlen, _key);
        }

    private:
        SipHashKey32 _key;
    };

} // namespace turbo
