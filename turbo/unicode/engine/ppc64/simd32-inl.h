// file included directly

template <typename T>
struct simd32;

template <typename T>
struct base32 {
    using vector_type = vector_u32_type_for_element<T>;
    static const int SIZE = sizeof(vector_type);
    static const int ELEMENTS = sizeof(vector_type) / sizeof(T);

    vector_type value;

    // Zero constructor
    KUMO_FORCE_INLINE base32()
        : value { vector_type() } { }

    // Conversion from SIMD register
    KUMO_FORCE_INLINE base32(const vector_type _value)
        : value { _value } { }

    // Splat for scalar
    KUMO_FORCE_INLINE base32(T scalar)
        : value { vec_splats(scalar) } { }

    template <typename Pointer>
    KUMO_FORCE_INLINE base32(const Pointer* ptr)
        : base32(vec_xl(0, reinterpret_cast<const T*>(ptr))) { }

    // Store to array
    template <typename U>
    KUMO_FORCE_INLINE void store(U* dst) const {
#if defined(__clang__)
        return vec_xst(this->value, 0, reinterpret_cast<T*>(dst));
#else
        return vec_xst(this->value, 0, reinterpret_cast<vector_type*>(dst));
#endif // defined(__clang__)
    }
    void dump(const char* name = nullptr) const {
        if (name != nullptr) {
            printf("%-10s = ", name);
        }

        uint32_t tmp[4];
        vec_xst(value, 0, reinterpret_cast<vector_type*>(tmp));
        for (int i = 0; i < 4; i++) {
            if (i == 0) {
                printf("[%08x", tmp[i]);
            } else if (i == 4 - 1) {
                printf(" %08x]", tmp[i]);
            } else {
                printf(" %08x", tmp[i]);
            }
        }
        putchar('\n');
    }
};

template <typename T>
struct base32_numeric : base32<T> {
    using super = base32<T>;
    using vector_type = typename super::vector_type;

    static KUMO_FORCE_INLINE simd32<T> splat(T _value) {
        return vec_splats(_value);
    }

    static KUMO_FORCE_INLINE simd32<T> zero() { return splat(0); }

    template <typename U>
    static KUMO_FORCE_INLINE simd32<T> load(const U* values) {
        return vec_xl(0, reinterpret_cast<const T*>(values));
    }

    KUMO_FORCE_INLINE base32_numeric()
        : base32<T>() { }

    KUMO_FORCE_INLINE base32_numeric(const vector_type _value)
        : base32<T>(_value) { }

    // Addition/subtraction are the same for signed and unsigned
    KUMO_FORCE_INLINE simd32<T> operator+(const simd32<T> other) const {
        return vec_add(this->value, other.value);
    }

    KUMO_FORCE_INLINE simd32<T> operator-(const simd32<T> other) const {
        return vec_sub(this->value, other.value);
    }

    KUMO_FORCE_INLINE simd32<T>& operator+=(const simd32<T> other) {
        *this = *this + other;
        return *static_cast<simd32<T>*>(this);
    }

    KUMO_FORCE_INLINE simd32<T>& operator-=(const simd32<T> other) {
        *this = *this - other;
        return *static_cast<simd32<T>*>(this);
    }
};

// Forward declaration
template <typename>
struct simd32;

template <typename T>
simd32<bool> operator==(const simd32<T> a, const simd32<T> b);

template <typename T>
simd32<bool> operator!=(const simd32<T> a, const simd32<T> b);

template <typename T>
simd32<bool> operator>(const simd32<T> a, const simd32<T> b);

template <typename T>
simd32<bool> operator==(const simd32<T> a, T b);

template <typename T>
simd32<bool> operator!=(const simd32<T> a, T b);

template <typename T>
simd32<T> operator&(const simd32<T> a, const simd32<T> b);

template <typename T>
simd32<T> operator|(const simd32<T> a, const simd32<T> b);

template <typename T>
simd32<T> operator^(const simd32<T> a, const simd32<T> b);

// SIMD byte mask type (returned by things like eq and gt)
template <>
struct simd32<bool> : base32<bool> {
    static KUMO_FORCE_INLINE simd32<bool> splat(bool _value) {
        return (vector_type)vec_splats(uint32_t(-(!!_value)));
    }

    KUMO_FORCE_INLINE simd32(const vector_type _value)
        : base32<bool>(_value) { }

    // Splat constructor
    KUMO_FORCE_INLINE simd32(bool _value)
        : base32<bool>(splat(_value)) { }

    KUMO_FORCE_INLINE uint16_t to_bitmask() const {
        return move_mask_u8(value);
    }

    KUMO_FORCE_INLINE bool any() const {
        const vec_u64_t tmp = (vec_u64_t)value;

        return tmp[0] || tmp[1]; // Note: logical or, not binary one
    }

    KUMO_FORCE_INLINE bool is_zero() const {
        const vec_u64_t tmp = (vec_u64_t)value;

        return (tmp[0] | tmp[1]) == 0;
    }

    KUMO_FORCE_INLINE simd32<bool> operator~() const {
        return (vec_bool32_t)vec_xor(this->value, vec_splats(uint32_t(0xffffffff)));
    }
};

// Unsigned code units
template <>
struct simd32<uint32_t> : base32_numeric<uint32_t> {
    KUMO_FORCE_INLINE simd32()
        : base32_numeric<uint32_t>() { }

    KUMO_FORCE_INLINE simd32(const vector_type _value)
        : base32_numeric<uint32_t>(_value) { }

    // Splat constructor
    KUMO_FORCE_INLINE simd32(uint32_t _value)
        : simd32(splat(_value)) { }

    // Array constructor
    KUMO_FORCE_INLINE simd32(const char32_t* values)
        : simd32(load(reinterpret_cast<const uint32_t*>(values))) { }

    // Bit-specific operations
    template <int N>
    KUMO_FORCE_INLINE simd32<uint32_t> shr() const {
        return vec_sr(value, vec_splats(uint32_t(N)));
    }

    template <int N>
    KUMO_FORCE_INLINE simd32<uint32_t> shl() const {
        return vec_sl(value, vec_splats(uint32_t(N)));
    }

    // Change the Endian
    KUMO_FORCE_INLINE simd32<uint32_t> swap_bytes() const {
        return vec_revb(value);
    }

    KUMO_FORCE_INLINE uint64_t sum() const {
        return uint64_t(value[0]) + uint64_t(value[1]) + uint64_t(value[2]) + uint64_t(value[3]);
    }

    static KUMO_FORCE_INLINE simd16<uint16_t>
    pack(const simd32<uint32_t>& v0, const simd32<uint32_t>& v1) {
        return vec_packs(v0.value, v1.value);
    }
};

template <typename T>
simd32<bool> operator==(const simd32<T> a, const simd32<T> b) {
    return vec_cmpeq(a.value, b.value);
}

template <typename T>
simd32<bool> operator!=(const simd32<T> a, const simd32<T> b) {
    return vec_cmpne(a.value, b.value);
}

template <typename T>
simd32<bool> operator==(const simd32<T> a, T b) {
    return vec_cmpeq(a.value, vec_splats(b));
}

template <typename T>
simd32<bool> operator!=(const simd32<T> a, T b) {
    return vec_cmpne(a.value, vec_splats(b));
}

template <typename T>
simd32<bool> operator>(const simd32<T> a, const simd32<T> b) {
    return vec_cmpgt(a.value, b.value);
}

template <typename T>
simd32<bool> operator>=(const simd32<T> a, const simd32<T> b) {
    return vec_cmpge(a.value, b.value);
}

template <typename T>
simd32<T> operator&(const simd32<T> a, const simd32<T> b) {
    return vec_and(a.value, b.value);
}

template <typename T, typename U>
simd32<T> operator&(const simd32<T> a, U b) {
    return vec_and(a.value, vec_splats(T(b)));
}

template <typename T>
simd32<T> operator|(const simd32<T> a, const simd32<T> b) {
    return vec_or(a.value, b.value);
}

template <typename T>
simd32<T> operator^(const simd32<T> a, const simd32<T> b) {
    return vec_xor(a.value, b.value);
}

template <typename T, typename U>
simd32<T> operator^(const simd32<T> a, U b) {
    return vec_xor(a.value, vec_splats(T(b)));
}

template <typename T>
simd32<T> max_val(const simd32<T> a, const simd32<T> b) {
    return vec_max(a.value, b.value);
}

template <typename T>
KUMO_FORCE_INLINE simd32<T> min(const simd32<T> b, const simd32<T> a) {
    return vec_min(a.value, b.value);
}
