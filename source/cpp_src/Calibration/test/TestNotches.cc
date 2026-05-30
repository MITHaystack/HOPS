#include <cmath>
#include <iostream>
#include <string>
#include <complex>
#include <vector>

#include "MHO_Notches.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

/* Build the shared USB fixture: 1 pol, 1 chan, 1 AP, 4 spectral points.
   Channel 0: sky=8000, net_sideband="U", bandwidth=8.0.
   FREQ_AXIS deltaf = {1.0, 3.0, 5.0, 7.0}
   => sp_freqs = {8001, 8003, 8005, 8007}.
   All visibilities (1,0); weights all 1.0. */
static void build_fixture_usb(visibility_type& vis, weight_type& wt)
{
    vis.Resize(1, 1, 1, 4);
    wt.Resize(1, 1, 1, 1);

    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    chan_ax.at(0) = 8000.0;
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("U"));
    chan_ax.InsertIndexLabelKeyValue(0, "bandwidth", 8.0);

    auto& freq_ax = std::get<FREQ_AXIS>(vis);
    freq_ax.at(0) = 1.0;
    freq_ax.at(1) = 3.0;
    freq_ax.at(2) = 5.0;
    freq_ax.at(3) = 7.0;

    for (std::size_t sp = 0; sp < 4; sp++)
        vis(0, 0, 0, sp) = std::complex<double>(1.0, 0.0);

    wt(0, 0, 0, 0) = 1.0;
}

/* Build the shared LSB fixture: 1 pol, 1 chan, 1 AP, 4 spectral points.
   Channel 0: sky=8000, net_sideband="L", bandwidth=8.0.
   FREQ_AXIS deltaf = {1.0, 3.0, 5.0, 7.0}
   sb = -1 => sp_freqs = {7999, 7997, 7995, 7993}. */
static void build_fixture_lsb(visibility_type& vis, weight_type& wt)
{
    vis.Resize(1, 1, 1, 4);
    wt.Resize(1, 1, 1, 1);

    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    chan_ax.at(0) = 8000.0;
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("L"));
    chan_ax.InsertIndexLabelKeyValue(0, "bandwidth", 8.0);

    auto& freq_ax = std::get<FREQ_AXIS>(vis);
    freq_ax.at(0) = 1.0;
    freq_ax.at(1) = 3.0;
    freq_ax.at(2) = 5.0;
    freq_ax.at(3) = 7.0;

    for (std::size_t sp = 0; sp < 4; sp++)
        vis(0, 0, 0, sp) = std::complex<double>(1.0, 0.0);

    wt(0, 0, 0, 0) = 1.0;
}

// Test cases

/* CASE 1 - Single notch zeroes interior points.
   Notch (8002, 8006) catches sp1 (8003) and sp2 (8005).
   sp0 (8001) and sp3 (8007) remain intact. */
static int test_case1_single_notch()
{
    visibility_type vis;
    weight_type wt;
    build_fixture_usb(vis, wt);

    MHO_Notches op;
    op.SetNotchBoundaries(std::vector<double>{8002.0, 8006.0});
    op.SetWeights(&wt);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // sp0 (8001) and sp3 (8007) intact
    CHECK_CLOSE(vis(0, 0, 0, 0).real(), 1.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 0).imag(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 3).real(), 1.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 3).imag(), 0.0, 1e-12);

    // sp1 (8003) and sp2 (8005) zeroed
    CHECK_CLOSE(vis(0, 0, 0, 1).real(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 1).imag(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 2).real(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 2).imag(), 0.0, 1e-12);

    // count=2, npts=4, frac=0.5, factor=2.0
    CHECK_CLOSE(wt(0, 0, 0, 0), 2.0, 1e-12);

    // Labels on VIS channel axis
    {
        double ubf, rf;
        auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
        REQUIRE(chan_ax.RetrieveIndexLabelKeyValue(0, "used_bandwidth_fraction", ubf));
        CHECK_CLOSE(ubf, 0.5, 1e-12);
        REQUIRE(chan_ax.RetrieveIndexLabelKeyValue(0, "rescaling_factor", rf));
        CHECK_CLOSE(rf, 2.0, 1e-12);
    }

    return 0;
}

/* CASE 2 - Two disjoint notches accumulate count.
   Notch A (8000.5, 8002.0) catches sp0 (8001).
   Notch B (8004.0, 8006.0) catches sp2 (8005). */
static int test_case2_two_notches()
{
    visibility_type vis;
    weight_type wt;
    build_fixture_usb(vis, wt);

    MHO_Notches op;
    op.SetNotchBoundaries(std::vector<double>{8000.5, 8002.0, 8004.0, 8006.0});
    op.SetWeights(&wt);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // sp0 (8001) and sp2 (8005) zeroed
    CHECK_CLOSE(vis(0, 0, 0, 0).real(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 0).imag(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 2).real(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 2).imag(), 0.0, 1e-12);

    // sp1 (8003) and sp3 (8007) intact
    CHECK_CLOSE(vis(0, 0, 0, 1).real(), 1.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 1).imag(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 3).real(), 1.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 3).imag(), 0.0, 1e-12);

    // count=2, npts=4, factor=2.0
    CHECK_CLOSE(wt(0, 0, 0, 0), 2.0, 1e-12);

    return 0;
}

/* CASE 3 - Odd-length boundary list drops the trailing value.
   {8002.0, 8006.0, 8007.5} -> N decremented to 2, only (8002, 8006) used.
   Identical result to CASE 1. */
static int test_case3_odd_boundary_list()
{
    visibility_type vis;
    weight_type wt;
    build_fixture_usb(vis, wt);

    MHO_Notches op;
    op.SetNotchBoundaries(std::vector<double>{8002.0, 8006.0, 8007.5});
    op.SetWeights(&wt);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // sp1 (8003) and sp2 (8005) zeroed - same as CASE 1
    CHECK_CLOSE(vis(0, 0, 0, 1).real(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 1).imag(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 2).real(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 2).imag(), 0.0, 1e-12);

    // sp0 (8001) and sp3 (8007) intact
    CHECK_CLOSE(vis(0, 0, 0, 0).real(), 1.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 0).imag(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 3).real(), 1.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 3).imag(), 0.0, 1e-12);

    // count=2, npts=4, factor=2.0
    CHECK_CLOSE(wt(0, 0, 0, 0), 2.0, 1e-12);

    return 0;
}

/* CASE 4 - LSB sign handling.
   net_sideband="L" => sp_freqs = {7999, 7997, 7995, 7993}.
   Notch (7994, 7998) catches sp1 (7997) and sp2 (7995). */
static int test_case4_lsb_sign()
{
    visibility_type vis;
    weight_type wt;
    build_fixture_lsb(vis, wt);

    MHO_Notches op;
    op.SetNotchBoundaries(std::vector<double>{7994.0, 7998.0});
    op.SetWeights(&wt);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // sp1 (7997) and sp2 (7995) zeroed
    CHECK_CLOSE(vis(0, 0, 0, 1).real(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 1).imag(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 2).real(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 2).imag(), 0.0, 1e-12);

    // sp0 (7999) and sp3 (7993) intact
    CHECK_CLOSE(vis(0, 0, 0, 0).real(), 1.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 0).imag(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 3).real(), 1.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 3).imag(), 0.0, 1e-12);

    // count=2, npts=4, factor=2.0
    CHECK_CLOSE(wt(0, 0, 0, 0), 2.0, 1e-12);

    return 0;
}

/* CASE 5 - Notch entirely outside the channel => no-op.
   Notch (9000, 9100) does not overlap channel [8000, 8008]. */
static int test_case5_notch_outside()
{
    visibility_type vis;
    weight_type wt;
    build_fixture_usb(vis, wt);

    MHO_Notches op;
    op.SetNotchBoundaries(std::vector<double>{9000.0, 9100.0});
    op.SetWeights(&wt);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // vis unchanged - all (1,0)
    for (std::size_t sp = 0; sp < 4; sp++) {
        CHECK_CLOSE(vis(0, 0, 0, sp).real(), 1.0, 1e-12);
        CHECK_CLOSE(vis(0, 0, 0, sp).imag(), 0.0, 1e-12);
    }

    // count=0, npts=4, frac=1.0, factor=1.0; weight = 1.0 * 1.0 = 1.0
    CHECK_CLOSE(wt(0, 0, 0, 0), 1.0, 1e-12);

    // Labels written even on no-overlap
    {
        double ubf, rf;
        auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
        REQUIRE(chan_ax.RetrieveIndexLabelKeyValue(0, "used_bandwidth_fraction", ubf));
        CHECK_CLOSE(ubf, 1.0, 1e-12);
        REQUIRE(chan_ax.RetrieveIndexLabelKeyValue(0, "rescaling_factor", rf));
        CHECK_CLOSE(rf, 1.0, 1e-12);
    }

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_case1_single_notch()) return 1;
    if (test_case2_two_notches()) return 1;
    if (test_case3_odd_boundary_list()) return 1;
    if (test_case4_lsb_sign()) return 1;
    if (test_case5_notch_outside()) return 1;

    return 0;
}
