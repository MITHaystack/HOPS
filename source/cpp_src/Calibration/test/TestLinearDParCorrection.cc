#include <cmath>
#include <iostream>
#include <string>
#include <complex>
#include <vector>
#include <algorithm>

#include "MHO_LinearDParCorrection.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Helpers

/* Build the shared fixture: npol=4, nchan=2, nap=2, nspec=2.
   Pol-products: XX, YY, XY, YX. All elements (2.0, -3.0). */
static void build_fixture(visibility_type& vis)
{
    vis.Resize(4, 2, 2, 2);

    auto& pp_ax = std::get<POLPROD_AXIS>(vis);
    pp_ax.at(0) = "XX";
    pp_ax.at(1) = "YY";
    pp_ax.at(2) = "XY";
    pp_ax.at(3) = "YX";

    // Fill all elements with (2.0, -3.0)
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    vis(pp, ch, ap, sp) = std::complex<double>(2.0, -3.0);
}

// Verify every element in vis equals (re, im) within tolerance.
static int assert_all_near(visibility_type& vis, double re, double im, double tol)
{
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++) {
                    CHECK_CLOSE(vis(pp, ch, ap, sp).real(), re, tol);
                    CHECK_CLOSE(vis(pp, ch, ap, sp).imag(), im, tol);
                }
    return 0;
}

// Verify a specific pol-product sub-view equals (re, im) within tolerance.
static int assert_pol_near(visibility_type& vis, std::size_t pp,
                           double re, double im, double tol)
{
    for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
        for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
            for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++) {
                CHECK_CLOSE(vis(pp, ch, ap, sp).real(), re, tol);
                CHECK_CLOSE(vis(pp, ch, ap, sp).imag(), im, tol);
            }
    return 0;
}

// Test cases

/* CASE 1 - dpar = +30 deg: cos>0, sin>0
   XX: +1, YY: +1, YX: +1, XY: -1 */
static int test_dpar_30()
{
    visibility_type vis;
    build_fixture(vis);

    std::vector<std::string> ppset = {"XX", "YY", "XY", "YX"};

    MHO_LinearDParCorrection op;
    op.SetPolProductSet(ppset);
    op.SetReferenceParallacticAngle(10.0);
    op.SetRemoteParallacticAngle(40.0);  // dpar = +30 deg

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // XX: *+1 => (2.0, -3.0)
    assert_pol_near(vis, 0,  2.0, -3.0, 1e-12);
    // YY: *+1 => (2.0, -3.0)
    assert_pol_near(vis, 1,  2.0, -3.0, 1e-12);
    // XY: *-1 => (-2.0, 3.0)
    assert_pol_near(vis, 2, -2.0,  3.0, 1e-12);
    // YX: *+1 => (2.0, -3.0)
    assert_pol_near(vis, 3,  2.0, -3.0, 1e-12);

    return 0;
}

/* CASE 2 - dpar = +120 deg: cos<0, sin>0
   XX/YY: -1, YX: +1, XY: -1 */
static int test_dpar_120()
{
    visibility_type vis;
    build_fixture(vis);

    std::vector<std::string> ppset = {"XX", "YY", "XY", "YX"};

    MHO_LinearDParCorrection op;
    op.SetPolProductSet(ppset);
    op.SetReferenceParallacticAngle(0.0);
    op.SetRemoteParallacticAngle(120.0);  // dpar = +120 deg

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // XX: *-1 => (-2.0, 3.0)
    assert_pol_near(vis, 0, -2.0,  3.0, 1e-12);
    // YY: *-1 => (-2.0, 3.0)
    assert_pol_near(vis, 1, -2.0,  3.0, 1e-12);
    // XY: *-1 => (-2.0, 3.0)
    assert_pol_near(vis, 2, -2.0,  3.0, 1e-12);
    // YX: *+1 => (2.0, -3.0)
    assert_pol_near(vis, 3,  2.0, -3.0, 1e-12);

    return 0;
}

/* CASE 3 - Pol-product not in the configured set is zeroed.
   ppset = {"XX", "YY"}, dpar = +30 deg */
static int test_missing_pol_zeroed()
{
    visibility_type vis;
    build_fixture(vis);

    std::vector<std::string> ppset = {"XX", "YY"};

    MHO_LinearDParCorrection op;
    op.SetPolProductSet(ppset);
    op.SetReferenceParallacticAngle(10.0);
    op.SetRemoteParallacticAngle(40.0);  // dpar = +30 deg

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // XX: in set, *+1 => (2.0, -3.0)
    assert_pol_near(vis, 0,  2.0, -3.0, 1e-12);
    // YY: in set, *+1 => (2.0, -3.0)
    assert_pol_near(vis, 1,  2.0, -3.0, 1e-12);
    // XY: not in set => (0.0, 0.0)
    assert_pol_near(vis, 2,  0.0,  0.0, 1e-12);
    // YX: not in set => (0.0, 0.0)
    assert_pol_near(vis, 3,  0.0,  0.0, 1e-12);

    return 0;
}

/* CASE 4 - Boundary: dpar == 0 (sin==0, cos==1).
   All prefactors +1 => unchanged. */
static int test_dpar_zero()
{
    visibility_type vis;
    build_fixture(vis);

    std::vector<std::string> ppset = {"XX", "YY", "XY", "YX"};

    MHO_LinearDParCorrection op;
    op.SetPolProductSet(ppset);
    op.SetReferenceParallacticAngle(25.0);
    op.SetRemoteParallacticAngle(25.0);  // dpar = 0 deg

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // All *+1 => unchanged (2.0, -3.0)
    assert_all_near(vis, 2.0, -3.0, 1e-12);

    return 0;
}

// CASE 5 - Empty pol-product set zeroes everything.
static int test_empty_polset()
{
    visibility_type vis;
    build_fixture(vis);

    std::vector<std::string> ppset;  // empty

    MHO_LinearDParCorrection op;
    op.SetPolProductSet(ppset);
    op.SetReferenceParallacticAngle(10.0);
    op.SetRemoteParallacticAngle(40.0);  // dpar = +30 deg

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // All zeroed => (0.0, 0.0)
    assert_all_near(vis, 0.0, 0.0, 1e-12);

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_dpar_30())           return 1;
    if (test_dpar_120())          return 1;
    if (test_missing_pol_zeroed()) return 1;
    if (test_dpar_zero())         return 1;
    if (test_empty_polset())      return 1;

    return 0;
}
