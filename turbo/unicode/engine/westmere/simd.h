#ifndef UNICODE_WESTMERE_SIMD_H
#define UNICODE_WESTMERE_SIMD_H

namespace turbo {
    namespace UNICODE_IMPLEMENTATION {
        namespace {
            namespace simd {

                template <typename Child>
                struct base {
                    __m128i value;

                    // Zero constructor
                    KUMO_FORCE_INLINE base()
                        : value { __m128i() } { }

                    // Conversion from SIMD register
                    KUMO_FORCE_INLINE base(const __m128i _value)
                        : value(_value) { }
                    // Conversion to SIMD register
                    KUMO_FORCE_INLINE operator const __m128i&() const { return this->value; }
                    template <Endian big_endian>
                    KUMO_FORCE_INLINE void store_ascii_as_utf16(char16_t* p) const {
                        __m128i first = _mm_cvtepu8_epi16(*this);
                        __m128i second = _mm_cvtepu8_epi16(_mm_srli_si128(*this, 8));
                        if (big_endian == Endian::big) {
                            const __m128i swap = _mm_setr_epi8(1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14);
                            first = _mm_shuffle_epi8(first, swap);
                            second = _mm_shuffle_epi8(second, swap);
                        }
                        _mm_storeu_si128(reinterpret_cast<__m128i*>(p), first);
                        _mm_storeu_si128(reinterpret_cast<__m128i*>(p + 8), second);
                    }
                    KUMO_FORCE_INLINE void store_ascii_as_utf32(char32_t* p) const {
                        _mm_storeu_si128(reinterpret_cast<__m128i*>(p), _mm_cvtepu8_epi32(*this));
                        _mm_storeu_si128(reinterpret_cast<__m128i*>(p + 4),
                            _mm_cvtepu8_epi32(_mm_srli_si128(*this, 4)));
                        _mm_storeu_si128(reinterpret_cast<__m128i*>(p + 8),
                            _mm_cvtepu8_epi32(_mm_srli_si128(*this, 8)));
                        _mm_storeu_si128(reinterpret_cast<__m128i*>(p + 12),
                            _mm_cvtepu8_epi32(_mm_srli_si128(*this, 12)));
                    }
                    // Bit operations
                    KUMO_FORCE_INLINE Child operator|(const Child other) const {
                        return _mm_or_si128(*this, other);
                    }
                    KUMO_FORCE_INLINE Child operator&(const Child other) const {
                        return _mm_and_si128(*this, other);
                    }
                    KUMO_FORCE_INLINE Child operator^(const Child other) const {
                        return _mm_xor_si128(*this, other);
                    }
                    KUMO_FORCE_INLINE Child& operator|=(const Child other) {
                        auto this_cast = static_cast<Child*>(this);
                        *this_cast = *this_cast | other;
                        return *this_cast;
                    }
                };

                // Forward-declared so they can be used by splat and friends.
                template <typename T>
                struct simd8;

                template <typename T, typename Mask = simd8<bool>>
                struct base8 : base<simd8<T>> {
                    typedef uint16_t bitmask_t;
                    typedef uint32_t bitmask2_t;

                    KUMO_FORCE_INLINE T first() const { return _mm_extract_epi8(*this, 0); }
                    KUMO_FORCE_INLINE T last() const { return _mm_extract_epi8(*this, 15); }
                    KUMO_FORCE_INLINE base8()
                        : base<simd8<T>>() { }
                    KUMO_FORCE_INLINE base8(const __m128i _value)
                        : base<simd8<T>>(_value) { }

                    friend KUMO_FORCE_INLINE Mask operator==(const simd8<T> lhs,
                        const simd8<T> rhs) {
                        return _mm_cmpeq_epi8(lhs, rhs);
                    }

                    static const int SIZE = sizeof(base<simd8<T>>::value);

                    template <int N = 1>
                    KUMO_FORCE_INLINE simd8<T> prev(const simd8<T> prev_chunk) const {
                        return _mm_alignr_epi8(*this, prev_chunk, 16 - N);
                    }
                };

                // SIMD byte mask type (returned by things like eq and gt)
                template <>
                struct simd8<bool> : base8<bool> {
                    static KUMO_FORCE_INLINE simd8<bool> splat(bool _value) {
                        return _mm_set1_epi8(uint8_t(-(!!_value)));
                    }

                    KUMO_FORCE_INLINE simd8()
                        : base8() { }
                    KUMO_FORCE_INLINE simd8(const __m128i _value)
                        : base8<bool>(_value) { }
                    // Splat constructor
                    KUMO_FORCE_INLINE simd8(bool _value)
                        : base8<bool>(splat(_value)) { }

                    KUMO_FORCE_INLINE int to_bitmask() const {
                        return _mm_movemask_epi8(*this);
                    }
                    KUMO_FORCE_INLINE simd8<bool> operator~() const { return *this ^ true; }
                };

                template <typename T>
                struct base8_numeric : base8<T> {
                    static KUMO_FORCE_INLINE simd8<T> splat(T _value) {
                        return _mm_set1_epi8(_value);
                    }
                    static KUMO_FORCE_INLINE simd8<T> zero() { return _mm_setzero_si128(); }
                    static KUMO_FORCE_INLINE simd8<T> load(const T values[16]) {
                        return _mm_loadu_si128(reinterpret_cast<const __m128i*>(values));
                    }
                    // Repeat 16 values as many times as necessary (usually for lookup tables)
                    static KUMO_FORCE_INLINE simd8<T> repeat_16(T v0, T v1, T v2, T v3, T v4,
                        T v5, T v6, T v7, T v8, T v9,
                        T v10, T v11, T v12, T v13,
                        T v14, T v15) {
                        return simd8<T>(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13,
                            v14, v15);
                    }

                    KUMO_FORCE_INLINE base8_numeric()
                        : base8<T>() { }
                    KUMO_FORCE_INLINE base8_numeric(const __m128i _value)
                        : base8<T>(_value) { }

                    // Store to array
                    KUMO_FORCE_INLINE void store(T dst[16]) const {
                        return _mm_storeu_si128(reinterpret_cast<__m128i*>(dst), *this);
                    }

                    // Override to distinguish from bool version
                    KUMO_FORCE_INLINE simd8<T> operator~() const { return *this ^ 0xFFu; }

                    // Addition/subtraction are the same for signed and unsigned
                    KUMO_FORCE_INLINE simd8<T> operator-(const simd8<T> other) const {
                        return _mm_sub_epi8(*this, other);
                    }
                    KUMO_FORCE_INLINE simd8<T>& operator-=(const simd8<T> other) {
                        *this = *this - other;
                        return *static_cast<simd8<T>*>(this);
                    }

                    // Perform a lookup assuming the value is between 0 and 16 (undefined behavior
                    // for out of range values)
                    template <typename L>
                    KUMO_FORCE_INLINE simd8<L> lookup_16(simd8<L> lookup_table) const {
                        return _mm_shuffle_epi8(lookup_table, *this);
                    }

                    template <typename L>
                    KUMO_FORCE_INLINE simd8<L>
                    lookup_16(L replace0, L replace1, L replace2, L replace3, L replace4,
                        L replace5, L replace6, L replace7, L replace8, L replace9,
                        L replace10, L replace11, L replace12, L replace13, L replace14,
                        L replace15) const {
                        return lookup_16(simd8<L>::repeat_16(
                            replace0, replace1, replace2, replace3, replace4, replace5, replace6,
                            replace7, replace8, replace9, replace10, replace11, replace12,
                            replace13, replace14, replace15));
                    }
                };

                // Signed bytes
                template <>
                struct simd8<int8_t> : base8_numeric<int8_t> {
                    KUMO_FORCE_INLINE simd8()
                        : base8_numeric<int8_t>() { }
                    KUMO_FORCE_INLINE simd8(const __m128i _value)
                        : base8_numeric<int8_t>(_value) { }
                    // Splat constructor
                    KUMO_FORCE_INLINE simd8(int8_t _value)
                        : simd8(splat(_value)) { }
                    // Member-by-member initialization
                    KUMO_FORCE_INLINE operator simd8<uint8_t>() const;
                    KUMO_FORCE_INLINE bool is_ascii() const {
                        return _mm_movemask_epi8(*this) == 0;
                    }

                    // Order-sensitive comparisons
                    KUMO_FORCE_INLINE simd8<bool> operator>(const simd8<int8_t> other) const {
                        return _mm_cmpgt_epi8(*this, other);
                    }
                    KUMO_FORCE_INLINE simd8<bool> operator<(const simd8<int8_t> other) const {
                        return _mm_cmpgt_epi8(other, *this);
                    }
                };

                // Unsigned bytes
                template <>
                struct simd8<uint8_t> : base8_numeric<uint8_t> {
                    KUMO_FORCE_INLINE simd8()
                        : base8_numeric<uint8_t>() { }
                    KUMO_FORCE_INLINE simd8(const __m128i _value)
                        : base8_numeric<uint8_t>(_value) { }

                    // Splat constructor
                    KUMO_FORCE_INLINE simd8(uint8_t _value)
                        : simd8(splat(_value)) { }
                    // Array constructor
                    KUMO_FORCE_INLINE simd8(const uint8_t* values)
                        : simd8(load(values)) { }
                    // Member-by-member initialization
                    KUMO_FORCE_INLINE
                    simd8(uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v3, uint8_t v4, uint8_t v5,
                        uint8_t v6, uint8_t v7, uint8_t v8, uint8_t v9, uint8_t v10,
                        uint8_t v11, uint8_t v12, uint8_t v13, uint8_t v14, uint8_t v15)
                        : simd8(_mm_setr_epi8(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11,
                              v12, v13, v14, v15)) { }

                    // Saturated math
                    KUMO_FORCE_INLINE simd8<uint8_t>
                    saturating_sub(const simd8<uint8_t> other) const {
                        return _mm_subs_epu8(*this, other);
                    }

                    // Order-specific operations
                    KUMO_FORCE_INLINE simd8<uint8_t>
                    min_val(const simd8<uint8_t> other) const {
                        return _mm_min_epu8(*this, other);
                    }
                    // Same as >, but only guarantees true is nonzero (< guarantees true = -1)
                    KUMO_FORCE_INLINE simd8<uint8_t>
                    gt_bits(const simd8<uint8_t> other) const {
                        return this->saturating_sub(other);
                    }
                    // Same as <, but only guarantees true is nonzero (< guarantees true = -1)
                    KUMO_FORCE_INLINE simd8<bool>
                    operator>=(const simd8<uint8_t> other) const {
                        return other.min_val(*this) == other;
                    }

                    // Bit-specific operations
                    KUMO_FORCE_INLINE simd8<bool> bits_not_set() const {
                        return *this == uint8_t(0);
                    }
                    KUMO_FORCE_INLINE simd8<bool> any_bits_set() const {
                        return ~this->bits_not_set();
                    }
                    KUMO_FORCE_INLINE bool is_ascii() const {
                        return _mm_movemask_epi8(*this) == 0;
                    }

                    KUMO_FORCE_INLINE bool bits_not_set_anywhere() const {
                        return _mm_testz_si128(*this, *this);
                    }
                    KUMO_FORCE_INLINE bool any_bits_set_anywhere() const {
                        return !bits_not_set_anywhere();
                    }
                    template <int N>
                    KUMO_FORCE_INLINE simd8<uint8_t> shr() const {
                        return simd8<uint8_t>(_mm_srli_epi16(*this, N)) & uint8_t(0xFFu >> N);
                    }
                    template <int N>
                    KUMO_FORCE_INLINE simd8<uint8_t> shl() const {
                        return simd8<uint8_t>(_mm_slli_epi16(*this, N)) & uint8_t(0xFFu << N);
                    }

                    KUMO_FORCE_INLINE uint64_t sum_bytes() const {
                        const auto tmp = _mm_sad_epu8(value, _mm_setzero_si128());
                        return _mm_extract_epi64(tmp, 0) + _mm_extract_epi64(tmp, 1);
                    }
                };

                KUMO_FORCE_INLINE simd8<int8_t>::operator simd8<uint8_t>() const {
                    return this->value;
                }

                template <typename T>
                struct simd8x64 {
                    static constexpr int NUM_CHUNKS = 64 / sizeof(simd8<T>);
                    static_assert(NUM_CHUNKS == 4,
                        "Westmere kernel should use four registers per 64-byte block.");
                    simd8<T> chunks[NUM_CHUNKS];

                    simd8x64(const simd8x64<T>& o) = delete; // no copy allowed
                    simd8x64<T>&
                    operator=(const simd8<T> other)
                        = delete; // no assignment allowed
                    simd8x64() = delete; // no default constructor allowed

                    KUMO_FORCE_INLINE simd8x64(const simd8<T> chunk0, const simd8<T> chunk1,
                        const simd8<T> chunk2, const simd8<T> chunk3)
                        : chunks { chunk0, chunk1, chunk2, chunk3 } { }
                    KUMO_FORCE_INLINE simd8x64(const T* ptr)
                        : chunks { simd8<T>::load(ptr),
                            simd8<T>::load(ptr + sizeof(simd8<T>) / sizeof(T)),
                            simd8<T>::load(ptr + 2 * sizeof(simd8<T>) / sizeof(T)),
                            simd8<T>::load(ptr + 3 * sizeof(simd8<T>) / sizeof(T)) } { }

                    KUMO_FORCE_INLINE void store(T* ptr) const {
                        this->chunks[0].store(ptr + sizeof(simd8<T>) * 0 / sizeof(T));
                        this->chunks[1].store(ptr + sizeof(simd8<T>) * 1 / sizeof(T));
                        this->chunks[2].store(ptr + sizeof(simd8<T>) * 2 / sizeof(T));
                        this->chunks[3].store(ptr + sizeof(simd8<T>) * 3 / sizeof(T));
                    }

                    KUMO_FORCE_INLINE simd8x64<T>& operator|=(const simd8x64<T>& other) {
                        this->chunks[0] |= other.chunks[0];
                        this->chunks[1] |= other.chunks[1];
                        this->chunks[2] |= other.chunks[2];
                        this->chunks[3] |= other.chunks[3];
                        return *this;
                    }

                    KUMO_FORCE_INLINE simd8<T> reduce_or() const {
                        return (this->chunks[0] | this->chunks[1]) | (this->chunks[2] | this->chunks[3]);
                    }

                    KUMO_FORCE_INLINE bool is_ascii() const {
                        return this->reduce_or().is_ascii();
                    }

                    template <Endian endian>
                    KUMO_FORCE_INLINE void store_ascii_as_utf16(char16_t* ptr) const {
                        this->chunks[0].template store_ascii_as_utf16<endian>(ptr + sizeof(simd8<T>) * 0);
                        this->chunks[1].template store_ascii_as_utf16<endian>(ptr + sizeof(simd8<T>) * 1);
                        this->chunks[2].template store_ascii_as_utf16<endian>(ptr + sizeof(simd8<T>) * 2);
                        this->chunks[3].template store_ascii_as_utf16<endian>(ptr + sizeof(simd8<T>) * 3);
                    }

                    KUMO_FORCE_INLINE void store_ascii_as_utf32(char32_t* ptr) const {
                        this->chunks[0].store_ascii_as_utf32(ptr + sizeof(simd8<T>) * 0);
                        this->chunks[1].store_ascii_as_utf32(ptr + sizeof(simd8<T>) * 1);
                        this->chunks[2].store_ascii_as_utf32(ptr + sizeof(simd8<T>) * 2);
                        this->chunks[3].store_ascii_as_utf32(ptr + sizeof(simd8<T>) * 3);
                    }

                    KUMO_FORCE_INLINE uint64_t to_bitmask() const {
                        uint64_t r0 = uint32_t(this->chunks[0].to_bitmask());
                        uint64_t r1 = this->chunks[1].to_bitmask();
                        uint64_t r2 = this->chunks[2].to_bitmask();
                        uint64_t r3 = this->chunks[3].to_bitmask();
                        return r0 | (r1 << 16) | (r2 << 32) | (r3 << 48);
                    }

                    KUMO_FORCE_INLINE uint64_t lt(const T m) const {
                        const simd8<T> mask = simd8<T>::splat(m);
                        return simd8x64<bool>(this->chunks[0] < mask, this->chunks[1] < mask,
                            this->chunks[2] < mask, this->chunks[3] < mask)
                            .to_bitmask();
                    }

                    KUMO_FORCE_INLINE uint64_t gt(const T m) const {
                        const simd8<T> mask = simd8<T>::splat(m);
                        return simd8x64<bool>(this->chunks[0] > mask, this->chunks[1] > mask,
                            this->chunks[2] > mask, this->chunks[3] > mask)
                            .to_bitmask();
                    }
                    KUMO_FORCE_INLINE uint64_t gteq(const T m) const {
                        const simd8<T> mask = simd8<T>::splat(m);
                        return simd8x64<bool>(this->chunks[0] >= mask, this->chunks[1] >= mask,
                            this->chunks[2] >= mask, this->chunks[3] >= mask)
                            .to_bitmask();
                    }
                    KUMO_FORCE_INLINE uint64_t eq(const T m) const {
                        const simd8<T> mask = simd8<T>::splat(m);
                        return simd8x64<bool>(this->chunks[0] == mask, this->chunks[1] == mask,
                            this->chunks[2] == mask, this->chunks[3] == mask)
                            .to_bitmask();
                    }

                    KUMO_FORCE_INLINE uint64_t gteq_unsigned(const uint8_t m) const {
                        const simd8<uint8_t> mask = simd8<uint8_t>::splat(m);
                        return simd8x64<bool>(simd8<uint8_t>(__m128i(this->chunks[0])) >= mask,
                            simd8<uint8_t>(__m128i(this->chunks[1])) >= mask,
                            simd8<uint8_t>(__m128i(this->chunks[2])) >= mask,
                            simd8<uint8_t>(__m128i(this->chunks[3])) >= mask)
                            .to_bitmask();
                    }
                }; // struct simd8x64<T>

#include <turbo/unicode/engine/westmere/simd16-inl.h>
#include <turbo/unicode/engine/westmere/simd32-inl.h>
#include <turbo/unicode/engine/westmere/simd64-inl.h>

                KUMO_FORCE_INLINE simd64<uint64_t> sum_8bytes(const simd8<uint8_t> v) {
                    return _mm_sad_epu8(v.value, simd8<uint8_t>::zero());
                }

                KUMO_FORCE_INLINE simd8<uint8_t> as_vector_u8(const simd32<uint32_t> v) {
                    return simd8<uint8_t>(v.value);
                }

            } // namespace simd
        } // unnamed namespace
    } // namespace UNICODE_IMPLEMENTATION
} // namespace turbo

#endif // UNICODE_WESTMERE_SIMD_INPUT_H
