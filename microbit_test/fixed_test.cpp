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

TEST_CASE("fixed_pt test")
{
    FixedPt<int16_t, 8> x = 3;
    FixedPt<int16_t, 8> y = 0.5f;

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

    // sqrt test
    for (auto x : { 0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 16.0f, 32.0f, 64.0f, 15.0f })
    {
        fixed_9_7 y = x;
        REQUIRE(float(y.sqrt()) == Approx(sqrtf(x)).epsilon(0.1));
    }
}



TEST_CASE("timing tests")
{

}
