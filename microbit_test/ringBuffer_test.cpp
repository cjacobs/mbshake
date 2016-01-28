#include "ringBuffer.h"

#include "catch.hpp"

#include <vector>
using std::vector;

// See catch tutorial: https://github.com/philsquared/Catch/blob/master/docs/tutorial.md

//
// ringBuffer tests
//
TEST_CASE("ringBuffer test")
{
    vector<float> vals {10, 11, 12, 13, 14, 15, 16, 17, 18, 1, 2, 3, 4, 5};

    ringBuffer<float, 5> ringBuf;
    for (auto v: vals)
    {
        ringBuf.push_back(v);
    }

    // Is this really what we want? Maybe fix it...
    REQUIRE(ringBuf[0] == 5);
    REQUIRE(ringBuf[-1] == 4);
    REQUIRE(ringBuf[-2] == 3);
    REQUIRE(ringBuf[-3] == 2);
    REQUIRE(ringBuf[-4] == 1);
}
