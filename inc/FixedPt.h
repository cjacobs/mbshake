#pragma once

#include <cstdint> // for int8_t, int32_t types
#include <cmath>
#include <limits>
#include <type_traits>

#include <iostream>
using std::cout;
using std::endl;

namespace
{
    template <typename T, int Mbits>
    constexpr T sqrtVal(const int x) 
    {
        return T(sqrtf(x) * (1 << Mbits));
    }

    template <int M1, int M2>
    constexpr int ShiftValue(int x)
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

    int leading_zeros (uint16_t a)
    {
        uint32_t r = 16;
        if (a >= 0x00000100) { a >>=  8; r -=  8; }
        if (a >= 0x00000010) { a >>=  4; r -=  4; }
        if (a >= 0x00000004) { a >>=  2; r -=  2; }
        r -= a - (a & (a >> 1));
        return r;
    }

    int leading_zeros (uint32_t a)
    {
        uint32_t r = 32;
        if (a >= 0x00010000) { a >>= 16; r -= 16; }
        if (a >= 0x00000100) { a >>=  8; r -=  8; }
        if (a >= 0x00000010) { a >>=  4; r -=  4; }
        if (a >= 0x00000004) { a >>=  2; r -=  2; }
        r -= a - (a & (a >> 1));
        return r;
    }

    template <typename T>
    struct num_bits
    {
    };

    template <>
    struct num_bits<int8_t>
    {
        static constexpr int value = 8;
    };

    template <>
    struct num_bits<uint8_t>
    {
        static constexpr int value = 8;
    };

    template <>
    struct num_bits<int16_t>
    {
        static constexpr int value = 16;
    };

    template <>
    struct num_bits<uint16_t>
    {
        static constexpr int value = 16;
    };

    template <>
    struct num_bits<int32_t>
    {
        static constexpr int value = 32;
    };

    template <>
    struct num_bits<uint32_t>
    {
        static constexpr int value = 32;
    };

    template <typename T>
    struct bigger_int
    {
    };

    template <>
    struct bigger_int<int8_t>
    {
    public:
        typedef int16_t type;
    };

    template <>
    struct bigger_int<uint8_t>
    {
    public:
        typedef uint16_t type;
    };

    template <>
    struct bigger_int<int16_t>
    {
    public:
        typedef int32_t type;
    };

    template <>
    struct bigger_int<uint16_t>
    {
    public:
        typedef uint32_t type;
    };

    template <>
    struct bigger_int<int32_t>
    {
    public:
        typedef int64_t type;
    };

    template <>
    struct bigger_int<uint32_t>
    {
    public:
        typedef uint64_t type;
    };


    template <typename T>
    constexpr T fixed_cast(float x, int mbits)
    {
        return static_cast<T>(x * (1<<mbits));
    }

    template <typename T>
    constexpr T gen_entry(double x, int cubedBits, int x3Bits)
    {
        double y = 1.0 / sqrt(x);
        T loPart = fixed_cast<T>(3*y, x3Bits-2);
        T hiPart = fixed_cast<T> (y*y*y, cubedBits);
        if(x == 1.0)
        {
            hiPart = ~0;
        }
        return (hiPart << x3Bits) | loPart;
    }

    template <typename T>
    T gen_entry2(double x, int cubedBits, int x3Bits)
    {
        double y = sqrt(x);
        T loPart = fixed_cast<T>(3*y, x3Bits-2);
        T hiPart = fixed_cast<T> (y*y*y, cubedBits);

        if(x == 1.0)
        {
            hiPart = ~0;
        }
        return (hiPart << x3Bits) | loPart;
    }

    constexpr int cBits = 12;
    constexpr int tBits = 4;

    // lookup table for 16-bit fixed-pt inv sqrt
    // upper 12 bits store y^3 in 0.12 format, lower 4 store 3y in 2.2
    uint16_t inv_sqrt_table[] = // 12-entry version
        {
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
        };
    
    
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

    void operator=(const FixedPt<T, Mbits>& x)
    {
        value_ = x.value_;
    }

    template <int Mbits2>
    FixedPt(const FixedPt<T, Mbits2>& x)
    {
        value_ = ShiftValue<Mbits, Mbits2>(x.value_);
    }

    static const int max_int_value = std::numeric_limits<T>::max() >> Mbits;
    static const int min_int_value = std::numeric_limits<T>::min() >> Mbits;
    static const int mantissa_bits = Mbits;

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
    void operator +=(int x)
    {
        value_ += (x<<Mbits);
    }

    void operator +=(const FixedPt& x)
    {
        value_ += x.value_;
    }

    void operator -=(int x)
    {
        value_ -= (x<<Mbits);
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
    
    void operator /=(float s)
    {
        value_ /= s;
    }


    /* // TODO: check this
    template <int Mbits2>
    void operator /=(const FixedPt<T, Mbits2>& x)
    {
        long long prod = value_ / x.value_;
        value_ = prod << Mbits2;
    }
    */
    
    // TODO: Nope. need to implement this only for int, float, ...
    FixedPt<T, Mbits> operator +(int b) const
    {
        FixedPt<T, Mbits> result(*this);
        result += b;
        return result;
    }

    FixedPt<T, Mbits> operator +(float b) const
    {
        FixedPt<T, Mbits> result(*this);
        result += b;
        return result;
    }

    // TODO:
    template <int Mbits2>
    FixedPt<T, Mbits> operator +(FixedPt<T, Mbits2> b) const
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

    FixedPt<T, Mbits> operator *(int b) const
    {
        FixedPt<T, Mbits> result(*this);
        result *= b;
        return result;
    }

    FixedPt<T, Mbits> operator *(float b) const
    {
        FixedPt<T, Mbits> result(*this);
        result *= b;
        return result;
    }

    template <typename S>
    FixedPt<T, Mbits> operator /(const S& b) const
    {
        FixedPt<T, Mbits> result(*this);
        result /= b;
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


        // First try integer part
        unsigned int mask = 0b01111;
        T intPart = value_ >> Mbits;
        T out = 0;
        if(intPart != 0 || Mbits == 0)
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
            int outShift = 0;
            int shift = (4-1) - ((Mbits-1)%4);
            tmp <<= shift;
            int numBlocks = (Mbits+shift)/4;
            int n = 2*numBlocks;
            while(tmp != 0 && tmp != ~0)
            {
                out = lookupTable[tmp&mask] >> n;
                tmp >>= 4;
                n -= 2;
            }
        }

        return FixedPt<T, Mbits>(out, true);
    }

    // http://stackoverflow.com/questions/6286450/inverse-sqrt-for-fixed-point
    FixedPt<T, Mbits> inv_sqrt()
    {
        if(value_ <= 0) // 
        {
            return FixedPt<T, Mbits>(~0, true);
        }

        constexpr int nBits = num_bits<T>::value;
        typedef typename std::make_unsigned<T>::type uT;
        typedef typename bigger_int<T>::type bigger_t;

        uT val = static_cast<uT>(value_);
        int scale = leading_zeros(val) & (~0x01); // round scale down to be even
        val = val << scale; // val now in 2.X fixed format, in [1,3)  (so, top 2 bits are 01, 10, or 11 (but not 00)
        float valFloat = float(val) / float(1<<(nBits-2));

        // Lookup table thing
        const int tableIndex = (val>>(nBits-4)) - 4;
        uint16_t lookupVal = inv_sqrt_table[tableIndex];

#if DEBUG_INV_SQRT
        float inv_sqrt = 1.0 / sqrtf(valFloat);
        uT yCubedReal = uT((inv_sqrt*inv_sqrt*inv_sqrt) * (1<<(nBits-1)) *2); // oh crap, this fails for 1.0 --- we can't represent 1 as 0.X
        if(yCubedReal == 0)
        {
            yCubedReal = ~0;
        }
        // check for 1 case ?
        uT threeYReal = uT(3*inv_sqrt * (1 << (nBits-2)));
#endif

        uT yCubed = lookupVal;
        uT threeY = lookupVal << cBits;
        
        // check this:
        uint32_t y = (threeY - (((bigger_t)yCubed*val)>>nBits)); // y is 3y/2 + xy^3/2 ---  in 1.X fixed format 
        
        uint32_t s = ((bigger_t)y*val) >> nBits; // s = y*x in 3.X fixed 

        const uint32_t three = 0x03 << (nBits-4); // 3 in 4.X fixed, == 3/2 in 3.X fixed

        s = three - (((bigger_t)y*s)>>nBits); // s now = 3 - y^2*x in 4.X fixed
        y = ((bigger_t)y*s) >> nBits; // now y = y(3-y^2*x) in 5.X fixed == (3/2)y - y^2*x/2 in 4.X fixed

        // now y is sqrt of our normalized value n 4.X format
        // first convert to sqrt of input
        //        y = y >> (scale>>1);

        // now convert to z.X fixed (where z = total_bits-Mbits)
        // y2 is in 4.X... 

        constexpr int intBits = (nBits-Mbits);
        constexpr int M_2 = Mbits >> 1;

        // phooey --- this only works when nBits == 16        
        int Z = 0;
        
        if(nBits == 16)
        {
            constexpr int shift = intBits-M_2+3;
            Z = shift-(scale >> 1); 
        }
        else
        {
            // here are values for nBits == 32
            constexpr int shift = intBits-M_2+8+3; // // works for 1, 2 (when scale == O
            Z = shift-(scale >> 1); 
        }

        uT newVal = y >> Z;
        auto result = FixedPt<T, Mbits>(T(newVal), true);
        
        return result;
    }

private:
    template <typename U, int Mbits2>
    friend class FixedPt;

    FixedPt(T val, bool) : value_(val) {}
    T value_;
};

template <typename T, int Mbits, typename S>
FixedPt<T, Mbits> operator *(const S& a, FixedPt<T, Mbits> b)
{
    FixedPt<T, Mbits> result(b);
    result *= a;
    return result;
}

typedef FixedPt<int16_t, 14> fixed_2_14;
typedef FixedPt<int16_t, 8> fixed_8_8;
typedef FixedPt<int16_t, 7> fixed_9_7;


