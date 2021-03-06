#pragma once

#include "BitUtil.h"

#include <cstdint> // for int8_t, int32_t types
#include <cmath>
#include <limits>
#include <type_traits>

namespace
{
    template <typename T, int Mbits>
    constexpr T sqrtVal(const int x)
    {
        return T(sqrtf(x) * (1 << Mbits));
    }

    template <typename T>
    constexpr T fixed_cast(float x, int mbits)
    {
        return static_cast<T>(x * (1 << mbits));
    }

    template <typename T>
    constexpr T gen_entry(double x, int cubedBits, int x3Bits)
    {
        double y = 1.0 / sqrt(x);
        T loPart = fixed_cast<T>(3 * y, x3Bits - 2);
        T hiPart = fixed_cast<T> (y*y*y, cubedBits);
        if (x == 1.0)
        {
            hiPart = ~0;
        }

        return (hiPart << x3Bits) | loPart;
    }

    constexpr int cBits = 12;
    constexpr int tBits = 4;

    // TODO: get

    // lookup table for 16-bit fixed-pt inv sqrt
    // upper 12 bits store y^3 in 0.12 format, lower 4 store 3y in 2.2
    uint16_t inv_sqrt_table[] = // 12-entry version
    {
#if 0
            gen_entry<uint16_t>(1.0,  cBits, tBits),
            gen_entry<uint16_t>(1.25, cBits, tBits),
            gen_entry<uint16_t>(1.5,  cBits, tBits),
            gen_entry<uint16_t>(1.75, cBits, tBits),
            gen_entry<uint16_t>(2.0,  cBits, tBits),
            gen_entry<uint16_t>(2.25, cBits, tBits),
            gen_entry<uint16_t>(2.5,  cBits, tBits),
            gen_entry<uint16_t>(2.75, cBits, tBits),
            gen_entry<uint16_t>(3.0,  cBits, tBits),
            gen_entry<uint16_t>(3.25, cBits, tBits),
            gen_entry<uint16_t>(3.5,  cBits, tBits),
            gen_entry<uint16_t>(3.75, cBits, tBits),

        // x = 01.00 = 1.0,  y = 1.0,         3y = 3.0,         y^3 = 1.0
        // x = 01.01 = 1.25, y = 0.894427191, 3y = 2.683281573, y^3 = 1.953125
        // x = 01.10 = 1.5,  y = 0.816496580, 3y = 2.449489742, y^3 = 3.375
        // x = 01.11 = 1.75, y = 0.755928946, 3y = 2.267786838, y^3 = 5.359375
        // x = 10.00 = 2.0,  y = 0.707106781, 3y = 2.121320343, y^3 = 8.0
        // x = 10.01 = 2.25, y = 0.666666666, 3y = 2.0,         y^3 = 11.390625
        // x = 10.10 = 2.5,  y = 0.632455532, 3y = 1.897366596, y^3 = 15.625
        // x = 10.11 = 2.75, y = 0.603022689, 3y = 1.809068067, y^3 = 20.796875
        // x = 11.00 = 3.0,  y = 0.577350269, 3y = 1.732050807, y^3 = 27.0
        // x = 11.01 = 3.25, y = 0.554700196, 3y = 1.664100588, y^3 = 34.328125
        // x = 11.10 = 3.5,  y = 0.534522483, 3y = 1.603567451, y^3 = 42.875
        // x = 11.11 = 3.75, y = 0.516397779, 3y = 1.549193338, y^3 = 52.734375
#else
                0xfffc,
                0xb72a,
                0x8b59,
                0x6e99,
                0x5a88,
                0x4bd8,
                0x40c7,
                0x3827,
                0x3146,
                0x2bb6,
                0x2716,
                0x2346
#endif                
    };


}

// TODO: allow IntBits to be > # bits
// (and allow negative # frac bits)
template <typename T> class Vector3;

template <int IntBits, int FracBits, typename T = typename int_of_size<IntBits + FracBits>::type>
class FixedPt
{
public:
    FixedPt() : value_(0) {}
    FixedPt(const FixedPt<IntBits, FracBits, T>& x) : value_(x.value_) {}

    explicit FixedPt(int val) : value_(::ShiftLeft<FracBits>(val)) {}

    explicit FixedPt(float val) : value_(T(std::ldexp(val, FracBits))) {}

    explicit FixedPt(double val) : value_(T(std::ldexp(val, FracBits))) {}

    template <int IntBits2, int FracBits2>
    FixedPt(const FixedPt<IntBits2, FracBits2, T>& x)
    {
        value_ = ShiftValue<FracBits, FracBits2>(x.value_);
    }

    void operator =(const FixedPt<IntBits, FracBits, T>& x)
    {
        value_ = x.value_;
    }

    bool operator ==(FixedPt<IntBits, FracBits, T> x) const
    {
        return value_ == x.value_;
    }

    bool operator ==(int x) const
    {
        return (*this) == FixedPt<IntBits, FracBits, T>(x);
    }

    bool operator !=(FixedPt<IntBits, FracBits, T> x) const
    {
        return !(*this == x);
    }

    bool operator !=(int x) const
    {
        return !(*this == x);
    }

    bool operator <(FixedPt<IntBits, FracBits, T> x)
    {
        return value_ < x.value_;
    }

    bool operator >(FixedPt<IntBits, FracBits, T> x)
    {
        return value_ > x.value_;
    }

    bool operator <=(FixedPt<IntBits, FracBits, T> x)
    {
        return value_ <= x.value_;
    }

    bool operator >=(FixedPt<IntBits, FracBits, T> x)
    {
        return value_ >= x.value_;
    }

    bool operator <(int x) const
    {
        auto intPart = static_cast<int>(*this);
        if (intPart > x)
        {
            return false;
        }
        else if (intPart < x)
        {
            return true;
        }
        else
        {
            auto fracPart = (~0 >> IntBits) & value_;
            if (fracPart == 0)
            {
                return false;
            }
            else
            {
                return (intPart < 0);
            }
        }
    }

    bool operator >(int x) const
    {
        auto intPart = static_cast<int>(*this);
        if (intPart > x)
        {
            return true;
        }
        else if (intPart < x)
        {
            return false;
        }
        else
        {
            auto fracPart = (~0 >> IntBits) & value_;
            if (fracPart == 0)
            {
                return false;
            }
            else
            {
                return (intPart >= 0);
            }
        }
    }

    bool operator >=(int x) const
    {
        return !((*this) < x);
    }

    bool operator <=(int x) const
    {
        return !((*this) > x);
    }

    static const int max_int_value = std::numeric_limits<T>::max() >> FracBits;
    static const int min_int_value = std::numeric_limits<T>::min() >> FracBits;
    static const int int_bits = IntBits;
    static const int frac_bits = FracBits;

    // casting operators
    operator int() const
    {
        return ::ShiftRight<FracBits>((int)value_);
    }

    operator long int() const
    {
        return ::ShiftRight<FracBits>((long int)value_);
    }

    operator float() const
    {
        return std::ldexp(float(value_), -FracBits);
    }

    operator double() const
    {
        return std::ldexp(double(value_), -FracBits);
    }

    // arithmetic operators of op= form 
    // ( x op y  operators are defined outside the class)

    //
    // operator +=
    //
    void operator +=(int x)
    {
        value_ += ShiftLeft<FracBits>(x);
    }

    template <typename Tb>
    void operator +=(const FixedPt<IntBits, FracBits, Tb>& b)
    {
        value_ += b.value_;
    }

    template <int Ib, int Fb, typename Tb>
    void operator +=(const FixedPt<Ib, Fb, Tb>& b)
    {
        value_ += ShiftValue<FracBits, Fb>(b.value_);
    }

    //
    // operator -=
    //
    void operator -=(int x)
    {
        value_ -= (x << FracBits);
    }

    template <typename Tb>
    void operator -=(const FixedPt<IntBits, FracBits, Tb>& b)
    {
        value_ -= b.value_;
    }

    template <int Ib, int Fb, typename Tb>
    void operator -=(const FixedPt<Ib, Fb, Tb>& b)
    {
        value_ -= ShiftValue<FracBits, Fb>(b.value_);
    }

    //
    // unary operator -
    //
    FixedPt<IntBits, FracBits, T> operator -()
    {
        return FixedPt<IntBits, FracBits, T>(-value_, true);
    }


    //
    // operator *=
    //
    void operator *=(int x)
    {
        value_ *= x;
    }

    template <int IntBits2, int FracBits2, typename T2>
    void operator *=(FixedPt<IntBits2, FracBits2, T2> x)
    {
        using bigT = typename next_bigger_int<T>::type;
        bigT prod = value_ * x.value_;
        value_ = ::ShiftRight<FracBits2>(prod);
    }

    // operator /=
    void operator /=(int s)
    {
        value_ /= s;
    }

    template <int IntBits2, int FracBits2, typename T2>
    void operator /=(FixedPt<IntBits2, FracBits2, T2> b)
    {
        typedef typename next_bigger_int<T>::type bigger_t;
        bigger_t rVal = (value_ << num_bits<T>::value) / b.value_;
        value_ = ::ShiftRight<num_bits<T>::value - FracBits2>(rVal);
        //        value_ = rVal >> (num_bits<T>::value - FracBits2);
    }

    /*
    template <int IntBits2, int FracBits2, typename T2>
    void operator /=(const FixedPt<IntBits2, FracBits2, T2>& x)
    {
        // TODO: this
        // I think we want to shift the value with the most int bits to put its first '1' in the MSB
    }
    */
    /* // TODO: check this
template <int FracBits2>
void operator /=(const FixedPt<T, FracBits2>& x)
{
    long long prod = value_ / x.value_;
    value_ = prod << FracBits2;
}
*/

//
// operator >>=
//
    void operator <<= (int s)
    {
        value_ <<= s;
    }

    void operator >>= (int s)
    {
        value_ >>= s;
    }

    void ShiftLeft(int s)
    {
        value_ = ::ShiftLeft(value_, s);
    }

    void ShiftRight(int s)
    {
        value_ = ::ShiftRight(value_, s);
    }


    //
    // Gross stuff
    //
    FixedPt<IntBits, FracBits, T> sqrtx()
    {
        // adapted from http://www.realitypixels.com/turk/computergraphics/FixedSqrt.pdf
        unsigned long root = 0;
        unsigned long remHi = 0;
        unsigned long remLo = value_;
        int count = 8; //?
        do {
            remHi = (remHi << FracBits) | (remLo >> FracBits);
            remLo <<= 2;
            root <<= 1;
            int testDiv = (root << 1) + 1;
            if (remHi >= testDiv) {
                remHi -= testDiv;
                root++;
            }

        } while (count-- != 0);

        return FixedPt<IntBits, FracBits, T>(root, true);
    }

    // my version
    FixedPt<IntBits, FracBits, T> sqrt()
    {
        static const T lookupTable[16] = { 0, 1 << FracBits, sqrtVal<T,FracBits>(2), sqrtVal<T,FracBits>(3), sqrtVal<T,FracBits>(4), sqrtVal<T,FracBits>(5), sqrtVal<T,FracBits>(6), sqrtVal<T,FracBits>(7),
            sqrtVal<T,FracBits>(8), sqrtVal<T,FracBits>(9), sqrtVal<T,FracBits>(10), sqrtVal<T,FracBits>(11), sqrtVal<T,FracBits>(12), sqrtVal<T,FracBits>(13), sqrtVal<T,FracBits>(14), sqrtVal<T,FracBits>(15) };

        // First try integer part
        unsigned int mask = 0b01111;
        T intPart = value_ >> FracBits;
        T out = 0;
        if (intPart != 0 || FracBits == 0)
        {
            int n = 0;
            while (intPart != 0 && intPart != ~0)
            {
                out = lookupTable[intPart&mask] << n;
                intPart >>= 4;
                n += 2;
            }
        }
        else
        {
            // shift left to fall on multiple of 4
            T tmp = value_;
            int shift = (4 - 1) - ((FracBits - 1) % 4);
            tmp <<= shift;
            int numBlocks = (FracBits + shift) / 4;
            int n = 2 * numBlocks;
            while (tmp != 0 && tmp != ~0)
            {
                out = lookupTable[tmp&mask] >> n;
                tmp >>= 4;
                n -= 2;
            }
        }

        return FixedPt<IntBits, FracBits, T>(out, true);
    }

    // based on second answer of:
    // http://stackoverflow.com/questions/6286450/inverse-sqrt-for-fixed-point
    FixedPt<IntBits, FracBits, T> inv_sqrt()
    {
        if (value_ <= 0) // 
        {
            return FixedPt<IntBits, FracBits, T>(~0, true);
        }

        constexpr int nBits = num_bits<T>::value;
        typedef typename std::make_unsigned<T>::type uT;
        typedef typename next_bigger_int<T>::type bigT;
        typedef typename next_bigger_int<uT>::type uBigT;

        uT val = static_cast<uT>(value_);
        int scale = leading_zeros(val) & (~0x01); // round scale down to be even
        val = val << scale; // val now in 2.X fixed format, in [1,3)  (so, top 2 bits are 01, 10, or 11 (but not 00)


        // Lookup table thing
        const int tableIndex = (val >> (nBits - 4)) - 4;
        uint16_t lookupVal = inv_sqrt_table[tableIndex];

#if DEBUG_INV_SQRT
        float valFloat = float(val) / float(1 << (nBits - (IntBits - scale)));
        float inv_sqrt = 1.0 / sqrtf(valFloat);
        uT yCubedReal = uT((inv_sqrt*inv_sqrt*inv_sqrt) * (1 << (nBits - 1)) * 2); // oh crap, this fails for 1.0 --- we can't represent 1 as 0.X
        if (yCubedReal == 0)
        {
            yCubedReal = ~0;
        }
        // check for 1 case ?
        uT threeYReal = uT(3 * inv_sqrt * (1 << (nBits - 2)));
#endif

        uT yCubed = lookupVal;
        uT threeY = lookupVal << cBits;

        // check this:
        bigT y = (threeY - (((bigT)yCubed*val) >> nBits)); // y is 3y/2 + xy^3/2 ---  in 1.X fixed format 
        bigT s = ((bigT)y*val) >> nBits; // s = y*x in 3.X fixed 
        const uBigT three = 0x03 << (nBits - 4); // 3 in 4.X fixed, == 3/2 in 3.X fixed
        s = three - (((bigT)y*s) >> nBits); // s now = 3 - y^2*x in 4.X fixed
        y = ((bigT)y*s) >> nBits; // now y = y(3-y^2*x) in 5.X fixed == (3/2)y - y^2*x/2 in 4.X fixed

        // now y is sqrt of our normalized value n 4.X format
        // first convert to sqrt of input
        //        y = y >> (scale>>1);

        // now convert to z.X fixed (where z = total_bits-FracBits)
        // y2 is in 4.X... 

        constexpr int M_2 = FracBits >> 1;

        // This only works when nBits == 16 --- TODO: fix for general # bits
        int Z = 0;

        if (nBits == 16)
        {
            constexpr int shift = IntBits - M_2 + 3;
            Z = shift - (scale >> 1);
        }
        else if (nBits == 32)
        {
            // here are values for nBits == 32
            constexpr int shift = IntBits - M_2 + 8 + 3; // // works for 1, 2 (when scale == O
            Z = shift - (scale >> 1);
        }

        uT newVal = ::ShiftRight(y, Z);
        auto result = FixedPt<IntBits, FracBits, T>(T(newVal), true);

        return result;
    }

#if 0
    void printLookupTable()
    {
        cout << "uint16_t inv_sqrt_table[] = // 12-entry version" << endl;
        cout << "\t{" << endl;

        int numEntries = sizeof(inv_sqrt_table) / sizeof(inv_sqrt_table[0]);
        for (int index = 0; index < numEntries); index++)
        {
            char delim = (index == numEntries - 1) ? ' ' : ',';
            cout << "\t0x" << std::hex << inv_sqrt_table[index] << delim << endl;
        }
        cout << "\t};" << endl;
    }
#endif


    // private:
    template <int IntBits2, int FracBits2, typename T2>
    friend class FixedPt;

    // Private constructor that takes a raw value
    FixedPt(T val, bool) : value_(val) {}
    T value_;
};

//
// operator +
//
template <int IntBits, int FracBits, typename T>
FixedPt<IntBits, FracBits, T> operator +(FixedPt<IntBits, FracBits, T> a, int b)
{
    FixedPt<IntBits, FracBits, T> x = a;
    x += b;
    return x;
}

template <int IntBits, int FracBits, typename T>
FixedPt<IntBits, FracBits, T> operator +(int a, FixedPt<IntBits, FracBits, T> b)
{
    FixedPt<IntBits, FracBits, T> x = b;
    x += a;
    return x;
}

template <int IntBits, int FracBits, typename T, int IntBits2, int FracBits2, typename T2>
FixedPt<IntBits, FracBits, T> operator +(FixedPt<IntBits, FracBits, T> a, FixedPt<IntBits2, FracBits2, T2> b)
{
    FixedPt<IntBits, FracBits, T> x = a;
    x += b;
    return x;
}

//
// operator -
//
template <int IntBits, int FracBits, typename T>
FixedPt<IntBits, FracBits, T> operator -(FixedPt<IntBits, FracBits, T> a, int b)
{
    FixedPt<IntBits, FracBits, T> x = a;
    x -= b;
    return x;
}

template <int IntBits, int FracBits, typename T>
FixedPt<IntBits, FracBits, T> operator -(int a, FixedPt<IntBits, FracBits, T> b)
{
    FixedPt<IntBits, FracBits, T> x = b;
    x -= a;
    return x;
}

template <int IntBits, int FracBits, typename T, int IntBits2, int FracBits2, typename T2>
FixedPt<IntBits, FracBits, T> operator -(FixedPt<IntBits, FracBits, T> a, FixedPt<IntBits2, FracBits2, T2> b)
{
    FixedPt<IntBits, FracBits, T> x = a;
    x -= b;
    return x;
}

//
// operator *
//
template <int IntBits, int FracBits, typename T>
FixedPt<IntBits, FracBits, T> operator *(FixedPt<IntBits, FracBits, T> a, int b)
{
    FixedPt<IntBits, FracBits, T> x = a;
    x *= b;
    return x;
}

template <int IntBits, int FracBits, typename T>
FixedPt<IntBits, FracBits, T> operator *(int a, FixedPt<IntBits, FracBits, T> b)
{
    FixedPt<IntBits, FracBits, T> x = b;
    x *= a;
    return x;
}

template <int IntBits, int FracBits, typename T, int IntBits2, int FracBits2, typename T2>
FixedPt<IntBits, FracBits, T> operator *(FixedPt<IntBits, FracBits, T> a, FixedPt<IntBits2, FracBits2, T2> b)
{
    FixedPt<IntBits, FracBits, T> x = a;
    x *= b;
    return x;
}

//
// operator /
//
template <int IntBits, int FracBits, typename T>
FixedPt<IntBits, FracBits, T> operator /(FixedPt<IntBits, FracBits, T> a, int b)
{
    FixedPt<IntBits, FracBits, T> x = a;
    x /= b;
    return x;
}

template <int IntBits, int FracBits, typename T, int IntBits2, int FracBits2, typename T2>
FixedPt<IntBits, FracBits, T> operator /(FixedPt<IntBits, FracBits, T> a, FixedPt<IntBits2, FracBits2, T2> b)
{
    FixedPt<IntBits, FracBits, T> x = a;
    x /= b;
    return x;
}


//
// fixed-pt math with arbitrary bit sizes
//

template <int Ir, int Fr, typename Tr, // = typename int_of_size<IntBits+FracBits>::type,
    int Ia, int Fa, typename Ta,
    int Ib, int Fb, typename Tb>
    FixedPt<Ir, Fr, Tr> fixMul(FixedPt<Ia, Fa, Ta> a, FixedPt<Ib, Fb, Tb> b)
{
    using bigT = typename next_bigger_int<Tr>::type;

    bigT r = (bigT)a.value_ * b.value_;
    FixedPt<Ir, Fr, Tr> result(ShiftRight<Fa + Fb - Fr>(r), true);
    return result;
}

template <int Ir, int Fr, typename Tr, // = typename int_of_size<IntBits+FracBits>::type,
    int Ib, int Fb, typename Tb>
    FixedPt<Ir, Fr, Tr> fixMul(int a, FixedPt<Ib, Fb, Tb> b)
{
    using bigT = typename next_bigger_int<Tr>::type;

    bigT r = (bigT)a * b.value_;
    FixedPt<Ir, Fr, Tr> result(ShiftRight<Fb - Fr>(r), true);
    return result;
}

template <int Ir, int Fr, typename Tr, // = typename int_of_size<IntBits+FracBits>::type,
    int Ia, int Fa, typename Ta>
    FixedPt<Ir, Fr, Tr> fixMul(FixedPt<Ia, Fa, Ta> a, int b)
{
    using bigT = typename next_bigger_int<Tr>::type;

    bigT r = (bigT)a.value_ * b;
    FixedPt<Ir, Fr, Tr> result(ShiftRight<Fa - Fr>(r), true);
    return result;
}

template <int Ir, int Fr, typename Tr, // = typename int_of_size<IntBits+FracBits>::type,
    int Ia, int Fa, typename Ta>
    FixedPt<Ir, Fr, Tr> fixShiftLeft(FixedPt<Ia, Fa, Ta> x, int s)
{
    using bigT = typename next_bigger_int<Tr>::type;

    bigT r = ShiftLeft((bigT)x.value_, s - Fr + Fa);
    FixedPt<Ir, Fr, Tr> result(r, true);
    return result;
}

template <int Ir, int Fr, typename Tr, // = typename int_of_size<IntBits+FracBits>::type,
    int Ia, int Fa, typename Ta>
    FixedPt<Ir, Fr, Tr> fixShiftRight(FixedPt<Ia, Fa, Ta> x, int s)
{
    using bigT = typename next_bigger_int<Tr>::type;

    bigT r = ShiftRight((bigT)x.value_, s + Fr - Fa);
    FixedPt<Ir, Fr, Tr> result(r, true);
    return result;
}


typedef FixedPt<2, 14> fixed_2_14;
typedef FixedPt<4, 12> fixed_4_12;
typedef FixedPt<8, 8>  fixed_8_8;
typedef FixedPt<9, 7>  fixed_9_7;
typedef FixedPt<10, 6> fixed_10_6;
typedef FixedPt<12, 4> fixed_12_4;
typedef FixedPt<14, 2> fixed_14_2;
typedef FixedPt<16, 0> fixed_16_0;



// TODO:
// * have operations return result w/ correct # fraction bits
//   mul: intBits = a.intBits + b.intBits
//   div: implement as a * b.reciprocal()
//   implement reciprocal(): not sure how many bits --- maybe just swap int and fraction? (smallest number is 0.00001, which translates to 10000, right? 1/2^-M = 2M
//   make a multInto op if we want to specify the format of the result of multiplying
