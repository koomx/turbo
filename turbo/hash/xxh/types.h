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

#include <turbo/hash/xxh/config.h>

namespace turbo {


    /// @brief Exit code for the streaming API.
    enum XXH_errorcode{
        /// OK
        XXH_OK = 0,
        /// Error
        XXH_ERROR
    };

    ////////////////////////////////////////////////////////////////////////////
    /// XXH3 32-bit variant
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Canonical (big endian) representation of @ref uint32_t.
    struct XXH32_canonical_t {
        /// Hash bytes, big endian
        unsigned char digest[4];
    };


    /// @brief Converts an @ref uint32_t to a big endian @ref XXH32_canonical_t.
    ///
    /// @param dst  The @ref XXH32_canonical_t pointer to be stored to.
    /// @param hash The @ref uint32_t to be converted.
    ///
    /// @pre
    ///   @p dst must not be `NULL`.
    ///
    /// @see @ref canonical_representation_example "Canonical Representation Example"
    KUMO_DLL void XXH32_canonicalFromHash(XXH32_canonical_t* dst, uint32_t hash);

    /// @brief Converts an @ref XXH32_canonical_t to a native @ref uint32_t.
    ///
    /// @param src The @ref XXH32_canonical_t to convert.
    ///
    /// @pre
    ///   @p src must not be `NULL`.
    ///
    /// @return The converted hash.
    ///
    /// @see @ref canonical_representation_example "Canonical Representation Example"
    KUMO_DLL KUMO_ATTRIBUTE_PURE_FUNCTION uint32_t XXH32_hashFromCanonical(const XXH32_canonical_t* src);


    ////////////////////////////////////////////////////////////////////////////
    /// XXH3 64-bit variant
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Canonical (big endian) representation of @ref uint64_t.
    struct XXH64_canonical_t {
        unsigned char digest[sizeof(uint64_t)];
    };


    /// @brief Converts an @ref uint64_t to a big endian @ref XXH64_canonical_t.
    ///
    /// @param dst The @ref XXH64_canonical_t pointer to be stored to.
    /// @param hash The @ref uint64_t to be converted.
    ///
    /// @pre
    ///   @p dst must not be `NULL`.
    ///
    /// @see @ref canonical_representation_example "Canonical Representation Example"
    KUMO_DLL void XXH64_canonicalFromHash(KUMO_ATTRIBUTE_NOESCAPE XXH64_canonical_t* dst, uint64_t hash);


    /// @brief Converts an @ref XXH64_canonical_t to a native @ref uint64_t.
    ///
    /// @param src The @ref XXH64_canonical_t to convert.
    ///
    /// @pre
    ///   @p src must not be `NULL`.
    ///
    /// @return The converted hash.
    ///
    /// @see @ref canonical_representation_example "Canonical Representation Example"
    KUMO_DLL KUMO_ATTRIBUTE_PURE_FUNCTION uint64_t XXH64_hashFromCanonical(KUMO_ATTRIBUTE_NOESCAPE const XXH64_canonical_t* src);

    ////////////////////////////////////////////////////////////////////////////
    /// XXH3 128-bit variant
    ////////////////////////////////////////////////////////////////////////////

    /// @brief The return value from 128-bit hashes.
    ///
    /// Stored in little endian order, although the fields themselves are in native
    /// endianness.
    struct XXH128_hash_t {
        /// `value & 0xFFFFFFFFFFFFFFFF`
        uint64_t low64;
        ///< `value >> 64`
        uint64_t high64;

        /// @brief Check equality of two XXH128_hash_t values
        ///
        /// @param rhs The 128-bit hash value.
        ///
        /// @return `true` if `this` and `rhs` are equal.
        /// @return `false` if they are not.
        bool operator==(XXH128_hash_t rhs) const;

        /// @brief Compares two @ref XXH128_hash_t
        ///
        /// This comparator is compatible with stdlib's `qsort()`/`bsearch()`.
        ///
        /// @param rhs Right-hand side value
        ///
        /// @return >0 if @p this  > @p rhs
        /// @return =0 if @p this == @p rhs
        /// @return <0 if @p this  < @p rhs
        int compare(XXH128_hash_t rhs) const;
    } ;

    struct XXH128_canonical_t{
        unsigned char digest[sizeof(XXH128_hash_t)];
    };

    /// @brief Compares two @ref XXH128_hash_t
    ///
    /// This comparator is compatible with stdlib's `qsort()`/`bsearch()`.
    ///
    /// @param h128_1 Left-hand side value
    /// @param h128_2 Right-hand side value
    ///
    /// @return >0 if @p h128_1  > @p h128_2
    /// @return =0 if @p h128_1 == @p h128_2
    /// @return <0 if @p h128_1  < @p h128_2
    KUMO_DLL KUMO_ATTRIBUTE_PURE_FUNCTION int XXH128_cmp(KUMO_ATTRIBUTE_NOESCAPE const void* h128_1, KUMO_ATTRIBUTE_NOESCAPE const void* h128_2);

    /// @brief Converts an @ref XXH128_hash_t to a big endian @ref XXH128_canonical_t.
    ///
    /// @param dst  The @ref XXH128_canonical_t pointer to be stored to.
    /// @param hash The @ref XXH128_hash_t to be converted.
    ///
    /// @pre
    ///   @p dst must not be `NULL`.
    ///
    /// @see @ref canonical_representation_example "Canonical Representation Example"
    KUMO_DLL void XXH128_canonicalFromHash(KUMO_ATTRIBUTE_NOESCAPE XXH128_canonical_t* dst, XXH128_hash_t hash);


    /// @brief Converts an @ref XXH128_canonical_t to a native @ref XXH128_hash_t.
    ///
    /// @param src The @ref XXH128_canonical_t to convert.
    ///
    /// @pre
    ///   @p src must not be `NULL`.
    ///
    /// @return The converted hash.
    ///
    /// @see @ref canonical_representation_example "Canonical Representation Example"
    KUMO_DLL KUMO_ATTRIBUTE_PURE_FUNCTION XXH128_hash_t XXH128_hashFromCanonical(KUMO_ATTRIBUTE_NOESCAPE const XXH128_canonical_t* src);


}  // namespace turbo
