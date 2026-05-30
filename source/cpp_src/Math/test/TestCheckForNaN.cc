#include "MHO_CheckForNaN.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"
#include <cmath>
#include <complex>
#include <iostream>
#include <limits>

using namespace hops;

/*
 * Note: the primary MHO_CheckForNaN<T>::isnan() template uses (x != x),
 * which is silently defeated by -ffast-math. HOPS does NOT enable
 * -ffast-math by default, so these tests are valid for the standard build.
 */

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // Verify IEEE-754 NaN support is available (always true on supported platforms)
    if(!std::numeric_limits< float >::has_quiet_NaN || !std::numeric_limits< double >::has_quiet_NaN ||
       !std::numeric_limits< long double >::has_quiet_NaN)
    {
        // Skip test - platform lacks NaN (extremely unlikely)
        return 0;
    }

    //  Scalar types
    // Case float
    {
        REQUIRE(MHO_CheckForNaN< float >::isnan(float(0)) == false);
        REQUIRE(MHO_CheckForNaN< float >::isnan(float(1.5f)) == false);
        REQUIRE(MHO_CheckForNaN< float >::isnan(float(-1.5f)) == false);
        REQUIRE(MHO_CheckForNaN< float >::isnan(std::numeric_limits< float >::quiet_NaN()) == true);
        REQUIRE(MHO_CheckForNaN< float >::isnan(std::numeric_limits< float >::infinity()) == false);
        REQUIRE(MHO_CheckForNaN< float >::isnan(-std::numeric_limits< float >::infinity()) == false);
    }

    // Case double
    {
        REQUIRE(MHO_CheckForNaN< double >::isnan(double(0)) == false);
        REQUIRE(MHO_CheckForNaN< double >::isnan(double(1.5)) == false);
        REQUIRE(MHO_CheckForNaN< double >::isnan(double(-1.5)) == false);
        REQUIRE(MHO_CheckForNaN< double >::isnan(std::numeric_limits< double >::quiet_NaN()) == true);
        REQUIRE(MHO_CheckForNaN< double >::isnan(std::numeric_limits< double >::infinity()) == false);
        REQUIRE(MHO_CheckForNaN< double >::isnan(-std::numeric_limits< double >::infinity()) == false);
    }

    // Case long double
    {
        long double ld0 = 0;
        long double ldPos = 1.5L;
        long double ldNeg = -1.5L;
        long double ldNaN = std::numeric_limits< long double >::quiet_NaN();
        long double ldInf = std::numeric_limits< long double >::infinity();
        REQUIRE(MHO_CheckForNaN< long double >::isnan(ld0) == false);
        REQUIRE(MHO_CheckForNaN< long double >::isnan(ldPos) == false);
        REQUIRE(MHO_CheckForNaN< long double >::isnan(ldNeg) == false);
        REQUIRE(MHO_CheckForNaN< long double >::isnan(ldNaN) == true);
        REQUIRE(MHO_CheckForNaN< long double >::isnan(ldInf) == false);
        REQUIRE(MHO_CheckForNaN< long double >::isnan(-ldInf) == false);
    }

    //  Complex types
    // Case complex<float>
    {
        auto n = std::numeric_limits< float >::quiet_NaN();
        auto inf = std::numeric_limits< float >::infinity();
        REQUIRE(MHO_CheckForNaN< std::complex< float > >::isnan(std::complex< float >(0, 0)) == false);
        REQUIRE(MHO_CheckForNaN< std::complex< float > >::isnan(std::complex< float >(1, 2)) == false);
        REQUIRE(MHO_CheckForNaN< std::complex< float > >::isnan(std::complex< float >(n, 0)) == true);
        REQUIRE(MHO_CheckForNaN< std::complex< float > >::isnan(std::complex< float >(0, n)) == true);
        REQUIRE(MHO_CheckForNaN< std::complex< float > >::isnan(std::complex< float >(n, n)) == true);
        REQUIRE(MHO_CheckForNaN< std::complex< float > >::isnan(std::complex< float >(inf, 0)) == false);
        REQUIRE(MHO_CheckForNaN< std::complex< float > >::isnan(std::complex< float >(0, inf)) == false);
    }

    // Case complex<double>
    {
        auto n = std::numeric_limits< double >::quiet_NaN();
        auto inf = std::numeric_limits< double >::infinity();
        REQUIRE(MHO_CheckForNaN< std::complex< double > >::isnan(std::complex< double >(0, 0)) == false);
        REQUIRE(MHO_CheckForNaN< std::complex< double > >::isnan(std::complex< double >(1, 2)) == false);
        REQUIRE(MHO_CheckForNaN< std::complex< double > >::isnan(std::complex< double >(n, 0)) == true);
        REQUIRE(MHO_CheckForNaN< std::complex< double > >::isnan(std::complex< double >(0, n)) == true);
        REQUIRE(MHO_CheckForNaN< std::complex< double > >::isnan(std::complex< double >(n, n)) == true);
        REQUIRE(MHO_CheckForNaN< std::complex< double > >::isnan(std::complex< double >(inf, 0)) == false);
        REQUIRE(MHO_CheckForNaN< std::complex< double > >::isnan(std::complex< double >(0, inf)) == false);
    }

    // Case complex<long double>
    {
        auto n = std::numeric_limits< long double >::quiet_NaN();
        auto inf = std::numeric_limits< long double >::infinity();
        REQUIRE(MHO_CheckForNaN< std::complex< long double > >::isnan(std::complex< long double >(0, 0)) == false);
        REQUIRE(MHO_CheckForNaN< std::complex< long double > >::isnan(std::complex< long double >(1, 2)) == false);
        REQUIRE(MHO_CheckForNaN< std::complex< long double > >::isnan(std::complex< long double >(n, 0)) == true);
        REQUIRE(MHO_CheckForNaN< std::complex< long double > >::isnan(std::complex< long double >(0, n)) == true);
        REQUIRE(MHO_CheckForNaN< std::complex< long double > >::isnan(std::complex< long double >(n, n)) == true);
        REQUIRE(MHO_CheckForNaN< std::complex< long double > >::isnan(std::complex< long double >(inf, 0)) == false);
        REQUIRE(MHO_CheckForNaN< std::complex< long double > >::isnan(std::complex< long double >(0, inf)) == false);
    }

    return 0;
}
