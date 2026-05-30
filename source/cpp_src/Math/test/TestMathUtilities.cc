#include <cmath>
#include <complex>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "MHO_Constants.hh"
#include "MHO_MathUtilities.hh"
#include "MHO_TestAssertions.hh"

int main()
{
    //  Case 1: dwin clamp (5 sub-cases)
    {
        CHECK_CLOSE(hops::MHO_MathUtilities::dwin(0.5, 0.0, 1.0), 0.5, 1e-12);
        CHECK_CLOSE(hops::MHO_MathUtilities::dwin(-1.0, 0.0, 1.0), 0.0, 1e-12);
        CHECK_CLOSE(hops::MHO_MathUtilities::dwin(2.0, 0.0, 1.0), 1.0, 1e-12);
        CHECK_CLOSE(hops::MHO_MathUtilities::dwin(0.0, 0.0, 1.0), 0.0, 1e-12);
        CHECK_CLOSE(hops::MHO_MathUtilities::dwin(1.0, 0.0, 1.0), 1.0, 1e-12);
    }

    //  Case 2: parabola symmetric peak at center
    {
        double y[3] = {-1, 0, -1};
        double xmax, amp_max, q[3];
        int rc = hops::MHO_MathUtilities::parabola(y, -1, 1, &xmax, &amp_max, q);
        REQUIRE(rc == 0);
        CHECK_CLOSE(xmax, 0.0, 1e-12);
        CHECK_CLOSE(amp_max, 0.0, 1e-12);
        CHECK_CLOSE(q[0], -1.0, 1e-12);
        CHECK_CLOSE(q[1], 0.0, 1e-12);
        CHECK_CLOSE(q[2], 0.0, 1e-12);
    }

    //  Case 3: parabola off-center peak
    {
        double y[3] = {-1, 3, 2};
        double xmax, amp_max, q[3];
        int rc = hops::MHO_MathUtilities::parabola(y, -1, 1, &xmax, &amp_max, q);
        REQUIRE(rc == 0);
        CHECK_CLOSE(xmax, 0.3, 1e-12);
        CHECK_CLOSE(amp_max, 3.225, 1e-12);
    }

    //  Case 4: parabola positive curvature (no max)
    {
        double y[3] = {0, -1, 0};
        double xmax, amp_max, q[3];
        int rc = hops::MHO_MathUtilities::parabola(y, -1, 1, &xmax, &amp_max, q);
        REQUIRE(rc == 2);
    }

    //  Case 5: parabola peak outside range -> rc==1
    {
        double y[3] = {5, 3, 0};
        double xmax, amp_max, q[3];
        int rc = hops::MHO_MathUtilities::parabola(y, -0.1, 0.1, &xmax, &amp_max, q);
        REQUIRE(rc == 1);
        CHECK_CLOSE(xmax, -0.1, 1e-12);
    }

    //  Case 6: minvert3 identity matrix
    {
        double a[3][3] = {
            {1, 0, 0},
            {0, 1, 0},
            {0, 0, 1}
        };
        double ainv[3][3];
        int rc = hops::MHO_MathUtilities::minvert3(a, ainv);
        REQUIRE(rc == 0);
        for(int i = 0; i < 3; ++i)
        {
            for(int j = 0; j < 3; ++j)
            {
                double expected = (i == j) ? 1.0 : 0.0;
                CHECK_CLOSE(ainv[i][j], expected, 1e-12);
            }
        }
    }

    //  Case 7: minvert3 known invertible matrix
    {
        double a[3][3] = {
            {1, 2, 3},
            {0, 1, 4},
            {5, 6, 0}
        };
        double ainv[3][3];
        int rc = hops::MHO_MathUtilities::minvert3(a, ainv);
        REQUIRE(rc == 0);
        // reference inverse: [[-24,18,5],[20,-15,-4],[-5,4,1]]
        CHECK_CLOSE(ainv[0][0], -24.0, 1e-10);
        CHECK_CLOSE(ainv[0][1], 18.0, 1e-10);
        CHECK_CLOSE(ainv[0][2], 5.0, 1e-10);
        CHECK_CLOSE(ainv[1][0], 20.0, 1e-10);
        CHECK_CLOSE(ainv[1][1], -15.0, 1e-10);
        CHECK_CLOSE(ainv[1][2], -4.0, 1e-10);
        CHECK_CLOSE(ainv[2][0], -5.0, 1e-10);
        CHECK_CLOSE(ainv[2][1], 4.0, 1e-10);
        CHECK_CLOSE(ainv[2][2], 1.0, 1e-10);
    }

    //  Case 8: minvert3 singular matrix
    {
        double a[3][3] = {
            {1, 2, 3},
            {2, 4, 6},
            {1, 1, 1}
        };
        double ainv[3][3];
        int rc = hops::MHO_MathUtilities::minvert3(a, ainv);
        REQUIRE(rc != 0);
    }

    //  Case 9: linterp midpoint
    {
        double v;
        int rc = hops::MHO_MathUtilities::linterp(0, 10, 1, 20, 0.5, &v);
        REQUIRE(rc == 0);
        CHECK_CLOSE(v, 15.0, 1e-12);
    }

    //  Case 10: linterp reversed coord order
    {
        double v;
        int rc = hops::MHO_MathUtilities::linterp(1, 20, 0, 10, 0.25, &v);
        REQUIRE(rc == 0);
        CHECK_CLOSE(v, 12.5, 1e-12);
    }

    //  Case 11: linterp out-of-range
    {
        double v;
        int rc = hops::MHO_MathUtilities::linterp(0, 10, 1, 20, 1.5, &v);
        REQUIRE(rc == -1);
    }

    //  Case 12: linterp degenerate (coord1==coord2)
    {
        double v;
        int rc = hops::MHO_MathUtilities::linterp(0.5, 7, 0.5, 9, 0.5, &v);
        REQUIRE(rc == 0);
        CHECK_CLOSE(v, 7.0, 1e-12);
    }

    //  Case 13: plain average
    {
        std::vector< double > v1 = {1, 2, 3, 4, 5};
        CHECK_CLOSE(hops::MHO_MathUtilities::average(v1), 3.0, 1e-12);
        std::vector< double > v2 = {};
        CHECK_CLOSE(hops::MHO_MathUtilities::average(v2), 0.0, 1e-12);
    }

    //  Case 14: angular average
    {// single zero
     {std::vector< double > v = {0.0};
    CHECK_CLOSE(hops::MHO_MathUtilities::angular_average(v), 0.0, 1e-12);
}

// symmetric around 0
{
    std::vector< double > v = {M_PI / 4.0, -M_PI / 4.0};
    CHECK_CLOSE(hops::MHO_MathUtilities::angular_average(v), 0.0, 1e-12);
}
// wrapping around +/-pi: result is approximately +pi or -pi
{
    std::vector< double > v = {M_PI - 0.01, -(M_PI - 0.01)};
    double result = hops::MHO_MathUtilities::angular_average(v);
    double diffPos = std::abs(result - M_PI);
    double diffNeg = std::abs(result + M_PI);
    REQUIRE((diffPos < 1e-12) || (diffNeg < 1e-12));
}
}

//  Case 15: DetermineChannelFrequencyLimits USB
{
    double lo, hi;
    hops::MHO_MathUtilities::DetermineChannelFrequencyLimits(8000.0, 32.0, "U", lo, hi);
    CHECK_CLOSE(lo, 8000.0, 1e-12);
    CHECK_CLOSE(hi, 8032.0, 1e-12);
}

//  Case 16: DetermineChannelFrequencyLimits LSB
{
    double lo, hi;
    hops::MHO_MathUtilities::DetermineChannelFrequencyLimits(8000.0, 32.0, "L", lo, hi);
    CHECK_CLOSE(lo, 7968.0, 1e-12);
    CHECK_CLOSE(hi, 8000.0, 1e-12);
}

//  Case 17: DetermineChannelFrequencyLimits DSB
{
    double lo, hi;
    hops::MHO_MathUtilities::DetermineChannelFrequencyLimits(8000.0, 32.0, "", lo, hi);
    CHECK_CLOSE(lo, 7968.0, 1e-12);
    CHECK_CLOSE(hi, 8032.0, 1e-12);
}

//  Case 18: FindIntersection disjoint
{
    double result[2];
    int rc = hops::MHO_MathUtilities::FindIntersection(0.0, 1.0, 2.0, 3.0, result);
    REQUIRE(rc == 0);
}

//  Case 19: FindIntersection touching
{
    double result[2];
    int rc = hops::MHO_MathUtilities::FindIntersection(0.0, 1.0, 1.0, 2.0, result);
    REQUIRE(rc == 1);
    CHECK_CLOSE(result[0], 1.0, 1e-12);
}

//  Case 20: FindIntersection partial overlap
{
    double result[2];
    int rc = hops::MHO_MathUtilities::FindIntersection(0.0, 2.0, 1.0, 3.0, result);
    REQUIRE(rc == 2);
    CHECK_CLOSE(result[0], 1.0, 1e-12);
    CHECK_CLOSE(result[1], 2.0, 1e-12);
}

//  Case 21: FindIntersection contained
{
    double result[2];
    int rc = hops::MHO_MathUtilities::FindIntersection(0.0, 5.0, 1.0, 2.0, result);
    REQUIRE(rc == 2);
    CHECK_CLOSE(result[0], 1.0, 1e-12);
    CHECK_CLOSE(result[1], 2.0, 1e-12);
}

//  Case 22: FindIntersection reversed endpoints
{
    double result[2];
    int rc = hops::MHO_MathUtilities::FindIntersection(2.0, 0.0, 3.0, 1.0, result);
    REQUIRE(rc == 2);
    CHECK_CLOSE(result[0], 1.0, 1e-12);
    CHECK_CLOSE(result[1], 2.0, 1e-12);
}

//  Case 23: FindIntersection integer instantiation
{
    int result[2];
    int rc = hops::MHO_MathUtilities::FindIntersection< int >(0, 10, 5, 15, result);
    REQUIRE(rc == 2);
    REQUIRE(result[0] == 5);
    REQUIRE(result[1] == 10);
}

//  Case 24: ap_mean constant series
{
    double coords[] = {0, 1, 2, 3, 4};
    double val1[] = {2, 2, 2, 2, 2};
    double val2[] = {3, 3, 3, 3, 3};
    int n = 5;
    int nstart = 0;
    double r1, r2;
    int rc = hops::MHO_MathUtilities::ap_mean(1, 3, coords, val1, val2, n, &nstart, &r1, &r2);
    REQUIRE(rc == 0);
    CHECK_CLOSE(r1, 2.0, 1e-12);
    CHECK_CLOSE(r2, 3.0, 1e-12);
}

//  Case 25: ap_mean out-of-range
{
    double coords[] = {10, 11, 12};
    double val1[] = {1, 1, 1};
    double val2[] = {1, 1, 1};
    int n = 3;
    int nstart = 0;
    double r1, r2;
    int rc = hops::MHO_MathUtilities::ap_mean(0, 5, coords, val1, val2, n, &nstart, &r1, &r2);
    REQUIRE(rc == 1);
    CHECK_CLOSE(r1, 0.0, 1e-12);
    CHECK_CLOSE(r2, 0.0, 1e-12);
}

return 0;
}
