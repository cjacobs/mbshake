#pragma once

#include <stdint.h>

template <typename T>
int8_t clampByte(const T& inVal)
{
    return inVal < -127 ? -127 : inVal > 128 ? 128 : inVal;
}

// adapted from https://en.wikipedia.org/wiki/Fast_inverse_square_root
inline float fast_inv_sqrt(float val)
{
	const float threehalfs = 1.5F;

	float x2 = val * 0.5F;
	float y  = val;
	long i  = * reinterpret_cast<long *>(&y);
	i  = 0x5f3759df - ( i >> 1 );
	y  = *reinterpret_cast<float *>(&i);
	y  = y * ( threehalfs - ( x2 * y * y ) );
//	y  = y * ( threehalfs - ( x2 * y * y ) );

	return y;
}
