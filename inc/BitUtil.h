#pragma once

#include <cstdint>

template <int Amount, typename T>
constexpr T ShiftLeft(T x)
{
    return (Amount >= 0) ? (x << Amount) : (x >> (-Amount));
}

template <typename T>
T ShiftLeft(T x, int amount)
{
    return (amount >= 0) ? (x << amount) : (x >> (-amount));
}

template <int Amount, typename T>
constexpr T ShiftRight(T x)
{
    return (Amount >= 0) ? (x >> Amount) : (x << (-Amount));
}

template <typename T>
T ShiftRight(T x, int amount)
{
    return (amount >= 0) ? (x >> amount) : (x << (-amount));
}

template <int M1, int M2>
constexpr int ShiftValue(int x)
{
    return (M1 > M2) ? (x << (M1 - M2)) : (x >> (M2 - M1));
}

inline int leading_zeros (uint16_t a)
{
    uint32_t r = 16;
    if (a >= 0x00000100) { a >>=  8; r -=  8; }
    if (a >= 0x00000010) { a >>=  4; r -=  4; }
    if (a >= 0x00000004) { a >>=  2; r -=  2; }
    r -= a - (a & (a >> 1));
    return r;
}

inline int leading_zeros (uint32_t a)
{
    uint32_t r = 32;
    if (a >= 0x00010000) { a >>= 16; r -= 16; }
    if (a >= 0x00000100) { a >>=  8; r -=  8; }
    if (a >= 0x00000010) { a >>=  4; r -=  4; }
    if (a >= 0x00000004) { a >>=  2; r -=  2; }
    r -= a - (a & (a >> 1));
    return r;
}

//
// int_of_size
//
template <int S> struct int_of_size {};

template <>
struct int_of_size<8>
{
    using type = int8_t;
    using utype = uint8_t;
};

template <>
struct int_of_size<16>
{
    using type = int16_t;
    using utype = uint16_t;
};

template <>
struct int_of_size<32>
{
    using type = int32_t;
    using utype = uint32_t;
};


//
// num_bits
//
template <typename T> struct num_bits {};

template <>
struct num_bits<int8_t>
{
    static constexpr int value = 8;
};

template <>
struct num_bits<uint8_t>
{
    static constexpr int value = 8;
};

template <>
struct num_bits<int16_t>
{
    static constexpr int value = 16;
};

template <>
struct num_bits<uint16_t>
{
    static constexpr int value = 16;
};

template <>
struct num_bits<int32_t>
{
    static constexpr int value = 32;
};

template <>
struct num_bits<uint32_t>
{
    static constexpr int value = 32;
};

//
// next_bigger_int
//
template <typename T>
struct next_bigger_int
{
};

template <>
struct next_bigger_int<int8_t>
{
public:
    typedef int16_t type;
};

template <>
struct next_bigger_int<uint8_t>
{
public:
    typedef uint16_t type;
};

template <>
struct next_bigger_int<int16_t>
{
public:
    typedef int32_t type;
};

template <>
struct next_bigger_int<uint16_t>
{
public:
    typedef uint32_t type;
};

template <>
struct next_bigger_int<int32_t>
{
public:
    typedef int64_t type;
};

template <>
struct next_bigger_int<uint32_t>
{
public:
    typedef uint64_t type;
};

