#pragma once

#include <cstdint> // for int8_t, int32_t types
#include <cmath>
//using std::sqrtf;

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


template <typename T, int Mbits>
constexpr T sqrtVal(const int x) 
{
    return T(sqrtf(x) * (1 << Mbits));
}


// TODO: implement fixed-point square root / inv square root
// http://stackoverflow.com/questions/6286450/inverse-sqrt-for-fixed-point
// also: http://www.realitypixels.com/turk/computergraphics/FixedSqrt.pdf

template <typename T, int Mbits>
class FixedPt
{
public:
    FixedPt() : value_(0) {}
    FixedPt(int val) : value_(val << Mbits) {}
    FixedPt(float val) : value_(T(val * (1 << Mbits))) {}
    FixedPt(double val) : value_(T(val * (1 << Mbits))) {}
    FixedPt(const FixedPt<T, Mbits>& x) : value_(x.value_) {}

    template <int Mbits2>
    FixedPt(const FixedPt<T, Mbits2>& x)
    {
        if(Mbits > Mbits2)
        {
            value_ = x.value_ << Mbits - Mbits2;
        }
        else
        {
            value_ = x.value_ >> Mbits2 - Mbits;
        }
    }

    operator int() const
    {
        return value_ >> Mbits;
    }

    operator float() const
    {
        return float(value_) / float(1 << Mbits);
    }

    operator double() const
    {
        return double(value_) / double(1 << Mbits);
    }

    // arithmetic ops: +, -, *, / (?)
    void operator+=(const FixedPt& x)
    {
        value_ += x.value_;
    }

    void operator-=(const FixedPt& x)
    {
        value_ -= x.value_;
    }

    void operator*=(const FixedPt& x)
    {
        value_ = (value_*x.value_) >> Mbits;
    }

    FixedPt<T, Mbits> operator +(const FixedPt<T, Mbits>& b)
    {
        FixedPt<T, Mbits> result(*this);
        result += b;
        return result;
    }

    FixedPt<T, Mbits> operator -(const FixedPt<T, Mbits>& b) const
    {
        FixedPt<T, Mbits> result(*this);
        result -= b;
        return result;
    }

    FixedPt<T, Mbits> operator *(const FixedPt<T, Mbits>& b)
    {
        FixedPt<T, Mbits> result(*this);
        result *= b;
        return result;
    }

    FixedPt<T, Mbits> sqrtx()
    {
        // adapted from http://www.realitypixels.com/turk/computergraphics/FixedSqrt.pdf
        unsigned long root = 0;
        unsigned long remHi = 0;
        unsigned long remLo = value_;
        int count = 8; //?
        do {
            remHi = (remHi << Mbits) | (remLo >> Mbits);
            remLo <<= 2;
            root <<= 1;
            int testDiv = (root << 1) + 1;
            if (remHi >= testDiv) {
                remHi -= testDiv;
                root++;
            }

        } while (count-- != 0);
        
        return FixedPt<T, Mbits>(root, true);
    }

    // my version
    FixedPt<T, Mbits> sqrt()
    {
        static const T lookupTable[16] = { 0, 1 << Mbits, sqrtVal<T,Mbits>(2), sqrtVal<T,Mbits>(3), sqrtVal<T,Mbits>(4), sqrtVal<T,Mbits>(5), sqrtVal<T,Mbits>(6), sqrtVal<T,Mbits>(7),
            sqrtVal<T,Mbits>(8), sqrtVal<T,Mbits>(9), sqrtVal<T,Mbits>(10), sqrtVal<T,Mbits>(11), sqrtVal<T,Mbits>(12), sqrtVal<T,Mbits>(13), sqrtVal<T,Mbits>(14), sqrtVal<T,Mbits>(15) };

        T tmp = value_;
        T out = 0;
        int mask = 0b01111;
        int n = Mbits - 4;
        while (tmp != 0 && n > 0)
        {
                out = lookupTable[tmp&mask] >> n;
                tmp >>= 4;
                n -= 2;
        }
        while (tmp != 0)
        {
            out = lookupTable[tmp&mask] << n;
            tmp >>= 4;
            n += 2;
        }

        return FixedPt<T, Mbits>(out, true);
    }

private:
    template <typename U, int Mbits2>
    friend class FixedPt;

    FixedPt(T val, bool) : value_(val) {}
    T value_;
};

typedef FixedPt<int16_t, 8> fixed_16;

