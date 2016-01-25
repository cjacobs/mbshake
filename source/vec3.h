#pragma once
#include "fastmath.h"

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

    // math
    void operator=(const vec3<T>& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
    }

    void operator+=(const vec3<T>& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
    }

    void operator-=(const vec3<T>& v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
    }
};


// vector arithmetic

template <typename T>
vec3<T> operator+(const vec3<T>& a, const vec3<T>& b)
{
    return vec3<T>(a.x+b.x, a.y+b.y, a.z+b.z);
}

template <typename T>
vec3<T> operator-(const vec3<T>& a, const vec3<T>& b)
{
    return vec3<T>(a.x-b.x, a.y-b.y, a.z-b.z);
}

template <typename T>
vec3<T> operator*(float a, const vec3<T>& b)
{
    return vec3<T>(a*b.x, a*b.y, a*b.z);
}

template <typename T>
vec3<T> operator*(const vec3<T>& b, float a)
{
    return vec3<T>(a*b.x, a*b.y, a*b.z);
}

// doofy accesor functions

template <typename T>
class GetX
{
public:
    static T get_val(const vec3<T>& vec)
    {
        return vec.x;
    }
};

template <typename T>
class GetY
{
public:
    static T get_val(const vec3<T>& vec)
    {
        return vec.y;
    }
};

template <typename T>
class GetZ
{
public:
    static T get_val(const vec3<T>& vec)
    {
        return vec.z;
    }
};

template <typename T, typename U=T>
class GetMagSq
{
public:
    static U get_val(const vec3<T>& vec)
    {
        U temp = U(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
        return temp;
    }
};


typedef vec3<int8_t> byteVec3;
typedef vec3<short> shortVec3;
typedef vec3<float> floatVec3;

template<typename T>
float dot(const vec3<T>& a, const vec3<T>& b)
{
    return float(a.x*b.x + a.y*b.y + a.z*b.z);
}

template <typename T>
float normSq(const vec3<T>& v)
{
    return dot(v,v);
}

template <typename T>
float norm(const vec3<T>& v)
{
    return 1.0 / fast_inv_sqrt(normSq(v));
}

template <typename T>
float dotNorm(const vec3<T>& a, const vec3<T>& b, float minLenThresh)
{
    float aLenSq = dot(a,a);
    float bLenSq = dot(b,b);

    if (aLenSq < minLenThresh || bLenSq < minLenThresh)
    {
        return 0;
    }

    float bdota = dot(a,b);
    return bdota * fast_inv_sqrt(aLenSq) * fast_inv_sqrt(bLenSq);
}

template <typename T>
float perpNorm(const vec3<T>& a, const vec3<T>& b, float minLenThresh)
{
    float aLenSq = dot(a,a);
    float bLenSq = dot(b,b);

    if (aLenSq < minLenThresh || bLenSq < minLenThresh)
    {
        return 0;
    }

    float bdota = dot(a,b);
    float bPerpScale = bdota / aLenSq;
    floatVec3 bPerpVec = (floatVec3(b) - bPerpScale * floatVec3(a)) * fast_inv_sqrt(bLenSq);

    return norm(bPerpVec); // this could surely be faster
}

