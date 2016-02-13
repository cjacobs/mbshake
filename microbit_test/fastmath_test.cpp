#include "FastMath.h"

#include "catch.hpp"

#include <vector>
#include <iostream>

using std::vector;
using std::cout;
using std::endl;

// See catch tutorial: https://github.com/philsquared/Catch/blob/master/docs/tutorial.md

//
// fastmath tests
//
TEST_CASE("clampByte test")
{
    for (int index = -5000; index < 5000; index++)
    {
        auto x = clampByte(index);
        if (index < -128)
        {
            REQUIRE(x == -128);
        }
        else if (index > 127)
        {
            REQUIRE(x == 127);
        }
        else
        {
            REQUIRE(index == (int)x);
        }
    }
}

TEST_CASE("fast_inv_sqrt test")
{
    vector<float> vals{ 1.1f, 2.2f, 100.1f, 500.5f, 1234.56f, 3456789.0f };
    for (auto v : vals)
    {
        REQUIRE(fast_inv_sqrt(v) == Approx(1.0 / sqrtf(v)).epsilon(0.001));
    }
}

TEST_CASE("fast_sqrt test")
{
    vector<float> vals{ 1.1f, 2.2f, 100.1f, 500.5f, 1234.56f, 3456789.0f };
    for (auto v : vals)
    {
        REQUIRE(fast_sqrt(v) == Approx(sqrtf(v)).epsilon(0.03*sqrtf(v)));
    }
}

TEST_CASE("timing tests")
{

}
