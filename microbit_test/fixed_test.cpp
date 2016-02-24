#include "FixedPt.h"

#include "catch.hpp"

#include <vector>
#include <iostream>

using std::vector;
using std::cout;
using std::endl;

// See catch tutorial: https://github.com/philsquared/Catch/blob/master/docs/tutorial.md

//
// FixedPt tests
//

template <typename Ta, typename Tb=Ta>
void TestPlus(float a, float b)
{
    Ta fixedA(a);
    Tb fixedB(b);
    REQUIRE(float(fixedA+fixedB) == Approx(a+b));
}

template <typename T>
void TestMinus(float a, float b)
{
    T fixedA(a);
    T fixedB(b);
    REQUIRE(float(fixedA-fixedB) == Approx(a-b));
}

template <typename T>
void TestTimes(float a, float b)
{
    T fixedA(a);
    T fixedB(b);
    REQUIRE(float(fixedA*fixedB) == Approx(a*b));
}

template <typename T>
void TestDiv(float a, float b)
{
    T fixedA(a);
    T fixedB(b);
    REQUIRE(float(fixedA/fixedB) == Approx(a/b).epsilon(0.01));
}

TEST_CASE("fixed_pt")
{
    fixed_8_8 x = fixed_8_8(3);
    fixed_8_8 y = fixed_8_8(0.5f);

    REQUIRE(int(x) == 3);
    REQUIRE(float(x) == 3.0f);
    REQUIRE(float(y) == 0.5f);
    
    REQUIRE(float(x+y) == 3.5);
    REQUIRE(float(x*y) == 1.5);

    // casting between widths
    fixed_12_4 x_4 = x;
    fixed_12_4 y_4 = y;
    REQUIRE(int(x_4) == 3);
    REQUIRE(float(y_4) == 0.5f);

    fixed_4_12 x_12 = x;
    fixed_4_12 y_12 = y;
    REQUIRE(int(x_12) == 3);
    REQUIRE(float(y_12) == 0.5f);


    // arithmetic
    for (auto x : { 0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 16.0f, 32.0f, 64.0f, 15.0f })
    {
        for (auto y : { 0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 16.0f, 32.0f, 64.0f, 15.0f })
        {
            TestPlus<fixed_9_7>(x, y);
            TestMinus<fixed_9_7>(x, y);
            if(x*y < fixed_9_7::max_int_value && x*y > fixed_9_7::min_int_value)
            {
                TestTimes<fixed_9_7>(x, y);
            }

            if(x/y < fixed_9_7::max_int_value && x/y > fixed_9_7::min_int_value)
            {
                TestDiv<fixed_9_7>(x, y);
            }
        }
    }

    // sqrt test
    for (float x : { 0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 16.0f, 32.0f, 64.0f, 15.0f })
    {
        fixed_12_4 y1 = fixed_12_4(x);
        REQUIRE(float(y1.sqrt()) == Approx(sqrtf(x)).epsilon(0.1));

        fixed_8_8 y2 = fixed_8_8(x);
        REQUIRE(float(y2.sqrt()) == Approx(sqrtf(x)).epsilon(0.1));

        if(x < fixed_4_12::max_int_value)
        {
            fixed_4_12 y3 = fixed_4_12(x);
            REQUIRE(float(y3.sqrt()) == Approx(sqrtf(x)).epsilon(0.1));
        }
    }

    // recip sqrt test
    for (float x : { 0.076f, 1.0f, 2.0f, 0.125f, 0.25f, 0.5f, 1.111f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 16.0f, 32.0f, 64.0f, 15.0f, 196.0f })
    {
        float y = 1.0 / sqrtf(x);

        if (x < fixed_4_12::max_int_value)
        {
            fixed_4_12 y4_12(x);
            REQUIRE(float(y4_12.inv_sqrt()) == Approx(y).epsilon(0.1));
        }

        fixed_8_8 y16_8 = fixed_8_8(x);
        REQUIRE(float(y16_8.inv_sqrt()) == Approx(y).epsilon(0.1));

        fixed_10_6 y2 = fixed_10_6(x);
        REQUIRE(float(y2.inv_sqrt()) == Approx(1.0 / sqrtf(x)).epsilon(0.1));

        fixed_12_4 y1 = fixed_12_4(x);
         REQUIRE(float(y1.inv_sqrt()) == Approx(y).epsilon(0.1));


        //        if(x < fixed_4_12::max_int_value)
        //        {
        //            fixed_4_12 y3 = x;
        //        //            REQUIRE(float(y3.inv_sqrt()) == Approx(1.0 / sqrtf(x)).epsilon(0.1));
        //        }

        // forget 32-bit for now
        //        FixedPt<int,8> y32_8 = x;
        //        REQUIRE(float(y32_8.inv_sqrt()) == Approx(y).epsilon(0.1));

    }
}

TEST_CASE("FixedPt fixMul tests")
{
    fixed_8_8 a(10);
    fixed_4_12 b(3);
    fixed_12_4 c = fixMul<12, 4, int16_t>(a, b);
    REQUIRE(float(c) == 10 * 3);

    FixedPt<20, 12> c_20_12 = fixMul<20, 12, int32_t>(a, b);

}

TEST_CASE("fixed_pt timing tests")
{

}
