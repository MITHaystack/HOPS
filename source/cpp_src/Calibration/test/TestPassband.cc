#include <cmath>
#include <iostream>
#include <string>
#include <complex>

#include "MHO_Passband.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;


/* Build the shared USB fixture: npol=1, nchan=2, nap=1, nspec=4.
   Channel 0: sky=8000.0, net_sideband="U", bandwidth=8.0.
   Channel 1: sky=8100.0, net_sideband="U", bandwidth=8.0.
   FREQ_AXIS deltaf = {1.0, 3.0, 5.0, 7.0}
   => chan0 sp_freqs = {8001, 8003, 8005, 8007}
   => chan1 sp_freqs = {8101, 8103, 8105, 8107}
   All visibilities (1,0); weights all 1.0.
   Weight container: npol=1, nchan=2, nap=1, nspec=1. */
static void build_fixture(visibility_type& vis, weight_type& wt)
{
    vis.Resize(1, 2, 1, 4);
    wt.Resize(1, 2, 1, 1);

    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    chan_ax.at(0) = 8000.0;
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("U"));
    chan_ax.InsertIndexLabelKeyValue(0, "bandwidth", 8.0);
    chan_ax.at(1) = 8100.0;
    chan_ax.InsertIndexLabelKeyValue(1, "net_sideband", std::string("U"));
    chan_ax.InsertIndexLabelKeyValue(1, "bandwidth", 8.0);

    auto& freq_ax = std::get<FREQ_AXIS>(vis);
    freq_ax.at(0) = 1.0;
    freq_ax.at(1) = 3.0;
    freq_ax.at(2) = 5.0;
    freq_ax.at(3) = 7.0;

    // Fill vis with (1,0)
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    vis(pp, ch, ap, sp) = std::complex<double>(1.0, 0.0);

    // Fill weights with 1.0
    for (std::size_t ch = 0; ch < 2; ch++)
        wt(0, ch, 0, 0) = 1.0;
}

/* Build the LSB fixture: same as above but channel 0 has net_sideband="L".
   For LSB: sp_freq = sky - deltaf = 8000 - {1,3,5,7} = {7999,7997,7995,7993}.
   Channel 0 LSB: DetermineChannelFrequencyLimits gives [7992, 8000]. */
static void build_fixture_lsb(visibility_type& vis, weight_type& wt)
{
    build_fixture(vis, wt);
    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("L"));
}

/* Check that all vis elements for channel ch equal expected value.
   Returns true on success, false on first mismatch (with cerr output). */
static bool check_channel_vis(const visibility_type& vis, std::size_t ch,
                              const std::complex<double>& expected, double tol)
{
    auto npol = vis.GetDimension(POLPROD_AXIS);
    auto nap  = vis.GetDimension(TIME_AXIS);
    auto nspec = vis.GetDimension(FREQ_AXIS);
    for (std::size_t pp = 0; pp < npol; pp++)
        for (std::size_t ap = 0; ap < nap; ap++)
            for (std::size_t sp = 0; sp < nspec; sp++) {
                if (std::fabs(vis(pp, ch, ap, sp).real() - expected.real()) > tol ||
                    std::fabs(vis(pp, ch, ap, sp).imag() - expected.imag()) > tol) {
                    std::cerr << "FAIL: vis channel " << ch
                              << " sp " << sp << " = " << vis(pp, ch, ap, sp)
                              << " expected " << expected
                              << " @ " << __FILE__ << ":" << __LINE__ << std::endl;
                    return false;
                }
            }
    return true;
}

// Test cases

/* CASE 1 -- Exclusion cuts an interior band in channel 0 only.
   SetPassband(8006.0, 8002.0) => exclusion of [8002, 8006].
   Chan0 sp_freqs {8001,8003,8005,8007}: sp1(8003), sp2(8005) inside strict.
   Chan1 sp_freqs {8101,...} no overlap with [8002,8006] => untouched. */
static int test_case1_exclusion_interior()
{
    visibility_type vis;
    weight_type wt;
    build_fixture(vis, wt);

    MHO_Passband pb;
    pb.SetPassband(8006.0, 8002.0);
    pb.SetWeights(&wt);
    pb.SetArgs(&vis);
    REQUIRE(pb.Initialize());
    REQUIRE(pb.Execute());

    // --- Channel 0: sp1(8003) and sp2(8005) zeroed; sp0(8001), sp3(8007) intact ---
    // sp0 intact
    CHECK_CLOSE(vis(0, 0, 0, 0).real(), 1.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 0).imag(), 0.0, 1e-12);
    // sp1 zeroed
    CHECK_CLOSE(vis(0, 0, 0, 1).real(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 1).imag(), 0.0, 1e-12);
    // sp2 zeroed
    CHECK_CLOSE(vis(0, 0, 0, 2).real(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 2).imag(), 0.0, 1e-12);
    // sp3 intact
    CHECK_CLOSE(vis(0, 0, 0, 3).real(), 1.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 3).imag(), 0.0, 1e-12);

    // Weight for chan0: count=2, npts=4, frac=0.5, factor=2.0; 1.0*2.0 = 2.0
    CHECK_CLOSE(wt(0, 0, 0, 0), 2.0, 1e-12);

    // used_bandwidth_fraction on chan axis
    {
        double ubf;
        auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
        REQUIRE(chan_ax.RetrieveIndexLabelKeyValue(0, "used_bandwidth_fraction", ubf));
        CHECK_CLOSE(ubf, 0.5, 1e-12);
    }

    // --- Channel 1: no overlap => fully intact ---
    REQUIRE(check_channel_vis(vis, 1, std::complex<double>(1.0, 0.0), 1e-12));

    // Weight for chan1 unchanged
    CHECK_CLOSE(wt(0, 1, 0, 0), 1.0, 1e-12);

    // No labels written on channel 1
    {
        double dummy;
        auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
        REQUIRE(chan_ax.RetrieveIndexLabelKeyValue(1, "used_bandwidth_fraction", dummy) == false);
    }

    return 0;
}

/* CASE 2 -- Inclusion keeps a window, cuts outside; disjoint channel zeroed.
   SetPassband(8002.0, 8006.0) => inclusion of [8002, 8006].
   Chan0: points OUTSIDE (8002,8006) zeroed: sp0(8001), sp3(8007).
          sp1(8003), sp2(8005) intact.
   Chan1: no intersection with [8002,8006] => whole channel zeroed + weight*0. */
static int test_case2_inclusion_disjoint()
{
    visibility_type vis;
    weight_type wt;
    build_fixture(vis, wt);

    MHO_Passband pb;
    pb.SetPassband(8002.0, 8006.0);
    pb.SetWeights(&wt);
    pb.SetArgs(&vis);
    REQUIRE(pb.Initialize());
    REQUIRE(pb.Execute());

    // --- Channel 0: sp0(8001) zeroed, sp3(8007) zeroed; sp1(8003), sp2(8005) intact ---
    CHECK_CLOSE(vis(0, 0, 0, 0).real(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 0).imag(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 1).real(), 1.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 1).imag(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 2).real(), 1.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 2).imag(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 3).real(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 3).imag(), 0.0, 1e-12);

    // count=2, frac=0.5, factor=2.0
    CHECK_CLOSE(wt(0, 0, 0, 0), 2.0, 1e-12);

    {
        double ubf;
        auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
        REQUIRE(chan_ax.RetrieveIndexLabelKeyValue(0, "used_bandwidth_fraction", ubf));
        CHECK_CLOSE(ubf, 0.5, 1e-12);
    }

    // --- Channel 1: no intersection => whole channel zeroed ---
    REQUIRE(check_channel_vis(vis, 1, std::complex<double>(0.0, 0.0), 1e-12));

    // Weight for chan1 zeroed
    CHECK_CLOSE(wt(0, 1, 0, 0), 0.0, 1e-12);

    // ubf[1]=0.0, rescaling_factor[1]=0.0
    {
        double ubf, rf;
        auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
        auto& wchan_ax = std::get<CHANNEL_AXIS>(wt);
        REQUIRE(chan_ax.RetrieveIndexLabelKeyValue(1, "used_bandwidth_fraction", ubf));
        CHECK_CLOSE(ubf, 0.0, 1e-12);
        REQUIRE(wchan_ax.RetrieveIndexLabelKeyValue(1, "rescaling_factor", rf));
        CHECK_CLOSE(rf, 0.0, 1e-12);
    }

    return 0;
}

/* CASE 3 -- LSB sign handling.
   Chan 0 LSB: sp_freqs = {7999, 7997, 7995, 7993}.
   SetPassband(7998.0, 7994.0) => exclusion of [7994, 7998].
   Strict 7994<f<7998: sp1(7997), sp2(7995) zeroed; sp0(7999), sp3(7993) intact. */
static int test_case3_lsb_exclusion()
{
    visibility_type vis;
    weight_type wt;
    build_fixture_lsb(vis, wt);

    MHO_Passband pb;
    pb.SetPassband(7998.0, 7994.0);
    pb.SetWeights(&wt);
    pb.SetArgs(&vis);
    REQUIRE(pb.Initialize());
    REQUIRE(pb.Execute());

    // --- Channel 0 (LSB): sp1(7997) and sp2(7995) zeroed ---
    // sp0(7999) intact
    CHECK_CLOSE(vis(0, 0, 0, 0).real(), 1.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 0).imag(), 0.0, 1e-12);
    // sp1(7997) zeroed
    CHECK_CLOSE(vis(0, 0, 0, 1).real(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 1).imag(), 0.0, 1e-12);
    // sp2(7995) zeroed
    CHECK_CLOSE(vis(0, 0, 0, 2).real(), 0.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 2).imag(), 0.0, 1e-12);
    // sp3(7993) intact
    CHECK_CLOSE(vis(0, 0, 0, 3).real(), 1.0, 1e-12);
    CHECK_CLOSE(vis(0, 0, 0, 3).imag(), 0.0, 1e-12);

    // count=2, npts=4, frac=0.5, factor=2.0
    CHECK_CLOSE(wt(0, 0, 0, 0), 2.0, 1e-12);

    return 0;
}

/* CASE 4 -- Window entirely covering a channel (exclusion) zeroes all points.
   SetPassband(8010.0, 7990.0) => exclusion of [7990, 8010].
   Chan0 sp_freqs {8001,8003,8005,8007}: all inside strict (7990<f<8010).
   => count=4, frac=0.0, factor=0.0. */
static int test_case4_full_exclusion()
{
    visibility_type vis;
    weight_type wt;
    build_fixture(vis, wt);

    MHO_Passband pb;
    pb.SetPassband(8010.0, 7990.0);
    pb.SetWeights(&wt);
    pb.SetArgs(&vis);
    REQUIRE(pb.Initialize());
    REQUIRE(pb.Execute());

    // --- Channel 0: all 4 points zeroed ---
    REQUIRE(check_channel_vis(vis, 0, std::complex<double>(0.0, 0.0), 1e-12));

    // count=4, frac=0.0, factor=0.0
    CHECK_CLOSE(wt(0, 0, 0, 0), 0.0, 1e-12);

    {
        double ubf, rf;
        auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
        auto& wchan_ax = std::get<CHANNEL_AXIS>(wt);
        REQUIRE(chan_ax.RetrieveIndexLabelKeyValue(0, "used_bandwidth_fraction", ubf));
        CHECK_CLOSE(ubf, 0.0, 1e-12);
        REQUIRE(wchan_ax.RetrieveIndexLabelKeyValue(0, "rescaling_factor", rf));
        CHECK_CLOSE(rf, 0.0, 1e-12);
    }

    return 0;
}

/* CASE 5 -- No-overlap exclusion is a no-op for that channel.
   SetPassband(9000.0, 8500.0) => exclusion of [8500, 9000].
   Neither chan0 [8000,8008] nor chan1 [8100,8108] intersects [8500,9000].
   => vis and wt identical to pristine; no labels written. */
static int test_case5_no_overlap_exclusion()
{
    visibility_type vis;
    weight_type wt;
    build_fixture(vis, wt);

    visibility_type pristine_vis;
    pristine_vis.Copy(vis);
    weight_type pristine_wt;
    pristine_wt.Copy(wt);

    MHO_Passband pb;
    pb.SetPassband(9000.0, 8500.0);
    pb.SetWeights(&wt);
    pb.SetArgs(&vis);
    REQUIRE(pb.Initialize());
    REQUIRE(pb.Execute());

    // --- Vis identical to pristine ---
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++) {
                    CHECK_CLOSE(vis(pp, ch, ap, sp).real(),
                                 pristine_vis(pp, ch, ap, sp).real(), 1e-12);
                    CHECK_CLOSE(vis(pp, ch, ap, sp).imag(),
                                 pristine_vis(pp, ch, ap, sp).imag(), 1e-12);
                }

    // --- Weight identical to pristine ---
    for (std::size_t ch = 0; ch < 2; ch++)
        CHECK_CLOSE(wt(0, ch, 0, 0), pristine_wt(0, ch, 0, 0), 1e-12);

    // --- No labels written on either channel ---
    {
        double dummy;
        auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
        REQUIRE(chan_ax.RetrieveIndexLabelKeyValue(0, "used_bandwidth_fraction", dummy) == false);
        REQUIRE(chan_ax.RetrieveIndexLabelKeyValue(1, "used_bandwidth_fraction", dummy) == false);
    }

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_case1_exclusion_interior())  return 1;
    if (test_case2_inclusion_disjoint())  return 1;
    if (test_case3_lsb_exclusion())       return 1;
    if (test_case4_full_exclusion())      return 1;
    if (test_case5_no_overlap_exclusion()) return 1;

    return 0;
}
