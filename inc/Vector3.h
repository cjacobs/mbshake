#pragma once
#include "FastMath.h"
#include "FixedPt.h"

#include <type_traits>

template <typename T>
struct Vector3
{
    T x;
    T y;
    T z;

    Vector3<T>() : x(0), y(0), z(0) {}
    Vector3<T>(T x, T y, T z) : x(x), y(y), z(z) {}

    template <typename S>
    explicit Vector3<T>(const Vector3<S>& v)
    {
        x = T(v.x);
        y = T(v.y);
        z = T(v.z);
    }

    // math
    void operator=(const Vector3<T>& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
    }

    void operator+=(const Vector3<T>& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
    }

    template <typename S=T>
    typename std::enable_if<!std::is_floating_point<S>::value, void>::type
    operator*=(S s)
    {
        x *= s;
        y *= s;
        z *= s;
    }

    void operator*=(float s)
    {
        x *= s;
        y *= s;
        z *= s;
    }

    void operator/=(float d)
    {
        x /= d;
        y /= d;
        z /= d;
    }

    void operator-=(const Vector3<T>& v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
    }
};


// vector arithmetic

template <typename T>
Vector3<T> operator+(const Vector3<T>& a, const Vector3<T>& b)
{
    return Vector3<T>(a.x+b.x, a.y+b.y, a.z+b.z);
}

template <typename T>
Vector3<T> operator-(const Vector3<T>& a, const Vector3<T>& b)
{
    return Vector3<T>(a.x-b.x, a.y-b.y, a.z-b.z);
}

template <typename S, typename T>
Vector3<T> operator*(S a, const Vector3<T>& b)
{
    return Vector3<T>(a*b.x, a*b.y, a*b.z);
}

template <typename S, typename T>
Vector3<T> operator*(const Vector3<T>& b, S a)
{
    return Vector3<T>(b.x*a, b.y*a, b.z*a);
}

template <typename T>
Vector3<T> operator/(const Vector3<T>& b, float a)
{
    return Vector3<T>(b.x / a, b.y / a, b.z / a);
}

// doofy accesor functions

template <typename T>
class GetX
{
public:
    static T get_val(const Vector3<T>& vec)
    {
        return vec.x;
    }
};

template <typename T>
class GetY
{
public:
    static T get_val(const Vector3<T>& vec)
    {
        return vec.y;
    }
};

template <typename T>
class GetZ
{
public:
    static T get_val(const Vector3<T>& vec)
    {
        return vec.z;
    }
};

template <typename T, typename U=T>
class GetMagSq
{
public:
    static U get_val(const Vector3<T>& vec)
    {
        U temp = U(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
        return temp;
    }
};

template <typename T>
struct DotType{};

template <> struct DotType<int8_t>
{
public:
    using type = float;
};

template <> struct DotType<int16_t>
{
public:
    using type = float;
};

template <> struct DotType<int32_t>
{
public:
    using type = float;
};

template <> struct DotType<int64_t>
{
public:
    using type = float;
};

template <> struct DotType<float>
{
public:
    using type = float;
};

template <> struct DotType<double>
{
public:
    using type = double;
};

template<int I, int F, typename T> struct DotType<FixedPt<I, F, T>>
{
 public:
    using type = FixedPt<2*I+2, F-I-2, T>; // dot product has to be able to hold 3*x^2, which means 2+2(I) integer bits
};

typedef Vector3<int8_t> byteVector3;
typedef Vector3<float> floatVector3;

template <typename T>
typename DotType<T>::type dot(const Vector3<T>& a, const Vector3<T>& b)
{
    using dot_t = typename DotType<T>::type;
    auto xPart = dot_t(a.x) * dot_t(b.x);
    auto yPart = dot_t(a.y) * dot_t(b.y);
    auto zPart = dot_t(a.z) * dot_t(b.z);
    auto result = typename DotType<T>::type(xPart + yPart + zPart);
    return result;
}

/*
template <typename T>
float dot(const Vector3<T>& a, const Vector3<T>& b)
{
    return float(a.x*b.x + a.y*b.y + a.z*b.z);
}
*/

template <typename T>
typename DotType<T>::type normSq(const Vector3<T>& v)
{
    return dot(v,v);
}

template <typename T>
float norm(const Vector3<T>& v)
{
    return 1.0 / fast_inv_sqrt(normSq(v));
}

template <typename T>
float dotNorm(const Vector3<T>& a, const Vector3<T>& b)
{
    float aLenSq = normSq(a);
    float bLenSq = normSq(b);

    if (aLenSq == T(0) || bLenSq == T(0))
    {
        return 0; // infinity, really
    }

    float bdota = dot(a,b);
    return bdota * fast_inv_sqrt(aLenSq*bLenSq);
}

template <typename T>
float dotNorm(const Vector3<T>& a, const Vector3<T>& b, float minLenThresh)
{
    float aLenSq = normSq(a);
    float bLenSq = normSq(b);

    if (aLenSq < minLenThresh || bLenSq < minLenThresh)
    {
        return 0;
    }

    float bdota = dot(a,b);
    return bdota * fast_inv_sqrt(aLenSq*bLenSq);
}

// dotNorm must be in [-1,1]
template <int IParam, int FParam, typename T>
FixedPt<2,(IParam+FParam)-2,T> dotNormFixed(const Vector3<FixedPt<IParam,FParam,T>>& a, const Vector3<FixedPt<IParam,FParam,T>>& b)
{
    // ugh, aLenSq and bLenSq will overflow their datatypes
    auto aLenSq = normSq(a);
    auto bLenSq = normSq(b);

    if (aLenSq == 0 || bLenSq == 0)
    {
        return FixedPt<2, (IParam+FParam)-2, T>(0); // TODO: ~0?
    }

auto denom = aLenSq*bLenSq; // yikes, we're going to double the # of int bits again
    auto bdota = dot(a,b); // ugh, need dot-product (and multiply) that returns fixed pt thing of correct size
    return bdota * denom.inv_sqrt(); // somehow do this intelligently
}

// dotNorm must be in [-1,1]
template <int I, int F, typename T>
FixedPt<I,F,T> dotNormFixed(const Vector3<FixedPt<I,F,T>>& a, const Vector3<FixedPt<I,F,T>>& b, const FixedPt<I,F,T> minLenThresh)
{
    // ugh, aLenSq and bLenSq will overflow their datatypes
    auto aLenSq = normSq(a);
    auto bLenSq = normSq(b);

    if (aLenSq < minLenThresh || bLenSq < minLenThresh)
    {
        return FixedPt<I,F,T>(0);
    }

    fixed_16_0 bdota = dot(a,b); // ugh, need dot-product (and multiply) that returns fixed pt thing of correct size
    return bdota * fast_inv_sqrt(aLenSq*bLenSq); // somehow do this intelligently
}

template <typename T>
float perpNorm(const Vector3<T>& a, const Vector3<T>& b, float minLenThresh)
{
    float aLenSq = dot(a,a);
    float bLenSq = dot(b,b);

    if (aLenSq < minLenThresh || bLenSq < minLenThresh)
    {
        return 0;
    }

    float bdota = dot(a,b);
    float bPerpScale = bdota / aLenSq;
    floatVector3 bPerpVec = (floatVector3(b) - bPerpScale * floatVector3(a)) * fast_inv_sqrt(bLenSq);

    return norm(bPerpVec); // this could surely be faster
}

