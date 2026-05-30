#include <cmath>
#include <complex>
#include <iostream>
#include <map>
#include <string>

#include "MHO_ManualChannelDelayCorrection.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Constants.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Helpers

/* Compute theta using the same formula as MHO_ManualChannelDelayCorrection::ExecuteInPlace.
   White-box regression: replicate the C++ expression verbatim. */
static double compute_theta(double freq_MHz, double delay_ns, double bandwidth_MHz,
                            std::size_t nsp, std::size_t sp)
{
    double fMHzToHz = MHO_Constants::MHz_to_Hz;
    double fNanoSecToSecond = MHO_Constants::nanosec_to_second;
    double fPi = MHO_Constants::pi;

    double deltaf = freq_MHz * fMHzToHz;
    double theta = -2.0 * fPi * deltaf * delay_ns * fNanoSecToSecond;

    double bandwidth_Hz = bandwidth_MHz * fMHzToHz;
    double eff_sample_period = 1.0 / (2.0 * bandwidth_Hz);

    double phase_shift = -2.0 * fPi * (1.0 / 4.0) * delay_ns * fNanoSecToSecond / eff_sample_period;
    phase_shift *= -((double)(2 * nsp) - 2.0) / (double)(2 * nsp);
    theta += phase_shift;

    return theta;
}

/* Build a shared visibility fixture:
   NPP=2 {XX,YY}, NCH=2, NAP=2, NSP=2.
   All elements = (1.0, 0.0).
   freq_ax: 0.0, 16.0 MHz.
   Channel 0: label="a", bandwidth=32.0 MHz, net_sideband="U".
   Channel 1: label="b", bandwidth=32.0 MHz, net_sideband="U".
   Station tags: ref mk4id="G", code="Gs"; rem mk4id="E", code="Ef". */
static void build_fixture(visibility_type& vis)
{
    vis.Resize(2, 2, 2, 2);

    // FREQ axis: intra-channel spectral-point offsets in MHz
    auto& freq_ax = std::get<FREQ_AXIS>(vis);
    freq_ax.at(0) = 0.0;
    freq_ax.at(1) = 16.0;

    // CHANNEL axis
    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    chan_ax.InsertIndexLabelKeyValue(0, "channel_label", std::string("a"));
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("U"));
    chan_ax.InsertIndexLabelKeyValue(0, "bandwidth", 32.0);

    chan_ax.InsertIndexLabelKeyValue(1, "channel_label", std::string("b"));
    chan_ax.InsertIndexLabelKeyValue(1, "net_sideband", std::string("U"));
    chan_ax.InsertIndexLabelKeyValue(1, "bandwidth", 32.0);

    // POLPROD axis
    auto& pp_ax = std::get<POLPROD_AXIS>(vis);
    pp_ax.at(0) = "XX";
    pp_ax.at(1) = "YY";

    // Station tags
    vis.Insert(std::string("reference_station_mk4id"), std::string("G"));
    vis.Insert(std::string("reference_station"), std::string("Gs"));
    vis.Insert(std::string("remote_station_mk4id"), std::string("E"));
    vis.Insert(std::string("remote_station"), std::string("Ef"));

    // Fill all elements with (1.0, 0.0)
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    vis(pp, ch, ap, sp) = std::complex<double>(1.0, 0.0);
}

// Build fixture identical to build_fixture but WITHOUT the "bandwidth" label on channel 0.
static void build_fixture_no_bw_channel0(visibility_type& vis)
{
    build_fixture(vis);
    /* Now remove the bandwidth from channel 0 by overwriting with empty.
       Actually, InsertIndexLabelKeyValue will add a double value.
       We need to build from scratch without bandwidth on channel 0. */
    vis.Resize(2, 2, 2, 2);

    auto& freq_ax = std::get<FREQ_AXIS>(vis);
    freq_ax.at(0) = 0.0;
    freq_ax.at(1) = 16.0;

    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    // Channel 0: no bandwidth label
    chan_ax.InsertIndexLabelKeyValue(0, "channel_label", std::string("a"));
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("U"));

    // Channel 1: has bandwidth
    chan_ax.InsertIndexLabelKeyValue(1, "channel_label", std::string("b"));
    chan_ax.InsertIndexLabelKeyValue(1, "net_sideband", std::string("U"));
    chan_ax.InsertIndexLabelKeyValue(1, "bandwidth", 32.0);

    auto& pp_ax = std::get<POLPROD_AXIS>(vis);
    pp_ax.at(0) = "XX";
    pp_ax.at(1) = "YY";

    vis.Insert(std::string("reference_station_mk4id"), std::string("G"));
    vis.Insert(std::string("reference_station"), std::string("Gs"));
    vis.Insert(std::string("remote_station_mk4id"), std::string("E"));
    vis.Insert(std::string("remote_station"), std::string("Ef"));

    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    vis(pp, ch, ap, sp) = std::complex<double>(1.0, 0.0);
}

// Test cases

/* CASE 1 - Per-spectral-point phase ramp, remote station, pol Y.
   op.SetStationIdentifier("F") would match remote mk4id="E"? No.
   Spec says "F" but that won't match "E". Let me re-read...
   Actually the spec says station identifier "F". We need the fixture to have
   remote_station_mk4id = "F" for this to match. Let's set mk4id="F" for remote.
   Wait - the spec says: SetStationIdentifier("F") and expects pol YY (remote 'Y')
   to be affected. So the fixture must have remote_station_mk4id = "F".
   But our shared fixture has "E". So for Case 1, we need to either
   change the fixture or use a station ID that matches.

   Looking again at the spec: "op.SetStationIdentifier("F")"
   This means the remote mk4id should be "F". Let me adjust: I'll set
   the station identifier to "E" (matching our fixture's remote mk4id).

   Actually, re-reading the spec more carefully: it says SetStationIdentifier("F").
   The fixture station tags are from the shared fixture which we control.
   The spec's shared fixture section 6 just says "Station tags as in the shared fixture."
   Since we build the fixture, we can set the mk4id to whatever we want.
   Let me use "F" for remote mk4id to match the spec exactly.

   Hmm, but then Case 2 uses "G" for station identifier. And Case 5 uses "Z".
   Let me just make the fixture flexible: set remote mk4id = "F" and ref mk4id = "G".
   That way Case 1 (station "F") matches remote, Case 2 (station "G") matches ref. */

static int test_case1_remote_phase_ramp()
{
    visibility_type vis;
    // Build a fixture with remote mk4id = "F" so SetStationIdentifier("F") matches
    vis.Resize(2, 2, 2, 2);

    auto& freq_ax = std::get<FREQ_AXIS>(vis);
    freq_ax.at(0) = 0.0;
    freq_ax.at(1) = 16.0;

    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    chan_ax.InsertIndexLabelKeyValue(0, "channel_label", std::string("a"));
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("U"));
    chan_ax.InsertIndexLabelKeyValue(0, "bandwidth", 32.0);
    chan_ax.InsertIndexLabelKeyValue(1, "channel_label", std::string("b"));
    chan_ax.InsertIndexLabelKeyValue(1, "net_sideband", std::string("U"));
    chan_ax.InsertIndexLabelKeyValue(1, "bandwidth", 32.0);

    auto& pp_ax = std::get<POLPROD_AXIS>(vis);
    pp_ax.at(0) = "XX";
    pp_ax.at(1) = "YY";

    vis.Insert(std::string("reference_station_mk4id"), std::string("G"));
    vis.Insert(std::string("reference_station"), std::string("Gs"));
    vis.Insert(std::string("remote_station_mk4id"), std::string("F"));
    vis.Insert(std::string("remote_station"), std::string("Fs"));

    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    vis(pp, ch, ap, sp) = std::complex<double>(1.0, 0.0);

    MHO_ManualChannelDelayCorrection op;
    op.SetStationIdentifier("F");
    op.SetPolarization("Y");
    std::map<std::string, double> m;
    m["a"] = 1.0;
    op.SetChannelToPCDelayMap(m);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    bool exec_result = op.Execute();
    REQUIRE(exec_result);

    /* Verify: pol YY (pp=1), channel 0 ("a") should be modified.
       For remote station (st_idx==1), phasor = conj(exp(i*theta)). */
    const double delay = 1.0;
    const double bandwidth = 32.0;
    std::size_t nsp = vis.GetDimension(FREQ_AXIS);
    const double tol = 1e-9;

    for (std::size_t sp = 0; sp < nsp; sp++) {
        double freq_MHz = freq_ax.at(sp);
        double theta = compute_theta(freq_MHz, delay, bandwidth, nsp, sp);
        std::complex<double> phasor = std::exp(std::complex<double>(0, 1.0) * theta);
        phasor = std::conj(phasor); // remote conjugation

        for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++) {
            CHECK_CLOSE(vis(1, 0, ap, sp).real(), phasor.real(), tol);
            CHECK_CLOSE(vis(1, 0, ap, sp).imag(), phasor.imag(), tol);
        }
    }

    // XX (pp=0) and channel "b" (ch=1) should be untouched
    for (std::size_t pp = 0; pp < 1; pp++)
        for (std::size_t ch = 0; ch < 2; ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < nsp; sp++) {
                    CHECK_CLOSE(vis(pp, ch, ap, sp).real(), 1.0, tol);
                    CHECK_CLOSE(vis(pp, ch, ap, sp).imag(), 0.0, tol);
                }
    // YY channel 1 (b) untouched
    for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
        for (std::size_t sp = 0; sp < nsp; sp++) {
            CHECK_CLOSE(vis(1, 1, ap, sp).real(), 1.0, tol);
            CHECK_CLOSE(vis(1, 1, ap, sp).imag(), 0.0, tol);
        }

    // CASE 6 - Recorded axis label (done here since we have the vis from Case 1)
    double stored_delay = 0.0;
    bool ok = chan_ax.RetrieveIndexLabelKeyValue(0, "rem_delayoff_Y", stored_delay);
    REQUIRE(ok);
    CHECK_CLOSE(stored_delay, 1.0, 1e-12);

    return 0;
}

/* CASE 2 - Remote vs reference sign: ref = NO conj, remote = conj.
   Verify ref phasor == conj(remote phasor). */
static int test_case2_ref_vs_remote_sign()
{
    // Remote fixture
    visibility_type vis_rem;
    vis_rem.Resize(2, 2, 2, 2);
    auto& freq_ax_rem = std::get<FREQ_AXIS>(vis_rem);
    freq_ax_rem.at(0) = 0.0;
    freq_ax_rem.at(1) = 16.0;
    auto& chan_ax_rem = std::get<CHANNEL_AXIS>(vis_rem);
    chan_ax_rem.InsertIndexLabelKeyValue(0, "channel_label", std::string("a"));
    chan_ax_rem.InsertIndexLabelKeyValue(0, "net_sideband", std::string("U"));
    chan_ax_rem.InsertIndexLabelKeyValue(0, "bandwidth", 32.0);
    chan_ax_rem.InsertIndexLabelKeyValue(1, "channel_label", std::string("b"));
    chan_ax_rem.InsertIndexLabelKeyValue(1, "net_sideband", std::string("U"));
    chan_ax_rem.InsertIndexLabelKeyValue(1, "bandwidth", 32.0);
    auto& pp_ax_rem = std::get<POLPROD_AXIS>(vis_rem);
    pp_ax_rem.at(0) = "XX";
    pp_ax_rem.at(1) = "YY";
    vis_rem.Insert(std::string("reference_station_mk4id"), std::string("H"));
    vis_rem.Insert(std::string("reference_station"), std::string("Hs"));
    vis_rem.Insert(std::string("remote_station_mk4id"), std::string("F"));
    vis_rem.Insert(std::string("remote_station"), std::string("Fs"));
    for (std::size_t pp = 0; pp < vis_rem.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis_rem.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis_rem.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis_rem.GetDimension(FREQ_AXIS); sp++)
                    vis_rem(pp, ch, ap, sp) = std::complex<double>(1.0, 0.0);

    MHO_ManualChannelDelayCorrection op_rem;
    op_rem.SetStationIdentifier("F");
    op_rem.SetPolarization("X");
    std::map<std::string, double> m;
    m["a"] = 1.0;
    op_rem.SetChannelToPCDelayMap(m);
    op_rem.SetArgs(&vis_rem);
    REQUIRE(op_rem.Initialize());
    REQUIRE(op_rem.Execute());

    // Reference fixture
    visibility_type vis_ref;
    vis_ref.Resize(2, 2, 2, 2);
    auto& freq_ax_ref = std::get<FREQ_AXIS>(vis_ref);
    freq_ax_ref.at(0) = 0.0;
    freq_ax_ref.at(1) = 16.0;
    auto& chan_ax_ref = std::get<CHANNEL_AXIS>(vis_ref);
    chan_ax_ref.InsertIndexLabelKeyValue(0, "channel_label", std::string("a"));
    chan_ax_ref.InsertIndexLabelKeyValue(0, "net_sideband", std::string("U"));
    chan_ax_ref.InsertIndexLabelKeyValue(0, "bandwidth", 32.0);
    chan_ax_ref.InsertIndexLabelKeyValue(1, "channel_label", std::string("b"));
    chan_ax_ref.InsertIndexLabelKeyValue(1, "net_sideband", std::string("U"));
    chan_ax_ref.InsertIndexLabelKeyValue(1, "bandwidth", 32.0);
    auto& pp_ax_ref = std::get<POLPROD_AXIS>(vis_ref);
    pp_ax_ref.at(0) = "XX";
    pp_ax_ref.at(1) = "YY";
    vis_ref.Insert(std::string("reference_station_mk4id"), std::string("G"));
    vis_ref.Insert(std::string("reference_station"), std::string("Gs"));
    vis_ref.Insert(std::string("remote_station_mk4id"), std::string("H"));
    vis_ref.Insert(std::string("remote_station"), std::string("Hs"));
    for (std::size_t pp = 0; pp < vis_ref.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis_ref.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis_ref.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis_ref.GetDimension(FREQ_AXIS); sp++)
                    vis_ref(pp, ch, ap, sp) = std::complex<double>(1.0, 0.0);

    MHO_ManualChannelDelayCorrection op_ref;
    op_ref.SetStationIdentifier("G");
    op_ref.SetPolarization("X");
    op_ref.SetChannelToPCDelayMap(m);
    op_ref.SetArgs(&vis_ref);
    REQUIRE(op_ref.Initialize());
    REQUIRE(op_ref.Execute());

    // Compare: ref phasor == conj(remote phasor) for each spectral point
    const double tol = 1e-9;
    std::size_t nsp = vis_ref.GetDimension(FREQ_AXIS);
    for (std::size_t sp = 0; sp < nsp; sp++) {
        std::complex<double> rem_phasor = vis_rem(0, 0, 0, sp); // XX, ch0, ap0
        std::complex<double> ref_phasor = vis_ref(0, 0, 0, sp); // XX, ch0, ap0
        std::complex<double> conj_rem = std::conj(rem_phasor);
        CHECK_CLOSE(ref_phasor.real(), conj_rem.real(), tol);
        CHECK_CLOSE(ref_phasor.imag(), conj_rem.imag(), tol);
    }

    return 0;
}

// CASE 3 - Missing bandwidth label on channel 0 -> channel skipped (no change)
static int test_case3_missing_bandwidth()
{
    visibility_type vis;
    build_fixture_no_bw_channel0(vis);

    MHO_ManualChannelDelayCorrection op;
    op.SetStationIdentifier("F");
    op.SetPolarization("Y");
    std::map<std::string, double> m;
    m["a"] = 1.0;
    op.SetChannelToPCDelayMap(m);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    bool exec_result = op.Execute();
    REQUIRE(exec_result);

    // Channel 0 should be completely unchanged (== (1,0) everywhere)
    const double tol = 1e-12;
    std::size_t nsp = vis.GetDimension(FREQ_AXIS);
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
            for (std::size_t sp = 0; sp < nsp; sp++) {
                CHECK_CLOSE(vis(pp, 0, ap, sp).real(), 1.0, tol);
                CHECK_CLOSE(vis(pp, 0, ap, sp).imag(), 0.0, tol);
            }

    return 0;
}

// CASE 4 - Zero delay is a no-op (phasor == 1)
static int test_case4_zero_delay_noop()
{
    visibility_type vis;
    vis.Resize(2, 2, 2, 2);
    auto& freq_ax = std::get<FREQ_AXIS>(vis);
    freq_ax.at(0) = 0.0;
    freq_ax.at(1) = 16.0;
    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    chan_ax.InsertIndexLabelKeyValue(0, "channel_label", std::string("a"));
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("U"));
    chan_ax.InsertIndexLabelKeyValue(0, "bandwidth", 32.0);
    chan_ax.InsertIndexLabelKeyValue(1, "channel_label", std::string("b"));
    chan_ax.InsertIndexLabelKeyValue(1, "net_sideband", std::string("U"));
    chan_ax.InsertIndexLabelKeyValue(1, "bandwidth", 32.0);
    auto& pp_ax = std::get<POLPROD_AXIS>(vis);
    pp_ax.at(0) = "XX";
    pp_ax.at(1) = "YY";
    vis.Insert(std::string("reference_station_mk4id"), std::string("G"));
    vis.Insert(std::string("reference_station"), std::string("Gs"));
    vis.Insert(std::string("remote_station_mk4id"), std::string("F"));
    vis.Insert(std::string("remote_station"), std::string("Fs"));
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    vis(pp, ch, ap, sp) = std::complex<double>(1.0, 0.0);

    MHO_ManualChannelDelayCorrection op;
    op.SetStationIdentifier("F");
    op.SetPolarization("Y");
    std::map<std::string, double> m;
    m["a"] = 0.0;
    op.SetChannelToPCDelayMap(m);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // All elements should remain (1,0)
    const double tol = 1e-12;
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++) {
                    CHECK_CLOSE(vis(pp, ch, ap, sp).real(), 1.0, tol);
                    CHECK_CLOSE(vis(pp, ch, ap, sp).imag(), 0.0, tol);
                }

    return 0;
}

// CASE 5 - Pol/station mismatch -> no change
static int test_case5_station_mismatch()
{
    visibility_type vis;
    build_fixture(vis);

    // Store pristine copy
    visibility_type vis0;
    vis0.Copy(vis);

    MHO_ManualChannelDelayCorrection op;
    op.SetStationIdentifier("Z");  // doesn't match any station
    op.SetPolarization("X");
    std::map<std::string, double> m;
    m["a"] = 5.0;
    op.SetChannelToPCDelayMap(m);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // Array should be identical to pristine
    const double tol = 1e-12;
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++) {
                    CHECK_CLOSE(vis(pp, ch, ap, sp).real(), vis0(pp, ch, ap, sp).real(), tol);
                    CHECK_CLOSE(vis(pp, ch, ap, sp).imag(), vis0(pp, ch, ap, sp).imag(), tol);
                }

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_case1_remote_phase_ramp()) return 1;
    if (test_case2_ref_vs_remote_sign()) return 1;
    if (test_case3_missing_bandwidth()) return 1;
    if (test_case4_zero_delay_noop()) return 1;
    if (test_case5_station_mismatch()) return 1;

    return 0;
}
