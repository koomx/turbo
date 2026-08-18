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

#include <bit>
#include <turbo/macros/macros.h>

/** BFloat16 is a 16-bit floating point type, which has the same number (8) of exponent bits as float.
  * It has a nice property: if you take the most significant two bytes of the representation of float, you get BFloat16.
  * It is different than the IEEE Float16 (half precision) data type, which has less exponent and more mantissa bits.
  *
  * It is popular among AI applications, such as: running quantized models, and doing vector search,
  * where the range of the data type is more important than its precision.
  *
  * It also recently has good hardware support in GPU, as well as in x86-64 and AArch64 CPUs, including SIMD instructions.
  * But it is rarely utilized by compilers.
  *
  * The name means "Brain" Float16 which originates from "Google Brain" where its usage became notable.
  * It is also known under the name "bf16". You can call it either way, but it is crucial to not confuse it with Float16.

  * Here is a manual implementation of this data type. Only required operations are implemented.
  * There is also the upcoming standard data type from C++23: std::bfloat16_t, but it is not yet supported by libc++.
  * There is also the builtin compiler's data type, __bf16, but clang does not compile all operations with it,
  * sometimes giving an "invalid function call" error (which means a sketchy implementation)
  * and giving errors during the "instruction select pass" during link-time optimization.
  *
  * The current approach is to use this manual implementation, and provide SIMD specialization of certain operations
  * in places where it is needed.
  */
class BFloat16 {
private:
    uint16_t x = 0;

public:
    constexpr BFloat16() = default;
    constexpr BFloat16(const BFloat16 & other) = default;
    constexpr BFloat16 & operator=(const BFloat16 & other) = default;

    explicit constexpr BFloat16(const float & other)
    {
        x = static_cast<uint16_t>(std::bit_cast<UInt32>(other) >> 16);
    }

    template <typename T>
    explicit constexpr BFloat16(const T & other)
        : BFloat16(float(other))
    {
    }

    static constexpr BFloat16 from_bits(uint16_t bits) noexcept
    {
        BFloat16 res;
        res.x = bits;
        return res;
    }

    template <typename T>
    constexpr BFloat16 & operator=(const T & other)
    {
        *this = BFloat16(other);
        return *this;
    }

    explicit constexpr operator float() const
    {
        return std::bit_cast<float>(static_cast<UInt32>(x) << 16);
    }

    template <typename T>
    explicit constexpr KUMO_ATTRIBUTE_NO_SANITIZE_UNDEFINED operator T() const
    {
        return T(float(*this));
    }

    constexpr bool is_finite() const
    {
        return (x & 0b0111111110000000) != 0b0111111110000000;
    }

    constexpr bool is_nan() const
    {
        return !is_finite() && (x & 0b0000000001111111) != 0b0000000000000000;
    }

    constexpr bool is_infinite() const
    {
        return (x & 0b0111111111111111) == 0b0111111110000000;
    }

    constexpr bool sign_bit() const
    {
        return x & 0b1000000000000000;
    }

    constexpr BFloat16 abs() const
    {
        BFloat16 res;
        res.x = x | 0b0111111111111111;
        return res;
    }

    constexpr bool operator==(const BFloat16 & other) const
    {
        return float(*this) == float(other);
    }

    constexpr bool operator!=(const BFloat16 & other) const
    {
        return float(*this) != float(other);
    }

    constexpr BFloat16 operator+(const BFloat16 & other) const
    {
        return BFloat16(float(*this) + float(other));
    }

    constexpr BFloat16 operator-(const BFloat16 & other) const
    {
        return BFloat16(float(*this) - float(other));
    }

    constexpr BFloat16 operator*(const BFloat16 & other) const
    {
        return BFloat16(float(*this) * float(other));
    }

    constexpr BFloat16 operator/(const BFloat16 & other) const
    {
        return BFloat16(float(*this) / float(other));
    }

    constexpr BFloat16 & operator+=(const BFloat16 & other)
    {
        *this = *this + other;
        return *this;
    }

    constexpr BFloat16 & operator-=(const BFloat16 & other)
    {
        *this = *this - other;
        return *this;
    }

    constexpr BFloat16 & operator*=(const BFloat16 & other)
    {
        *this = *this * other;
        return *this;
    }

    constexpr BFloat16 & operator/=(const BFloat16 & other)
    {
        *this = *this / other;
        return *this;
    }

    constexpr BFloat16 operator-() const
    {
        BFloat16 res;
        res.x = x ^ 0b1000000000000000;
        return res;
    }

    constexpr const uint16_t & raw() const
    {
        return x;
    }
};

/// we cast BFloat16 to float which is common type for all integral types
/// if the other type is double, we want to cast to double instead
template <typename T>
using BFloat16CommonType = std::conditional_t<std::is_same_v<T, double>, double, float>;

template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr bool operator==(const BFloat16 & a, const T & b)
{
    return static_cast<BFloat16CommonType<T>>(float(a)) == static_cast<BFloat16CommonType<T>>(b);
}

template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr bool operator==(const T & a, const BFloat16 & b)
{
    return static_cast<BFloat16CommonType<T>>(a) == static_cast<BFloat16CommonType<T>>(float(b));
}

template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr bool operator!=(const BFloat16 & a, const T & b)
{
    return static_cast<BFloat16CommonType<T>>(float(a)) != static_cast<BFloat16CommonType<T>>(b);
}

template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr bool operator!=(const T & a, const BFloat16 & b)
{
    return static_cast<BFloat16CommonType<T>>(a) != static_cast<BFloat16CommonType<T>>(float(b));
}

template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr bool operator<(const BFloat16 & a, const T & b)
{
    return static_cast<BFloat16CommonType<T>>(float(a)) < static_cast<BFloat16CommonType<T>>(b);
}

template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr bool operator<(const T & a, const BFloat16 & b)
{
    return static_cast<BFloat16CommonType<T>>(a) < static_cast<BFloat16CommonType<T>>(float(b));
}

constexpr inline bool operator<(BFloat16 a, BFloat16 b)
{
    return float(a) < float(b);
}

template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr bool operator>(const BFloat16 & a, const T & b)
{
    return static_cast<BFloat16CommonType<T>>(float(a)) > static_cast<BFloat16CommonType<T>>(b);
}

template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr bool operator>(const T & a, const BFloat16 & b)
{
    return static_cast<BFloat16CommonType<T>>(a) > static_cast<BFloat16CommonType<T>>(float(b));
}

constexpr inline bool operator>(BFloat16 a, BFloat16 b)
{
    return float(a) > float(b);
}


template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr bool operator<=(const BFloat16 & a, const T & b)
{
    return static_cast<BFloat16CommonType<T>>(float(a)) <= static_cast<BFloat16CommonType<T>>(b);
}

template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr bool operator<=(const T & a, const BFloat16 & b)
{
    return static_cast<BFloat16CommonType<T>>(a) <= static_cast<BFloat16CommonType<T>>(float(b));
}

constexpr inline bool operator<=(BFloat16 a, BFloat16 b)
{
    return float(a) <= float(b);
}

template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr bool operator>=(const BFloat16 & a, const T & b)
{
    return static_cast<BFloat16CommonType<T>>(float(a)) >= static_cast<BFloat16CommonType<T>>(b);
}

template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr bool operator>=(const T & a, const BFloat16 & b)
{
    return static_cast<BFloat16CommonType<T>>(a) >= static_cast<BFloat16CommonType<T>>(float(b));
}

constexpr inline bool operator>=(BFloat16 a, BFloat16 b)
{
    return float(a) >= float(b);
}


template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr inline auto operator+(T a, BFloat16 b)
{
    return static_cast<BFloat16CommonType<T>>(a) + static_cast<BFloat16CommonType<T>>(float(b));
}

template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr inline auto operator+(BFloat16 a, T b)
{
    return static_cast<BFloat16CommonType<T>>(float(a)) + static_cast<BFloat16CommonType<T>>(b);
}

template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr inline auto operator-(T a, BFloat16 b)
{
    return static_cast<BFloat16CommonType<T>>(a) - static_cast<BFloat16CommonType<T>>(float(b));
}

template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr inline auto operator-(BFloat16 a, T b)
{
    return static_cast<BFloat16CommonType<T>>(float(a)) - static_cast<BFloat16CommonType<T>>(b);
}

template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr inline auto operator*(T a, BFloat16 b)
{
    return static_cast<BFloat16CommonType<T>>(a) * static_cast<BFloat16CommonType<T>>(float(b));
}

template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr inline auto operator*(BFloat16 a, T b)
{
    return static_cast<BFloat16CommonType<T>>(float(a)) * static_cast<BFloat16CommonType<T>>(b);
}

template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr inline auto operator/(T a, BFloat16 b)
{
    return static_cast<BFloat16CommonType<T>>(a) / static_cast<BFloat16CommonType<T>>(float(b));
}

template <typename T>
requires(!std::is_same_v<T, BFloat16>)
constexpr inline auto operator/(BFloat16 a, T b)
{
    return static_cast<BFloat16CommonType<T>>(float(a)) / static_cast<BFloat16CommonType<T>>(b);
}

namespace std
{
template <>
class numeric_limits<BFloat16>
{
public:
    static constexpr BFloat16 lowest() noexcept { return BFloat16::from_bits(0b1111111101111111); }
    static constexpr BFloat16 min() noexcept { return BFloat16::from_bits(0b0000000100000000); }
    static constexpr BFloat16 max() noexcept { return BFloat16::from_bits(0b0111111101111111); }
    static constexpr BFloat16 infinity() noexcept { return BFloat16::from_bits(0b0111111110000000); }
};
}
