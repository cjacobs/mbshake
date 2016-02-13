#pragma once

#include <cstdint> // for int8_t, int32_t types
#include <cmath>
#include <type_traits>

namespace
{
    template <typename T, int Mbits>
    constexpr T sqrtVal(const int x) 
    {
        return T(sqrtf(x) * (1 << Mbits));
    }
}

// --- why doesn't this work when we template the argument / return type??

template <int M1, int M2>
int ShiftValue(int x)
{
    if(M1 > M2)
    {
        return x << (M1-M2);
    }
    else
    {
        return x >> (M2-M1);
    }
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
        value_ = ShiftValue<Mbits, Mbits2>(x.value_);
        /*

        // need to do some template magic to avoid compiler warnings (and to make more efficient)
        if(Mbits > Mbits2)
        {
            value_ = x.value_ << (Mbits - Mbits2);
        }
        else
        {
            value_ = x.value_ >> (Mbits2 - Mbits);
        }
        */
    }

    operator int() const
    {
        return value_ >> Mbits;
    }

    operator long int() const
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
    void operator +=(const FixedPt& x)
    {
        value_ += x.value_;
    }

    void operator -=(const FixedPt& x)
    {
        value_ -= x.value_;
    }

    template <int Mbits2>
    void operator *=(const FixedPt<T, Mbits2>& x)
    {
        long long prod = value_ * x.value_;
        value_ = prod >> Mbits2;
    }

    void operator *=(float s)
    {
        value_ *= s;
    }
    
    template <typename S>
    FixedPt<T, Mbits> operator +(const S& b) const
    {
        FixedPt<T, Mbits> result(*this);
        result += b;
        return result;
    }

    template <typename S>
    FixedPt<T, Mbits> operator -(const S& b) const
    {
        FixedPt<T, Mbits> result(*this);
        result -= b;
        return result;
    }

    template <typename S>
    FixedPt<T, Mbits> operator *(const S& b) const
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

//typedef FixedPt<int16_t, 7> fixed_16; // make it big enough for +/- 256
typedef FixedPt<int16_t, 14> fixed_2_14;
typedef FixedPt<int16_t, 7> fixed_9_7;
