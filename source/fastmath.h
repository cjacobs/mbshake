#pragma once

#include <stdint.h>

template <typename T>
int8_t clampByte(const T& inVal)
{
    return inVal < -127 ? -127 : inVal > 128 ? 128 : inVal;
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

// TODO: implement fixed-point square root / inv square root
// http://stackoverflow.com/questions/6286450/inverse-sqrt-for-fixed-point
// also: http://www.realitypixels.com/turk/computergraphics/FixedSqrt.pdf


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
	float y  = val;
    long i  = alias_cast<long>(y);
    i  = 0x5f3759df - ( i >> 1 );
    y = alias_cast<float>(i);
	y  = y * ( threehalfs - ( x2 * y * y ) );
//	y  = y * ( threehalfs - ( x2 * y * y ) );

	return y;
}

template <typename T, int Mbits>
class fixed_pt
{
public:
    fixed_pt(int val) : value_(val << Mbits) {}
    fixed_pt(float val) : value_(val*(1 << Mbits)) {}

    // TODO: arithmetic ops: +, -, *, / (?)

private:
    T value_;
};
