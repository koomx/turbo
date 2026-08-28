// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// This file is for Abseil internal use only.
// See //turbo/numeric/bits.h for supported functions related to endian-ness.

#ifndef TURBO_BASE_INTERNAL_ENDIAN_H_
#define TURBO_BASE_INTERNAL_ENDIAN_H_

#include <cstdint>
#include <cstdlib>

#include <turbo/base/nullability.h>
#include <turbo/bits/casts.h>
#include <turbo/bits/unaligned_access.h>
#include <turbo/macros/config.h>

namespace turbo {

    constexpr uint64_t gbswap_64(uint64_t x) {
#if KUMO_HAVE_BUILTIN(__builtin_bswap64) || defined(__GNUC__)
        return __builtin_bswap64(x);
#else
        return (((x & uint64_t { 0xFF }) << 56) | ((x & uint64_t { 0xFF00 }) << 40) | ((x & uint64_t { 0xFF0000 }) << 24) | ((x & uint64_t { 0xFF000000 }) << 8) | ((x & uint64_t { 0xFF00000000 }) >> 8) | ((x & uint64_t { 0xFF0000000000 }) >> 24) | ((x & uint64_t { 0xFF000000000000 }) >> 40) | ((x & uint64_t { 0xFF00000000000000 }) >> 56));
#endif
    }

    constexpr uint32_t gbswap_32(uint32_t x) {
#if KUMO_HAVE_BUILTIN(__builtin_bswap32) || defined(__GNUC__)
        return __builtin_bswap32(x);
#else
        return (((x & uint32_t { 0xFF }) << 24) | ((x & uint32_t { 0xFF00 }) << 8) | ((x & uint32_t { 0xFF0000 }) >> 8) | ((x & uint32_t { 0xFF000000 }) >> 24));
#endif
    }

    constexpr uint16_t gbswap_16(uint16_t x) {
#if KUMO_HAVE_BUILTIN(__builtin_bswap16) || defined(__GNUC__)
        return __builtin_bswap16(x);
#else
        return (((x & uint16_t { 0xFF }) << 8) | ((x & uint16_t { 0xFF00 }) >> 8));
#endif
    }

#if KUMO_ENDIAN_LITTLE

    // Portable definitions for htonl (host-to-network) and friends on little-endian
    // architectures.
    inline uint16_t ghtons(uint16_t x) {
        return gbswap_16(x);
    }
    inline uint32_t ghtonl(uint32_t x) {
        return gbswap_32(x);
    }
    inline uint64_t ghtonll(uint64_t x) {
        return gbswap_64(x);
    }

#elif KUMO_ENDIAN_BIG

    // Portable definitions for htonl (host-to-network) etc on big-endian
    // architectures. These definitions are simpler since the host byte order is the
    // same as network byte order.
    inline uint16_t ghtons(uint16_t x) {
        return x;
    }
    inline uint32_t ghtonl(uint32_t x) {
        return x;
    }
    inline uint64_t ghtonll(uint64_t x) {
        return x;
    }

#else
#error \
    "Unsupported byte order: Either KUMO_ENDIAN_BIG or " \
       "KUMO_ENDIAN_LITTLE must be defined"
#endif // byte order

    inline uint16_t gntohs(uint16_t x) {
        return ghtons(x);
    }
    inline uint32_t gntohl(uint32_t x) {
        return ghtonl(x);
    }
    inline uint64_t gntohll(uint64_t x) {
        return ghtonll(x);
    }

    // Utilities to convert numbers between the current hosts's native byte
    // order and little-endian byte order
    //
    // Load/Store methods are alignment safe
    namespace little_endian {
// Conversion functions.
#if KUMO_ENDIAN_LITTLE

        inline uint16_t from_host16(uint16_t x) {
            return x;
        }
        inline uint16_t to_host16(uint16_t x) {
            return x;
        }

        inline uint32_t from_host32(uint32_t x) {
            return x;
        }
        inline uint32_t to_host32(uint32_t x) {
            return x;
        }

        inline uint64_t from_host64(uint64_t x) {
            return x;
        }
        inline uint64_t to_host64(uint64_t x) {
            return x;
        }

        inline constexpr bool is_little_endian() {
            return true;
        }

#elif KUMO_ENDIAN_BIG

        inline uint16_t from_host16(uint16_t x) {
            return gbswap_16(x);
        }
        inline uint16_t to_host16(uint16_t x) {
            return gbswap_16(x);
        }

        inline uint32_t from_host32(uint32_t x) {
            return gbswap_32(x);
        }
        inline uint32_t to_host32(uint32_t x) {
            return gbswap_32(x);
        }

        inline uint64_t from_host64(uint64_t x) {
            return gbswap_64(x);
        }
        inline uint64_t to_host64(uint64_t x) {
            return gbswap_64(x);
        }

        inline constexpr bool is_little_endian() {
            return false;
        }

#endif /* ENDIAN */

        inline uint8_t from_host(uint8_t x) {
            return x;
        }
        inline uint16_t from_host(uint16_t x) {
            return from_host16(x);
        }
        inline uint32_t from_host(uint32_t x) {
            return from_host32(x);
        }
        inline uint64_t from_host(uint64_t x) {
            return from_host64(x);
        }
        inline uint8_t to_host(uint8_t x) {
            return x;
        }
        inline uint16_t to_host(uint16_t x) {
            return to_host16(x);
        }
        inline uint32_t to_host(uint32_t x) {
            return to_host32(x);
        }
        inline uint64_t to_host(uint64_t x) {
            return to_host64(x);
        }

        inline int8_t from_host(int8_t x) {
            return x;
        }
        inline int16_t from_host(int16_t x) {
            return bit_cast<int16_t>(from_host16(bit_cast<uint16_t>(x)));
        }
        inline int32_t from_host(int32_t x) {
            return bit_cast<int32_t>(from_host32(bit_cast<uint32_t>(x)));
        }
        inline int64_t from_host(int64_t x) {
            return bit_cast<int64_t>(from_host64(bit_cast<uint64_t>(x)));
        }
        inline int8_t to_host(int8_t x) {
            return x;
        }
        inline int16_t to_host(int16_t x) {
            return bit_cast<int16_t>(to_host16(bit_cast<uint16_t>(x)));
        }
        inline int32_t to_host(int32_t x) {
            return bit_cast<int32_t>(to_host32(bit_cast<uint32_t>(x)));
        }
        inline int64_t to_host(int64_t x) {
            return bit_cast<int64_t>(to_host64(bit_cast<uint64_t>(x)));
        }

        // Functions to do unaligned loads and stores in little-endian order.
        inline uint16_t Load16(const void* turbo_nonnull p) {
            return to_host16(TURBO_INTERNAL_UNALIGNED_LOAD16(p));
        }

        inline void Store16(void* turbo_nonnull p, uint16_t v) {
            TURBO_INTERNAL_UNALIGNED_STORE16(p, from_host16(v));
        }

        inline uint32_t Load32(const void* turbo_nonnull p) {
            return to_host32(TURBO_INTERNAL_UNALIGNED_LOAD32(p));
        }

        inline void Store32(void* turbo_nonnull p, uint32_t v) {
            TURBO_INTERNAL_UNALIGNED_STORE32(p, from_host32(v));
        }

        inline uint64_t Load64(const void* turbo_nonnull p) {
            return to_host64(TURBO_INTERNAL_UNALIGNED_LOAD64(p));
        }

        inline void Store64(void* turbo_nonnull p, uint64_t v) {
            TURBO_INTERNAL_UNALIGNED_STORE64(p, from_host64(v));
        }

    } // namespace little_endian

    // Utilities to convert numbers between the current hosts's native byte
    // order and big-endian byte order (same as network byte order)
    //
    // Load/Store methods are alignment safe
    namespace big_endian {
#if KUMO_ENDIAN_LITTLE

        inline uint16_t from_host16(uint16_t x) {
            return gbswap_16(x);
        }
        inline uint16_t to_host16(uint16_t x) {
            return gbswap_16(x);
        }

        inline uint32_t from_host32(uint32_t x) {
            return gbswap_32(x);
        }
        inline uint32_t to_host32(uint32_t x) {
            return gbswap_32(x);
        }

        inline uint64_t from_host64(uint64_t x) {
            return gbswap_64(x);
        }
        inline uint64_t to_host64(uint64_t x) {
            return gbswap_64(x);
        }

        inline constexpr bool is_little_endian() {
            return true;
        }

#elif KUMO_ENDIAN_BIG

        inline uint16_t from_host16(uint16_t x) {
            return x;
        }
        inline uint16_t to_host16(uint16_t x) {
            return x;
        }

        inline uint32_t from_host32(uint32_t x) {
            return x;
        }
        inline uint32_t to_host32(uint32_t x) {
            return x;
        }

        inline uint64_t from_host64(uint64_t x) {
            return x;
        }
        inline uint64_t to_host64(uint64_t x) {
            return x;
        }

        inline constexpr bool is_little_endian() {
            return false;
        }

#endif /* ENDIAN */

        inline uint8_t from_host(uint8_t x) {
            return x;
        }
        inline uint16_t from_host(uint16_t x) {
            return from_host16(x);
        }
        inline uint32_t from_host(uint32_t x) {
            return from_host32(x);
        }
        inline uint64_t from_host(uint64_t x) {
            return from_host64(x);
        }
        inline uint8_t to_host(uint8_t x) {
            return x;
        }
        inline uint16_t to_host(uint16_t x) {
            return to_host16(x);
        }
        inline uint32_t to_host(uint32_t x) {
            return to_host32(x);
        }
        inline uint64_t to_host(uint64_t x) {
            return to_host64(x);
        }

        inline int8_t from_host(int8_t x) {
            return x;
        }
        inline int16_t from_host(int16_t x) {
            return bit_cast<int16_t>(from_host16(bit_cast<uint16_t>(x)));
        }
        inline int32_t from_host(int32_t x) {
            return bit_cast<int32_t>(from_host32(bit_cast<uint32_t>(x)));
        }
        inline int64_t from_host(int64_t x) {
            return bit_cast<int64_t>(from_host64(bit_cast<uint64_t>(x)));
        }
        inline int8_t to_host(int8_t x) {
            return x;
        }
        inline int16_t to_host(int16_t x) {
            return bit_cast<int16_t>(to_host16(bit_cast<uint16_t>(x)));
        }
        inline int32_t to_host(int32_t x) {
            return bit_cast<int32_t>(to_host32(bit_cast<uint32_t>(x)));
        }
        inline int64_t to_host(int64_t x) {
            return bit_cast<int64_t>(to_host64(bit_cast<uint64_t>(x)));
        }

        // Functions to do unaligned loads and stores in big-endian order.
        inline uint16_t Load16(const void* turbo_nonnull p) {
            return to_host16(TURBO_INTERNAL_UNALIGNED_LOAD16(p));
        }

        inline void Store16(void* turbo_nonnull p, uint16_t v) {
            TURBO_INTERNAL_UNALIGNED_STORE16(p, from_host16(v));
        }

        inline uint32_t Load32(const void* turbo_nonnull p) {
            return to_host32(TURBO_INTERNAL_UNALIGNED_LOAD32(p));
        }

        inline void Store32(void* turbo_nonnull p, uint32_t v) {
            TURBO_INTERNAL_UNALIGNED_STORE32(p, from_host32(v));
        }

        inline uint64_t Load64(const void* turbo_nonnull p) {
            return to_host64(TURBO_INTERNAL_UNALIGNED_LOAD64(p));
        }

        inline void Store64(void* turbo_nonnull p, uint64_t v) {
            TURBO_INTERNAL_UNALIGNED_STORE64(p, from_host64(v));
        }

    } // namespace big_endian

} // namespace turbo

#endif // TURBO_BASE_INTERNAL_ENDIAN_H_
