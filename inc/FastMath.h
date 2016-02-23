#pragma once

#include <cstdint> // for int8_t, int32_t types
#include <cmath>

template <typename T>
int8_t clampByte(const T& inVal)
{
    return inVal < -128 ? -128 : inVal > 127 ? 127 : (int8_t)inVal;
}

// from http://stackoverflow.com/a/19807644
template <typename T, typename F>
struct alias_cast_t
{
    union
    {
        F raw;
        T data;
    };
};

template<typename T, typename F>
T alias_cast(F raw_value)
{
    alias_cast_t<T, F> c;
    c.raw = raw_value;
    return c.data;
}

// adapted from https://en.wikipedia.org/wiki/Fast_inverse_square_root
inline float fast_inv_sqrt(float val)
{
    const float threehalfs = 1.5F;

    float x2 = val * 0.5F;
    float y = val;

    int32_t i = alias_cast<long>(y);
    i = 0x5f3759df - (i >> 1);
    y = alias_cast<float>(i);

    y = y * (threehalfs - (x2 * y * y)); // repeat as necessary

    return y;
}

// adapted from http://h14s.p5r.org/2012/09/0x5f3759df.html
inline float fast_sqrt(float val)
{
    float y = val;

    int32_t i = alias_cast<long>(y);
    i = 0x1fbd1df5 + (i >> 1);
    y = alias_cast<float>(i);

    // TODO: figure out right eqn for newton step here
    //	y  = y * ( threehalfs - ( x2 * y * y ) ); // repeat as necessary

    y = 0.5f*(y + (val / y)); // yuck! divide
    return y;
}


