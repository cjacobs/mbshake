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

TEST_CASE("fixed_pt test")
{
    FixedPt<int16_t, 8> x = FixedPt<int16_t, 8>(3);
    FixedPt<int16_t, 8> y = FixedPt<int16_t, 8>(0.5f);

    REQUIRE(int(x) == 3);
    REQUIRE(float(x) == 3.0f);
    REQUIRE(float(y) == 0.5f);
    
    REQUIRE(float(x+y) == 3.5);
    REQUIRE(float(x*y) == 1.5);

    // casting between widths
    FixedPt<int16_t, 4> x_4 = x;
    FixedPt<int16_t, 4> y_4 = y;
    REQUIRE(int(x_4) == 3);
    REQUIRE(float(y_4) == 0.5f);

    FixedPt<int16_t, 12> x_12 = x;
    FixedPt<int16_t, 12> y_12 = y;
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
        FixedPt<int16_t,4> y1 = FixedPt<int16_t, 4>(x);
        REQUIRE(float(y1.sqrt()) == Approx(sqrtf(x)).epsilon(0.1));

        FixedPt<int16_t,8> y2 = FixedPt<int16_t, 8>(x);
        REQUIRE(float(y2.sqrt()) == Approx(sqrtf(x)).epsilon(0.1));

        if(x < FixedPt<int16_t,12>::max_int_value)
        {
            FixedPt<int16_t,12> y3 = FixedPt<int16_t,12>(x);
            REQUIRE(float(y3.sqrt()) == Approx(sqrtf(x)).epsilon(0.1));
        }
    }

    // recip sqrt test
    for (float x : { 1.0f, 2.0f, 0.125f, 0.25f, 0.5f, 1.111f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 16.0f, 32.0f, 64.0f, 15.0f })
    {
        float y = 1.0 / sqrtf(x);

        FixedPt<int16_t,8> y16_8 = FixedPt<int16_t, 8>(x);
        REQUIRE(float(y16_8.inv_sqrt()) == Approx(y).epsilon(0.1));

        FixedPt<int16_t,6> y2 = FixedPt<int16_t, 6>(x);
        REQUIRE(float(y2.inv_sqrt()) == Approx(1.0 / sqrtf(x)).epsilon(0.1));

        FixedPt<int16_t,4> y1 = FixedPt<int16_t, 4>(x);
         REQUIRE(float(y1.inv_sqrt()) == Approx(y).epsilon(0.1));


        //        if(x < FixedPt<int16_t,12>::max_int_value)
        //        {
        //            FixedPt<int16_t,12> y3 = x;
        //        //            REQUIRE(float(y3.inv_sqrt()) == Approx(1.0 / sqrtf(x)).epsilon(0.1));
        //        }

        // forget 32-bit for now
        //        FixedPt<int,8> y32_8 = x;
        //        REQUIRE(float(y32_8.inv_sqrt()) == Approx(y).epsilon(0.1));

    }
}



TEST_CASE("fixed_pt timing tests")
{

}
