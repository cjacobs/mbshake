#include "FixedPt.h"
#include "Vector3.h"

#include "catch.hpp"

#include <vector>
#include <iostream>

using std::vector;
using std::ostream;
using std::cout;
using std::endl;

template <typename T>
ostream& operator << (ostream& out, const Vector3<T>& v)
{
    out << "(" << float(v.x) << ", " << float(v.y) << ", " << float(v.z) << ")";
    return out;
}

// See catch tutorial: https://github.com/philsquared/Catch/blob/master/docs/tutorial.md
TEST_CASE("fixed_point_vector")
{
    for (auto floatVec: {floatVector3(1,2,3), floatVector3(5,6,7), floatVector3(10,20,30), floatVector3(0.5,0.25,0.1)})
    {
        Vector3<fixed_9_7> fixedVec = Vector3<fixed_9_7>(floatVec);
        REQUIRE(floatVec.x == Approx(float(fixedVec.x)).epsilon(0.01));
        REQUIRE(floatVec.y == Approx(float(fixedVec.y)).epsilon(0.01));
        REQUIRE(floatVec.z == Approx(float(fixedVec.z)).epsilon(0.01));
    }
}

TEST_CASE("fixed_point dot")
{
    for (auto floatVec: {floatVector3(1,2,3), floatVector3(5,6,7), floatVector3(10,10,12), floatVector3(0.5,0.25,0.1)})
    {
        Vector3<FixedPt<5, 11>> fixedVec = Vector3<FixedPt<5, 11>>(floatVec);
        REQUIRE(floatVec.x == Approx(float(fixedVec.x)).epsilon(0.01));
        REQUIRE(floatVec.y == Approx(float(fixedVec.y)).epsilon(0.01));

        float dotVal = dot(floatVec, floatVec);
        auto dotFixedVal = dot(fixedVec, fixedVec);

        float temp = float(dotFixedVal);

        REQUIRE(dotVal == Approx(float(dotFixedVal)).epsilon(0.01));
    }    
}

TEST_CASE("fixed_point dot norm")
{
    for (auto floatVec: {floatVector3(1,2,3), floatVector3(5,6,7), floatVector3(10,10,12), floatVector3(0.5,0.25,0.1)})
    {
        Vector3<FixedPt<9,7>> fixedVec = Vector3<FixedPt<9,7>>(floatVec);
        REQUIRE(floatVec.x == Approx(float(fixedVec.x)).epsilon(0.01));
        REQUIRE(floatVec.y == Approx(float(fixedVec.y)).epsilon(0.01));
        float dotNormVal = dotNorm(floatVec, floatVec);

        auto dotNormFixedVal = dotNormFixed(fixedVec, fixedVec);
        float dotNormFixedTemp = float(dotNormFixedVal);
        cout << "Calculating dotNorm(v,v) for vec " << floatVec << "  = " << dotNormVal << " --> " << dotNormFixedTemp << endl;
        REQUIRE(dotNormVal == Approx(float(dotNormFixedVal)).epsilon(0.3));
    }    
}
