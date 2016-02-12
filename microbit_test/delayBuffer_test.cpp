#include "DelayBuffer.h"

#include "catch.hpp"

// See catch tutorial: https://github.com/philsquared/Catch/blob/master/docs/tutorial.md

//
// delayBuffer tests
//
TEST_CASE("delayBuffer test")
{
    DelayBuffer<int, 5> delayBuf;

    for (int index = 0; index < 6; index++)
    {
        delayBuf.addSample(index);
    }
    REQUIRE(delayBuf.getDelayedSample(0) == 5);
    REQUIRE(delayBuf.getDelayedSample(1) == 4);
    REQUIRE(delayBuf.getDelayedSample(2) == 3);
    REQUIRE(delayBuf.getDelayedSample(3) == 2);
    REQUIRE(delayBuf.getDelayedSample(4) == 1);

    for (int index = 10; index < 12; index++)
    {
        delayBuf.addSample(index);
    }
    REQUIRE(delayBuf.getDelayedSample(0) == 11);
    REQUIRE(delayBuf.getDelayedSample(1) == 10);
    REQUIRE(delayBuf.getDelayedSample(2) == 5);
    REQUIRE(delayBuf.getDelayedSample(3) == 4);
    REQUIRE(delayBuf.getDelayedSample(4) == 3);

}
