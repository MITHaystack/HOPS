#include <cmath>
#include <iostream>
#include <string>
#include <complex>

#include "MHO_NotchComb.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Helpers

/* Build the shared USB fixture: 1 pol, 1 chan, 1 AP, 8 spectral points.
   Channel 0: sky=8000, net_sideband="U", bandwidth=8.0.
   FREQ_AXIS deltaf = {0.5,1.5,2.5,3.5,4.5,5.5,6.5,7.5}
   => sp_freqs = {8000.5, 8001.5, ..., 8007.5}.
   All visibilities (1,0); weights all 1.0. */
static void build_fixture_usb(visibility_type& vis, weight_type& wt)
{
    vis.Resize(1, 1, 1, 8);
    wt.Resize(1, 1, 1, 1);

    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    chan_ax.at(0) = 8000.0;
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("U"));
    chan_ax.InsertIndexLabelKeyValue(0, "bandwidth", 8.0);

    auto& freq_ax = std::get<FREQ_AXIS>(vis);
    freq_ax.at(0) = 0.5;
    freq_ax.at(1) = 1.5;
    freq_ax.at(2) = 2.5;
    freq_ax.at(3) = 3.5;
    freq_ax.at(4) = 4.5;
    freq_ax.at(5) = 5.5;
    freq_ax.at(6) = 6.5;
    freq_ax.at(7) = 7.5;

    // Fill vis with (1,0)
    for (std::size_t sp = 0; sp < 8; sp++)
        vis(0, 0, 0, sp) = std::complex<double>(1.0, 0.0);

    // Fill weight with 1.0
    wt(0, 0, 0, 0) = 1.0;
}

/* Build the shared LSB fixture: 1 pol, 1 chan, 1 AP, 8 spectral points.
   Channel 0: sky=8000, net_sideband="L", bandwidth=8.0.
   FREQ_AXIS deltaf = {0.5,1.5,2.5,3.5,4.5,5.5,6.5,7.5}
   sb = -1 => sp_freqs = {7999.5, 7998.5, 7997.5, 7996.5, 7995.5, 7994.5, 7993.5, 7992.5}. */
static void build_fixture_lsb(visibility_type& vis, weight_type& wt)
{
    vis.Resize(1, 1, 1, 8);
    wt.Resize(1, 1, 1, 1);

    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    chan_ax.at(0) = 8000.0;
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("L"));
    chan_ax.InsertIndexLabelKeyValue(0, "bandwidth", 8.0);

    auto& freq_ax = std::get<FREQ_AXIS>(vis);
    freq_ax.at(0) = 0.5;
    freq_ax.at(1) = 1.5;
    freq_ax.at(2) = 2.5;
    freq_ax.at(3) = 3.5;
    freq_ax.at(4) = 4.5;
    freq_ax.at(5) = 5.5;
    freq_ax.at(6) = 6.5;
    freq_ax.at(7) = 7.5;

    for (std::size_t sp = 0; sp < 8; sp++)
        vis(0, 0, 0, sp) = std::complex<double>(1.0, 0.0);

    wt(0, 0, 0, 0) = 1.0;
}

// Test cases

// CASE 1 - Comb at period 2 MHz, narrow width, offset 0: no points zeroed
static int test_case1_narrow_comb_no_hit()
{
    visibility_type vis;
    weight_type wt;
    build_fixture_usb(vis, wt);

    MHO_NotchComb op;
    op.SetNotchOffset(0.0);
    op.SetNotchPeriod(2.0);
    op.SetNotchWidth(0.4);
    op.SetWeights(&wt);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    /* Notch centers at ..., 8000, 8002, 8004, 8006, 8008, ...
       Half-width 0.2; sp_freqs are at .5 offsets from even integers.
       All points are 0.5 MHz from nearest center => outside notch. */
    for (std::size_t sp = 0; sp < 8; sp++) {
        CHECK_CLOSE(vis(0, 0, 0, sp).real(), 1.0, 1e-12);
        CHECK_CLOSE(vis(0, 0, 0, sp).imag(), 0.0, 1e-12);
    }
    CHECK_CLOSE(wt(0, 0, 0, 0), 1.0, 1e-12);

    // Labels: ubf=1.0, rescaling_factor=1.0
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

// CASE 2 - Comb offset to land ON the sample points: 4 points zeroed
static int test_case2_comb_hits_points()
{
    visibility_type vis;
    weight_type wt;
    build_fixture_usb(vis, wt);

    MHO_NotchComb op;
    op.SetNotchOffset(8000.5);
    op.SetNotchPeriod(2.0);
    op.SetNotchWidth(0.4);
    op.SetWeights(&wt);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    /* Centers: 8000.5, 8002.5, 8004.5, 8006.5
       Half-width 0.2; sp_freqs exactly on centers => inside.
       sp {0,2,4,6} zeroed; {1,3,5,7} intact. */
    // Zeroed: indices 0, 2, 4, 6
    for (std::size_t sp = 0; sp < 8; sp += 2) {
        CHECK_CLOSE(vis(0, 0, 0, sp).real(), 0.0, 1e-12);
        CHECK_CLOSE(vis(0, 0, 0, sp).imag(), 0.0, 1e-12);
    }
    // Intact: indices 1, 3, 5, 7
    for (std::size_t sp = 1; sp < 8; sp += 2) {
        CHECK_CLOSE(vis(0, 0, 0, sp).real(), 1.0, 1e-12);
        CHECK_CLOSE(vis(0, 0, 0, sp).imag(), 0.0, 1e-12);
    }

    // Weight rescale: count=4, npts=8, frac=0.5, factor=2.0
    CHECK_CLOSE(wt(0, 0, 0, 0), 2.0, 1e-12);

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

// CASE 3 - No-op guard: period <= 0
static int test_case3_noop_period_zero()
{
    visibility_type vis;
    weight_type wt;
    build_fixture_usb(vis, wt);

    visibility_type pristine;
    pristine.Copy(vis);
    weight_type pristine_wt;
    pristine_wt.Copy(wt);

    MHO_NotchComb op;
    op.SetNotchOffset(8000.5);
    op.SetNotchPeriod(0.0);
    op.SetNotchWidth(0.4);
    op.SetWeights(&wt);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // vis and wt unchanged
    for (std::size_t sp = 0; sp < 8; sp++) {
        CHECK_CLOSE(vis(0, 0, 0, sp).real(), pristine(0, 0, 0, sp).real(), 1e-12);
        CHECK_CLOSE(vis(0, 0, 0, sp).imag(), pristine(0, 0, 0, sp).imag(), 1e-12);
    }
    CHECK_CLOSE(wt(0, 0, 0, 0), pristine_wt(0, 0, 0, 0), 1e-12);

    // Labels should NOT be written
    {
        double dummy;
        auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
        REQUIRE(chan_ax.RetrieveIndexLabelKeyValue(0, "used_bandwidth_fraction", dummy) == false);
    }

    return 0;
}

// CASE 4 - No-op guard: width <= 0
static int test_case4_noop_width_zero()
{
    visibility_type vis;
    weight_type wt;
    build_fixture_usb(vis, wt);

    visibility_type pristine;
    pristine.Copy(vis);
    weight_type pristine_wt;
    pristine_wt.Copy(wt);

    MHO_NotchComb op;
    op.SetNotchOffset(8000.5);
    op.SetNotchPeriod(2.0);
    op.SetNotchWidth(0.0);
    op.SetWeights(&wt);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // vis and wt unchanged
    for (std::size_t sp = 0; sp < 8; sp++) {
        CHECK_CLOSE(vis(0, 0, 0, sp).real(), pristine(0, 0, 0, sp).real(), 1e-12);
        CHECK_CLOSE(vis(0, 0, 0, sp).imag(), pristine(0, 0, 0, sp).imag(), 1e-12);
    }
    CHECK_CLOSE(wt(0, 0, 0, 0), pristine_wt(0, 0, 0, 0), 1e-12);

    // Labels should NOT be written
    {
        double dummy;
        auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
        REQUIRE(chan_ax.RetrieveIndexLabelKeyValue(0, "used_bandwidth_fraction", dummy) == false);
    }

    return 0;
}

// CASE 5 - LSB sign handling
static int test_case5_lsb_sign()
{
    visibility_type vis;
    weight_type wt;
    build_fixture_lsb(vis, wt);

    MHO_NotchComb op;
    op.SetNotchOffset(7999.5);
    op.SetNotchPeriod(2.0);
    op.SetNotchWidth(0.4);
    op.SetWeights(&wt);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    /* LSB: sp_freqs = {7999.5, 7998.5, 7997.5, 7996.5, 7995.5, 7994.5, 7993.5, 7992.5}
       Centers: 7999.5, 7997.5, 7995.5, 7993.5
       Half-width 0.2; exact match on sp {0,2,4,6} => zeroed.
       sp {1,3,5,7} intact. */
    for (std::size_t sp = 0; sp < 8; sp += 2) {
        CHECK_CLOSE(vis(0, 0, 0, sp).real(), 0.0, 1e-12);
        CHECK_CLOSE(vis(0, 0, 0, sp).imag(), 0.0, 1e-12);
    }
    for (std::size_t sp = 1; sp < 8; sp += 2) {
        CHECK_CLOSE(vis(0, 0, 0, sp).real(), 1.0, 1e-12);
        CHECK_CLOSE(vis(0, 0, 0, sp).imag(), 0.0, 1e-12);
    }

    // count=4, npts=8, frac=0.5, factor=2.0
    CHECK_CLOSE(wt(0, 0, 0, 0), 2.0, 1e-12);

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

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_case1_narrow_comb_no_hit()) return 1;
    if (test_case2_comb_hits_points()) return 1;
    if (test_case3_noop_period_zero()) return 1;
    if (test_case4_noop_width_zero()) return 1;
    if (test_case5_lsb_sign()) return 1;

    return 0;
}
