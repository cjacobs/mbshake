#include "delayBuffer.h"
#include "runningStats.h"

#include "catch.hpp"

#include <vector>
using std::vector;

// See catch tutorial: https://github.com/philsquared/Catch/blob/master/docs/tutorial.md

//
// runningStats tests
//

TEST_CASE("runningStats test")
{
    vector<float> vals{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
    delayBuffer<float, 5> delayBuf;
    runningStats<4, 5, float> stats(delayBuf);

    for (int index = 0; index < 4; index++)
    {
        delayBuf.addSample(vals[index]);
        stats.addSample(vals[index]);
    }
    REQUIRE(stats.getMean() == (0 + 1 + 2 + 3) / 4.0);
    REQUIRE(stats.getVar() == Approx(1.25));
    REQUIRE(stats.getStdDev() == Approx(1.1180339887498949).epsilon(0.001));


    for (int index = 4; index < 8; index++)
    {
        delayBuf.addSample(vals[index]);
        stats.addSample(vals[index]);
    }
    REQUIRE(stats.getMean() == (4 + 5 + 6 + 7) / 4.0);
    REQUIRE(stats.getVar() == Approx(1.25));
    REQUIRE(stats.getStdDev() == Approx(1.1180339887498949).epsilon(0.001));
}
