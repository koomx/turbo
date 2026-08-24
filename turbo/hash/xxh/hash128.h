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
#include <turbo/hash/xxh/scalar.h>
#include <turbo/hash/xxh/types.h>

namespace turbo {

    /// @internal
    /// @brief Structure for XXH3 streaming API.
    ///
    /// @note This is only defined when @ref XXH_STATIC_LINKING_ONLY,
    /// @ref XXH_INLINE_ALL, or @ref XXH_IMPLEMENTATION is defined.
    /// Otherwise it is an opaque type.
    /// Never use this definition in combination with dynamic library.
    /// This allows fields to safely be changed in the future.
    ///
    /// @note ** This structure has a strict alignment requirement of 64 bytes!! **
    /// Do not allocate this with `malloc()` or `new`,
    /// it will not be sufficiently aligned.
    /// Use @ref XXH3_createState() and @ref XXH3_freeState(), or stack allocation.
    ///
    /// Typedef'd to @ref XXH3_state_t.
    /// Do never access the members of this struct directly.
    ///
    /// @see XXH3_INITSTATE() for stack initialization.
    /// @see XXH3_createState(), XXH3_freeState().
    /// @see XXH32_state_s, XXH64_state_s
    struct XXH3_state_t {
        /// The 8 accumulators. See @ref XXH32_state_s::acc and @ref XXH64_state_s::acc
        KUMO_ALIGN_MEMBER(64, uint64_t acc[8]);

        /// Used to store a custom secret generated from a seed.
        KUMO_ALIGN_MEMBER(64, unsigned char customSecret[XXH3_SECRET_DEFAULT_SIZE]);

        /// The internal buffer. @see XXH32_state_s::mem32
        KUMO_ALIGN_MEMBER(64, unsigned char buffer[XXH3_INTERNALBUFFER_SIZE]);

        /// The amount of memory in @ref buffer, @see XXH32_state_s::memsize
        uint32_t bufferedSize;

        /// Reserved field. Needed for padding on 64-bit.
        uint32_t useSeed;

        /// Number or stripes processed.
        size_t nbStripesSoFar;

        /// Total length hashed. 64-bit even on 32-bit targets.
        uint64_t totalLen;

        /// Number of stripes per block.
        size_t nbStripesPerBlock;

        /// Size of @ref customSecret or @ref extSecret
        size_t secretLimit;

        /// Seed for _withSeed variants. Must be zero otherwise, @see XXH3_INITSTATE()
        uint64_t seed;

        /// Reserved field.
        uint64_t reserved64;

        /// Reference to an external secret for the _withSecret variants, NULL
        /// for other variants.
        const unsigned char* extSecret;
        /// note: there may be some padding at the end due to alignment on 64 bytes
    };

    typedef uint64_t (*xxh3_hashLong64_func)(const void* KUMO_RESTRICT, size_t, uint64_t, const uint8_t* KUMO_RESTRICT, size_t);

    typedef void (*XXH3_f_accumulate)(uint64_t* KUMO_RESTRICT, const uint8_t* KUMO_RESTRICT, const uint8_t* KUMO_RESTRICT, size_t);
    typedef void (*XXH3_f_scrambleAcc)(void* KUMO_RESTRICT, const void*);
    typedef void (*XXH3_f_initCustomSecret)(void* KUMO_RESTRICT, uint64_t);

    /// @}
    /// ************************************************************************
    /// @defgroup XXH3_family XXH3 family
    /// @ingroup public
    /// @{
    ///
    /// XXH3 is a more recent hash algorithm featuring:
    ///  - Improved speed for both small and large inputs
    ///  - True 64-bit and 128-bit outputs
    ///  - SIMD acceleration
    ///  - Improved 32-bit viability
    ///
    /// Speed analysis methodology is explained here:
    ///
    ///    https://fastcompression.blogspot.com/2019/03/presenting-xxh3.html
    ///
    /// Compared to XXH64, expect XXH3 to run approximately
    /// ~2x faster on large inputs and >3x faster on small ones,
    /// exact differences vary depending on platform.
    ///
    /// XXH3's speed benefits greatly from SIMD and 64-bit arithmetic,
    /// but does not require it.
    /// Most 32-bit and 64-bit targets that can run XXH32 smoothly can run XXH3
    /// at competitive speeds, even without vector support. Further details are
    /// explained in the implementation.
    ///
    /// XXH3 has a fast scalar implementation, but it also includes accelerated SIMD
    /// implementations for many common platforms:
    ///   - AVX512
    ///   - AVX2
    ///   - SSE2
    ///   - ARM NEON
    ///   - WebAssembly SIMD128
    ///   - POWER8 VSX
    ///   - s390x ZVector
    /// This can be controlled via the @ref XXH_VECTOR macro, but it automatically
    /// selects the best version according to predefined macros. For the x86 family, an
    /// automatic runtime dispatcher is included separately in @ref xxh_x86dispatch.c.
    ///
    /// XXH3 implementation is portable:
    /// it has a generic C90 formulation that can be compiled on any platform,
    /// all implementations generate exactly the same hash value on all platforms.
    /// Starting from v0.8.0, it's also labelled "stable", meaning that
    /// any future version will also generate the same hash value.
    ///
    /// XXH3 offers 2 variants, _64bits and _128bits.
    ///
    /// When only 64 bits are needed, prefer invoking the _64bits variant, as it
    /// reduces the amount of mixing, resulting in faster speed on small inputs.
    /// It's also generally simpler to manipulate a scalar return type than a struct.
    ///
    /// The API supports one-shot hashing, streaming mode, and custom secrets.

    /*!
     * @brief Calculates 64-bit unseeded variant of XXH3 hash of @p input.
     *
     * @param input  The block of data to be hashed, at least @p length bytes in size.
     * @param length The length of @p input, in bytes.
     *
     * @pre
     *   The memory between @p input and @p input + @p length must be valid,
     *   readable, contiguous memory. However, if @p length is `0`, @p input may be
     *   `NULL`. In C++, this also must be *TriviallyCopyable*.
     *
     * @return The calculated 64-bit XXH3 hash value.
     *
     * @note
     *   This is equivalent to @ref XXH3_64bits_withSeed() with a seed of `0`, however
     *   it may have slightly better performance due to constant propagation of the
     *   defaults.
     *
     * @see
     *    XXH3_64bits_withSeed(), XXH3_64bits_withSecret(): other seeding variants
     * @see @ref single_shot_example "Single Shot Example" for an example.
     */
    KUMO_DLL KUMO_ATTRIBUTE_PURE_FUNCTION uint64_t XXH3_64bits(KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t length);

    /*!
     * @brief Calculates 64-bit seeded variant of XXH3 hash of @p input.
     *
     * @param input  The block of data to be hashed, at least @p length bytes in size.
     * @param length The length of @p input, in bytes.
     * @param seed   The 64-bit seed to alter the hash result predictably.
     *
     * @pre
     *   The memory between @p input and @p input + @p length must be valid,
     *   readable, contiguous memory. However, if @p length is `0`, @p input may be
     *   `NULL`. In C++, this also must be *TriviallyCopyable*.
     *
     * @return The calculated 64-bit XXH3 hash value.
     *
     * @note
     *    seed == 0 produces the same results as @ref XXH3_64bits().
     *
     * This variant generates a custom secret on the fly based on default secret
     * altered using the @p seed value.
     *
     * While this operation is decently fast, note that it's not completely free.
     *
     * @see @ref single_shot_example "Single Shot Example" for an example.
     */
    KUMO_DLL KUMO_ATTRIBUTE_PURE_FUNCTION uint64_t XXH3_64bits_withSeed(KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t length, uint64_t seed);

    /*!
     * @brief Calculates 64-bit variant of XXH3 with a custom "secret".
     *
     * @param data       The block of data to be hashed, at least @p len bytes in size.
     * @param len        The length of @p data, in bytes.
     * @param secret     The secret data.
     * @param secretSize The length of @p secret, in bytes.
     *
     * @return The calculated 64-bit XXH3 hash value.
     *
     * @pre
     *   The memory between @p data and @p data + @p len must be valid,
     *   readable, contiguous memory. However, if @p length is `0`, @p data may be
     *   `NULL`. In C++, this also must be *TriviallyCopyable*.
     *
     * It's possible to provide any blob of bytes as a "secret" to generate the hash.
     * This makes it more difficult for an external actor to prepare an intentional collision.
     * The main condition is that @p secretSize *must* be large enough (>= @ref XXH3_SECRET_SIZE_MIN).
     * However, the quality of the secret impacts the dispersion of the hash algorithm.
     * Therefore, the secret _must_ look like a bunch of random bytes.
     * Avoid "trivial" or structured data such as repeated sequences or a text document.
     * Whenever in doubt about the "randomness" of the blob of bytes,
     * consider employing @ref XXH3_generateSecret() instead (see below).
     * It will generate a proper high entropy secret derived from the blob of bytes.
     * Another advantage of using XXH3_generateSecret() is that
     * it guarantees that all bits within the initial blob of bytes
     * will impact every bit of the output.
     * This is not necessarily the case when using the blob of bytes directly
     * because, when hashing _small_ inputs, only a portion of the secret is employed.
     *
     * @see @ref single_shot_example "Single Shot Example" for an example.
     */
    KUMO_DLL KUMO_ATTRIBUTE_PURE_FUNCTION uint64_t XXH3_64bits_withSecret(KUMO_ATTRIBUTE_NOESCAPE const void* data, size_t len, KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize);

    /*!
     * @brief Calculates 128-bit unseeded variant of XXH3 of @p data.
     *
     * @param data The block of data to be hashed, at least @p length bytes in size.
     * @param len  The length of @p data, in bytes.
     *
     * @return The calculated 128-bit variant of XXH3 value.
     *
     * The 128-bit variant of XXH3 has more strength, but it has a bit of overhead
     * for shorter inputs.
     *
     * This is equivalent to @ref XXH3_128bits_withSeed() with a seed of `0`, however
     * it may have slightly better performance due to constant propagation of the
     * defaults.
     *
     * @see XXH3_128bits_withSeed(), XXH3_128bits_withSecret(): other seeding variants
     * @see @ref single_shot_example "Single Shot Example" for an example.
     */
    KUMO_DLL KUMO_ATTRIBUTE_PURE_FUNCTION XXH128_hash_t XXH3_128bits(KUMO_ATTRIBUTE_NOESCAPE const void* data, size_t len);
    /*! @brief Calculates 128-bit seeded variant of XXH3 hash of @p data.
     *
     * @param data The block of data to be hashed, at least @p length bytes in size.
     * @param len  The length of @p data, in bytes.
     * @param seed The 64-bit seed to alter the hash result predictably.
     *
     * @return The calculated 128-bit variant of XXH3 value.
     *
     * @note
     *    seed == 0 produces the same results as @ref XXH3_64bits().
     *
     * This variant generates a custom secret on the fly based on default secret
     * altered using the @p seed value.
     *
     * While this operation is decently fast, note that it's not completely free.
     *
     * @see XXH3_128bits(), XXH3_128bits_withSecret(): other seeding variants
     * @see @ref single_shot_example "Single Shot Example" for an example.
     */
    KUMO_DLL KUMO_ATTRIBUTE_PURE_FUNCTION XXH128_hash_t XXH3_128bits_withSeed(KUMO_ATTRIBUTE_NOESCAPE const void* data, size_t len, uint64_t seed);
    /*!
     * @brief Calculates 128-bit variant of XXH3 with a custom "secret".
     *
     * @param data       The block of data to be hashed, at least @p len bytes in size.
     * @param len        The length of @p data, in bytes.
     * @param secret     The secret data.
     * @param secretSize The length of @p secret, in bytes.
     *
     * @return The calculated 128-bit variant of XXH3 value.
     *
     * It's possible to provide any blob of bytes as a "secret" to generate the hash.
     * This makes it more difficult for an external actor to prepare an intentional collision.
     * The main condition is that @p secretSize *must* be large enough (>= @ref XXH3_SECRET_SIZE_MIN).
     * However, the quality of the secret impacts the dispersion of the hash algorithm.
     * Therefore, the secret _must_ look like a bunch of random bytes.
     * Avoid "trivial" or structured data such as repeated sequences or a text document.
     * Whenever in doubt about the "randomness" of the blob of bytes,
     * consider employing @ref XXH3_generateSecret() instead (see below).
     * It will generate a proper high entropy secret derived from the blob of bytes.
     * Another advantage of using XXH3_generateSecret() is that
     * it guarantees that all bits within the initial blob of bytes
     * will impact every bit of the output.
     * This is not necessarily the case when using the blob of bytes directly
     * because, when hashing _small_ inputs, only a portion of the secret is employed.
     *
     * @see @ref single_shot_example "Single Shot Example" for an example.
     */
    KUMO_DLL KUMO_ATTRIBUTE_PURE_FUNCTION XXH128_hash_t XXH3_128bits_withSecret(KUMO_ATTRIBUTE_NOESCAPE const void* data, size_t len, KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize);

} // namespace turbo
