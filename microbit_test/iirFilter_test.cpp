#include "IirFilter.h"
#include "FixedPt.h"

#include "catch.hpp"

#include <vector>
using std::vector;

// See catch tutorial: https://github.com/philsquared/Catch/blob/master/docs/tutorial.md

//
// iirFilter tests
//

TEST_CASE("iirFilter")
{
    float x = 0.0f;

    IirFilter<float, 2, 0> filt({0.5, 0.5}, {});
    vector<float> vals {10, 11, 12, 13, 14, 15, 16, 17, 18, 1, 2, 3, 4, 5};
    x = filt.filterSample(1.0);
    x = filt.filterSample(1.0);
    x = filt.filterSample(1.0);
    x = filt.filterSample(1.0);
    x = filt.filterSample(1.0);
    REQUIRE(x == Approx(1.0));

    IirFilter<float, 1, 1> filt2({1.0}, {-0.5});
    x = filt2.filterSample(1.0);
    REQUIRE(x == 1.0);
    x = filt2.filterSample(0.0);
    REQUIRE(x == 0.5);
    x = filt2.filterSample(0.0);
    REQUIRE(x == 0.25);
}

TEST_CASE("simpleIIRFilter")
{
    float x = 0.0f;

    SimpleIirFilter<float> filt(0.5);
    filt.init(1.0);
    vector<float> vals {10, 11, 12, 13, 14, 15, 16, 17, 18, 1, 2, 3, 4, 5};
    x = filt.filterSample(1.0);
    x = filt.filterSample(1.0);
    x = filt.filterSample(1.0);
    x = filt.filterSample(1.0);
    x = filt.filterSample(1.0);
    REQUIRE(x == Approx(1.0));

    IirFilter<float, 1, 1> filt2({1.0}, {-0.5});
    x = filt2.filterSample(1.0);
    REQUIRE(x == 1.0);
    x = filt2.filterSample(0.0);
    REQUIRE(x == 0.5);
    x = filt2.filterSample(0.0);
    REQUIRE(x == 0.25);
}

TEST_CASE("simpleIIRFilter_fixed")
{
    float x = 0.0f;

    fixed_9_7 alpha = 0.5;
    SimpleIirFilter<fixed_9_7> filt(alpha);
    filt.init(1.0);
    vector<float> vals {10, 11, 12, 13, 14, 15, 16, 17, 18, 1, 2, 3, 4, 5};
    x = filt.filterSample(1.0);
    x = filt.filterSample(1.0);
    x = filt.filterSample(1.0);
    x = filt.filterSample(1.0);
    x = filt.filterSample(1.0);
    REQUIRE(x == Approx(1.0));

    IirFilter<float, 1, 1> filt2({1.0}, {-0.5});
    x = filt2.filterSample(1.0);
    REQUIRE(x == 1.0);
    x = filt2.filterSample(0.0);
    REQUIRE(x == 0.5);
    x = filt2.filterSample(0.0);
    REQUIRE(x == 0.25);
}


