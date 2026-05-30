#include <cmath>
#include <complex>
#include <iostream>
#include <map>
#include <string>

#include "MHO_ManualChannelPhaseCorrection.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Constants.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Helpers

static bool cclose(std::complex<double> a, std::complex<double> b,
                   double tol = 1e-9)
{
    return std::abs(a - b) <= tol;
}

/* Build the shared visibility fixture:
   NPP=2 {XX,YY}, NCH=2, NAP=2, NSP=2.
   All elements = (1,0).
   Channel 0: label="a", net_sideband="U" (USB).
   Channel 1: label="b", net_sideband="L" (LSB).
   Station tags: ref mk4id="G", code="Gs"; rem mk4id="F", code="Wf". */
static void build_fixture(visibility_type& vis)
{
    vis.Resize(2, 2, 2, 2);

    auto& pp_ax = std::get<POLPROD_AXIS>(vis);
    pp_ax.at(0) = "XX";
    pp_ax.at(1) = "YY";

    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    chan_ax.InsertIndexLabelKeyValue(0, "channel_label", std::string("a"));
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("U"));
    chan_ax.InsertIndexLabelKeyValue(1, "channel_label", std::string("b"));
    chan_ax.InsertIndexLabelKeyValue(1, "net_sideband", std::string("L"));

    vis.Insert(std::string("reference_station_mk4id"), std::string("G"));
    vis.Insert(std::string("reference_station"), std::string("Gs"));
    vis.Insert(std::string("remote_station_mk4id"), std::string("F"));
    vis.Insert(std::string("remote_station"), std::string("Wf"));

    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    vis(pp, ch, ap, sp) = std::complex<double>(1.0, 0.0);
}

/* Build fixture with channel 1's label changed to "b+" (sideband-half suffix).
   Channel 0 remains "a". */
static void build_fixture_bplus(visibility_type& vis)
{
    build_fixture(vis);
    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    chan_ax.InsertIndexLabelKeyValue(1, "channel_label", std::string("b+"));
}

// Test cases

/* CASE 1 - Remote station, pol Y, both channels.
   ch0 "a" USB rem: phasor = exp(i*90deg) = (0, 1).
   ch1 "b" LSB rem: phasor = conj(exp(i*45deg)) = exp(-i*45) = (0.7071, -0.7071).
   XX pol untouched. */
static int test_case1_remote_polY_both_channels()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_ManualChannelPhaseCorrection op;
    op.SetStationIdentifier("F");
    op.SetPolarization("Y");
    std::map<std::string, double> m;
    m["a"] = 90.0;
    m["b"] = 45.0;
    op.SetChannelToPCPhaseMap(m);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-9;
    const std::complex<double> exp90(0.0, 1.0);
    const std::complex<double> exp_neg45(
        std::cos(MHO_Constants::deg_to_rad * 45.0),
       -std::sin(MHO_Constants::deg_to_rad * 45.0));

    // YY (pp=1), ch0 USB rem: every element == exp(i*90)
    for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
        for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
            REQUIRE(cclose(vis(1, 0, ap, sp), exp90, tol));

    // YY (pp=1), ch1 LSB rem: every element == exp(-i*45)
    for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
        for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
            REQUIRE(cclose(vis(1, 1, ap, sp), exp_neg45, tol));

    // XX (pp=0) untouched: all (1,0)
    for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
        for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
            for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                REQUIRE(cclose(vis(0, ch, ap, sp), std::complex<double>(1.0, 0.0), tol));

    return 0;
}

/* CASE 2 - Reference station extra conjugation.
   XX,ch0 USB ref: conj(exp(i*90)) = (0, -1). */
static int test_case2_ref_conjugation()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_ManualChannelPhaseCorrection op;
    op.SetStationIdentifier("G");
    op.SetPolarization("X");
    std::map<std::string, double> m;
    m["a"] = 90.0;
    op.SetChannelToPCPhaseMap(m);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-9;
    const std::complex<double> expected(0.0, -1.0);

    // XX (pp=0), ch0 USB ref: conj(exp(i*90)) = (0,-1)
    for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
        for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
            REQUIRE(cclose(vis(0, 0, ap, sp), expected, tol));

    return 0;
}

/* CASE 3 - Channel-label +/- stripping (LabelMatch).
   ch1 label is "b+", map key is "b" (no sign) => strip '+' => match.
   ch1 LSB rem: phasor = conj(exp(i*45)) = exp(-i*45). */
static int test_case3_label_stripping()
{
    visibility_type vis;
    build_fixture_bplus(vis);

    MHO_ManualChannelPhaseCorrection op;
    op.SetStationIdentifier("F");
    op.SetPolarization("Y");
    std::map<std::string, double> m;
    m["b"] = 45.0;
    op.SetChannelToPCPhaseMap(m);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-9;
    const std::complex<double> exp_neg45(
        std::cos(MHO_Constants::deg_to_rad * 45.0),
       -std::sin(MHO_Constants::deg_to_rad * 45.0));

    // YY (pp=1), ch1 (label "b+") LSB rem: matched via stripping, phasor = exp(-i*45)
    for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
        for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
            REQUIRE(cclose(vis(1, 1, ap, sp), exp_neg45, tol));

    // XX untouched
    for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
        for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
            for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                REQUIRE(cclose(vis(0, ch, ap, sp), std::complex<double>(1.0, 0.0), tol));

    return 0;
}

/* CASE 4 - Map key with explicit suffix is exact.
   ch1 label is "b+", map key is "b-" => exact match fails => no change. */
static int test_case4_exact_suffix_no_match()
{
    visibility_type vis;
    build_fixture_bplus(vis);

    visibility_type vis0;
    vis0.Copy(vis);

    MHO_ManualChannelPhaseCorrection op;
    op.SetStationIdentifier("F");
    op.SetPolarization("Y");
    std::map<std::string, double> m;
    m["b-"] = 45.0;
    op.SetChannelToPCPhaseMap(m);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-12;
    // All elements should be identical to pristine copy
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    REQUIRE(cclose(vis(pp, ch, ap, sp), vis0(pp, ch, ap, sp), tol));

    return 0;
}

// CASE 5 - Pol mismatch -> no change.
static int test_case5_pol_mismatch()
{
    visibility_type vis;
    build_fixture(vis);

    visibility_type vis0;
    vis0.Copy(vis);

    MHO_ManualChannelPhaseCorrection op;
    op.SetStationIdentifier("F");
    op.SetPolarization("L");
    std::map<std::string, double> m;
    m["a"] = 90.0;
    op.SetChannelToPCPhaseMap(m);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-12;
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    REQUIRE(cclose(vis(pp, ch, ap, sp), vis0(pp, ch, ap, sp), tol));

    return 0;
}

// CASE 6 - Channel not present in map -> that channel untouched.
static int test_case6_channel_not_in_map()
{
    visibility_type vis;
    build_fixture(vis);

    visibility_type vis0;
    vis0.Copy(vis);

    MHO_ManualChannelPhaseCorrection op;
    op.SetStationIdentifier("F");
    op.SetPolarization("Y");
    std::map<std::string, double> m;
    m["zzz"] = 90.0;
    op.SetChannelToPCPhaseMap(m);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-12;
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    REQUIRE(cclose(vis(pp, ch, ap, sp), vis0(pp, ch, ap, sp), tol));

    return 0;
}

/* CASE 7 - Recorded axis label.
   After Case 1, retrieve channel-axis label "rem_pcphase_Y" at channel 0. */
static int test_case7_recorded_axis_label()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_ManualChannelPhaseCorrection op;
    op.SetStationIdentifier("F");
    op.SetPolarization("Y");
    std::map<std::string, double> m;
    m["a"] = 90.0;
    m["b"] = 45.0;
    op.SetChannelToPCPhaseMap(m);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    double val = 0.0;
    bool ok = chan_ax.RetrieveIndexLabelKeyValue(0, "rem_pcphase_Y", val);
    REQUIRE(ok);

    // 90 degrees in radians = pi/2
    const double expected_rad = MHO_Constants::deg_to_rad * 90.0;
    CHECK_CLOSE(val, expected_rad, 1e-9);

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_case1_remote_polY_both_channels()) return 1;
    if (test_case2_ref_conjugation()) return 1;
    if (test_case3_label_stripping()) return 1;
    if (test_case4_exact_suffix_no_match()) return 1;
    if (test_case5_pol_mismatch()) return 1;
    if (test_case6_channel_not_in_map()) return 1;
    if (test_case7_recorded_axis_label()) return 1;

    return 0;
}
