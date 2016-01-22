#include "classifiers.h"
#include "delayBuffer.h"
#include "eventThresholdFilter.h"
#include "fastmath.h"
#include "iirFilter.h"
#include "ringBuffer.h"
#include "runningStats.h"
#include "vec3.h"

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include <cmath>
#include <vector>
using std::vector;

#include <iostream>
using std::cout;
using std::endl;

// See catch tutorial: https://github.com/philsquared/Catch/blob/master/docs/tutorial.md

//
// delayBuffer tests
//
TEST_CASE("delayBuffer test")
{
    vector<float> vals{ 10, 11, 12, 13, 14, 15, 16, 17, 18, 1, 2, 3, 4, 5 };

    float delayBufferMem[5];
    delayBuffer<float> delayBuf(delayBufferMem, 5);
    for (auto v : vals)
    {
        delayBuf.addSample(v);
    }

    // Is this really what we want? Maybe fix it...
    REQUIRE(delayBuf.getDelayedSample(0) == 5);
    REQUIRE(delayBuf.getDelayedSample(1) == 4);
    REQUIRE(delayBuf.getDelayedSample(2) == 3);
    REQUIRE(delayBuf.getDelayedSample(3) == 2);
    REQUIRE(delayBuf.getDelayedSample(4) == 1);
}

//
// fastmath tests
//
TEST_CASE("fast_inv_sqrt test")
{
    vector<float> vals{ 1.1f, 2.2f, 100.1f, 500.5f, 1234.56f, 3456789.0f };
    for (auto v : vals)
    {
        REQUIRE(fast_inv_sqrt(v) == Approx(1.0 / sqrtf(v)).epsilon(0.001));
    }
}

TEST_CASE("fixed_pt test")
{
    fixed_pt<int16_t, 8> x = 3;
    fixed_pt<int16_t, 8> y = 0.5f;
    REQUIRE(int(x) == 3);
    REQUIRE(float(x) == 3.0);
    
    REQUIRE(float(x+y) == 3.5);
    REQUIRE(float(x*y) == 1.5);
}

//
// iirFilter tests
//

TEST_CASE("iirFilter")
{
    float x = 0.0f;

    iir_filter<float, 2, 0> filt({0.5, 0.5}, {});
    vector<float> vals {10, 11, 12, 13, 14, 15, 16, 17, 18, 1, 2, 3, 4, 5};
    filt.filterSample(1.0);
    filt.filterSample(1.0);
    filt.filterSample(1.0);
    filt.filterSample(1.0);
    x = filt.filterSample(1.0);
    REQUIRE(x == Approx(1.0));

    iir_filter<float, 0, 1> filt2({}, {0.5});
    filt2.filterSample(1.0);
    filt2.filterSample(0.0);
    x = filt2.filterSample(0.0);
    REQUIRE(x == 0.25);
}

//
// ringBuffer tests
//
TEST_CASE("ringBuffer test")
{
    vector<float> vals {10, 11, 12, 13, 14, 15, 16, 17, 18, 1, 2, 3, 4, 5};

    ring_buffer<float, 5> ringBuf;
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

//
// runningStats tests
//

TEST_CASE("runningStats test")
{
    vector<float> vals{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
    float delayBufferMem[5];
    delayBuffer<float> delayBuf(delayBufferMem, 5); // ERROR(?) size of delayBuf must be one more than runningStats
    runningStats<float> stats(4, delayBuf);

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

//
// vec3 tests
//

// TODO: test element access, Accessor template things (GetX, GetY, GetZ, GetMagSq)
//         dot, dotNorm, normSq, op-
