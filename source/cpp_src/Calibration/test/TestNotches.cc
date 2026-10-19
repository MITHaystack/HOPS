#include <cmath>
#include <complex>
#include <iostream>
#include <string>
#include <vector>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_Notches.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

/* The notch mask reproduces fourfit3 (apply_usb_notch/apply_lsb_notch):
   spectral point i sits at sky + i*df (USB) or sky - i*df (LSB), the notch edges are
   rounded to the nearest point index, and only points strictly between them are zeroed.

   Shared fixture: 1 pol, 1 chan, 1 AP, 80 spectral points.
   Channel 0: sky=8000, bandwidth=8.0 => df = 0.1 MHz.
   All visibilities (1,0); weights all 1.0. */
static const std::size_t NPTS = 80;

static void build_fixture(visibility_type& vis, weight_type& wt, const std::string& sideband)
{
    vis.Resize(1, 1, 1, NPTS);
    wt.Resize(1, 1, 1, 1);

    auto& chan_ax = std::get< CHANNEL_AXIS >(vis);
    chan_ax.at(0) = 8000.0;
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", sideband);
    chan_ax.InsertIndexLabelKeyValue(0, "bandwidth", 8.0);

    auto& freq_ax = std::get< FREQ_AXIS >(vis);
    for(std::size_t sp = 0; sp < NPTS; sp++)
    {
        freq_ax.at(sp) = sp * 0.1;
        vis(0, 0, 0, sp) = std::complex< double >(1.0, 0.0);
    }

    wt(0, 0, 0, 0) = 1.0;
}

static int run_notches(visibility_type& vis, weight_type& wt, const std::vector< double >& boundaries)
{
    MHO_Notches op;
    op.SetNotchBoundaries(boundaries);
    op.SetWeights(&wt);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());
    return 0;
}

/* check that exactly the points in [first, last] are zeroed, and the weight/labels reflect the count */
static int check_mask(visibility_type& vis, weight_type& wt, int first, int last)
{
    int count = 0;
    for(int sp = 0; sp < (int)NPTS; sp++)
    {
        double expected = (sp >= first && sp <= last) ? 0.0 : 1.0;
        if(expected == 0.0)
        {
            count++;
        }
        CHECK_CLOSE(vis(0, 0, 0, sp).real(), expected, 1e-12);
        CHECK_CLOSE(vis(0, 0, 0, sp).imag(), 0.0, 1e-12);
    }

    double frac = (NPTS - count) / (double)NPTS;
    double factor = (frac > 0.0) ? 1.0 / frac : 0.0;
    CHECK_CLOSE(wt(0, 0, 0, 0), factor, 1e-12);

    double ubf, rf;
    auto& chan_ax = std::get< CHANNEL_AXIS >(vis);
    REQUIRE(chan_ax.RetrieveIndexLabelKeyValue(0, "used_bandwidth_fraction", ubf));
    CHECK_CLOSE(ubf, frac, 1e-12);
    REQUIRE(chan_ax.RetrieveIndexLabelKeyValue(0, "rescaling_factor", rf));
    CHECK_CLOSE(rf, factor, 1e-12);
    return 0;
}

// Test cases

/* CASE 1 - USB single notch (8001.81, 8002.11).
   Edges at 18.1 and 21.1 points => rounded to 18 and 21 => zero points 19, 20.
   Note point 21 (8002.1) is inside the notch but is kept, as in fourfit3. */
static int test_case1_usb_single_notch()
{
    visibility_type vis;
    weight_type wt;
    build_fixture(vis, wt, "U");
    if(run_notches(vis, wt, {8001.81, 8002.11}))
        return 1;
    return check_mask(vis, wt, 19, 20);
}

/* CASE 2 - Duplicate and overlapping notches are only counted once.
   (8001.81, 8002.11) twice => points 19, 20;
   (8001.96, 8002.36) => edges 19.6, 23.6 rounded to 20, 24 => points 21..23.
   Union => points 19..23 (5 points). */
static int test_case2_overlapping_notches()
{
    visibility_type vis;
    weight_type wt;
    build_fixture(vis, wt, "U");
    if(run_notches(vis, wt, {8001.81, 8002.11, 8001.81, 8002.11, 8001.96, 8002.36}))
        return 1;
    return check_mask(vis, wt, 19, 23);
}

/* CASE 3 - Odd-length boundary list drops the trailing value, identical result to CASE 1. */
static int test_case3_odd_boundary_list()
{
    visibility_type vis;
    weight_type wt;
    build_fixture(vis, wt, "U");
    if(run_notches(vis, wt, {8001.81, 8002.11, 8007.5}))
        return 1;
    return check_mask(vis, wt, 19, 20);
}

/* CASE 4 - LSB mirror of CASE 1: point i sits at 8000 - i*0.1.
   Notch (7997.89, 7998.19) => edges at 18.1 and 21.1 points from the DC edge => zero points 19, 20. */
static int test_case4_lsb_single_notch()
{
    visibility_type vis;
    weight_type wt;
    build_fixture(vis, wt, "L");
    if(run_notches(vis, wt, {7997.89, 7998.19}))
        return 1;
    return check_mask(vis, wt, 19, 20);
}

/* CASE 5 - Notch entirely outside the channel => no-op, labels still written. */
static int test_case5_notch_outside()
{
    visibility_type vis;
    weight_type wt;
    build_fixture(vis, wt, "U");
    if(run_notches(vis, wt, {9000.0, 9100.0}))
        return 1;
    return check_mask(vis, wt, 1, 0);
}

/* CASE 6 - Notch straddling the DC edge of a USB channel (7999.9, 8000.11).
   Lower edge below the channel => -1, upper edge at 1.1 points => 1 => zero point 0 only. */
static int test_case6_usb_dc_edge()
{
    visibility_type vis;
    weight_type wt;
    build_fixture(vis, wt, "U");
    if(run_notches(vis, wt, {7999.9, 8000.11}))
        return 1;
    return check_mask(vis, wt, 0, 0);
}

/* CASE 7 - Notch straddling the DC edge of an LSB channel (7999.89, 8000.1).
   Upper edge above the channel => -1, lower edge at 1.1 points from DC => 1 => zero point 0 only. */
static int test_case7_lsb_dc_edge()
{
    visibility_type vis;
    weight_type wt;
    build_fixture(vis, wt, "L");
    if(run_notches(vis, wt, {7999.89, 8000.1}))
        return 1;
    return check_mask(vis, wt, 0, 0);
}

/* CASE 8 - Notch covering the whole channel => everything zeroed, weights zeroed. */
static int test_case8_full_channel()
{
    visibility_type vis;
    weight_type wt;
    build_fixture(vis, wt, "U");
    if(run_notches(vis, wt, {7990.0, 8010.0}))
        return 1;
    return check_mask(vis, wt, 0, NPTS - 1);
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if(test_case1_usb_single_notch())
        return 1;
    if(test_case2_overlapping_notches())
        return 1;
    if(test_case3_odd_boundary_list())
        return 1;
    if(test_case4_lsb_single_notch())
        return 1;
    if(test_case5_notch_outside())
        return 1;
    if(test_case6_usb_dc_edge())
        return 1;
    if(test_case7_lsb_dc_edge())
        return 1;
    if(test_case8_full_channel())
        return 1;

    return 0;
}
