#pragma once

template <typename T>
struct vec3
{
    T x;
    T y;
    T z;

    vec3<T>() : x(0), y(0), z(0) {}
    vec3<T>(T x, T y, T z) : x(x), y(y), z(z) {}

    template <typename S>
    explicit vec3<T>(const vec3<S>& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
    }
};

typedef vec3<int8_t> byteVec3;
typedef vec3<short> shortVec3;
typedef vec3<float> floatVec3;

// adapted from https://en.wikipedia.org/wiki/Fast_inverse_square_root
float fast_inv_sqrt(float val)
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

template<typename T>
float dot(const vec3<T>& a, const vec3<T>& b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

template <typename T>
float dotNorm(const vec3<T>& a, const vec3<T>& b, int minLenThresh)
{
    float val = dot(a,b);
    float aLenSq = dot(a,a);
    float bLenSq = dot(b,b);

    if (aLenSq < minLenThresh || bLenSq < minLenThresh)
    {
        return 0;
    }

    //    return val / (sqrt(aLenSq)*sqrt(bLenSq));
    return val * fast_inv_sqrt(aLenSq) * fast_inv_sqrt(bLenSq);
}
