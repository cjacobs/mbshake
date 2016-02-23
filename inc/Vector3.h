#pragma once
#include "FastMath.h"
#include "FixedPt.h"

#include <type_traits>


#include <iostream>
using namespace std;





template <typename T>
class Vector3
{
public:
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

    template <typename S = T>
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
    return Vector3<T>(a.x + b.x, a.y + b.y, a.z + b.z);
}

template <typename T>
Vector3<T> operator-(const Vector3<T>& a, const Vector3<T>& b)
{
    return Vector3<T>(a.x - b.x, a.y - b.y, a.z - b.z);
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

// TODO: make version of this that returns fixed pt or something
template <typename T, typename U = T>
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
struct DotType {};

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
    using type = FixedPt<2 * I + 2, F - I - 2, T>; // dot product has to be able to hold 3*x^2, which means 2+2(I) integer bits
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
    return dot(v, v);
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

    float aDotB = dot(a, b);
    return aDotB * fast_inv_sqrt(aLenSq*bLenSq);
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

    float aDotB = dot(a, b);
    return aDotB * fast_inv_sqrt(aLenSq*bLenSq);
}

// dotNorm must be in [-1,1]
/*
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
    auto aDotB = dot(a,b); // ugh, need dot-product (and multiply) that returns fixed pt thing of correct size
    return aDotB * denom.inv_sqrt(); // somehow do this intelligently
}
*/
// dotNorm must be in [-1,1]
template <int I, int F, typename T>
FixedPt<I, F, T> dotNormFixed(const Vector3<FixedPt<I, F, T>>& a, const Vector3<FixedPt<I, F, T>>& b, const FixedPt<I, F, T> minLenThresh)
{
    // ugh, aLenSq and bLenSq will overflow their datatypes
    auto aLenSq = normSq(a);
    auto bLenSq = normSq(b);

    if (aLenSq < minLenThresh || bLenSq < minLenThresh)
    {
        return FixedPt<I, F, T>(0);
    }

    fixed_16_0 aDotB = dot(a, b); // ugh, need dot-product (and multiply) that returns fixed pt thing of correct size
    return aDotB * fast_inv_sqrt(aLenSq*bLenSq); // somehow do this intelligently
}

template <int I, int F, typename T>
FixedPt<2, (I + F) - 2, T> dotNormFixed(const Vector3<FixedPt<I, F, T>>& a, const Vector3<FixedPt<I, F, T>>& b)
{
    // Accumulate the squared lengths and dot products in larger integer types, reserve 2 more bits to prevent overflow
    using uT = typename std::make_unsigned<T>::type;
    using bigT = typename next_bigger_int<T>::type;
    using uBigT = typename next_bigger_int<uT>::type;

    // Compute squared length of a , squared length of b, and a dot b
    // as a next-bigger int, but add 2 integer bits because we're summing 3 of them.
    // If T is Fixed<9,7>, then these will be in 20.12 format
    // int bits = 2*I+2
    uBigT aLenSq = ((bigT)a.x.value_*(bigT)a.x.value_) >> 2;
    aLenSq += ((bigT)a.y.value_*(bigT)a.y.value_) >> 2;
    aLenSq += ((bigT)a.z.value_*(bigT)a.z.value_) >> 2;

    uBigT bLenSq = ((bigT)b.x.value_*(bigT)b.x.value_) >> 2;
    bLenSq += ((bigT)b.y.value_*(bigT)b.y.value_) >> 2;
    bLenSq += ((bigT)b.z.value_*(bigT)b.z.value_) >> 2;

    if (aLenSq == 0 || bLenSq == 0)
    {
        return FixedPt<2, (I + F) - 2, T>(0); // TODO: ~0?    
    }

    bigT aDotB = ((bigT)a.x.value_*(bigT)b.x.value_) >> 2;
    aDotB += ((bigT)a.y.value_*(bigT)b.y.value_) >> 2;
    aDotB += ((bigT)a.z.value_*(bigT)b.z.value_) >> 2;

    // ####
    float aLenSqFloat = float(aLenSq)* (1 << 2);
    float bLenSqFloat = float(bLenSq)* (1 << 2);
    float aDotBFloat = float(aDotB)* (1 << 2);
    //

    // make aDotB positive
    bool negative = false;
    if (aDotB < 0)
    {
        negative = true;
        aDotB = -aDotB;
    }

    // Find optimal amount of shifting for each result so as not to lose precision
    int aLenSqScale = leading_zeros(aLenSq) & (~0x01);
    int bLenSqScale = leading_zeros(bLenSq) & (~0x01);
    int aDotBScale = leading_zeros((uBigT)aDotB) & (~0x01);

    cout << "a scale: " << aLenSqScale << endl;
    cout << "b scale: " << bLenSqScale << endl;
    cout << "adotb scale: " << aDotBScale << endl;

    // "scaled" values are fractional, in [0,1), scaled
    // if we shift aLenSq left by aLenSqScale bits, now it has (2*I+2)-scale int bits
    uT aLenSqTrunc = ShiftRight(aLenSq, num_bits<T>::value - aLenSqScale); // truncate to small type, still has 2*I+2-scale int bits
    uT bLenSqTrunc = ShiftRight(bLenSq, num_bits<T>::value - bLenSqScale);
    uBigT aDotBBigScaled = ShiftLeft(aDotB, aDotBScale); // aDotBBig is a bigT type with ((2*I+2)-scale) integer bits. "Real" value = (aDotBBig >> bits(uBigT)) << ((2*I+2)-scale)


    cout << "aLenSq: " << float(FixedPt<0, I + F, uT>(aLenSqTrunc, true)) * (1 << (2 * I + 2 - aLenSqScale)) << endl;
    cout << "bLenSq: " << float(FixedPt<0, I + F, uT>(bLenSqTrunc, true)) * (1 << (2 * I + 2 - bLenSqScale)) << endl;
    cout << "a dot b: " << float(FixedPt<0, I + F, uT>(aDotBBigScaled >> num_bits<T>::value, true)) * (1 << (2 * I + 2 - aDotBScale)) << endl;

    // denom = sqrt(aSq*bSq), denomSq = aSq*bSq
    uT denomSqScaled = (uBigT(aLenSqTrunc) * uBigT(bLenSqTrunc)) >> num_bits<uT>::value; // has 2*(2*I+2) - scaleA - scaleB == 4*I+4-scaleA-scaleB int bits
    cout << "denomSqScaled: " << denomSqScaled << endl;
    cout << "denomSq: " << float(FixedPt<0, I + F, uT>(denomSqScaled, true)) * (1 << (4 * I + 4 - aLenSqScale - bLenSqScale)) << endl; // OK
    cout << "denomSq 2: " << double(denomSqScaled) * (1 << (4 * I + 4 - aLenSqScale - bLenSqScale - num_bits<T>::value)) << endl; // OK

    cout << "denom scale: " << (4 * I + 4 - aLenSqScale - bLenSqScale) << endl;
    
 
    // i.e., denomSq = denomSqScaled * 2^(4*I+4-scaleA-scaleB)
    // then, sqrt(denomSq) = sqrt(denomSqScaled) * 2^(2*I+2 - (scaleA+scaleB)/2)
    // then, 1/sqrt(denomSq) = 1/sqrt(denomSqScaled) * 2^( (scaleA+scaleB)/2 - (2*I+2) ) 
    FixedPt<0, I + F, uT> denomSqFixed(denomSqScaled, true); // cast it to a fixed-pt denormalized value so we can take sqrt // TODO: do this directly

    cout << "denomSqFixed " << float(denomSqFixed) << endl;

    FixedPt<2, I + F - 2, uT> recipDenomFixed = denomSqFixed.inv_sqrt(); // * 2^( (scaleA+scaleB)/2 - (2*I+2)) // Note, in 2.X format, not denormalized

    float recipSqrtVal = float(recipDenomFixed);
    cout << "1/sqrtVal: " << recipSqrtVal << endl;
    cout << "sqrtVal: " << 1.0 / recipSqrtVal << endl;

    // sqrtVal should be scaled by 
    cout << "sqrtVal rescaled: " << (1.0 / recipSqrtVal) * (1 << ((aLenSqScale + bLenSqScale) / 2 - num_bits<T>::value)) << endl; // This should == adotbsq
    cout << "sqrtVal rescaled2: " << (1.0 / recipSqrtVal) * (1 << ((4*I+4-aLenSqScale-bLenSqScale)/2)) << endl; // This should == adotbsq

    cout << "recipDenomFixed.val: " << recipDenomFixed.value_ << endl;
    cout << "adotb val: " << (aDotBBigScaled >> num_bits<uT>::value) << endl;
    cout << "adotb scale: " << aDotBScale << endl;

    // a dot b = aDotBBigScaled * 2^(2*I+2-scaleDot) 
    // recip denom = recipDenomScaled * 2^((scaleA+scaleB)/2 - (2*I+2))
    uBigT quotientScaled = ((aDotBBigScaled >> num_bits<uT>::value) * recipDenomFixed.value_);
    cout << "(int) quotientScaled: " << (quotientScaled >> num_bits<T>::value) << endl;

    int resultShift = 2 * I + 2 - aDotBScale;
    cout << "resultShift: " << resultShift << endl;
    // quotient  = quotientScaled * 2^( (2*I+2) - scaleDot) + ((scaleA+scaleB)/2 - (2*I+2)) )
    //           = quotientScaled * 2^( ((scaleA+scaleB)/2) - scaleDot)
    FixedPt<2, I + F - 2, T> result(ShiftRight(quotientScaled, num_bits<T>::value - resultShift), true); // need to shift right by num_bits to move to T

    cout << "returning " << float(result) << endl;
    
    return negative ? -result : result;
}



template <typename T>
float perpNorm(const Vector3<T>& a, const Vector3<T>& b, float minLenThresh)
{
    float aLenSq = dot(a, a);
    float bLenSq = dot(b, b);

    if (aLenSq < minLenThresh || bLenSq < minLenThresh)
    {
        return 0;
    }

    float aDotB = dot(a, b);
    float bPerpScale = aDotB / aLenSq;
    floatVector3 bPerpVec = (floatVector3(b) - bPerpScale * floatVector3(a)) * fast_inv_sqrt(bLenSq);

    return norm(bPerpVec); // this could surely be faster
}

