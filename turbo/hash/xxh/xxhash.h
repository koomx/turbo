
#include <turbo/bits/bits.h>
#include <turbo/hash/xxh/config.h>
#include <turbo/hash/xxh/types.h>
#include <turbo/hash/xxh/scalar.h>
#include <turbo/hash/xxh/hash32.h>
#include <turbo/hash/xxh/hash64.h>
#include <turbo/hash/xxh/hash128.h>
#include <turbo/hash/xxh/engine/scalar.h>
#include <turbo/hash/xxh/engine/arm64.h>

namespace turbo {

#define XXH_CAT(A, B) A##B
#define XXH_NAME2(A, B) XXH_CAT(A, B)
#define XXH_IPREF(Id) XXH_NAME2(XXH_NAMESPACE, Id)

#if (defined(XXH_INLINE_ALL) || defined(XXH_PRIVATE_API)) \
    && !defined(XXH_INLINE_ALL_31684351384)
    /* this section should be traversed only once */
#define XXH_INLINE_ALL_31684351384
    /* give access to the advanced API, required to compile implementations */
#undef XXH_STATIC_LINKING_ONLY /* avoid macro redef */
#define XXH_STATIC_LINKING_ONLY
    /* make all functions private */
#undef XXH3_64bits
#undef XXH3_64bits_withSecret
#undef XXH3_64bits_withSeed
#undef XXH3_64bits_withSecretandSeed
#undef XXH3_createState
#undef XXH3_freeState
#undef XXH3_copyState
#undef XXH3_64bits_reset
#undef XXH3_64bits_reset_withSeed
#undef XXH3_64bits_reset_withSecret
#undef XXH3_64bits_update
#undef XXH3_64bits_digest
#undef XXH3_generateSecret
    /* XXH3_128bits */
#undef XXH128
#undef XXH3_128bits
#undef XXH3_128bits_withSeed
#undef XXH3_128bits_withSecret
#undef XXH3_128bits_reset
#undef XXH3_128bits_reset_withSeed
#undef XXH3_128bits_reset_withSecret
#undef XXH3_128bits_reset_withSecretandSeed
#undef XXH3_128bits_update
#undef XXH3_128bits_digest
#undef XXH_NAMESPACE

    /* employ the namespace for XXH_INLINE_ALL */
#define XXH_NAMESPACE XXH_INLINE_
    /*
     * Some identifiers (enums, type names) are not symbols,
     * but they must nonetheless be renamed to avoid redeclaration.
     * Alternative solution: do not redeclare them.
     * However, this requires some #ifdefs, and has a more dispersed impact.
     * Meanwhile, renaming can be achieved in a single place.
     */
#define XXH_OK XXH_IPREF(XXH_OK)
#define XXH_ERROR XXH_IPREF(XXH_ERROR)
#define XXH_errorcode XXH_IPREF(XXH_errorcode)
#define XXH3_state_t XXH_IPREF(XXH3_state_t)
#undef XXHASH_H_5627135585666179
#undef XXHASH_H_STATIC_13879238742
#endif /* XXH_INLINE_ALL || XXH_PRIVATE_API */

    /* ****************************************************************
     *  Stable API
     *****************************************************************/
#ifndef XXHASH_H_5627135585666179
#define XXHASH_H_5627135585666179 1

#ifdef XXH_NAMESPACE
#define XXH3_64bits XXH_IPREF(XXH3_64bits)
#define XXH3_64bits_withSecret XXH_IPREF(XXH3_64bits_withSecret)
#define XXH3_64bits_withSeed XXH_IPREF(XXH3_64bits_withSeed)
#define XXH3_64bits_withSecretandSeed XXH_IPREF(XXH3_64bits_withSecretandSeed)
#define XXH3_createState XXH_IPREF(XXH3_createState)
#define XXH3_freeState XXH_IPREF(XXH3_freeState)
#define XXH3_copyState XXH_IPREF(XXH3_copyState)
#define XXH3_64bits_reset XXH_IPREF(XXH3_64bits_reset)
#define XXH3_64bits_reset_withSeed XXH_IPREF(XXH3_64bits_reset_withSeed)
#define XXH3_64bits_reset_withSecret XXH_IPREF(XXH3_64bits_reset_withSecret)
#define XXH3_64bits_reset_withSecretandSeed XXH_IPREF(XXH3_64bits_reset_withSecretandSeed)
#define XXH3_64bits_update XXH_IPREF(XXH3_64bits_update)
#define XXH3_64bits_digest XXH_IPREF(XXH3_64bits_digest)
#define XXH3_generateSecret XXH_IPREF(XXH3_generateSecret)
#define XXH3_generateSecret_fromSeed XXH_IPREF(XXH3_generateSecret_fromSeed)
    /* XXH3_128bits */
#define XXH128 XXH_IPREF(XXH128)
#define XXH3_128bits XXH_IPREF(XXH3_128bits)
#define XXH3_128bits_withSeed XXH_IPREF(XXH3_128bits_withSeed)
#define XXH3_128bits_withSecret XXH_IPREF(XXH3_128bits_withSecret)
#define XXH3_128bits_withSecretandSeed XXH_IPREF(XXH3_128bits_withSecretandSeed)
#define XXH3_128bits_reset XXH_IPREF(XXH3_128bits_reset)
#define XXH3_128bits_reset_withSeed XXH_IPREF(XXH3_128bits_reset_withSeed)
#define XXH3_128bits_reset_withSecret XXH_IPREF(XXH3_128bits_reset_withSecret)
#define XXH3_128bits_reset_withSecretandSeed XXH_IPREF(XXH3_128bits_reset_withSecretandSeed)
#define XXH3_128bits_update XXH_IPREF(XXH3_128bits_update)
#define XXH3_128bits_digest XXH_IPREF(XXH3_128bits_digest)
#endif

#ifndef XXH_NO_LONG_LONG

#ifndef XXH_NO_XXH3
#ifndef XXH_NO_STREAM

    KUMO_DLL KUMO_ATTRIBUTE_MALLOC_FUNCTION XXH3_state_t* XXH3_createState(void);
    KUMO_DLL XXH_errorcode XXH3_freeState(XXH3_state_t* statePtr);

    /*!
     * @brief Copies one @ref XXH3_state_t to another.
     *
     * @param dst_state The state to copy to.
     * @param src_state The state to copy from.
     * @pre
     *   @p dst_state and @p src_state must not be `NULL` and must not overlap.
     */
    KUMO_DLL void XXH3_copyState(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* dst_state, KUMO_ATTRIBUTE_NOESCAPE const XXH3_state_t* src_state);

    /*!
     * @brief Resets an @ref XXH3_state_t to begin a new hash.
     *
     * @param statePtr The state struct to reset.
     *
     * @pre
     *   @p statePtr must not be `NULL`.
     *
     * @return @ref XXH_OK on success.
     * @return @ref XXH_ERROR on failure.
     *
     * @note
     *   - This function resets `statePtr` and generate a secret with default parameters.
     *   - Call this function before @ref XXH3_64bits_update().
     *   - Digest will be equivalent to `XXH3_64bits()`.
     *
     * @see @ref streaming_example "Streaming Example"
     *
     */
    KUMO_DLL XXH_errorcode XXH3_64bits_reset(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr);

    /*!
     * @brief Resets an @ref XXH3_state_t with 64-bit seed to begin a new hash.
     *
     * @param statePtr The state struct to reset.
     * @param seed     The 64-bit seed to alter the hash result predictably.
     *
     * @pre
     *   @p statePtr must not be `NULL`.
     *
     * @return @ref XXH_OK on success.
     * @return @ref XXH_ERROR on failure.
     *
     * @note
     *   - This function resets `statePtr` and generate a secret from `seed`.
     *   - Call this function before @ref XXH3_64bits_update().
     *   - Digest will be equivalent to `XXH3_64bits_withSeed()`.
     *
     * @see @ref streaming_example "Streaming Example"
     *
     */
    KUMO_DLL XXH_errorcode XXH3_64bits_reset_withSeed(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr, uint64_t seed);

    /*!
     * @brief Resets an @ref XXH3_state_t with secret data to begin a new hash.
     *
     * @param statePtr The state struct to reset.
     * @param secret     The secret data.
     * @param secretSize The length of @p secret, in bytes.
     *
     * @pre
     *   @p statePtr must not be `NULL`.
     *
     * @return @ref XXH_OK on success.
     * @return @ref XXH_ERROR on failure.
     *
     * @note
     *   `secret` is referenced, it _must outlive_ the hash streaming session.
     *
     * Similar to one-shot API, `secretSize` must be >= @ref XXH3_SECRET_SIZE_MIN,
     * and the quality of produced hash values depends on secret's entropy
     * (secret's content should look like a bunch of random bytes).
     * When in doubt about the randomness of a candidate `secret`,
     * consider employing `XXH3_generateSecret()` instead (see below).
     *
     * @see @ref streaming_example "Streaming Example"
     */
    KUMO_DLL XXH_errorcode XXH3_64bits_reset_withSecret(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr, KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize);

    /*!
     * @brief Consumes a block of @p input to an @ref XXH3_state_t.
     *
     * @param statePtr The state struct to update.
     * @param input The block of data to be hashed, at least @p length bytes in size.
     * @param length The length of @p input, in bytes.
     *
     * @pre
     *   @p statePtr must not be `NULL`.
     * @pre
     *   The memory between @p input and @p input + @p length must be valid,
     *   readable, contiguous memory. However, if @p length is `0`, @p input may be
     *   `NULL`. In C++, this also must be *TriviallyCopyable*.
     *
     * @return @ref XXH_OK on success.
     * @return @ref XXH_ERROR on failure.
     *
     * @note Call this to incrementally consume blocks of data.
     *
     * @see @ref streaming_example "Streaming Example"
     */
    KUMO_DLL XXH_errorcode XXH3_64bits_update(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr, KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t length);

    /*!
     * @brief Returns the calculated XXH3 64-bit hash value from an @ref XXH3_state_t.
     *
     * @param statePtr The state struct to calculate the hash from.
     *
     * @pre
     *  @p statePtr must not be `NULL`.
     *
     * @return The calculated XXH3 64-bit hash value from that state.
     *
     * @note
     *   Calling XXH3_64bits_digest() will not affect @p statePtr, so you can update,
     *   digest, and update again.
     *
     * @see @ref streaming_example "Streaming Example"
     */
    KUMO_DLL KUMO_ATTRIBUTE_PURE_FUNCTION uint64_t XXH3_64bits_digest(KUMO_ATTRIBUTE_NOESCAPE const XXH3_state_t* statePtr);
#endif /* !XXH_NO_STREAM */
    /*******   Streaming   *******/
#ifndef XXH_NO_STREAM
    /*
     * Streaming requires state maintenance.
     * This operation costs memory and CPU.
     * As a consequence, streaming is slower than one-shot hashing.
     * For better performance, prefer one-shot functions whenever applicable.
     *
     * XXH3_128bits uses the same XXH3_state_t as XXH3_64bits().
     * Use already declared XXH3_createState() and XXH3_freeState().
     *
     * All reset and streaming functions have same meaning as their 64-bit counterpart.
     */

    /*!
     * @brief Resets an @ref XXH3_state_t to begin a new hash.
     *
     * @param statePtr The state struct to reset.
     *
     * @pre
     *   @p statePtr must not be `NULL`.
     *
     * @return @ref XXH_OK on success.
     * @return @ref XXH_ERROR on failure.
     *
     * @note
     *   - This function resets `statePtr` and generate a secret with default parameters.
     *   - Call it before @ref XXH3_128bits_update().
     *   - Digest will be equivalent to `XXH3_128bits()`.
     *
     * @see @ref streaming_example "Streaming Example"
     */
    KUMO_DLL XXH_errorcode XXH3_128bits_reset(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr);

    /*!
     * @brief Resets an @ref XXH3_state_t with 64-bit seed to begin a new hash.
     *
     * @param statePtr The state struct to reset.
     * @param seed     The 64-bit seed to alter the hash result predictably.
     *
     * @pre
     *   @p statePtr must not be `NULL`.
     *
     * @return @ref XXH_OK on success.
     * @return @ref XXH_ERROR on failure.
     *
     * @note
     *   - This function resets `statePtr` and generate a secret from `seed`.
     *   - Call it before @ref XXH3_128bits_update().
     *   - Digest will be equivalent to `XXH3_128bits_withSeed()`.
     *
     * @see @ref streaming_example "Streaming Example"
     */
    KUMO_DLL XXH_errorcode XXH3_128bits_reset_withSeed(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr, uint64_t seed);
    /*!
     * @brief Resets an @ref XXH3_state_t with secret data to begin a new hash.
     *
     * @param statePtr   The state struct to reset.
     * @param secret     The secret data.
     * @param secretSize The length of @p secret, in bytes.
     *
     * @pre
     *   @p statePtr must not be `NULL`.
     *
     * @return @ref XXH_OK on success.
     * @return @ref XXH_ERROR on failure.
     *
     * `secret` is referenced, it _must outlive_ the hash streaming session.
     * Similar to one-shot API, `secretSize` must be >= @ref XXH3_SECRET_SIZE_MIN,
     * and the quality of produced hash values depends on secret's entropy
     * (secret's content should look like a bunch of random bytes).
     * When in doubt about the randomness of a candidate `secret`,
     * consider employing `XXH3_generateSecret()` instead (see below).
     *
     * @see @ref streaming_example "Streaming Example"
     */
    KUMO_DLL XXH_errorcode XXH3_128bits_reset_withSecret(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr, KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize);

    /*!
     * @brief Consumes a block of @p input to an @ref XXH3_state_t.
     *
     * Call this to incrementally consume blocks of data.
     *
     * @param statePtr The state struct to update.
     * @param input The block of data to be hashed, at least @p length bytes in size.
     * @param length The length of @p input, in bytes.
     *
     * @pre
     *   @p statePtr must not be `NULL`.
     *
     * @return @ref XXH_OK on success.
     * @return @ref XXH_ERROR on failure.
     *
     * @note
     *   The memory between @p input and @p input + @p length must be valid,
     *   readable, contiguous memory. However, if @p length is `0`, @p input may be
     *   `NULL`. In C++, this also must be *TriviallyCopyable*.
     *
     */
    KUMO_DLL XXH_errorcode XXH3_128bits_update(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr, KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t length);

    /*!
     * @brief Returns the calculated XXH3 128-bit hash value from an @ref XXH3_state_t.
     *
     * @param statePtr The state struct to calculate the hash from.
     *
     * @pre
     *  @p statePtr must not be `NULL`.
     *
     * @return The calculated XXH3 128-bit hash value from that state.
     *
     * @note
     *   Calling XXH3_128bits_digest() will not affect @p statePtr, so you can update,
     *   digest, and update again.
     *
     */
    KUMO_DLL KUMO_ATTRIBUTE_PURE_FUNCTION XXH128_hash_t XXH3_128bits_digest(KUMO_ATTRIBUTE_NOESCAPE const XXH3_state_t* statePtr);
#endif /* !XXH_NO_STREAM */
#endif /* !XXH_NO_XXH3 */
#endif /* XXH_NO_LONG_LONG */

#endif /* XXHASH_H_5627135585666179 */

#if defined(XXH_STATIC_LINKING_ONLY) && !defined(XXHASH_H_STATIC_13879238742)
#define XXHASH_H_STATIC_13879238742

#ifndef XXH_NO_LONG_LONG /* defined when there is no 64-bit support */

#ifndef XXH_NO_XXH3

    /*!
     * @brief Initializes a stack-allocated `XXH3_state_s`.
     *
     * When the @ref XXH3_state_t structure is merely emplaced on stack,
     * it should be initialized with XXH3_INITSTATE() or a memset()
     * in case its first reset uses XXH3_NNbits_reset_withSeed().
     * This init can be omitted if the first reset uses default or _withSecret mode.
     * This operation isn't necessary when the state is created with XXH3_createState().
     * Note that this doesn't prepare the state for a streaming operation,
     * it's still necessary to use XXH3_NNbits_reset*() afterwards.
     */
#define XXH3_INITSTATE(XXH3_state_ptr)                       \
    do {                                                     \
        XXH3_state_t* tmp_xxh3_state_ptr = (XXH3_state_ptr); \
        tmp_xxh3_state_ptr->seed = 0;                        \
        tmp_xxh3_state_ptr->extSecret = NULL;                \
    } while (0)

    /*!
     * @brief Calculates the 128-bit hash of @p data using XXH3.
     *
     * @param data The block of data to be hashed, at least @p len bytes in size.
     * @param len  The length of @p data, in bytes.
     * @param seed The 64-bit seed to alter the hash's output predictably.
     *
     * @pre
     *   The memory between @p data and @p data + @p len must be valid,
     *   readable, contiguous memory. However, if @p len is `0`, @p data may be
     *   `NULL`. In C++, this also must be *TriviallyCopyable*.
     *
     * @return The calculated 128-bit XXH3 value.
     *
     * @see @ref single_shot_example "Single Shot Example" for an example.
     */
    KUMO_DLL KUMO_ATTRIBUTE_PURE_FUNCTION XXH128_hash_t XXH128(KUMO_ATTRIBUTE_NOESCAPE const void* data, size_t len, uint64_t seed);

    /* ===   Experimental API   === */
    /* Symbols defined below must be considered tied to a specific library version. */

    /*!
     * @brief Derive a high-entropy secret from any user-defined content, named customSeed.
     *
     * @param secretBuffer    A writable buffer for derived high-entropy secret data.
     * @param secretSize      Size of secretBuffer, in bytes.  Must be >= XXH3_SECRET_SIZE_MIN.
     * @param customSeed      A user-defined content.
     * @param customSeedSize  Size of customSeed, in bytes.
     *
     * @return @ref XXH_OK on success.
     * @return @ref XXH_ERROR on failure.
     *
     * The generated secret can be used in combination with `*_withSecret()` functions.
     * The `_withSecret()` variants are useful to provide a higher level of protection
     * than 64-bit seed, as it becomes much more difficult for an external actor to
     * guess how to impact the calculation logic.
     *
     * The function accepts as input a custom seed of any length and any content,
     * and derives from it a high-entropy secret of length @p secretSize into an
     * already allocated buffer @p secretBuffer.
     *
     * The generated secret can then be used with any `*_withSecret()` variant.
     * The functions @ref XXH3_128bits_withSecret(), @ref XXH3_64bits_withSecret(),
     * @ref XXH3_128bits_reset_withSecret() and @ref XXH3_64bits_reset_withSecret()
     * are part of this list. They all accept a `secret` parameter
     * which must be large enough for implementation reasons (>= @ref XXH3_SECRET_SIZE_MIN)
     * _and_ feature very high entropy (consist of random-looking bytes).
     * These conditions can be a high bar to meet, so @ref XXH3_generateSecret() can
     * be employed to ensure proper quality.
     *
     * @p customSeed can be anything. It can have any size, even small ones,
     * and its content can be anything, even "poor entropy" sources such as a bunch
     * of zeroes. The resulting `secret` will nonetheless provide all required qualities.
     *
     * @pre
     *   - @p secretSize must be >= @ref XXH3_SECRET_SIZE_MIN
     *   - When @p customSeedSize > 0, supplying NULL as customSeed is undefined behavior.
     *
     * Example code:
     * @code{.c}
     *    #include <stdio.h>
     *    #include <stdlib.h>
     *    #include <string.h>
     *    #define XXH_STATIC_LINKING_ONLY // expose unstable API
     *    #include "xxhash.h"
     *    // Hashes argv[2] using the entropy from argv[1].
     *    int main(int argc, char* argv[])
     *    {
     *        char secret[XXH3_SECRET_SIZE_MIN];
     *        if (argv != 3) { return 1; }
     *        XXH3_generateSecret(secret, sizeof(secret), argv[1], strlen(argv[1]));
     *        uint64_t h = XXH3_64bits_withSecret(
     *             argv[2], strlen(argv[2]),
     *             secret, sizeof(secret)
     *        );
     *        printf("%016llx\n", (unsigned long long) h);
     *    }
     * @endcode
     */
    KUMO_DLL XXH_errorcode XXH3_generateSecret(KUMO_ATTRIBUTE_NOESCAPE void* secretBuffer, size_t secretSize, KUMO_ATTRIBUTE_NOESCAPE const void* customSeed, size_t customSeedSize);

    /*!
     * @brief Generate the same secret as the _withSeed() variants.
     *
     * @param secretBuffer A writable buffer of @ref XXH3_SECRET_DEFAULT_SIZE bytes
     * @param seed         The 64-bit seed to alter the hash result predictably.
     *
     * The generated secret can be used in combination with
     *`*_withSecret()` and `_withSecretandSeed()` variants.
     *
     * Example C++ `std::string` hash class:
     * @code{.cpp}
     *    #include <string>
     *    #define XXH_STATIC_LINKING_ONLY // expose unstable API
     *    #include "xxhash.h"
     *    // Slow, seeds each time
     *    class HashSlow {
     *        uint64_t seed;
     *    public:
     *        HashSlow(uint64_t s) : seed{s} {}
     *        size_t operator()(const std::string& x) const {
     *            return size_t{XXH3_64bits_withSeed(x.c_str(), x.length(), seed)};
     *        }
     *    };
     *    // Fast, caches the seeded secret for future uses.
     *    class HashFast {
     *        unsigned char secret[XXH3_SECRET_DEFAULT_SIZE];
     *    public:
     *        HashFast(uint64_t s) {
     *            XXH3_generateSecret_fromSeed(secret, seed);
     *        }
     *        size_t operator()(const std::string& x) const {
     *            return size_t{
     *                XXH3_64bits_withSecret(x.c_str(), x.length(), secret, sizeof(secret))
     *            };
     *        }
     *    };
     * @endcode
     */
    KUMO_DLL void XXH3_generateSecret_fromSeed(KUMO_ATTRIBUTE_NOESCAPE void* secretBuffer, uint64_t seed);


    /*!
     * @brief Calculates 64/128-bit seeded variant of XXH3 hash of @p data.
     *
     * @param data       The block of data to be hashed, at least @p len bytes in size.
     * @param len        The length of @p data, in bytes.
     * @param secret     The secret data.
     * @param secretSize The length of @p secret, in bytes.
     * @param seed       The 64-bit seed to alter the hash result predictably.
     *
     * These variants generate hash values using either:
     * - @p seed for "short" keys (< @ref XXH3_MIDSIZE_MAX = 240 bytes)
     * - @p secret for "large" keys (>= @ref XXH3_MIDSIZE_MAX).
     *
     * This generally benefits speed, compared to `_withSeed()` or `_withSecret()`.
     * `_withSeed()` has to generate the secret on the fly for "large" keys.
     * It's fast, but can be perceptible for "not so large" keys (< 1 KB).
     * `_withSecret()` has to generate the masks on the fly for "small" keys,
     * which requires more instructions than _withSeed() variants.
     * Therefore, _withSecretandSeed variant combines the best of both worlds.
     *
     * When @p secret has been generated by XXH3_generateSecret_fromSeed(),
     * this variant produces *exactly* the same results as `_withSeed()` variant,
     * hence offering only a pure speed benefit on "large" input,
     * by skipping the need to regenerate the secret for every large input.
     *
     * Another usage scenario is to hash the secret to a 64-bit hash value,
     * for example with XXH3_64bits(), which then becomes the seed,
     * and then employ both the seed and the secret in _withSecretandSeed().
     * On top of speed, an added benefit is that each bit in the secret
     * has a 50% chance to swap each bit in the output, via its impact to the seed.
     *
     * This is not guaranteed when using the secret directly in "small data" scenarios,
     * because only portions of the secret are employed for small data.
     */
    KUMO_DLL KUMO_ATTRIBUTE_PURE_FUNCTION uint64_t
    XXH3_64bits_withSecretandSeed(KUMO_ATTRIBUTE_NOESCAPE const void* data, size_t len,
        KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize,
        uint64_t seed);

    /*!
     * @brief Calculates 128-bit seeded variant of XXH3 hash of @p data.
     *
     * @param input      The memory segment to be hashed, at least @p len bytes in size.
     * @param length     The length of @p data, in bytes.
     * @param secret     The secret used to alter hash result predictably.
     * @param secretSize The length of @p secret, in bytes (must be >= XXH3_SECRET_SIZE_MIN)
     * @param seed64     The 64-bit seed to alter the hash result predictably.
     *
     * @return @ref XXH_OK on success.
     * @return @ref XXH_ERROR on failure.
     *
     * @see XXH3_64bits_withSecretandSeed(): contract is the same.
     */
    KUMO_DLL KUMO_ATTRIBUTE_PURE_FUNCTION XXH128_hash_t
    XXH3_128bits_withSecretandSeed(KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t length,
        KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize,
        uint64_t seed64);

#ifndef XXH_NO_STREAM
    /*!
     * @brief Resets an @ref XXH3_state_t with secret data to begin a new hash.
     *
     * @param statePtr   A pointer to an @ref XXH3_state_t allocated with @ref XXH3_createState().
     * @param secret     The secret data.
     * @param secretSize The length of @p secret, in bytes.
     * @param seed64     The 64-bit seed to alter the hash result predictably.
     *
     * @return @ref XXH_OK on success.
     * @return @ref XXH_ERROR on failure.
     *
     * @see XXH3_64bits_withSecretandSeed(). Contract is identical.
     */
    KUMO_DLL XXH_errorcode
    XXH3_64bits_reset_withSecretandSeed(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr,
        KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize,
        uint64_t seed64);

    /*!
     * @brief Resets an @ref XXH3_state_t with secret data to begin a new hash.
     *
     * @param statePtr   A pointer to an @ref XXH3_state_t allocated with @ref XXH3_createState().
     * @param secret     The secret data.
     * @param secretSize The length of @p secret, in bytes.
     * @param seed64     The 64-bit seed to alter the hash result predictably.
     *
     * @return @ref XXH_OK on success.
     * @return @ref XXH_ERROR on failure.
     *
     * @see XXH3_64bits_withSecretandSeed(). Contract is identical.
     *
     * Note: there was a bug in an earlier version of this function (<= v0.8.2)
     * that would make it generate an incorrect hash value
     * when @p seed == 0 and @p length < XXH3_MIDSIZE_MAX
     * and @p secret is different from XXH3_generateSecret_fromSeed().
     * As stated in the contract, the correct hash result must be
     * the same as XXH3_128bits_withSeed() when @p length <= XXH3_MIDSIZE_MAX.
     * Results generated by this older version are wrong, hence not comparable.
     */
    KUMO_DLL XXH_errorcode
    XXH3_128bits_reset_withSecretandSeed(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr,
        KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize,
        uint64_t seed64);

#endif /* !XXH_NO_STREAM */

#endif /* !XXH_NO_XXH3 */
#endif /* XXH_NO_LONG_LONG */
#if defined(XXH_INLINE_ALL) || defined(XXH_PRIVATE_API)
#define XXH_IMPLEMENTATION
#endif

#endif /* defined(XXH_STATIC_LINKING_ONLY) && !defined(XXHASH_H_STATIC_13879238742) */

#if (defined(XXH_INLINE_ALL) || defined(XXH_PRIVATE_API) \
    || defined(XXH_IMPLEMENTATION))                      \
    && !defined(XXH_IMPLEM_13a8737387)
#define XXH_IMPLEM_13a8737387

    /* *************************************
     *  Tuning parameters
     ***************************************/

#ifndef XXH_SIZE_OPT
    /* default to 1 for -Os or -Oz */
#if (defined(__GNUC__) || defined(__clang__)) && defined(__OPTIMIZE_SIZE__)
#define XXH_SIZE_OPT 1
#else
#define XXH_SIZE_OPT 0
#endif
#endif

#ifndef XXH_FORCE_ALIGN_CHECK /* can be defined externally */
    /* don't check on sizeopt, x86, aarch64, or arm when unaligned access is available */
#if XXH_SIZE_OPT >= 1 || defined(__i386) || defined(__x86_64__) || defined(__aarch64__) || defined(__ARM_FEATURE_UNALIGNED) \
    || defined(_M_IX86) || defined(_M_X64) || defined(_M_ARM64) || defined(_M_ARM) /* visual */
#define XXH_FORCE_ALIGN_CHECK 0
#else
#define XXH_FORCE_ALIGN_CHECK 1
#endif
#endif

#ifndef XXH3_INLINE_SECRET
#if (defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 12) \
    || !defined(XXH_INLINE_ALL)
#define XXH3_INLINE_SECRET 0
#else
#define XXH3_INLINE_SECRET 1
#endif
#endif

    /* *************************************
     *  Includes & Memory related functions
     ***************************************/
#if defined(XXH_NO_STREAM)
    /* nothing */
#elif defined(XXH_NO_STDLIB)

    /* When requesting to disable any mention of stdlib,
     * the library loses the ability to invoked malloc / free.
     * In practice, it means that functions like `XXH*_createState()`
     * will always fail, and return NULL.
     * This flag is useful in situations where
     * xxhash.h is integrated into some kernel, embedded or limited environment
     * without access to dynamic allocation.
     */

    static KUMO_ATTRIBUTE_CONST_FUNCTION void* XXH_malloc(size_t s) {
        (void)s;
        return NULL;
    }
    static void XXH_free(void* p) {
        (void)p;
    }

#else

    /*!
     * @internal
     * @brief Modify this function to use a different routine than malloc().
     */
    static KUMO_ATTRIBUTE_MALLOC_FUNCTION void* XXH_malloc(size_t s) {
        return malloc(s);
    }

    /*!
     * @internal
     * @brief Modify this function to use a different routine than free().
     */
    static void XXH_free(void* p) {
        free(p);
    }

#endif /* XXH_NO_STDLIB */

    /* *************************************
     *  Compiler Specific Options
     ***************************************/
#ifdef _MSC_VER /* Visual Studio warning fix */
#pragma warning(disable : 4127) /* disable: C4127: conditional expression is constant */
#endif

#if defined(XXH_INLINE_ALL)
#define XXH_STATIC KUMO_FORCE_INLINE
#else
#define XXH_STATIC static
#endif

#if XXH3_INLINE_SECRET
#define XXH3_WITH_SECRET_INLINE KUMO_FORCE_INLINE
#else
#define XXH3_WITH_SECRET_INLINE KUMO_ATTRIBUTE_NOINLINE
#endif


#if (XXH_DEBUGLEVEL >= 1)
#define XXH_ASSERT(c) assert(c)
#else
#if defined(__INTEL_COMPILER)
#define XXH_ASSERT(c) XXH_ASSUME((unsigned char)(c))
#else
#define XXH_ASSERT(c) XXH_ASSUME(c)
#endif
#endif

    /* note: use after variable declarations */
#ifndef XXH_STATIC_ASSERT
#if defined(__cplusplus) && (__cplusplus >= 201103L) /* C++11 */
#define XXH_STATIC_ASSERT_WITH_MESSAGE(c, m) \
    do {                                     \
        static_assert((c), m);               \
    } while (0)
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) /* C11 */
#define XXH_STATIC_ASSERT_WITH_MESSAGE(c, m) \
    do {                                     \
        _Static_assert((c), m);              \
    } while (0)
#else
#define XXH_STATIC_ASSERT_WITH_MESSAGE(c, m) \
    do {                                     \
        struct xxh_sa {                      \
            char x[(c) ? 1 : -1];            \
        };                                   \
    } while (0)
#endif
#define XXH_STATIC_ASSERT(c) XXH_STATIC_ASSERT_WITH_MESSAGE((c), #c)
#endif


#if defined(__GNUC__) || defined(__clang__)
#define XXH_COMPILER_GUARD(var) __asm__("" : "+r"(var))
#else
#define XXH_COMPILER_GUARD(var) ((void)0)
#endif

    /* Specifically for NEON vectors which use the "w" constraint, on
     * Clang. */
#if defined(__clang__) && defined(__ARM_ARCH) && !defined(__wasm__)
#define XXH_COMPILER_GUARD_CLANG_NEON(var) __asm__("" : "+w"(var))
#else
#define XXH_COMPILER_GUARD_CLANG_NEON(var) ((void)0)
#endif

    /* ****************************************
     *  Compiler-specific Functions and Macros
     ******************************************/
#define XXH_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)

#ifdef __has_builtin
#define XXH_HAS_BUILTIN(x) __has_builtin(x)
#else
#define XXH_HAS_BUILTIN(x) 0
#endif


#if XXH_HAS_BUILTIN(__builtin_assume)
#define XXH_ASSUME(c) __builtin_assume(c)
#else
#define XXH_ASSUME(c)      \
    if (!(c)) {            \
        KUMO_UNREACHABLE(); \
    }
#endif

    /* *******************************************************************
     *  32-bit hash functions
     *********************************************************************/

#ifndef XXH_NO_LONG_LONG
#ifndef XXH_NO_XXH3
    /* *********************************************************************
     *  XXH3
     *  New generation hash designed for speed on small keys and vectorization
     ************************************************************************ */

    /* ===   Compiler specifics   === */


    /*
     * One goal of XXH3 is to make it fast on both 32-bit and 64-bit, while
     * remaining a true 64-bit/128-bit hash function.
     *
     * This is done by prioritizing a subset of 64-bit operations that can be
     * emulated without too many steps on the average 32-bit machine.
     *
     * For example, these two lines seem similar, and run equally fast on 64-bit:
     *
     *   uint64_t x;
     *   x ^= (x >> 47); // good
     *   x ^= (x >> 13); // bad
     *
     * However, to a 32-bit machine, there is a major difference.
     *
     * x ^= (x >> 47) looks like this:
     *
     *   x.lo ^= (x.hi >> (47 - 32));
     *
     * while x ^= (x >> 13) looks like this:
     *
     *   // note: funnel shifts are not usually cheap.
     *   x.lo ^= (x.lo >> 13) | (x.hi << (32 - 13));
     *   x.hi ^= (x.hi >> 13);
     *
     * The first one is significantly faster than the second, simply because the
     * shift is larger than 32. This means:
     *  - All the bits we need are in the upper 32 bits, so we can ignore the lower
     *    32 bits in the shift.
     *  - The shift result will always fit in the lower 32 bits, and therefore,
     *    we can ignore the upper 32 bits in the xor.
     *
     * Thanks to this optimization, XXH3 only requires these features to be efficient:
     *
     *  - Usable unaligned access
     *  - A 32-bit or 64-bit ALU
     *      - If 32-bit, a decent ADC instruction
     *  - A 32 or 64-bit multiply with a 64-bit result
     *  - For the 128-bit variant, a decent byteswap helps short inputs.
     *
     * The first two are already required by XXH32, and almost all 32-bit and 64-bit
     * platforms which can run XXH32 can run XXH3 efficiently.
     *
     * Thumb-1, the classic 16-bit only subset of ARM's instruction set, is one
     * notable exception.
     *
     * First of all, Thumb-1 lacks support for the UMULL instruction which
     * performs the important long multiply. This means numerous __aeabi_lmul
     * calls.
     *
     * Second of all, the 8 functional registers are just not enough.
     * Setup for __aeabi_lmul, byteshift loads, pointers, and all arithmetic need
     * Lo registers, and this shuffling results in thousands more MOVs than A32.
     *
     * A32 and T32 don't have this limitation. They can access all 14 registers,
     * do a 32->64 multiply with UMULL, and the flexible operand allowing free
     * shifts is helpful, too.
     *
     * Therefore, we do a quick sanity check.
     *
     * If compiling Thumb-1 for a target which supports ARM instructions, we will
     * emit a warning, as it is not a "sane" platform to compile for.
     *
     * Usually, if this happens, it is because of an accident and you probably need
     * to specify -march, as you likely meant to compile for a newer architecture.
     *
     * Credit: large sections of the vectorial and asm source code paths
     *         have been contributed by @easyaspi314
     */
#if defined(__thumb__) && !defined(__thumb2__) && defined(__ARM_ARCH_ISA_ARM)
#warning "XXH3 is highly inefficient without ARM or Thumb-2."
#endif

    /* ==========================================
     * Vectorization detection
     * ========================================== */

    /* Actual definition */

#ifndef XXH_VECTOR /* can be defined on command line */
#if (                                                                                            \
    defined(__ARM_NEON__) || defined(__ARM_NEON) /* gcc */                                       \
    || defined(_M_ARM) || defined(_M_ARM64) || defined(_M_ARM64EC) /* msvc */                    \
    || (defined(__wasm_simd128__) && KUMO_HAS_INCLUDE(<arm_neon.h>)) /* wasm simd128 via SIMDe */ \
    )                                                                                            \
    && (defined(_WIN32) || defined(__LITTLE_ENDIAN__) /* little endian only */                   \
        || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__))
#define XXH_VECTOR XXH_NEON
#elif defined(__ARM_FEATURE_SVE)
#define XXH_VECTOR XXH_SVE
#elif defined(__AVX512F__)
#define XXH_VECTOR XXH_AVX512
#elif defined(__AVX2__)
#define XXH_VECTOR XXH_AVX2
#elif defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP == 2))
#define XXH_VECTOR XXH_SSE2
#elif (defined(__PPC64__) && defined(__POWER8_VECTOR__)) \
    || (defined(__s390x__) && defined(__VEC__))          \
        && defined(__GNUC__) /* TODO: IBM XL */
#define XXH_VECTOR XXH_VSX
#elif defined(__loongarch_asx)
#define XXH_VECTOR XXH_LASX
#elif defined(__loongarch_sx)
#define XXH_VECTOR XXH_LSX
#elif defined(__riscv_vector)
#define XXH_VECTOR XXH_RVV
#else
#define XXH_VECTOR XXH_SCALAR
#endif
#endif

    /* __ARM_FEATURE_SVE is only supported by GCC & Clang. */
#if (XXH_VECTOR == XXH_SVE) && !defined(__ARM_FEATURE_SVE)
#ifdef _MSC_VER
#pragma warning(once : 4606)
#else
#warning "__ARM_FEATURE_SVE isn't supported. Use SCALAR instead."
#endif
#undef XXH_VECTOR
#define XXH_VECTOR XXH_SCALAR
#endif

    /*
     * Controls the alignment of the accumulator,
     * for compatibility with aligned vector loads, which are usually faster.
     */
#ifndef XXH_ACC_ALIGN
#if defined(XXH_X86DISPATCH)
#define XXH_ACC_ALIGN 64 /* for compatibility with avx512 */
#elif XXH_VECTOR == XXH_SCALAR /* scalar */
#define XXH_ACC_ALIGN 8
#elif XXH_VECTOR == XXH_SSE2 /* sse2 */
#define XXH_ACC_ALIGN 16
#elif XXH_VECTOR == XXH_AVX2 /* avx2 */
#define XXH_ACC_ALIGN 32
#elif XXH_VECTOR == XXH_NEON /* neon */
#define XXH_ACC_ALIGN 16
#elif XXH_VECTOR == XXH_VSX /* vsx */
#define XXH_ACC_ALIGN 16
#elif XXH_VECTOR == XXH_AVX512 /* avx512 */
#define XXH_ACC_ALIGN 64
#elif XXH_VECTOR == XXH_SVE /* sve */
#define XXH_ACC_ALIGN 64
#elif XXH_VECTOR == XXH_LASX /* lasx */
#define XXH_ACC_ALIGN 64
#elif XXH_VECTOR == XXH_LSX /* lsx */
#define XXH_ACC_ALIGN 64
#elif XXH_VECTOR == XXH_RVV /* rvv */
#define XXH_ACC_ALIGN 64 /* could be 8, but 64 may be faster */
#endif
#endif

#if defined(XXH_X86DISPATCH) || XXH_VECTOR == XXH_SSE2 \
    || XXH_VECTOR == XXH_AVX2 || XXH_VECTOR == XXH_AVX512
#define XXH_SEC_ALIGN XXH_ACC_ALIGN
#elif XXH_VECTOR == XXH_SVE
#define XXH_SEC_ALIGN XXH_ACC_ALIGN
#elif XXH_VECTOR == XXH_RVV
#define XXH_SEC_ALIGN XXH_ACC_ALIGN
#else
#define XXH_SEC_ALIGN 8
#endif

#if defined(__GNUC__) || defined(__clang__)
#define XXH_ALIASING __attribute__((__may_alias__))
#else
#define XXH_ALIASING /* nothing */
#endif

    /*
     * UGLY HACK:
     * GCC usually generates the best code with -O3 for xxHash.
     *
     * However, when targeting AVX2, it is overzealous in its unrolling resulting
     * in code roughly 3/4 the speed of Clang.
     *
     * There are other issues, such as GCC splitting _mm256_loadu_si256 into
     * _mm_loadu_si128 + _mm256_inserti128_si256. This is an optimization which
     * only applies to Sandy and Ivy Bridge... which don't even support AVX2.
     *
     * That is why when compiling the AVX2 version, it is recommended to use either
     *   -O2 -mavx2 -march=haswell
     * or
     *   -O2 -mavx2 -mno-avx256-split-unaligned-load
     * for decent performance, or to use Clang instead.
     *
     * Fortunately, we can control the first one with a pragma that forces GCC into
     * -O2, but the other one we can't control without "failed to inline always
     * inline function due to target mismatch" warnings.
     */
#if XXH_VECTOR == XXH_AVX2 /* AVX2 */                                \
    && defined(__GNUC__) && !defined(__clang__) /* GCC, not Clang */ \
    && defined(__OPTIMIZE__) && XXH_SIZE_OPT <= 0 /* respect -O0 and -Os */
#pragma GCC push_options
#pragma GCC optimize("-O2")
#endif

#if XXH_VECTOR == XXH_NEON

    /*
     * UGLY HACK: While AArch64 GCC on Linux does not seem to care, on macOS, GCC -O3
     * optimizes out the entire hashLong loop because of the aliasing violation.
     *
     * However, GCC is also inefficient at load-store optimization with vld1q/vst1q,
     * so the only option is to mark it as aliasing.
     */
    typedef uint64x2_t xxh_aliasing_uint64x2_t XXH_ALIASING;

    /*!
     * @internal
     * @brief `vld1q_u64` but faster and alignment-safe.
     *
     * On AArch64, unaligned access is always safe, but on ARMv7-a, it is only
     * *conditionally* safe (`vld1` has an alignment bit like `movdq[ua]` in x86).
     *
     * GCC for AArch64 sees `vld1q_u8` as an intrinsic instead of a load, so it
     * prohibits load-store optimizations. Therefore, a direct dereference is used.
     *
     * Otherwise, `vld1q_u8` is used with `vreinterpretq_u8_u64` to do a safe
     * unaligned load.
     */
#if defined(__aarch64__) && defined(__GNUC__) && !defined(__clang__)
    KUMO_FORCE_INLINE uint64x2_t XXH_vld1q_u64(void const* ptr) /* silence -Wcast-align */
    {
        return *(xxh_aliasing_uint64x2_t const*)ptr;
    }
#else
    KUMO_FORCE_INLINE uint64x2_t XXH_vld1q_u64(void const* ptr) {
        return vreinterpretq_u64_u8(vld1q_u8((uint8_t const*)ptr));
    }
#endif

    /*!
     * @internal
     * @brief `vmlal_u32` on low and high halves of a vector.
     *
     * This is a workaround for AArch64 GCC < 11 which implemented arm_neon.h with
     * inline assembly and were therefore incapable of merging the `vget_{low, high}_u32`
     * with `vmlal_u32`.
     */
#if defined(__aarch64__) && defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 11
    KUMO_FORCE_INLINE uint64x2_t
    XXH_vmlal_low_u32(uint64x2_t acc, uint32x4_t lhs, uint32x4_t rhs) {
        /* Inline assembly is the only way */
        __asm__("umlal   %0.2d, %1.2s, %2.2s" : "+w"(acc) : "w"(lhs), "w"(rhs));
        return acc;
    }
    KUMO_FORCE_INLINE uint64x2_t
    XXH_vmlal_high_u32(uint64x2_t acc, uint32x4_t lhs, uint32x4_t rhs) {
        /* This intrinsic works as expected */
        return vmlal_high_u32(acc, lhs, rhs);
    }
#else
    /* Portable intrinsic versions */
    KUMO_FORCE_INLINE uint64x2_t
    XXH_vmlal_low_u32(uint64x2_t acc, uint32x4_t lhs, uint32x4_t rhs) {
        return vmlal_u32(acc, vget_low_u32(lhs), vget_low_u32(rhs));
    }
    /*! @copydoc XXH_vmlal_low_u32
     * Assume the compiler converts this to vmlal_high_u32 on aarch64 */
    KUMO_FORCE_INLINE uint64x2_t
    XXH_vmlal_high_u32(uint64x2_t acc, uint32x4_t lhs, uint32x4_t rhs) {
        return vmlal_u32(acc, vget_high_u32(lhs), vget_high_u32(rhs));
    }
#endif

    /*!
     * @ingroup tuning
     * @brief Controls the NEON to scalar ratio for XXH3
     *
     * This can be set to 2, 4, 6, or 8.
     *
     * ARM Cortex CPUs are _very_ sensitive to how their pipelines are used.
     *
     * For example, the Cortex-A73 can dispatch 3 micro-ops per cycle, but only 2 of those
     * can be NEON. If you are only using NEON instructions, you are only using 2/3 of the CPU
     * bandwidth.
     *
     * This is even more noticeable on the more advanced cores like the Cortex-A76 which
     * can dispatch 8 micro-ops per cycle, but still only 2 NEON micro-ops at once.
     *
     * Therefore, to make the most out of the pipeline, it is beneficial to run 6 NEON lanes
     * and 2 scalar lanes, which is chosen by default.
     *
     * This does not apply to Apple processors or 32-bit processors, which run better with
     * full NEON. These will default to 8. Additionally, size-optimized builds run 8 lanes.
     *
     * This change benefits CPUs with large micro-op buffers without negatively affecting
     * most other CPUs:
     *
     *  | Chipset               | Dispatch type       | NEON only | 6:2 hybrid | Diff. |
     *  |:----------------------|:--------------------|----------:|-----------:|------:|
     *  | Snapdragon 730 (A76)  | 2 NEON/8 micro-ops  |  8.8 GB/s |  10.1 GB/s |  ~16% |
     *  | Snapdragon 835 (A73)  | 2 NEON/3 micro-ops  |  5.1 GB/s |   5.3 GB/s |   ~5% |
     *  | Marvell PXA1928 (A53) | In-order dual-issue |  1.9 GB/s |   1.9 GB/s |    0% |
     *  | Apple M1              | 4 NEON/8 micro-ops  | 37.3 GB/s |  36.1 GB/s |  ~-3% |
     *
     * It also seems to fix some bad codegen on GCC, making it almost as fast as clang.
     *
     * When using WASM SIMD128, if this is 2 or 6, SIMDe will scalarize 2 of the lanes meaning
     * it effectively becomes worse 4.
     *
     * @see XXH3_accumulate_512_neon()
     */
#ifndef XXH3_NEON_LANES
#if (defined(__aarch64__) || defined(__arm64__) || defined(_M_ARM64) || defined(_M_ARM64EC)) \
    && !defined(__APPLE__) && XXH_SIZE_OPT <= 0
#define XXH3_NEON_LANES 6
#else
#define XXH3_NEON_LANES XXH_ACC_NB
#endif
#endif
#endif /* XXH_VECTOR == XXH_NEON */

    /*
     * VSX and Z Vector helpers.
     *
     * This is very messy, and any pull requests to clean this up are welcome.
     *
     * There are a lot of problems with supporting VSX and s390x, due to
     * inconsistent intrinsics, spotty coverage, and multiple endiannesses.
     */
#if XXH_VECTOR == XXH_VSX
    /* Annoyingly, these headers _may_ define three macros: `bool`, `vector`,
     * and `pixel`. This is a problem for obvious reasons.
     *
     * These keywords are unnecessary; the spec literally says they are
     * equivalent to `__bool`, `__vector`, and `__pixel` and may be undef'd
     * after including the header.
     *
     * We use pragma push_macro/pop_macro to keep the namespace clean. */
#pragma push_macro("bool")
#pragma push_macro("pixel")
    /* silence potential macro redefined warnings */
#undef bool
#undef pixel

    /* Restore the original macro values, if applicable. */
#pragma pop_macro("pixel")
#pragma pop_macro("bool")

    typedef __vector unsigned long long xxh_u64x2;
    typedef __vector unsigned char xxh_u8x16;
    typedef __vector unsigned xxh_u32x4;

    /*
     * UGLY HACK: Similar to aarch64 macOS GCC, s390x GCC has the same aliasing issue.
     */
    typedef xxh_u64x2 xxh_aliasing_u64x2 XXH_ALIASING;

#ifndef XXH_VSX_BE
#if defined(__BIG_ENDIAN__) \
    || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define XXH_VSX_BE 1
#elif defined(__VEC_ELEMENT_REG_ORDER__) && __VEC_ELEMENT_REG_ORDER__ == __ORDER_BIG_ENDIAN__
#warning "-maltivec=be is not recommended. Please use native endianness."
#define XXH_VSX_BE 1
#else
#define XXH_VSX_BE 0
#endif
#endif /* !defined(XXH_VSX_BE) */

#if XXH_VSX_BE
#if defined(__POWER9_VECTOR__) || (defined(__clang__) && defined(__s390x__))
#define XXH_vec_revb vec_revb
#else
    /*!
     * A polyfill for POWER9's vec_revb().
     */
    KUMO_FORCE_INLINE xxh_u64x2 XXH_vec_revb(xxh_u64x2 val) {
        xxh_u8x16 const vByteSwap = { 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
            0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08 };
        return vec_perm(val, val, vByteSwap);
    }
#endif
#endif /* XXH_VSX_BE */

    /*!
     * Performs an unaligned vector load and byte swaps it on big endian.
     */
    KUMO_FORCE_INLINE xxh_u64x2 XXH_vec_loadu(const void* ptr) {
        xxh_u64x2 ret;
        memcpy(&ret, ptr, sizeof(xxh_u64x2));
#if XXH_VSX_BE
        ret = XXH_vec_revb(ret);
#endif
        return ret;
    }

    /*
     * vec_mulo and vec_mule are very problematic intrinsics on PowerPC
     *
     * These intrinsics weren't added until GCC 8, despite existing for a while,
     * and they are endian dependent. Also, their meaning swap depending on version.
     * */
#if defined(__s390x__)
    /* s390x is always big endian, no issue on this platform */
#define XXH_vec_mulo vec_mulo
#define XXH_vec_mule vec_mule
#elif defined(__clang__) && XXH_HAS_BUILTIN(__builtin_altivec_vmuleuw) && !defined(__ibmxl__)
    /* Clang has a better way to control this, we can just use the builtin which doesn't swap. */
    /* The IBM XL Compiler (which defined __clang__) only implements the vec_* operations */
#define XXH_vec_mulo __builtin_altivec_vmulouw
#define XXH_vec_mule __builtin_altivec_vmuleuw
#else
    /* gcc needs inline assembly */
    /* Adapted from https://github.com/google/highwayhash/blob/master/highwayhash/hh_vsx.h. */
    KUMO_FORCE_INLINE xxh_u64x2 XXH_vec_mulo(xxh_u32x4 a, xxh_u32x4 b) {
        xxh_u64x2 result;
        __asm__("vmulouw %0, %1, %2" : "=v"(result) : "v"(a), "v"(b));
        return result;
    }
    KUMO_FORCE_INLINE xxh_u64x2 XXH_vec_mule(xxh_u32x4 a, xxh_u32x4 b) {
        xxh_u64x2 result;
        __asm__("vmuleuw %0, %1, %2" : "=v"(result) : "v"(a), "v"(b));
        return result;
    }
#endif /* XXH_vec_mulo, XXH_vec_mule */
#endif /* XXH_VECTOR == XXH_VSX */

#if XXH_VECTOR == XXH_SVE
#define ACCRND(acc, offset)                                              \
    do {                                                                 \
        svuint64_t input_vec = svld1_u64(mask, xinput + offset);         \
        svuint64_t secret_vec = svld1_u64(mask, xsecret + offset);       \
        svuint64_t mixed = sveor_u64_x(mask, secret_vec, input_vec);     \
        svuint64_t swapped = svtbl_u64(input_vec, kSwap);                \
        svuint64_t mixed_lo = svextw_u64_x(mask, mixed);                 \
        svuint64_t mixed_hi = svlsr_n_u64_x(mask, mixed, 32);            \
        svuint64_t mul = svmad_u64_x(mask, mixed_lo, mixed_hi, swapped); \
        acc = svadd_u64_x(mask, acc, mul);                               \
    } while (0)
#endif /* XXH_VECTOR == XXH_SVE */

    /*
     * These macros are to generate an XXH3_accumulate() function.
     * The two arguments select the name suffix and target attribute.
     *
     * The name of this symbol is XXH3_accumulate_<name>() and it calls
     * XXH3_accumulate_512_<name>().
     *
     * It may be useful to hand implement this function if the compiler fails to
     * optimize the inline function.
     */
#define XXH3_ACCUMULATE_TEMPLATE(name)                            \
    void                                                          \
    XXH3_accumulate_##name(uint64_t* KUMO_RESTRICT acc,           \
        const uint8_t* KUMO_RESTRICT input,                       \
        const uint8_t* KUMO_RESTRICT secret,                      \
        size_t nbStripes) {                                       \
        size_t n;                                                 \
        for (n = 0; n < nbStripes; n++) {                         \
            const uint8_t* const in = input + n * XXH_STRIPE_LEN; \
            XXH_PREFETCH(in + XXH_PREFETCH_DIST);                 \
            XXH3_accumulate_512_##name(                           \
                acc,                                              \
                in,                                               \
                secret + n * XXH_SECRET_CONSUME_RATE);            \
        }                                                         \
    }

    KUMO_FORCE_INLINE void XXH_writeLE64(void* dst, uint64_t v64) {
        if (!KUMO_ENDIAN_LITTLE)
            v64 = turbo::byteswap(v64);
        memcpy(dst, &v64, sizeof(v64));
    }

    /* Several intrinsic functions below are supposed to accept __int64 as argument,
     * as documented in https://software.intel.com/sites/landingpage/IntrinsicsGuide/ .
     * However, several environments do not define __int64 type,
     * requiring a workaround.
     */
#if !defined(__VMS)          \
    && (defined(__cplusplus) \
        || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) /* C99 */))
    typedef int64_t xxh_i64;
#else
    /* the following type must have a width of 64-bit */
    typedef long long xxh_i64;
#endif

    /*
     * XXH3_accumulate_512 is the tightest loop for long inputs, and it is the most optimized.
     *
     * It is a hardened version of UMAC, based off of FARSH's implementation.
     *
     * This was chosen because it adapts quite well to 32-bit, 64-bit, and SIMD
     * implementations, and it is ridiculously fast.
     *
     * We harden it by mixing the original input to the accumulators as well as the product.
     *
     * This means that in the (relatively likely) case of a multiply by zero, the
     * original input is preserved.
     *
     * On 128-bit inputs, we swap 64-bit pairs when we add the input to improve
     * cross-pollination, as otherwise the upper and lower halves would be
     * essentially independent.
     *
     * This doesn't matter on 64-bit hashes since they all get merged together in
     * the end, so we skip the extra step.
     *
     * Both XXH3_64bits and XXH3_128bits use this subroutine.
     */

#if (XXH_VECTOR == XXH_AVX512) \
    || (defined(XXH_DISPATCH_AVX512) && XXH_DISPATCH_AVX512 != 0)

#ifndef XXH_TARGET_AVX512
#define XXH_TARGET_AVX512 /* disable attribute target */
#endif

    KUMO_FORCE_INLINE XXH_TARGET_AVX512 void
    XXH3_accumulate_512_avx512(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        __m512i* const xacc = (__m512i*)acc;
        XXH_ASSERT((((size_t)acc) & 63) == 0);
        XXH_STATIC_ASSERT(XXH_STRIPE_LEN == sizeof(__m512i));

        {
            /* data_vec    = input[0]; */
            __m512i const data_vec = _mm512_loadu_si512(input);
            /* key_vec     = secret[0]; */
            __m512i const key_vec = _mm512_loadu_si512(secret);
            /* data_key    = data_vec ^ key_vec; */
            __m512i const data_key = _mm512_xor_si512(data_vec, key_vec);
            /* data_key_lo = data_key >> 32; */
            __m512i const data_key_lo = _mm512_srli_epi64(data_key, 32);
            /* product     = (data_key & 0xffffffff) * (data_key_lo & 0xffffffff); */
            __m512i const product = _mm512_mul_epu32(data_key, data_key_lo);
            /* xacc[0] += swap(data_vec); */
            __m512i const data_swap = _mm512_shuffle_epi32(data_vec, (_MM_PERM_ENUM)_MM_SHUFFLE(1, 0, 3, 2));
            __m512i const sum = _mm512_add_epi64(*xacc, data_swap);
            /* xacc[0] += product; */
            *xacc = _mm512_add_epi64(product, sum);
        }
    }
    KUMO_FORCE_INLINE XXH_TARGET_AVX512 XXH3_ACCUMULATE_TEMPLATE(avx512)

        /*
         * XXH3_scrambleAcc: Scrambles the accumulators to improve mixing.
         *
         * Multiplication isn't perfect, as explained by Google in HighwayHash:
         *
         *  // Multiplication mixes/scrambles bytes 0-7 of the 64-bit result to
         *  // varying degrees. In descending order of goodness, bytes
         *  // 3 4 2 5 1 6 0 7 have quality 228 224 164 160 100 96 36 32.
         *  // As expected, the upper and lower bytes are much worse.
         *
         * Source: https://github.com/google/highwayhash/blob/0aaf66b/highwayhash/hh_avx2.h#L291
         *
         * Since our algorithm uses a pseudorandom secret to add some variance into the
         * mix, we don't need to (or want to) mix as often or as much as HighwayHash does.
         *
         * This isn't as tight as XXH3_accumulate, but still written in SIMD to avoid
         * extraction.
         *
         * Both XXH3_64bits and XXH3_128bits use this subroutine.
         */

        KUMO_FORCE_INLINE XXH_TARGET_AVX512 void XXH3_scrambleAcc_avx512(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT secret) {
        XXH_ASSERT((((size_t)acc) & 63) == 0);
        XXH_STATIC_ASSERT(XXH_STRIPE_LEN == sizeof(__m512i));
        {
            __m512i* const xacc = (__m512i*)acc;
            const __m512i prime32 = _mm512_set1_epi32((int)XXH_PRIME32_1);

            /* xacc[0] ^= (xacc[0] >> 47) */
            __m512i const acc_vec = *xacc;
            __m512i const shifted = _mm512_srli_epi64(acc_vec, 47);
            /* xacc[0] ^= secret; */
            __m512i const key_vec = _mm512_loadu_si512(secret);
            __m512i const data_key = _mm512_ternarylogic_epi32(key_vec, acc_vec, shifted, 0x96 /* key_vec ^ acc_vec ^ shifted */);

            /* xacc[0] *= XXH_PRIME32_1; */
            __m512i const data_key_hi = _mm512_srli_epi64(data_key, 32);
            __m512i const prod_lo = _mm512_mul_epu32(data_key, prime32);
            __m512i const prod_hi = _mm512_mul_epu32(data_key_hi, prime32);
            *xacc = _mm512_add_epi64(prod_lo, _mm512_slli_epi64(prod_hi, 32));
        }
    }

    KUMO_FORCE_INLINE XXH_TARGET_AVX512 void
    XXH3_initCustomSecret_avx512(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        XXH_STATIC_ASSERT((XXH_SECRET_DEFAULT_SIZE & 63) == 0);
        XXH_STATIC_ASSERT(XXH_SEC_ALIGN == 64);
        XXH_ASSERT(((size_t)customSecret & 63) == 0);
        (void)(&XXH_writeLE64);
        {
            int const nbRounds = XXH_SECRET_DEFAULT_SIZE / sizeof(__m512i);
            __m512i const seed_pos = _mm512_set1_epi64((xxh_i64)seed64);
            __m512i const seed = _mm512_mask_sub_epi64(seed_pos, 0xAA, _mm512_set1_epi8(0), seed_pos);

            const __m512i* const src = (const __m512i*)((const void*)XXH3_kSecret);
            __m512i* const dest = (__m512i*)customSecret;
            int i;
            XXH_ASSERT(((size_t)src & 63) == 0); /* control alignment */
            XXH_ASSERT(((size_t)dest & 63) == 0);
            for (i = 0; i < nbRounds; ++i) {
                dest[i] = _mm512_add_epi64(_mm512_load_si512(src + i), seed);
            }
        }
    }

#endif

#if (XXH_VECTOR == XXH_AVX2) \
    || (defined(XXH_DISPATCH_AVX2) && XXH_DISPATCH_AVX2 != 0)

#ifndef XXH_TARGET_AVX2
#define XXH_TARGET_AVX2 /* disable attribute target */
#endif

    KUMO_FORCE_INLINE XXH_TARGET_AVX2 void
    XXH3_accumulate_512_avx2(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        XXH_ASSERT((((size_t)acc) & 31) == 0);
        {
            __m256i* const xacc = (__m256i*)acc;
            /* Unaligned. This is mainly for pointer arithmetic, and because
             * _mm256_loadu_si256 requires  a const __m256i * pointer for some reason. */
            const __m256i* const xinput = (const __m256i*)input;
            /* Unaligned. This is mainly for pointer arithmetic, and because
             * _mm256_loadu_si256 requires a const __m256i * pointer for some reason. */
            const __m256i* const xsecret = (const __m256i*)secret;

            size_t i;
            for (i = 0; i < XXH_STRIPE_LEN / sizeof(__m256i); i++) {
                /* data_vec    = xinput[i]; */
                __m256i const data_vec = _mm256_loadu_si256(xinput + i);
                /* key_vec     = xsecret[i]; */
                __m256i const key_vec = _mm256_loadu_si256(xsecret + i);
                /* data_key    = data_vec ^ key_vec; */
                __m256i const data_key = _mm256_xor_si256(data_vec, key_vec);
                /* data_key_lo = data_key >> 32; */
                __m256i const data_key_lo = _mm256_srli_epi64(data_key, 32);
                /* product     = (data_key & 0xffffffff) * (data_key_lo & 0xffffffff); */
                __m256i const product = _mm256_mul_epu32(data_key, data_key_lo);
                /* xacc[i] += swap(data_vec); */
                __m256i const data_swap = _mm256_shuffle_epi32(data_vec, _MM_SHUFFLE(1, 0, 3, 2));
                __m256i const sum = _mm256_add_epi64(xacc[i], data_swap);
                /* xacc[i] += product; */
                xacc[i] = _mm256_add_epi64(product, sum);
            }
        }
    }
    KUMO_FORCE_INLINE XXH_TARGET_AVX2 XXH3_ACCUMULATE_TEMPLATE(avx2)

        KUMO_FORCE_INLINE XXH_TARGET_AVX2 void XXH3_scrambleAcc_avx2(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT secret) {
        XXH_ASSERT((((size_t)acc) & 31) == 0);
        {
            __m256i* const xacc = (__m256i*)acc;
            /* Unaligned. This is mainly for pointer arithmetic, and because
             * _mm256_loadu_si256 requires a const __m256i * pointer for some reason. */
            const __m256i* const xsecret = (const __m256i*)secret;
            const __m256i prime32 = _mm256_set1_epi32((int)XXH_PRIME32_1);

            size_t i;
            for (i = 0; i < XXH_STRIPE_LEN / sizeof(__m256i); i++) {
                /* xacc[i] ^= (xacc[i] >> 47) */
                __m256i const acc_vec = xacc[i];
                __m256i const shifted = _mm256_srli_epi64(acc_vec, 47);
                __m256i const data_vec = _mm256_xor_si256(acc_vec, shifted);
                /* xacc[i] ^= xsecret; */
                __m256i const key_vec = _mm256_loadu_si256(xsecret + i);
                __m256i const data_key = _mm256_xor_si256(data_vec, key_vec);

                /* xacc[i] *= XXH_PRIME32_1; */
                __m256i const data_key_hi = _mm256_srli_epi64(data_key, 32);
                __m256i const prod_lo = _mm256_mul_epu32(data_key, prime32);
                __m256i const prod_hi = _mm256_mul_epu32(data_key_hi, prime32);
                xacc[i] = _mm256_add_epi64(prod_lo, _mm256_slli_epi64(prod_hi, 32));
            }
        }
    }

    KUMO_FORCE_INLINE XXH_TARGET_AVX2 void XXH3_initCustomSecret_avx2(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        XXH_STATIC_ASSERT((XXH_SECRET_DEFAULT_SIZE & 31) == 0);
        XXH_STATIC_ASSERT((XXH_SECRET_DEFAULT_SIZE / sizeof(__m256i)) == 6);
        XXH_STATIC_ASSERT(XXH_SEC_ALIGN <= 64);
        (void)(&XXH_writeLE64);
        XXH_PREFETCH(customSecret);
        {
            __m256i const seed = _mm256_set_epi64x((xxh_i64)(0U - seed64), (xxh_i64)seed64, (xxh_i64)(0U - seed64), (xxh_i64)seed64);

            const __m256i* const src = (const __m256i*)((const void*)XXH3_kSecret);
            __m256i* dest = (__m256i*)customSecret;

#if defined(__GNUC__) || defined(__clang__)
            /*
             * On GCC & Clang, marking 'dest' as modified will cause the compiler:
             *   - do not extract the secret from sse registers in the internal loop
             *   - use less common registers, and avoid pushing these reg into stack
             */
            XXH_COMPILER_GUARD(dest);
#endif
            XXH_ASSERT(((size_t)src & 31) == 0); /* control alignment */
            XXH_ASSERT(((size_t)dest & 31) == 0);

            /* GCC -O2 need unroll loop manually */
            dest[0] = _mm256_add_epi64(_mm256_load_si256(src + 0), seed);
            dest[1] = _mm256_add_epi64(_mm256_load_si256(src + 1), seed);
            dest[2] = _mm256_add_epi64(_mm256_load_si256(src + 2), seed);
            dest[3] = _mm256_add_epi64(_mm256_load_si256(src + 3), seed);
            dest[4] = _mm256_add_epi64(_mm256_load_si256(src + 4), seed);
            dest[5] = _mm256_add_epi64(_mm256_load_si256(src + 5), seed);
        }
    }

#endif

    /* x86dispatch always generates SSE2 */
#if (XXH_VECTOR == XXH_SSE2) || defined(XXH_X86DISPATCH)

#ifndef XXH_TARGET_SSE2
#define XXH_TARGET_SSE2 /* disable attribute target */
#endif

    KUMO_FORCE_INLINE XXH_TARGET_SSE2 void
    XXH3_accumulate_512_sse2(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        /* SSE2 is just a half-scale version of the AVX2 version. */
        XXH_ASSERT((((size_t)acc) & 15) == 0);
        {
            __m128i* const xacc = (__m128i*)acc;
            /* Unaligned. This is mainly for pointer arithmetic, and because
             * _mm_loadu_si128 requires a const __m128i * pointer for some reason. */
            const __m128i* const xinput = (const __m128i*)input;
            /* Unaligned. This is mainly for pointer arithmetic, and because
             * _mm_loadu_si128 requires a const __m128i * pointer for some reason. */
            const __m128i* const xsecret = (const __m128i*)secret;

            size_t i;
            for (i = 0; i < XXH_STRIPE_LEN / sizeof(__m128i); i++) {
                /* data_vec    = xinput[i]; */
                __m128i const data_vec = _mm_loadu_si128(xinput + i);
                /* key_vec     = xsecret[i]; */
                __m128i const key_vec = _mm_loadu_si128(xsecret + i);
                /* data_key    = data_vec ^ key_vec; */
                __m128i const data_key = _mm_xor_si128(data_vec, key_vec);
                /* data_key_lo = data_key >> 32; */
                __m128i const data_key_lo = _mm_shuffle_epi32(data_key, _MM_SHUFFLE(0, 3, 0, 1));
                /* product     = (data_key & 0xffffffff) * (data_key_lo & 0xffffffff); */
                __m128i const product = _mm_mul_epu32(data_key, data_key_lo);
                /* xacc[i] += swap(data_vec); */
                __m128i const data_swap = _mm_shuffle_epi32(data_vec, _MM_SHUFFLE(1, 0, 3, 2));
                __m128i const sum = _mm_add_epi64(xacc[i], data_swap);
                /* xacc[i] += product; */
                xacc[i] = _mm_add_epi64(product, sum);
            }
        }
    }
    KUMO_FORCE_INLINE XXH_TARGET_SSE2 XXH3_ACCUMULATE_TEMPLATE(sse2)

        KUMO_FORCE_INLINE XXH_TARGET_SSE2 void XXH3_scrambleAcc_sse2(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT secret) {
        XXH_ASSERT((((size_t)acc) & 15) == 0);
        {
            __m128i* const xacc = (__m128i*)acc;
            /* Unaligned. This is mainly for pointer arithmetic, and because
             * _mm_loadu_si128 requires a const __m128i * pointer for some reason. */
            const __m128i* const xsecret = (const __m128i*)secret;
            const __m128i prime32 = _mm_set1_epi32((int)XXH_PRIME32_1);

            size_t i;
            for (i = 0; i < XXH_STRIPE_LEN / sizeof(__m128i); i++) {
                /* xacc[i] ^= (xacc[i] >> 47) */
                __m128i const acc_vec = xacc[i];
                __m128i const shifted = _mm_srli_epi64(acc_vec, 47);
                __m128i const data_vec = _mm_xor_si128(acc_vec, shifted);
                /* xacc[i] ^= xsecret[i]; */
                __m128i const key_vec = _mm_loadu_si128(xsecret + i);
                __m128i const data_key = _mm_xor_si128(data_vec, key_vec);

                /* xacc[i] *= XXH_PRIME32_1; */
                __m128i const data_key_hi = _mm_shuffle_epi32(data_key, _MM_SHUFFLE(0, 3, 0, 1));
                __m128i const prod_lo = _mm_mul_epu32(data_key, prime32);
                __m128i const prod_hi = _mm_mul_epu32(data_key_hi, prime32);
                xacc[i] = _mm_add_epi64(prod_lo, _mm_slli_epi64(prod_hi, 32));
            }
        }
    }

    KUMO_FORCE_INLINE XXH_TARGET_SSE2 void XXH3_initCustomSecret_sse2(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        XXH_STATIC_ASSERT((XXH_SECRET_DEFAULT_SIZE & 15) == 0);
        (void)(&XXH_writeLE64);
        {
            int const nbRounds = XXH_SECRET_DEFAULT_SIZE / sizeof(__m128i);

#if defined(_MSC_VER) && defined(_M_IX86) && _MSC_VER <= 1900
            /* MSVC 32bit mode does not support _mm_set_epi64x before 2015
             * and some specific variants of 2015 may also lack it */
            /* Cast to unsigned 64-bit first to avoid signed arithmetic issues */
            uint64_t const seed64_unsigned = (uint64_t)seed64;
            uint64_t const neg_seed64 = (uint64_t)(0ULL - seed64_unsigned);
            __m128i const seed = _mm_set_epi32(
                (int)(neg_seed64 >> 32), /* high 32 bits of negated seed */
                (int)(neg_seed64), /* low 32 bits of negated seed */
                (int)(seed64_unsigned >> 32), /* high 32 bits of original seed */
                (int)(seed64_unsigned) /* low 32 bits of original seed */
            );
#else
            __m128i const seed = _mm_set_epi64x((xxh_i64)(0U - seed64), (xxh_i64)seed64);
#endif
            int i;

            const void* const src16 = XXH3_kSecret;
            __m128i* dst16 = (__m128i*)customSecret;
#if defined(__GNUC__) || defined(__clang__)
            /*
             * On GCC & Clang, marking 'dest' as modified will cause the compiler:
             *   - do not extract the secret from sse registers in the internal loop
             *   - use less common registers, and avoid pushing these reg into stack
             */
            XXH_COMPILER_GUARD(dst16);
#endif
            XXH_ASSERT(((size_t)src16 & 15) == 0); /* control alignment */
            XXH_ASSERT(((size_t)dst16 & 15) == 0);

            for (i = 0; i < nbRounds; ++i) {
                dst16[i] = _mm_add_epi64(_mm_load_si128((const __m128i*)src16 + i), seed);
            }
        }
    }

#endif

#if (XXH_VECTOR == XXH_NEON)

    /* forward declarations for the scalar routines */
    KUMO_FORCE_INLINE void
    XXH3_scalarRound(void* KUMO_RESTRICT acc, void const* KUMO_RESTRICT input,
        void const* KUMO_RESTRICT secret, size_t lane);

    /*!
     * @internal
     * @brief The bulk processing loop for NEON and WASM SIMD128.
     *
     * The NEON code path is actually partially scalar when running on AArch64. This
     * is to optimize the pipelining and can have up to 15% speedup depending on the
     * CPU, and it also mitigates some GCC codegen issues.
     *
     * @see XXH3_NEON_LANES for configuring this and details about this optimization.
     *
     * NEON's 32-bit to 64-bit long multiply takes a half vector of 32-bit
     * integers instead of the other platforms which mask full 64-bit vectors,
     * so the setup is more complicated than just shifting right.
     *
     * Additionally, there is an optimization for 4 lanes at once noted below.
     *
     * Since, as stated, the most optimal amount of lanes for Cortexes is 6,
     * there needs to be *three* versions of the accumulate operation used
     * for the remaining 2 lanes.
     *
     * WASM's SIMD128 uses SIMDe's arm_neon.h polyfill because the intrinsics overlap
     * nearly perfectly.
     */

    KUMO_FORCE_INLINE void
    XXH3_accumulate_512_neon(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        XXH_ASSERT((((size_t)acc) & 15) == 0);
        XXH_STATIC_ASSERT(XXH3_NEON_LANES > 0 && XXH3_NEON_LANES <= XXH_ACC_NB && XXH3_NEON_LANES % 2 == 0);
        { /* GCC for darwin arm64 does not like aliasing here */
            xxh_aliasing_uint64x2_t* const xacc = (xxh_aliasing_uint64x2_t*)acc;
            /* We don't use a uint32x4_t pointer because it causes bus errors on ARMv7. */
            uint8_t const* xinput = (const uint8_t*)input;
            uint8_t const* xsecret = (const uint8_t*)secret;

            size_t i;
#ifdef __wasm_simd128__
            /*
             * On WASM SIMD128, Clang emits direct address loads when XXH3_kSecret
             * is constant propagated, which results in it converting it to this
             * inside the loop:
             *
             *    a = v128.load(XXH3_kSecret +  0 + $secret_offset, offset = 0)
             *    b = v128.load(XXH3_kSecret + 16 + $secret_offset, offset = 0)
             *    ...
             *
             * This requires a full 32-bit address immediate (and therefore a 6 byte
             * instruction) as well as an add for each offset.
             *
             * Putting an asm guard prevents it from folding (at the cost of losing
             * the alignment hint), and uses the free offset in `v128.load` instead
             * of adding secret_offset each time which overall reduces code size by
             * about a kilobyte and improves performance.
             */
            XXH_COMPILER_GUARD(xsecret);
#endif
            /* Scalar lanes use the normal scalarRound routine */
            for (i = XXH3_NEON_LANES; i < XXH_ACC_NB; i++) {
                XXH3_scalarRound(acc, input, secret, i);
            }
            i = 0;
            /* 4 NEON lanes at a time. */
            for (; i + 1 < XXH3_NEON_LANES / 2; i += 2) {
                /* data_vec = xinput[i]; */
                uint64x2_t data_vec_1 = XXH_vld1q_u64(xinput + (i * 16));
                uint64x2_t data_vec_2 = XXH_vld1q_u64(xinput + ((i + 1) * 16));
                /* key_vec  = xsecret[i];  */
                uint64x2_t key_vec_1 = XXH_vld1q_u64(xsecret + (i * 16));
                uint64x2_t key_vec_2 = XXH_vld1q_u64(xsecret + ((i + 1) * 16));
                /* data_swap = swap(data_vec) */
                uint64x2_t data_swap_1 = vextq_u64(data_vec_1, data_vec_1, 1);
                uint64x2_t data_swap_2 = vextq_u64(data_vec_2, data_vec_2, 1);
                /* data_key = data_vec ^ key_vec; */
                uint64x2_t data_key_1 = veorq_u64(data_vec_1, key_vec_1);
                uint64x2_t data_key_2 = veorq_u64(data_vec_2, key_vec_2);

                /*
                 * If we reinterpret the 64x2 vectors as 32x4 vectors, we can use a
                 * de-interleave operation for 4 lanes in 1 step with `vuzpq_u32` to
                 * get one vector with the low 32 bits of each lane, and one vector
                 * with the high 32 bits of each lane.
                 *
                 * The intrinsic returns a double vector because the original ARMv7-a
                 * instruction modified both arguments in place. AArch64 and SIMD128 emit
                 * two instructions from this intrinsic.
                 *
                 *  [ dk11L | dk11H | dk12L | dk12H ] -> [ dk11L | dk12L | dk21L | dk22L ]
                 *  [ dk21L | dk21H | dk22L | dk22H ] -> [ dk11H | dk12H | dk21H | dk22H ]
                 */
                uint32x4x2_t unzipped = vuzpq_u32(
                    vreinterpretq_u32_u64(data_key_1),
                    vreinterpretq_u32_u64(data_key_2));
                /* data_key_lo = data_key & 0xFFFFFFFF */
                uint32x4_t data_key_lo = unzipped.val[0];
                /* data_key_hi = data_key >> 32 */
                uint32x4_t data_key_hi = unzipped.val[1];
                /*
                 * Then, we can split the vectors horizontally and multiply which, as for most
                 * widening intrinsics, have a variant that works on both high half vectors
                 * for free on AArch64. A similar instruction is available on SIMD128.
                 *
                 * sum = data_swap + (u64x2) data_key_lo * (u64x2) data_key_hi
                 */
                uint64x2_t sum_1 = XXH_vmlal_low_u32(data_swap_1, data_key_lo, data_key_hi);
                uint64x2_t sum_2 = XXH_vmlal_high_u32(data_swap_2, data_key_lo, data_key_hi);
                /*
                 * Clang reorders
                 *    a += b * c;     // umlal   swap.2d, dkl.2s, dkh.2s
                 *    c += a;         // add     acc.2d, acc.2d, swap.2d
                 * to
                 *    c += a;         // add     acc.2d, acc.2d, swap.2d
                 *    c += b * c;     // umlal   acc.2d, dkl.2s, dkh.2s
                 *
                 * While it would make sense in theory since the addition is faster,
                 * for reasons likely related to umlal being limited to certain NEON
                 * pipelines, this is worse. A compiler guard fixes this.
                 */
                XXH_COMPILER_GUARD_CLANG_NEON(sum_1);
                XXH_COMPILER_GUARD_CLANG_NEON(sum_2);
                /* xacc[i] = acc_vec + sum; */
                xacc[i] = vaddq_u64(xacc[i], sum_1);
                xacc[i + 1] = vaddq_u64(xacc[i + 1], sum_2);
            }
            /* Operate on the remaining NEON lanes 2 at a time. */
            for (; i < XXH3_NEON_LANES / 2; i++) {
                /* data_vec = xinput[i]; */
                uint64x2_t data_vec = XXH_vld1q_u64(xinput + (i * 16));
                /* key_vec  = xsecret[i];  */
                uint64x2_t key_vec = XXH_vld1q_u64(xsecret + (i * 16));
                /* acc_vec_2 = swap(data_vec) */
                uint64x2_t data_swap = vextq_u64(data_vec, data_vec, 1);
                /* data_key = data_vec ^ key_vec; */
                uint64x2_t data_key = veorq_u64(data_vec, key_vec);
                /* For two lanes, just use VMOVN and VSHRN. */
                /* data_key_lo = data_key & 0xFFFFFFFF; */
                uint32x2_t data_key_lo = vmovn_u64(data_key);
                /* data_key_hi = data_key >> 32; */
                uint32x2_t data_key_hi = vshrn_n_u64(data_key, 32);
                /* sum = data_swap + (u64x2) data_key_lo * (u64x2) data_key_hi; */
                uint64x2_t sum = vmlal_u32(data_swap, data_key_lo, data_key_hi);
                /* Same Clang workaround as before */
                XXH_COMPILER_GUARD_CLANG_NEON(sum);
                /* xacc[i] = acc_vec + sum; */
                xacc[i] = vaddq_u64(xacc[i], sum);
            }
        }
    }
    KUMO_FORCE_INLINE XXH3_ACCUMULATE_TEMPLATE(neon)

        KUMO_FORCE_INLINE void XXH3_scrambleAcc_neon(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT secret) {
        XXH_ASSERT((((size_t)acc) & 15) == 0);

        {
            xxh_aliasing_uint64x2_t* xacc = (xxh_aliasing_uint64x2_t*)acc;
            uint8_t const* xsecret = (uint8_t const*)secret;

            size_t i;
            /* WASM uses operator overloads and doesn't need these. */
#ifndef __wasm_simd128__
            /* { prime32_1, prime32_1 } */
            uint32x2_t const kPrimeLo = vdup_n_u32(XXH_PRIME32_1);
            /* { 0, prime32_1, 0, prime32_1 } */
            uint32x4_t const kPrimeHi = vreinterpretq_u32_u64(vdupq_n_u64((uint64_t)XXH_PRIME32_1 << 32));
#endif

            /* AArch64 uses both scalar and neon at the same time */
            for (i = XXH3_NEON_LANES; i < XXH_ACC_NB; i++) {
                XXH3_scalarScrambleRound(acc, secret, i);
            }
            for (i = 0; i < XXH3_NEON_LANES / 2; i++) {
                /* xacc[i] ^= (xacc[i] >> 47); */
                uint64x2_t acc_vec = xacc[i];
                uint64x2_t shifted = vshrq_n_u64(acc_vec, 47);
                uint64x2_t data_vec = veorq_u64(acc_vec, shifted);

                /* xacc[i] ^= xsecret[i]; */
                uint64x2_t key_vec = XXH_vld1q_u64(xsecret + (i * 16));
                uint64x2_t data_key = veorq_u64(data_vec, key_vec);
                /* xacc[i] *= XXH_PRIME32_1 */
#ifdef __wasm_simd128__
                /* SIMD128 has multiply by u64x2, use it instead of expanding and scalarizing */
                xacc[i] = data_key * XXH_PRIME32_1;
#else
                /*
                 * Expanded version with portable NEON intrinsics
                 *
                 *    lo(x) * lo(y) + (hi(x) * lo(y) << 32)
                 *
                 * prod_hi = hi(data_key) * lo(prime) << 32
                 *
                 * Since we only need 32 bits of this multiply a trick can be used, reinterpreting the vector
                 * as a uint32x4_t and multiplying by { 0, prime, 0, prime } to cancel out the unwanted bits
                 * and avoid the shift.
                 */
                uint32x4_t prod_hi = vmulq_u32(vreinterpretq_u32_u64(data_key), kPrimeHi);
                /* Extract low bits for vmlal_u32  */
                uint32x2_t data_key_lo = vmovn_u64(data_key);
                /* xacc[i] = prod_hi + lo(data_key) * XXH_PRIME32_1; */
                xacc[i] = vmlal_u32(vreinterpretq_u64_u32(prod_hi), data_key_lo, kPrimeLo);
#endif
            }
        }
    }
#endif

#if (XXH_VECTOR == XXH_VSX)

    KUMO_FORCE_INLINE void
    XXH3_accumulate_512_vsx(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        /* presumed aligned */
        xxh_aliasing_u64x2* const xacc = (xxh_aliasing_u64x2*)acc;
        uint8_t const* const xinput = (uint8_t const*)input; /* no alignment restriction */
        uint8_t const* const xsecret = (uint8_t const*)secret; /* no alignment restriction */
        xxh_u64x2 const v32 = { 32, 32 };
        size_t i;
        for (i = 0; i < XXH_STRIPE_LEN / sizeof(xxh_u64x2); i++) {
            /* data_vec = xinput[i]; */
            xxh_u64x2 const data_vec = XXH_vec_loadu(xinput + 16 * i);
            /* key_vec = xsecret[i]; */
            xxh_u64x2 const key_vec = XXH_vec_loadu(xsecret + 16 * i);
            xxh_u64x2 const data_key = data_vec ^ key_vec;
            /* shuffled = (data_key << 32) | (data_key >> 32); */
            xxh_u32x4 const shuffled = (xxh_u32x4)vec_rl(data_key, v32);
            /* product = ((xxh_u64x2)data_key & 0xFFFFFFFF) * ((xxh_u64x2)shuffled & 0xFFFFFFFF); */
            xxh_u64x2 const product = XXH_vec_mulo((xxh_u32x4)data_key, shuffled);
            /* acc_vec = xacc[i]; */
            xxh_u64x2 acc_vec = xacc[i];
            acc_vec += product;

            /* swap high and low halves */
#ifdef __s390x__
            acc_vec += vec_permi(data_vec, data_vec, 2);
#else
            acc_vec += vec_xxpermdi(data_vec, data_vec, 2);
#endif
            xacc[i] = acc_vec;
        }
    }
    KUMO_FORCE_INLINE XXH3_ACCUMULATE_TEMPLATE(vsx)

        KUMO_FORCE_INLINE void XXH3_scrambleAcc_vsx(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT secret) {
        XXH_ASSERT((((size_t)acc) & 15) == 0);

        {
            xxh_aliasing_u64x2* const xacc = (xxh_aliasing_u64x2*)acc;
            const uint8_t* const xsecret = (const uint8_t*)secret;
            /* constants */
            xxh_u64x2 const v32 = { 32, 32 };
            xxh_u64x2 const v47 = { 47, 47 };
            xxh_u32x4 const prime = { XXH_PRIME32_1, XXH_PRIME32_1, XXH_PRIME32_1, XXH_PRIME32_1 };
            size_t i;
            for (i = 0; i < XXH_STRIPE_LEN / sizeof(xxh_u64x2); i++) {
                /* xacc[i] ^= (xacc[i] >> 47); */
                xxh_u64x2 const acc_vec = xacc[i];
                xxh_u64x2 const data_vec = acc_vec ^ (acc_vec >> v47);

                /* xacc[i] ^= xsecret[i]; */
                xxh_u64x2 const key_vec = XXH_vec_loadu(xsecret + 16 * i);
                xxh_u64x2 const data_key = data_vec ^ key_vec;

                /* xacc[i] *= XXH_PRIME32_1 */
                /* prod_lo = ((xxh_u64x2)data_key & 0xFFFFFFFF) * ((xxh_u64x2)prime & 0xFFFFFFFF);  */
                xxh_u64x2 const prod_even = XXH_vec_mule((xxh_u32x4)data_key, prime);
                /* prod_hi = ((xxh_u64x2)data_key >> 32) * ((xxh_u64x2)prime >> 32);  */
                xxh_u64x2 const prod_odd = XXH_vec_mulo((xxh_u32x4)data_key, prime);
                xacc[i] = prod_odd + (prod_even << v32);
            }
        }
    }

#endif

#if (XXH_VECTOR == XXH_SVE)

    KUMO_FORCE_INLINE void
    XXH3_accumulate_512_sve(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        uint64_t* xacc = (uint64_t*)acc;
        const uint64_t* xinput = (const uint64_t*)(const void*)input;
        const uint64_t* xsecret = (const uint64_t*)(const void*)secret;
        svuint64_t kSwap = sveor_n_u64_z(svptrue_b64(), svindex_u64(0, 1), 1);
        uint64_t element_count = svcntd();
        if (element_count >= 8) {
            svbool_t mask = svptrue_pat_b64(SV_VL8);
            svuint64_t vacc = svld1_u64(mask, xacc);
            ACCRND(vacc, 0);
            svst1_u64(mask, xacc, vacc);
        } else if (element_count == 2) { /* sve128 */
            svbool_t mask = svptrue_pat_b64(SV_VL2);
            svuint64_t acc0 = svld1_u64(mask, xacc + 0);
            svuint64_t acc1 = svld1_u64(mask, xacc + 2);
            svuint64_t acc2 = svld1_u64(mask, xacc + 4);
            svuint64_t acc3 = svld1_u64(mask, xacc + 6);
            ACCRND(acc0, 0);
            ACCRND(acc1, 2);
            ACCRND(acc2, 4);
            ACCRND(acc3, 6);
            svst1_u64(mask, xacc + 0, acc0);
            svst1_u64(mask, xacc + 2, acc1);
            svst1_u64(mask, xacc + 4, acc2);
            svst1_u64(mask, xacc + 6, acc3);
        } else {
            svbool_t mask = svptrue_pat_b64(SV_VL4);
            svuint64_t acc0 = svld1_u64(mask, xacc + 0);
            svuint64_t acc1 = svld1_u64(mask, xacc + 4);
            ACCRND(acc0, 0);
            ACCRND(acc1, 4);
            svst1_u64(mask, xacc + 0, acc0);
            svst1_u64(mask, xacc + 4, acc1);
        }
    }

    KUMO_FORCE_INLINE void
    XXH3_accumulate_sve(uint64_t* KUMO_RESTRICT acc,
        const uint8_t* KUMO_RESTRICT input,
        const uint8_t* KUMO_RESTRICT secret,
        size_t nbStripes) {
        if (nbStripes != 0) {
            uint64_t* xacc = (uint64_t*)acc;
            const uint64_t* xinput = (const uint64_t*)(const void*)input;
            const uint64_t* xsecret = (const uint64_t*)(const void*)secret;
            svuint64_t kSwap = sveor_n_u64_z(svptrue_b64(), svindex_u64(0, 1), 1);
            uint64_t element_count = svcntd();
            if (element_count >= 8) {
                svbool_t mask = svptrue_pat_b64(SV_VL8);
                svuint64_t vacc = svld1_u64(mask, xacc + 0);
                do {
                    /* svprfd(svbool_t, void *, enum svfprop); */
                    svprfd(mask, xinput + 128, SV_PLDL1STRM);
                    ACCRND(vacc, 0);
                    xinput += 8;
                    xsecret += 1;
                    nbStripes--;
                } while (nbStripes != 0);

                svst1_u64(mask, xacc + 0, vacc);
            } else if (element_count == 2) { /* sve128 */
                svbool_t mask = svptrue_pat_b64(SV_VL2);
                svuint64_t acc0 = svld1_u64(mask, xacc + 0);
                svuint64_t acc1 = svld1_u64(mask, xacc + 2);
                svuint64_t acc2 = svld1_u64(mask, xacc + 4);
                svuint64_t acc3 = svld1_u64(mask, xacc + 6);
                do {
                    svprfd(mask, xinput + 128, SV_PLDL1STRM);
                    ACCRND(acc0, 0);
                    ACCRND(acc1, 2);
                    ACCRND(acc2, 4);
                    ACCRND(acc3, 6);
                    xinput += 8;
                    xsecret += 1;
                    nbStripes--;
                } while (nbStripes != 0);

                svst1_u64(mask, xacc + 0, acc0);
                svst1_u64(mask, xacc + 2, acc1);
                svst1_u64(mask, xacc + 4, acc2);
                svst1_u64(mask, xacc + 6, acc3);
            } else {
                svbool_t mask = svptrue_pat_b64(SV_VL4);
                svuint64_t acc0 = svld1_u64(mask, xacc + 0);
                svuint64_t acc1 = svld1_u64(mask, xacc + 4);
                do {
                    svprfd(mask, xinput + 128, SV_PLDL1STRM);
                    ACCRND(acc0, 0);
                    ACCRND(acc1, 4);
                    xinput += 8;
                    xsecret += 1;
                    nbStripes--;
                } while (nbStripes != 0);

                svst1_u64(mask, xacc + 0, acc0);
                svst1_u64(mask, xacc + 4, acc1);
            }
        }
    }

#endif

#if (XXH_VECTOR == XXH_LSX)
#define _LSX_SHUFFLE(z, y, x, w) (((z) << 6) | ((y) << 4) | ((x) << 2) | (w))

    KUMO_FORCE_INLINE void
    XXH3_accumulate_512_lsx(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        XXH_ASSERT((((size_t)acc) & 15) == 0);
        {
            __m128i* const xacc = (__m128i*)acc;
            const __m128i* const xinput = (const __m128i*)input;
            const __m128i* const xsecret = (const __m128i*)secret;
            size_t i;

            for (i = 0; i < XXH_STRIPE_LEN / sizeof(__m128i); i++) {
                /* data_vec = xinput[i]; */
                __m128i const data_vec = __lsx_vld(xinput + i, 0);
                /* key_vec = xsecret[i]; */
                __m128i const key_vec = __lsx_vld(xsecret + i, 0);
                /* data_key = data_vec ^ key_vec; */
                __m128i const data_key = __lsx_vxor_v(data_vec, key_vec);
                /* data_key_lo = data_key >> 32; */
                __m128i const data_key_lo = __lsx_vsrli_d(data_key, 32);
                // __m128i const data_key_lo = __lsx_vsrli_d(data_key, 32);
                /* product = (data_key & 0xffffffff) * (data_key_lo & 0xffffffff); */
                __m128i const product = __lsx_vmulwev_d_wu(data_key, data_key_lo);
                /* xacc[i] += swap(data_vec); */
                __m128i const data_swap = __lsx_vshuf4i_w(data_vec, _LSX_SHUFFLE(1, 0, 3, 2));
                __m128i const sum = __lsx_vadd_d(xacc[i], data_swap);
                /* xacc[i] += product; */
                xacc[i] = __lsx_vadd_d(product, sum);
            }
        }
    }
    KUMO_FORCE_INLINE XXH3_ACCUMULATE_TEMPLATE(lsx)

        KUMO_FORCE_INLINE void XXH3_scrambleAcc_lsx(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT secret) {
        XXH_ASSERT((((size_t)acc) & 15) == 0);
        {
            __m128i* const xacc = (__m128i*)acc;
            const __m128i* const xsecret = (const __m128i*)secret;
            const __m128i prime32 = __lsx_vreplgr2vr_d(XXH_PRIME32_1);
            size_t i;

            for (i = 0; i < XXH_STRIPE_LEN / sizeof(__m128i); i++) {
                /* xacc[i] ^= (xacc[i] >> 47) */
                __m128i const acc_vec = xacc[i];
                __m128i const shifted = __lsx_vsrli_d(acc_vec, 47);
                __m128i const data_vec = __lsx_vxor_v(acc_vec, shifted);
                /* xacc[i] ^= xsecret[i]; */
                __m128i const key_vec = __lsx_vld(xsecret + i, 0);
                __m128i const data_key = __lsx_vxor_v(data_vec, key_vec);

                /* xacc[i] *= XXH_PRIME32_1; */
                xacc[i] = __lsx_vmul_d(data_key, prime32);
            }
        }
    }

#endif

#if (XXH_VECTOR == XXH_LASX)
#define _LASX_SHUFFLE(z, y, x, w) (((z) << 6) | ((y) << 4) | ((x) << 2) | (w))

    KUMO_FORCE_INLINE void
    XXH3_accumulate_512_lasx(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        XXH_ASSERT((((size_t)acc) & 31) == 0);
        {
            size_t i;
            __m256i* const xacc = (__m256i*)acc;
            const __m256i* const xinput = (const __m256i*)input;
            const __m256i* const xsecret = (const __m256i*)secret;

            for (i = 0; i < XXH_STRIPE_LEN / sizeof(__m256i); i++) {
                /* data_vec = xinput[i]; */
                __m256i const data_vec = __lasx_xvld(xinput + i, 0);
                /* key_vec = xsecret[i]; */
                __m256i const key_vec = __lasx_xvld(xsecret + i, 0);
                /* data_key = data_vec ^ key_vec; */
                __m256i const data_key = __lasx_xvxor_v(data_vec, key_vec);
                /* data_key_lo = data_key >> 32; */
                __m256i const data_key_lo = __lasx_xvsrli_d(data_key, 32);
                // __m256i const data_key_lo = __lasx_xvsrli_d(data_key, 32);
                /* product = (data_key & 0xffffffff) * (data_key_lo & 0xffffffff); */
                __m256i const product = __lasx_xvmulwev_d_wu(data_key, data_key_lo);
                /* xacc[i] += swap(data_vec); */
                __m256i const data_swap = __lasx_xvshuf4i_w(data_vec, _LASX_SHUFFLE(1, 0, 3, 2));
                __m256i const sum = __lasx_xvadd_d(xacc[i], data_swap);
                /* xacc[i] += product; */
                xacc[i] = __lasx_xvadd_d(product, sum);
            }
        }
    }
    KUMO_FORCE_INLINE XXH3_ACCUMULATE_TEMPLATE(lasx)

        KUMO_FORCE_INLINE void XXH3_scrambleAcc_lasx(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT secret) {
        XXH_ASSERT((((size_t)acc) & 31) == 0);
        {
            __m256i* const xacc = (__m256i*)acc;
            const __m256i* const xsecret = (const __m256i*)secret;
            const __m256i prime32 = __lasx_xvreplgr2vr_d(XXH_PRIME32_1);
            size_t i;

            for (i = 0; i < XXH_STRIPE_LEN / sizeof(__m256i); i++) {
                /* xacc[i] ^= (xacc[i] >> 47) */
                __m256i const acc_vec = xacc[i];
                __m256i const shifted = __lasx_xvsrli_d(acc_vec, 47);
                __m256i const data_vec = __lasx_xvxor_v(acc_vec, shifted);
                /* xacc[i] ^= xsecret[i]; */
                __m256i const key_vec = __lasx_xvld(xsecret + i, 0);
                __m256i const data_key = __lasx_xvxor_v(data_vec, key_vec);

                /* xacc[i] *= XXH_PRIME32_1; */
                xacc[i] = __lasx_xvmul_d(data_key, prime32);
            }
        }
    }

#endif

#if (XXH_VECTOR == XXH_RVV)
#define XXH_CONCAT2(X, Y) X##Y
#define XXH_CONCAT(X, Y) XXH_CONCAT2(X, Y)
#if ((defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 13) || (defined(__clang__) && __clang_major__ < 16))
#define XXH_RVOP(op) op
#define XXH_RVCAST(op) XXH_CONCAT(vreinterpret_v_, op)
#else
#define XXH_RVOP(op) XXH_CONCAT(__riscv_, op)
#define XXH_RVCAST(op) XXH_CONCAT(__riscv_vreinterpret_v_, op)
#endif
    KUMO_FORCE_INLINE void
    XXH3_accumulate_512_rvv(void* KUMO_RESTRICT acc,
        const void* KUMO_RESTRICT input,
        const void* KUMO_RESTRICT secret) {
        XXH_ASSERT((((size_t)acc) & 63) == 0);
        {
            // Try to set vector lenght to 512 bits.
            // If this length is unavailable, then maximum available will be used
            size_t vl = XXH_RVOP(vsetvl_e64m2)(8);

            uint64_t* xacc = (uint64_t*)acc;
            const uint64_t* xinput = (const uint64_t*)input;
            const uint64_t* xsecret = (const uint64_t*)secret;
            static const uint64_t swap_mask[16] = { 1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14 };
            vuint64m2_t xswap_mask = XXH_RVOP(vle64_v_u64m2)(swap_mask, vl);

            size_t i;
            for (i = 0; i < XXH_STRIPE_LEN / 8; i += vl) {
                /* data_vec = xinput[i]; */
                vuint64m2_t data_vec = XXH_RVCAST(u8m2_u64m2)(XXH_RVOP(vle8_v_u8m2)((const uint8_t*)(xinput + i), vl * 8));
                /* key_vec = xsecret[i]; */
                vuint64m2_t key_vec = XXH_RVCAST(u8m2_u64m2)(XXH_RVOP(vle8_v_u8m2)((const uint8_t*)(xsecret + i), vl * 8));
                /* acc_vec = xacc[i]; */
                vuint64m2_t acc_vec = XXH_RVOP(vle64_v_u64m2)(xacc + i, vl);
                /* data_key = data_vec ^ key_vec; */
                vuint64m2_t data_key = XXH_RVOP(vxor_vv_u64m2)(data_vec, key_vec, vl);
                /* data_key_hi = data_key >> 32; */
                vuint64m2_t data_key_hi = XXH_RVOP(vsrl_vx_u64m2)(data_key, 32, vl);
                /* data_key_lo = data_key & 0xffffffff; */
                vuint64m2_t data_key_lo = XXH_RVOP(vand_vx_u64m2)(data_key, 0xffffffff, vl);
                /* swap high and low halves */
                vuint64m2_t data_swap = XXH_RVOP(vrgather_vv_u64m2)(data_vec, xswap_mask, vl);
                /* acc_vec += data_key_lo * data_key_hi; */
                acc_vec = XXH_RVOP(vmacc_vv_u64m2)(acc_vec, data_key_lo, data_key_hi, vl);
                /* acc_vec += data_swap; */
                acc_vec = XXH_RVOP(vadd_vv_u64m2)(acc_vec, data_swap, vl);
                /* xacc[i] = acc_vec; */
                XXH_RVOP(vse64_v_u64m2)(xacc + i, acc_vec, vl);
            }
        }
    }

    KUMO_FORCE_INLINE XXH3_ACCUMULATE_TEMPLATE(rvv)

        KUMO_FORCE_INLINE void XXH3_scrambleAcc_rvv(void* KUMO_RESTRICT acc, const void* KUMO_RESTRICT secret) {
        XXH_ASSERT((((size_t)acc) & 15) == 0);
        {
            size_t count = XXH_STRIPE_LEN / 8;
            uint64_t* xacc = (uint64_t*)acc;
            const uint8_t* xsecret = (const uint8_t*)secret;
            size_t vl;
            for (; count > 0; count -= vl, xacc += vl, xsecret += vl * 8) {
                vl = XXH_RVOP(vsetvl_e64m2)(count);
                {
                    /* key_vec = xsecret[i]; */
                    vuint64m2_t key_vec = XXH_RVCAST(u8m2_u64m2)(XXH_RVOP(vle8_v_u8m2)(xsecret, vl * 8));
                    /* acc_vec = xacc[i]; */
                    vuint64m2_t acc_vec = XXH_RVOP(vle64_v_u64m2)(xacc, vl);
                    /* acc_vec ^= acc_vec >> 47; */
                    vuint64m2_t vsrl = XXH_RVOP(vsrl_vx_u64m2)(acc_vec, 47, vl);
                    acc_vec = XXH_RVOP(vxor_vv_u64m2)(acc_vec, vsrl, vl);
                    /* acc_vec ^= key_vec; */
                    acc_vec = XXH_RVOP(vxor_vv_u64m2)(acc_vec, key_vec, vl);
                    /* acc_vec *= XXH_PRIME32_1; */
                    acc_vec = XXH_RVOP(vmul_vx_u64m2)(acc_vec, XXH_PRIME32_1, vl);
                    /* xacc[i] *= acc_vec; */
                    XXH_RVOP(vse64_v_u64m2)(xacc, acc_vec, vl);
                }
            }
        }
    }

    KUMO_FORCE_INLINE void
    XXH3_initCustomSecret_rvv(void* KUMO_RESTRICT customSecret, uint64_t seed64) {
        XXH_STATIC_ASSERT(XXH_SEC_ALIGN >= 8);
        XXH_ASSERT(((size_t)customSecret & 7) == 0);
        (void)(&XXH_writeLE64);
        {
            size_t count = XXH_SECRET_DEFAULT_SIZE / 8;
            size_t vl;
            size_t VLMAX = XXH_RVOP(vsetvlmax_e64m2)();
            int64_t* cSecret = (int64_t*)customSecret;
            const int64_t* kSecret = (const int64_t*)(const void*)XXH3_kSecret;

#if __riscv_v_intrinsic >= 1000000
            // ratified v1.0 intrinics version
            vbool32_t mneg = XXH_RVCAST(u8m1_b32)(
                XXH_RVOP(vmv_v_x_u8m1)(0xaa, XXH_RVOP(vsetvlmax_e8m1)()));
#else
            // support pre-ratification intrinics, which lack mask to vector casts
            size_t vlmax = XXH_RVOP(vsetvlmax_e8m1)();
            vbool32_t mneg = XXH_RVOP(vmseq_vx_u8mf4_b32)(
                XXH_RVOP(vand_vx_u8mf4)(
                    XXH_RVOP(vid_v_u8mf4)(vlmax), 1, vlmax),
                1, vlmax);
#endif
            vint64m2_t seed = XXH_RVOP(vmv_v_x_i64m2)((int64_t)seed64, VLMAX);
            seed = XXH_RVOP(vneg_v_i64m2_mu)(mneg, seed, seed, VLMAX);

            for (; count > 0; count -= vl, cSecret += vl, kSecret += vl) {
                /* make sure vl=VLMAX until last iteration */
                vl = XXH_RVOP(vsetvl_e64m2)(count < VLMAX ? count : VLMAX);
                {
                    vint64m2_t src = XXH_RVOP(vle64_v_i64m2)(kSecret, vl);
                    vint64m2_t res = XXH_RVOP(vadd_vv_i64m2)(src, seed, vl);
                    XXH_RVOP(vse64_v_i64m2)(cSecret, res, vl);
                }
            }
        }
    }
#endif

    KUMO_FORCE_INLINE XXH3_ACCUMULATE_TEMPLATE(scalar)

#if (XXH_VECTOR == XXH_AVX512)

#define XXH3_accumulate_512 XXH3_accumulate_512_avx512
#define XXH3_accumulate XXH3_accumulate_avx512
#define XXH3_scrambleAcc XXH3_scrambleAcc_avx512
#define XXH3_initCustomSecret XXH3_initCustomSecret_avx512

#elif (XXH_VECTOR == XXH_AVX2)

#define XXH3_accumulate_512 XXH3_accumulate_512_avx2
#define XXH3_accumulate XXH3_accumulate_avx2
#define XXH3_scrambleAcc XXH3_scrambleAcc_avx2
#define XXH3_initCustomSecret XXH3_initCustomSecret_avx2

#elif (XXH_VECTOR == XXH_SSE2)

#define XXH3_accumulate_512 XXH3_accumulate_512_sse2
#define XXH3_accumulate XXH3_accumulate_sse2
#define XXH3_scrambleAcc XXH3_scrambleAcc_sse2
#define XXH3_initCustomSecret XXH3_initCustomSecret_sse2

#elif (XXH_VECTOR == XXH_NEON)

#define XXH3_accumulate_512 XXH3_accumulate_512_neon
#define XXH3_accumulate XXH3_accumulate_neon
#define XXH3_scrambleAcc XXH3_scrambleAcc_neon
#define XXH3_initCustomSecret XXH3_initCustomSecret_scalar

#elif (XXH_VECTOR == XXH_VSX)

#define XXH3_accumulate_512 XXH3_accumulate_512_vsx
#define XXH3_accumulate XXH3_accumulate_vsx
#define XXH3_scrambleAcc XXH3_scrambleAcc_vsx
#define XXH3_initCustomSecret XXH3_initCustomSecret_scalar

#elif (XXH_VECTOR == XXH_SVE)
#define XXH3_accumulate_512 XXH3_accumulate_512_sve
#define XXH3_accumulate XXH3_accumulate_sve
#define XXH3_scrambleAcc XXH3_scrambleAcc_scalar
#define XXH3_initCustomSecret XXH3_initCustomSecret_scalar

#elif (XXH_VECTOR == XXH_LASX)
#define XXH3_accumulate_512 XXH3_accumulate_512_lasx
#define XXH3_accumulate XXH3_accumulate_lasx
#define XXH3_scrambleAcc XXH3_scrambleAcc_lasx
#define XXH3_initCustomSecret XXH3_initCustomSecret_scalar

#elif (XXH_VECTOR == XXH_LSX)
#define XXH3_accumulate_512 XXH3_accumulate_512_lsx
#define XXH3_accumulate XXH3_accumulate_lsx
#define XXH3_scrambleAcc XXH3_scrambleAcc_lsx
#define XXH3_initCustomSecret XXH3_initCustomSecret_scalar

#elif (XXH_VECTOR == XXH_RVV)
#define XXH3_accumulate_512 XXH3_accumulate_512_rvv
#define XXH3_accumulate XXH3_accumulate_rvv
#define XXH3_scrambleAcc XXH3_scrambleAcc_rvv
#define XXH3_initCustomSecret XXH3_initCustomSecret_rvv

#else /* scalar */

#define XXH3_accumulate_512 XXH3_accumulate_512_scalar
#define XXH3_accumulate XXH3_accumulate_scalar
#define XXH3_scrambleAcc XXH3_scrambleAcc_scalar
#define XXH3_initCustomSecret XXH3_initCustomSecret_scalar

#endif

#if XXH_SIZE_OPT >= 1 /* don't do SIMD for initialization */
#undef XXH3_initCustomSecret
#define XXH3_initCustomSecret XXH3_initCustomSecret_scalar
#endif

    KUMO_FORCE_INLINE void
    XXH3_hashLong_internal_loop(uint64_t* KUMO_RESTRICT acc,
        const uint8_t* KUMO_RESTRICT input, size_t len,
        const uint8_t* KUMO_RESTRICT secret, size_t secretSize,
        XXH3_f_accumulate f_acc,
        XXH3_f_scrambleAcc f_scramble) {
        size_t const nbStripesPerBlock = (secretSize - XXH_STRIPE_LEN) / XXH_SECRET_CONSUME_RATE;
        size_t const block_len = XXH_STRIPE_LEN * nbStripesPerBlock;
        size_t const nb_blocks = (len - 1) / block_len;

        size_t n;

        XXH_ASSERT(secretSize >= XXH3_SECRET_SIZE_MIN);

        for (n = 0; n < nb_blocks; n++) {
            f_acc(acc, input + n * block_len, secret, nbStripesPerBlock);
            f_scramble(acc, secret + secretSize - XXH_STRIPE_LEN);
        }

        /* last partial block */
        XXH_ASSERT(len > XXH_STRIPE_LEN);
        {
            size_t const nbStripes = ((len - 1) - (block_len * nb_blocks)) / XXH_STRIPE_LEN;
            XXH_ASSERT(nbStripes <= (secretSize / XXH_SECRET_CONSUME_RATE));
            f_acc(acc, input + nb_blocks * block_len, secret, nbStripes);

            /* last stripe */
            {
                const uint8_t* const p = input + len - XXH_STRIPE_LEN;
#define XXH_SECRET_LASTACC_START 7 /* not aligned on 8, last secret is different from acc & scrambler */
                XXH3_accumulate_512(acc, p, secret + secretSize - XXH_STRIPE_LEN - XXH_SECRET_LASTACC_START);
            }
        }
    }

    KUMO_FORCE_INLINE uint64_t
    XXH3_mix2Accs(const uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT secret) {
        return xxh3_mul128_fold64(
            acc[0] ^ turbo::little_endian::Load64(secret),
            acc[1] ^ turbo::little_endian::Load64(secret + 8));
    }

    KUMO_ATTRIBUTE_PURE_FUNCTION static uint64_t
    XXH3_mergeAccs(const uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT secret, uint64_t start) {
        uint64_t result64 = start;
        size_t i = 0;

        for (i = 0; i < 4; i++) {
            result64 += XXH3_mix2Accs(acc + 2 * i, secret + 16 * i);
#if defined(__clang__) /* Clang */                               \
    && (defined(__arm__) || defined(__thumb__)) /* ARMv7 */      \
    && (defined(__ARM_NEON) || defined(__ARM_NEON__)) /* NEON */ \
    && !defined(XXH_ENABLE_AUTOVECTORIZE) /* Define to disable */
            /*
             * UGLY HACK:
             * Prevent autovectorization on Clang ARMv7-a. Exact same problem as
             * the one in xxh3_len_129to240_64b. Speeds up shorter keys > 240b.
             * XXH3_64bits, len == 256, Snapdragon 835:
             *   without hack: 2063.7 MB/s
             *   with hack:    2560.7 MB/s
             */
            XXH_COMPILER_GUARD(result64);
#endif
        }

        return xxh3_avalanche(result64);
    }


    KUMO_ATTRIBUTE_PURE_FUNCTION static uint64_t
    XXH3_finalizeLong_64b(const uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT secret, uint64_t len) {
        return XXH3_mergeAccs(acc, secret + XXH_SECRET_MERGEACCS_START, len * XXH_PRIME64_1);
    }

#define XXH3_INIT_ACC { XXH_PRIME32_3, XXH_PRIME64_1, XXH_PRIME64_2, XXH_PRIME64_3, \
    XXH_PRIME64_4, XXH_PRIME32_2, XXH_PRIME64_5, XXH_PRIME32_1 }

    KUMO_FORCE_INLINE uint64_t
    XXH3_hashLong_64b_internal(const void* KUMO_RESTRICT input, size_t len,
        const void* KUMO_RESTRICT secret, size_t secretSize,
        XXH3_f_accumulate f_acc,
        XXH3_f_scrambleAcc f_scramble) {
        KUMO_ALIGN(XXH_ACC_ALIGN)
        uint64_t acc[XXH_ACC_NB] = XXH3_INIT_ACC;

        XXH3_hashLong_internal_loop(acc, (const uint8_t*)input, len, (const uint8_t*)secret, secretSize, f_acc, f_scramble);

        /* converge into final hash */
        XXH_STATIC_ASSERT(sizeof(acc) == 64);
        XXH_ASSERT(secretSize >= sizeof(acc) + XXH_SECRET_MERGEACCS_START);
        return XXH3_finalizeLong_64b(acc, (const uint8_t*)secret, (uint64_t)len);
    }

    /*
     * It's important for performance to transmit secret's size (when it's static)
     * so that the compiler can properly optimize the vectorized loop.
     * This makes a big performance difference for "medium" keys (<1 KB) when using AVX instruction set.
     * When the secret size is unknown, or on GCC 12 where the mix of NO_INLINE and FORCE_INLINE
     * breaks -Og, this is KUMO_ATTRIBUTE_NOINLINE.
     */
    XXH3_WITH_SECRET_INLINE uint64_t
    XXH3_hashLong_64b_withSecret(const void* KUMO_RESTRICT input, size_t len,
        uint64_t seed64, const uint8_t* KUMO_RESTRICT secret, size_t secretLen) {
        (void)seed64;
        return XXH3_hashLong_64b_internal(input, len, secret, secretLen, XXH3_accumulate, XXH3_scrambleAcc);
    }

    /*
     * It's preferable for performance that XXH3_hashLong is not inlined,
     * as it results in a smaller function for small data, easier to the instruction cache.
     * Note that inside this no_inline function, we do inline the internal loop,
     * and provide a statically defined secret size to allow optimization of vector loop.
     */
    KUMO_ATTRIBUTE_PURE_FUNCTION KUMO_ATTRIBUTE_NOINLINE uint64_t
    XXH3_hashLong_64b_default(const void* KUMO_RESTRICT input, size_t len,
        uint64_t seed64, const uint8_t* KUMO_RESTRICT secret, size_t secretLen) {
        (void)seed64;
        (void)secret;
        (void)secretLen;
        return XXH3_hashLong_64b_internal(input, len, XXH3_kSecret, sizeof(XXH3_kSecret), XXH3_accumulate, XXH3_scrambleAcc);
    }

    /*
     * XXH3_hashLong_64b_withSeed():
     * Generate a custom key based on alteration of default XXH3_kSecret with the seed,
     * and then use this key for long mode hashing.
     *
     * This operation is decently fast but nonetheless costs a little bit of time.
     * Try to avoid it whenever possible (typically when seed==0).
     *
     * It's important for performance that XXH3_hashLong is not inlined. Not sure
     * why (uop cache maybe?), but the difference is large and easily measurable.
     */
    KUMO_FORCE_INLINE uint64_t
    XXH3_hashLong_64b_withSeed_internal(const void* input, size_t len,
        uint64_t seed,
        XXH3_f_accumulate f_acc,
        XXH3_f_scrambleAcc f_scramble,
        XXH3_f_initCustomSecret f_initSec) {
#if XXH_SIZE_OPT <= 0
        if (seed == 0)
            return XXH3_hashLong_64b_internal(input, len,
                XXH3_kSecret, sizeof(XXH3_kSecret),
                f_acc, f_scramble);
#endif
        {
            KUMO_ALIGN(XXH_SEC_ALIGN)
            uint8_t secret[XXH_SECRET_DEFAULT_SIZE];
            f_initSec(secret, seed);
            return XXH3_hashLong_64b_internal(input, len, secret, sizeof(secret),
                f_acc, f_scramble);
        }
    }

    /*
     * It's important for performance that XXH3_hashLong is not inlined.
     */
    KUMO_ATTRIBUTE_NOINLINE uint64_t
    XXH3_hashLong_64b_withSeed(const void* KUMO_RESTRICT input, size_t len,
        uint64_t seed, const uint8_t* KUMO_RESTRICT secret, size_t secretLen) {
        (void)secret;
        (void)secretLen;
        return XXH3_hashLong_64b_withSeed_internal(input, len, seed,
            XXH3_accumulate, XXH3_scrambleAcc, XXH3_initCustomSecret);
    }

    KUMO_FORCE_INLINE uint64_t
    XXH3_64bits_internal(const void* KUMO_RESTRICT input, size_t len,uint64_t seed64, const void* KUMO_RESTRICT secret, size_t secretLen,xxh3_hashLong64_func f_hashLong) {
        XXH_ASSERT(secretLen >= XXH3_SECRET_SIZE_MIN);
        /*
         * If an action is to be taken if `secretLen` condition is not respected,
         * it should be done here.
         * For now, it's a contract pre-condition.
         * Adding a check and a branch here would cost performance at every hash.
         * Also, note that function signature doesn't offer room to return an error.
         */
        if (len <= 16)
            return xxh3_len_0to16_64b((const uint8_t*)input, len, (const uint8_t*)secret, seed64);
        if (len <= 128)
            return xxh3_len_17to128_64b((const uint8_t*)input, len, (const uint8_t*)secret, secretLen, seed64);
        if (len <= XXH3_MIDSIZE_MAX)
            return xxh3_len_129to240_64b((const uint8_t*)input, len, (const uint8_t*)secret, secretLen, seed64);
        return f_hashLong(input, len, seed64, (const uint8_t*)secret, secretLen);
    }

    /* ===   Public entry point   === */

    /*! @ingroup XXH3_family */
    KUMO_DLL uint64_t XXH3_64bits(KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t length) {
        return XXH3_64bits_internal(input, length, 0, XXH3_kSecret, sizeof(XXH3_kSecret), XXH3_hashLong_64b_default);
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL uint64_t
    XXH3_64bits_withSecret(KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t length, KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize) {
        return XXH3_64bits_internal(input, length, 0, secret, secretSize, XXH3_hashLong_64b_withSecret);
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL uint64_t
    XXH3_64bits_withSeed(KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t length, uint64_t seed) {
        return XXH3_64bits_internal(input, length, seed, XXH3_kSecret, sizeof(XXH3_kSecret), XXH3_hashLong_64b_withSeed);
    }

    KUMO_DLL uint64_t
    XXH3_64bits_withSecretandSeed(KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t length, KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize, uint64_t seed) {
        if (length <= XXH3_MIDSIZE_MAX)
            return XXH3_64bits_internal(input, length, seed, XXH3_kSecret, sizeof(XXH3_kSecret), NULL);
        return XXH3_hashLong_64b_withSecret(input, length, seed, (const uint8_t*)secret, secretSize);
    }

    /* ===   XXH3 streaming   === */
#ifndef XXH_NO_STREAM
    /*
     * Malloc's a pointer that is always aligned to @align.
     *
     * This must be freed with `XXH_alignedFree()`.
     *
     * malloc typically guarantees 16 byte alignment on 64-bit systems and 8 byte
     * alignment on 32-bit. This isn't enough for the 32 byte aligned loads in AVX2
     * or on 32-bit, the 16 byte aligned loads in SSE2 and NEON.
     *
     * This underalignment previously caused a rather obvious crash which went
     * completely unnoticed due to XXH3_createState() not actually being tested.
     * Credit to RedSpah for noticing this bug.
     *
     * The alignment is done manually: Functions like posix_memalign or _mm_malloc
     * are avoided: To maintain portability, we would have to write a fallback
     * like this anyways, and besides, testing for the existence of library
     * functions without relying on external build tools is impossible.
     *
     * The method is simple: Overallocate, manually align, and store the offset
     * to the original behind the returned pointer.
     *
     * Align must be a power of 2 and 8 <= align <= 128.
     */
    static KUMO_ATTRIBUTE_MALLOC_FUNCTION void* XXH_alignedMalloc(size_t s, size_t align) {
        XXH_ASSERT(align <= 128 && align >= 8); /* range check */
        XXH_ASSERT((align & (align - 1)) == 0); /* power of 2 */
        XXH_ASSERT(s != 0 && s < (s + align)); /* empty/overflow */
        { /* Overallocate to make room for manual realignment and an offset byte */
            uint8_t* base = (uint8_t*)XXH_malloc(s + align);
            if (base != NULL) {
                /*
                 * Get the offset needed to align this pointer.
                 *
                 * Even if the returned pointer is aligned, there will always be
                 * at least one byte to store the offset to the original pointer.
                 */
                size_t offset = align - ((size_t)base & (align - 1)); /* base % align */
                /* Add the offset for the now-aligned pointer */
                uint8_t* ptr = base + offset;

                XXH_ASSERT((size_t)ptr % align == 0);

                /* Store the offset immediately before the returned pointer. */
                ptr[-1] = (uint8_t)offset;
                return ptr;
            }
            return NULL;
        }
    }
    /*
     * Frees an aligned pointer allocated by XXH_alignedMalloc(). Don't pass
     * normal malloc'd pointers, XXH_alignedMalloc has a specific data layout.
     */
    static void XXH_alignedFree(void* p) {
        if (p != NULL) {
            uint8_t* ptr = (uint8_t*)p;
            /* Get the offset byte we added in XXH_malloc. */
            uint8_t offset = ptr[-1];
            /* Free the original malloc'd pointer */
            uint8_t* base = ptr - offset;
            XXH_free(base);
        }
    }
    /*! @ingroup XXH3_family */
    /*!
     * @brief Allocate an @ref XXH3_state_t.
     *
     * @return An allocated pointer of @ref XXH3_state_t on success.
     * @return `NULL` on failure.
     *
     * @note Must be freed with XXH3_freeState().
     *
     * @see @ref streaming_example "Streaming Example"
     */
    KUMO_DLL XXH3_state_t* XXH3_createState(void) {
        XXH3_state_t* const state = (XXH3_state_t*)XXH_alignedMalloc(sizeof(XXH3_state_t), 64);
        if (state == NULL)
            return NULL;
        XXH3_INITSTATE(state);
        return state;
    }

    /*! @ingroup XXH3_family */
    /*!
     * @brief Frees an @ref XXH3_state_t.
     *
     * @param statePtr A pointer to an @ref XXH3_state_t allocated with @ref XXH3_createState().
     *
     * @return @ref XXH_OK.
     *
     * @note Must be allocated with XXH3_createState().
     *
     * @see @ref streaming_example "Streaming Example"
     */
    KUMO_DLL XXH_errorcode XXH3_freeState(XXH3_state_t* statePtr) {
        XXH_alignedFree(statePtr);
        return XXH_OK;
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL void
    XXH3_copyState(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* dst_state, KUMO_ATTRIBUTE_NOESCAPE const XXH3_state_t* src_state) {
        memcpy(dst_state, src_state, sizeof(*dst_state));
    }

    static void
    XXH3_reset_internal(XXH3_state_t* statePtr,uint64_t seed,const void* secret, size_t secretSize) {
        size_t const initStart = offsetof(XXH3_state_t, bufferedSize);
        size_t const initLength = offsetof(XXH3_state_t, nbStripesPerBlock) - initStart;
        XXH_ASSERT(offsetof(XXH3_state_t, nbStripesPerBlock) > initStart);
        XXH_ASSERT(statePtr != NULL);
        /* set members from bufferedSize to nbStripesPerBlock (excluded) to 0 */
        memset((char*)statePtr + initStart, 0, initLength);
        statePtr->acc[0] = XXH_PRIME32_3;
        statePtr->acc[1] = XXH_PRIME64_1;
        statePtr->acc[2] = XXH_PRIME64_2;
        statePtr->acc[3] = XXH_PRIME64_3;
        statePtr->acc[4] = XXH_PRIME64_4;
        statePtr->acc[5] = XXH_PRIME32_2;
        statePtr->acc[6] = XXH_PRIME64_5;
        statePtr->acc[7] = XXH_PRIME32_1;
        statePtr->seed = seed;
        statePtr->useSeed = (seed != 0);
        statePtr->extSecret = (const unsigned char*)secret;
        XXH_ASSERT(secretSize >= XXH3_SECRET_SIZE_MIN);
        statePtr->secretLimit = secretSize - XXH_STRIPE_LEN;
        statePtr->nbStripesPerBlock = statePtr->secretLimit / XXH_SECRET_CONSUME_RATE;
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL XXH_errorcode
    XXH3_64bits_reset(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr) {
        if (statePtr == NULL)
            return XXH_ERROR;
        XXH3_reset_internal(statePtr, 0, XXH3_kSecret, XXH_SECRET_DEFAULT_SIZE);
        return XXH_OK;
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL XXH_errorcode
    XXH3_64bits_reset_withSecret(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr, KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize) {
        if (statePtr == NULL)
            return XXH_ERROR;
        XXH3_reset_internal(statePtr, 0, secret, secretSize);
        if (secret == NULL)
            return XXH_ERROR;
        if (secretSize < XXH3_SECRET_SIZE_MIN)
            return XXH_ERROR;
        return XXH_OK;
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL XXH_errorcode
    XXH3_64bits_reset_withSeed(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr, uint64_t seed) {
        if (statePtr == NULL)
            return XXH_ERROR;
        if (seed == 0)
            return XXH3_64bits_reset(statePtr);
        if ((seed != statePtr->seed) || (statePtr->extSecret != NULL))
            XXH3_initCustomSecret(statePtr->customSecret, seed);
        XXH3_reset_internal(statePtr, seed, NULL, XXH_SECRET_DEFAULT_SIZE);
        return XXH_OK;
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL XXH_errorcode
    XXH3_64bits_reset_withSecretandSeed(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr, KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize, uint64_t seed64) {
        if (statePtr == NULL)
            return XXH_ERROR;
        if (secret == NULL)
            return XXH_ERROR;
        if (secretSize < XXH3_SECRET_SIZE_MIN)
            return XXH_ERROR;
        XXH3_reset_internal(statePtr, seed64, secret, secretSize);
        statePtr->useSeed = 1; /* always, even if seed64==0 */
        return XXH_OK;
    }

    /*!
     * @internal
     * @brief Processes a large input for XXH3_update() and XXH3_digest_long().
     *
     * Unlike XXH3_hashLong_internal_loop(), this can process data that overlaps a block.
     *
     * @param acc                Pointer to the 8 accumulator lanes
     * @param nbStripesSoFarPtr  In/out pointer to the number of leftover stripes in the block*
     * @param nbStripesPerBlock  Number of stripes in a block
     * @param input              Input pointer
     * @param nbStripes          Number of stripes to process
     * @param secret             Secret pointer
     * @param secretLimit        Offset of the last block in @p secret
     * @param f_acc              Pointer to an XXH3_accumulate implementation
     * @param f_scramble         Pointer to an XXH3_scrambleAcc implementation
     * @return                   Pointer past the end of @p input after processing
     */
    KUMO_FORCE_INLINE const uint8_t*
    XXH3_consumeStripes(uint64_t* KUMO_RESTRICT acc,
        size_t* KUMO_RESTRICT nbStripesSoFarPtr, size_t nbStripesPerBlock,
        const uint8_t* KUMO_RESTRICT input, size_t nbStripes,
        const uint8_t* KUMO_RESTRICT secret, size_t secretLimit,
        XXH3_f_accumulate f_acc,
        XXH3_f_scrambleAcc f_scramble) {
        const uint8_t* initialSecret = secret + *nbStripesSoFarPtr * XXH_SECRET_CONSUME_RATE;
        /* Process full blocks */
        if (nbStripes >= (nbStripesPerBlock - *nbStripesSoFarPtr)) {
            /* Process the initial partial block... */
            size_t nbStripesThisIter = nbStripesPerBlock - *nbStripesSoFarPtr;

            do {
                /* Accumulate and scramble */
                f_acc(acc, input, initialSecret, nbStripesThisIter);
                f_scramble(acc, secret + secretLimit);
                input += nbStripesThisIter * XXH_STRIPE_LEN;
                nbStripes -= nbStripesThisIter;
                /* Then continue the loop with the full block size */
                nbStripesThisIter = nbStripesPerBlock;
                initialSecret = secret;
            } while (nbStripes >= nbStripesPerBlock);
            *nbStripesSoFarPtr = 0;
        }
        /* Process a partial block */
        if (nbStripes > 0) {
            f_acc(acc, input, initialSecret, nbStripes);
            input += nbStripes * XXH_STRIPE_LEN;
            *nbStripesSoFarPtr += nbStripes;
        }
        /* Return end pointer */
        return input;
    }

#ifndef XXH3_STREAM_USE_STACK
#if XXH_SIZE_OPT <= 0 && !defined(__clang__) /* clang doesn't need additional stack space */
#define XXH3_STREAM_USE_STACK 1
#endif
#endif
    /* This function accepts f_acc and f_scramble as function pointers,
     * making it possible to implement multiple variants with different acc & scramble stages.
     * This is notably useful to implement multiple vector variants with different intrinsics.
     */
    KUMO_FORCE_INLINE XXH_errorcode
    XXH3_update(XXH3_state_t* KUMO_RESTRICT const state,
        const uint8_t* KUMO_RESTRICT input, size_t len,
        XXH3_f_accumulate f_acc,
        XXH3_f_scrambleAcc f_scramble) {
        if (input == NULL) {
            XXH_ASSERT(len == 0);
            return XXH_OK;
        }

        XXH_ASSERT(state != NULL);
        state->totalLen += len;

        /* small input : just fill in tmp buffer */
        XXH_ASSERT(state->bufferedSize <= XXH3_INTERNALBUFFER_SIZE);
        if (len <= XXH3_INTERNALBUFFER_SIZE - state->bufferedSize) {
            memcpy(state->buffer + state->bufferedSize, input, len);
            state->bufferedSize += (uint32_t)len;
            return XXH_OK;
        }

        {
            const uint8_t* const bEnd = input + len;
            const unsigned char* const secret = (state->extSecret == NULL) ? state->customSecret : state->extSecret;
#if defined(XXH3_STREAM_USE_STACK) && XXH3_STREAM_USE_STACK >= 1
            /* For some reason, gcc and MSVC seem to suffer greatly
             * when operating accumulators directly into state.
             * Operating into stack space seems to enable proper optimization.
             * clang, on the other hand, doesn't seem to need this trick */
            KUMO_ALIGN(XXH_ACC_ALIGN)
            uint64_t acc[8];
            memcpy(acc, state->acc, sizeof(acc));
#else
            uint64_t* KUMO_RESTRICT const acc = state->acc;
#endif

            /* total input is now > XXH3_INTERNALBUFFER_SIZE */
#define XXH3_INTERNALBUFFER_STRIPES (XXH3_INTERNALBUFFER_SIZE / XXH_STRIPE_LEN)
            XXH_STATIC_ASSERT(XXH3_INTERNALBUFFER_SIZE % XXH_STRIPE_LEN == 0); /* clean multiple */

            /*
             * Internal buffer is partially filled (always, except at beginning)
             * Complete it, then consume it.
             */
            if (state->bufferedSize) {
                size_t const loadSize = XXH3_INTERNALBUFFER_SIZE - state->bufferedSize;
                memcpy(state->buffer + state->bufferedSize, input, loadSize);
                input += loadSize;
                XXH3_consumeStripes(acc,
                    &state->nbStripesSoFar, state->nbStripesPerBlock,
                    state->buffer, XXH3_INTERNALBUFFER_STRIPES,
                    secret, state->secretLimit,
                    f_acc, f_scramble);
                state->bufferedSize = 0;
            }
            XXH_ASSERT(input < bEnd);
            if (bEnd - input > XXH3_INTERNALBUFFER_SIZE) {
                size_t nbStripes = (size_t)(bEnd - 1 - input) / XXH_STRIPE_LEN;
                input = XXH3_consumeStripes(acc,
                    &state->nbStripesSoFar, state->nbStripesPerBlock,
                    input, nbStripes,
                    secret, state->secretLimit,
                    f_acc, f_scramble);
                memcpy(state->buffer + sizeof(state->buffer) - XXH_STRIPE_LEN, input - XXH_STRIPE_LEN, XXH_STRIPE_LEN);
            }
            /* Some remaining input (always) : buffer it */
            XXH_ASSERT(input < bEnd);
            XXH_ASSERT(bEnd - input <= XXH3_INTERNALBUFFER_SIZE);
            XXH_ASSERT(state->bufferedSize == 0);
            memcpy(state->buffer, input, (size_t)(bEnd - input));
            state->bufferedSize = (uint32_t)(bEnd - input);
#if defined(XXH3_STREAM_USE_STACK) && XXH3_STREAM_USE_STACK >= 1
            /* save stack accumulators into state */
            memcpy(state->acc, acc, sizeof(acc));
#endif
        }

        return XXH_OK;
    }

    /*
     * Both XXH3_64bits_update and XXH3_128bits_update use this routine.
     */
    KUMO_ATTRIBUTE_NOINLINE XXH_errorcode
    XXH3_update_regular(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* state, KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t len) {
        return XXH3_update(state, (const uint8_t*)input, len,
            XXH3_accumulate, XXH3_scrambleAcc);
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL XXH_errorcode
    XXH3_64bits_update(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* state, KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t len) {
        return XXH3_update_regular(state, input, len);
    }

    KUMO_FORCE_INLINE void
    XXH3_digest_long(uint64_t* acc,
        const XXH3_state_t* state,
        const unsigned char* secret) {
        uint8_t lastStripe[XXH_STRIPE_LEN];
        const uint8_t* lastStripePtr;

        /*
         * Digest on a local copy. This way, the state remains unaltered, and it can
         * continue ingesting more input afterwards.
         */
        memcpy(acc, state->acc, sizeof(state->acc));
        if (state->bufferedSize >= XXH_STRIPE_LEN) {
            /* Consume remaining stripes then point to remaining data in buffer */
            size_t const nbStripes = (state->bufferedSize - 1) / XXH_STRIPE_LEN;
            size_t nbStripesSoFar = state->nbStripesSoFar;
            XXH3_consumeStripes(acc,
                &nbStripesSoFar, state->nbStripesPerBlock,
                state->buffer, nbStripes,
                secret, state->secretLimit,
                XXH3_accumulate, XXH3_scrambleAcc);
            lastStripePtr = state->buffer + state->bufferedSize - XXH_STRIPE_LEN;
        } else { /* bufferedSize < XXH_STRIPE_LEN */
            /* Copy to temp buffer */
            size_t const catchupSize = XXH_STRIPE_LEN - state->bufferedSize;
            XXH_ASSERT(state->bufferedSize > 0); /* there is always some input buffered */
            memcpy(lastStripe, state->buffer + sizeof(state->buffer) - catchupSize, catchupSize);
            memcpy(lastStripe + catchupSize, state->buffer, state->bufferedSize);
            lastStripePtr = lastStripe;
        }
        /* Last stripe */
        XXH3_accumulate_512(acc,
            lastStripePtr,
            secret + state->secretLimit - XXH_SECRET_LASTACC_START);
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL uint64_t XXH3_64bits_digest(KUMO_ATTRIBUTE_NOESCAPE const XXH3_state_t* state) {
        const unsigned char* const secret = (state->extSecret == NULL) ? state->customSecret : state->extSecret;
        if (state->totalLen > XXH3_MIDSIZE_MAX) {
            KUMO_ALIGN(XXH_ACC_ALIGN)
            uint64_t acc[XXH_ACC_NB];
            XXH3_digest_long(acc, state, secret);
            return XXH3_finalizeLong_64b(acc, secret, (uint64_t)state->totalLen);
        }
        /* totalLen <= XXH3_MIDSIZE_MAX: digesting a short input */
        if (state->useSeed)
            return XXH3_64bits_withSeed(state->buffer, (size_t)state->totalLen, state->seed);
        return XXH3_64bits_withSecret(state->buffer, (size_t)(state->totalLen),
            secret, state->secretLimit + XXH_STRIPE_LEN);
    }
#endif /* !XXH_NO_STREAM */

    KUMO_ATTRIBUTE_PURE_FUNCTION static XXH128_hash_t
    XXH3_finalizeLong_128b(const uint64_t* KUMO_RESTRICT acc, const uint8_t* KUMO_RESTRICT secret, size_t secretSize, uint64_t len) {
        XXH128_hash_t h128;
        h128.low64 = XXH3_finalizeLong_64b(acc, secret, len);
        h128.high64 = XXH3_mergeAccs(acc, secret + secretSize - XXH_STRIPE_LEN - XXH_SECRET_MERGEACCS_START,
            ~(len * XXH_PRIME64_2));
        return h128;
    }

    KUMO_FORCE_INLINE XXH128_hash_t
    XXH3_hashLong_128b_internal(const void* KUMO_RESTRICT input, size_t len,
        const uint8_t* KUMO_RESTRICT secret, size_t secretSize,
        XXH3_f_accumulate f_acc,
        XXH3_f_scrambleAcc f_scramble) {
        KUMO_ALIGN(XXH_ACC_ALIGN)
        uint64_t acc[XXH_ACC_NB] = XXH3_INIT_ACC;

        XXH3_hashLong_internal_loop(acc, (const uint8_t*)input, len, secret, secretSize, f_acc, f_scramble);

        /* converge into final hash */
        XXH_STATIC_ASSERT(sizeof(acc) == 64);
        XXH_ASSERT(secretSize >= sizeof(acc) + XXH_SECRET_MERGEACCS_START);
        return XXH3_finalizeLong_128b(acc, secret, secretSize, (uint64_t)len);
    }

    /*
     * It's important for performance that XXH3_hashLong() is not inlined.
     */
    KUMO_ATTRIBUTE_PURE_FUNCTION KUMO_ATTRIBUTE_NOINLINE XXH128_hash_t
    XXH3_hashLong_128b_default(const void* KUMO_RESTRICT input, size_t len,
        uint64_t seed64,
        const void* KUMO_RESTRICT secret, size_t secretLen) {
        (void)seed64;
        (void)secret;
        (void)secretLen;
        return XXH3_hashLong_128b_internal(input, len, XXH3_kSecret, sizeof(XXH3_kSecret),
            XXH3_accumulate, XXH3_scrambleAcc);
    }

    /*
     * It's important for performance to pass @p secretLen (when it's static)
     * to the compiler, so that it can properly optimize the vectorized loop.
     *
     * When the secret size is unknown, or on GCC 12 where the mix of NO_INLINE and FORCE_INLINE
     * breaks -Og, this is KUMO_ATTRIBUTE_NOINLINE.
     */
    XXH3_WITH_SECRET_INLINE XXH128_hash_t
    XXH3_hashLong_128b_withSecret(const void* KUMO_RESTRICT input, size_t len,
        uint64_t seed64,
        const void* KUMO_RESTRICT secret, size_t secretLen) {
        (void)seed64;
        return XXH3_hashLong_128b_internal(input, len, (const uint8_t*)secret, secretLen,
            XXH3_accumulate, XXH3_scrambleAcc);
    }

    KUMO_FORCE_INLINE XXH128_hash_t
    XXH3_hashLong_128b_withSeed_internal(const void* KUMO_RESTRICT input, size_t len,
        uint64_t seed64,
        XXH3_f_accumulate f_acc,
        XXH3_f_scrambleAcc f_scramble,
        XXH3_f_initCustomSecret f_initSec) {
        if (seed64 == 0)
            return XXH3_hashLong_128b_internal(input, len,
                XXH3_kSecret, sizeof(XXH3_kSecret),
                f_acc, f_scramble);
        {
            KUMO_ALIGN(XXH_SEC_ALIGN)
            uint8_t secret[XXH_SECRET_DEFAULT_SIZE];
            f_initSec(secret, seed64);
            return XXH3_hashLong_128b_internal(input, len, (const uint8_t*)secret, sizeof(secret),
                f_acc, f_scramble);
        }
    }

    /*
     * It's important for performance that XXH3_hashLong is not inlined.
     */
    KUMO_ATTRIBUTE_NOINLINE XXH128_hash_t
    XXH3_hashLong_128b_withSeed(const void* input, size_t len,
        uint64_t seed64, const void* KUMO_RESTRICT secret, size_t secretLen) {
        (void)secret;
        (void)secretLen;
        return XXH3_hashLong_128b_withSeed_internal(input, len, seed64,
            XXH3_accumulate, XXH3_scrambleAcc, XXH3_initCustomSecret);
    }

    typedef XXH128_hash_t (*XXH3_hashLong128_f)(const void* KUMO_RESTRICT, size_t,
        uint64_t, const void* KUMO_RESTRICT, size_t);

    KUMO_FORCE_INLINE XXH128_hash_t
    XXH3_128bits_internal(const void* input, size_t len,
        uint64_t seed64, const void* KUMO_RESTRICT secret, size_t secretLen,
        XXH3_hashLong128_f f_hl128) {
        XXH_ASSERT(secretLen >= XXH3_SECRET_SIZE_MIN);
        /*
         * If an action is to be taken if `secret` conditions are not respected,
         * it should be done here.
         * For now, it's a contract pre-condition.
         * Adding a check and a branch here would cost performance at every hash.
         */
        if (len <= 16)
            return xxh3_len_0to16_128b((const uint8_t*)input, len, (const uint8_t*)secret, seed64);
        if (len <= 128)
            return xxh3_len_17to128_128b((const uint8_t*)input, len, (const uint8_t*)secret, secretLen, seed64);
        if (len <= XXH3_MIDSIZE_MAX)
            return xxh3_len_129to240_128b((const uint8_t*)input, len, (const uint8_t*)secret, secretLen, seed64);
        return f_hl128(input, len, seed64, secret, secretLen);
    }

    /* ===   Public XXH128 API   === */

    /*! @ingroup XXH3_family */
    KUMO_DLL XXH128_hash_t XXH3_128bits(KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t len) {
        return XXH3_128bits_internal(input, len, 0,
            XXH3_kSecret, sizeof(XXH3_kSecret),
            XXH3_hashLong_128b_default);
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL XXH128_hash_t
    XXH3_128bits_withSecret(KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t len, KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize) {
        return XXH3_128bits_internal(input, len, 0,
            (const uint8_t*)secret, secretSize,
            XXH3_hashLong_128b_withSecret);
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL XXH128_hash_t
    XXH3_128bits_withSeed(KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t len, uint64_t seed) {
        return XXH3_128bits_internal(input, len, seed,
            XXH3_kSecret, sizeof(XXH3_kSecret),
            XXH3_hashLong_128b_withSeed);
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL XXH128_hash_t
    XXH3_128bits_withSecretandSeed(KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t len, KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize, uint64_t seed) {
        if (len <= XXH3_MIDSIZE_MAX)
            return XXH3_128bits_internal(input, len, seed, XXH3_kSecret, sizeof(XXH3_kSecret), NULL);
        return XXH3_hashLong_128b_withSecret(input, len, seed, secret, secretSize);
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL XXH128_hash_t
    XXH128(KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t len, uint64_t seed) {
        return XXH3_128bits_withSeed(input, len, seed);
    }

    /* ===   XXH3 128-bit streaming   === */
#ifndef XXH_NO_STREAM
    /*
     * All initialization and update functions are identical to 64-bit streaming variant.
     * The only difference is the finalization routine.
     */

    /*! @ingroup XXH3_family */
    KUMO_DLL XXH_errorcode
    XXH3_128bits_reset(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr) {
        return XXH3_64bits_reset(statePtr);
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL XXH_errorcode
    XXH3_128bits_reset_withSecret(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr, KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize) {
        return XXH3_64bits_reset_withSecret(statePtr, secret, secretSize);
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL XXH_errorcode
    XXH3_128bits_reset_withSeed(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr, uint64_t seed) {
        return XXH3_64bits_reset_withSeed(statePtr, seed);
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL XXH_errorcode
    XXH3_128bits_reset_withSecretandSeed(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* statePtr, KUMO_ATTRIBUTE_NOESCAPE const void* secret, size_t secretSize, uint64_t seed) {
        return XXH3_64bits_reset_withSecretandSeed(statePtr, secret, secretSize, seed);
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL XXH_errorcode
    XXH3_128bits_update(KUMO_ATTRIBUTE_NOESCAPE XXH3_state_t* state, KUMO_ATTRIBUTE_NOESCAPE const void* input, size_t len) {
        return XXH3_update_regular(state, input, len);
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL XXH128_hash_t XXH3_128bits_digest(KUMO_ATTRIBUTE_NOESCAPE const XXH3_state_t* state) {
        const unsigned char* const secret = (state->extSecret == NULL) ? state->customSecret : state->extSecret;
        if (state->totalLen > XXH3_MIDSIZE_MAX) {
            KUMO_ALIGN(XXH_ACC_ALIGN)
            uint64_t acc[XXH_ACC_NB];
            XXH3_digest_long(acc, state, secret);
            XXH_ASSERT(state->secretLimit + XXH_STRIPE_LEN >= sizeof(acc) + XXH_SECRET_MERGEACCS_START);
            return XXH3_finalizeLong_128b(acc, secret, state->secretLimit + XXH_STRIPE_LEN, (uint64_t)state->totalLen);
        }
        /* len <= XXH3_MIDSIZE_MAX : short code */
        if (state->useSeed)
            return XXH3_128bits_withSeed(state->buffer, (size_t)state->totalLen, state->seed);
        return XXH3_128bits_withSecret(state->buffer, (size_t)(state->totalLen),
            secret, state->secretLimit + XXH_STRIPE_LEN);
    }
#endif /* !XXH_NO_STREAM */


    /* ==========================================
     * Secret generators
     * ==========================================
     */
#define XXH_MIN(x, y) (((x) > (y)) ? (y) : (x))

    KUMO_FORCE_INLINE void XXH3_combine16(void* dst, XXH128_hash_t h128) {
        XXH_writeLE64(dst, turbo::little_endian::Load64(dst) ^ h128.low64);
        XXH_writeLE64((char*)dst + 8, turbo::little_endian::Load64((char*)dst + 8) ^ h128.high64);
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL XXH_errorcode
    XXH3_generateSecret(KUMO_ATTRIBUTE_NOESCAPE void* secretBuffer, size_t secretSize, KUMO_ATTRIBUTE_NOESCAPE const void* customSeed, size_t customSeedSize) {
#if (XXH_DEBUGLEVEL >= 1)
        XXH_ASSERT(secretBuffer != NULL);
        XXH_ASSERT(secretSize >= XXH3_SECRET_SIZE_MIN);
#else
        /* production mode, assert() are disabled */
        if (secretBuffer == NULL)
            return XXH_ERROR;
        if (secretSize < XXH3_SECRET_SIZE_MIN)
            return XXH_ERROR;
#endif

        if (customSeedSize == 0) {
            customSeed = XXH3_kSecret;
            customSeedSize = XXH_SECRET_DEFAULT_SIZE;
        }
#if (XXH_DEBUGLEVEL >= 1)
        XXH_ASSERT(customSeed != NULL);
#else
        if (customSeed == NULL)
            return XXH_ERROR;
#endif

        /* Fill secretBuffer with a copy of customSeed - repeat as needed */
        {
            size_t pos = 0;
            while (pos < secretSize) {
                size_t const toCopy = XXH_MIN((secretSize - pos), customSeedSize);
                memcpy((char*)secretBuffer + pos, customSeed, toCopy);
                pos += toCopy;
            }
        }

        {
            size_t const nbSeg16 = secretSize / 16;
            size_t n;
            XXH128_canonical_t scrambler;
            XXH128_canonicalFromHash(&scrambler, XXH128(customSeed, customSeedSize, 0));
            for (n = 0; n < nbSeg16; n++) {
                XXH128_hash_t const h128 = XXH128(&scrambler, sizeof(scrambler), n);
                XXH3_combine16((char*)secretBuffer + n * 16, h128);
            }
            /* last segment */
            XXH3_combine16((char*)secretBuffer + secretSize - 16, XXH128_hashFromCanonical(&scrambler));
        }
        return XXH_OK;
    }

    /*! @ingroup XXH3_family */
    KUMO_DLL void
    XXH3_generateSecret_fromSeed(KUMO_ATTRIBUTE_NOESCAPE void* secretBuffer, uint64_t seed) {
        KUMO_ALIGN(XXH_SEC_ALIGN)
        uint8_t secret[XXH_SECRET_DEFAULT_SIZE];
        XXH3_initCustomSecret(secret, seed);
        XXH_ASSERT(secretBuffer != NULL);
        memcpy(secretBuffer, secret, XXH_SECRET_DEFAULT_SIZE);
    }

    /* Pop our optimization override from above */
#if XXH_VECTOR == XXH_AVX2 /* AVX2 */                                \
    && defined(__GNUC__) && !defined(__clang__) /* GCC, not Clang */ \
    && defined(__OPTIMIZE__) && XXH_SIZE_OPT <= 0 /* respect -O0 and -Os */
#pragma GCC pop_options
#endif

#endif /* XXH_NO_LONG_LONG */

#endif /* XXH_NO_XXH3 */

#endif /* XXH_IMPLEMENTATION */
}
