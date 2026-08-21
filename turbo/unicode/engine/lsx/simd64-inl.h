template <typename T>
struct simd64;

template <>
struct simd64<uint64_t> {
    __m128i value;
    static const int SIZE = sizeof(value);
    static const int ELEMENTS = SIZE / sizeof(uint64_t);

    // constructors
    KUMO_FORCE_INLINE simd64(__m128i v)
        : value(v) { }

    template <typename Ptr>
    KUMO_FORCE_INLINE simd64(Ptr* ptr)
        : value(__lsx_vld(ptr, 0)) { }

    // in-place operators
    KUMO_FORCE_INLINE simd64& operator+=(const simd64 other) {
        value = __lsx_vadd_d(value, other.value);
        return *this;
    }

    // members
    KUMO_FORCE_INLINE uint64_t sum() const {
        return uint64_t(__lsx_vpickve2gr_du(value, 0)) + uint64_t(__lsx_vpickve2gr_du(value, 1));
    }

    // static members
    static KUMO_FORCE_INLINE simd64<uint64_t> zero() {
        return __lsx_vrepli_d(0);
    }
};

// ------------------------------------------------------------

template <>
struct simd64<bool> {
    __m128i value;
    static const int SIZE = sizeof(value);

    // constructors
    KUMO_FORCE_INLINE simd64(__m128i v)
        : value(v) { }
};

// ------------------------------------------------------------

simd64<uint64_t> sum_8bytes(const simd8<uint8_t> v) {
    const auto sum_u16 = __lsx_vhaddw_hu_bu(v, v);
    const auto sum_u32 = __lsx_vhaddw_wu_hu(sum_u16, sum_u16);
    const auto sum_u64 = __lsx_vhaddw_du_wu(sum_u32, sum_u32);

    return simd64<uint64_t>(sum_u64);
}
