#include <cmath>
#include <complex>
#include <iostream>
#include <string>

#include "MHO_ManualPolPhaseCorrection.hh"
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

// Test cases

/* CASE 1 - Remote station, USB channel, pol Y: plain phasor.
   Remote "F", pol "Y", 30deg offset.
   YY (pp=1), ch0 USB remote: phasor = exp(i*30deg) = (cos30, sin30).
   XX (pp=0) unchanged = (1,0). */
static int test_case1_remote_usb_polY()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_ManualPolPhaseCorrection op;
    op.SetStationIdentifier("F");
    op.SetPolarization("Y");
    op.SetPCPhaseOffset(30.0);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-9;
    const std::complex<double> exp30(
        std::cos(MHO_Constants::deg_to_rad * 30.0),
        std::sin(MHO_Constants::deg_to_rad * 30.0));
    const std::complex<double> one(1.0, 0.0);

    // YY (pp=1), ch0 USB: exp(i*30deg)
    for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
        for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
            REQUIRE(cclose(vis(1, 0, ap, sp), exp30, tol));

    // XX (pp=0) unchanged
    for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
        for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
            for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                REQUIRE(cclose(vis(0, ch, ap, sp), one, tol));

    return 0;
}

/* CASE 2 - LSB channel conjugation (continuation of Case 1 fixture).
   YY (pp=1), ch1 LSB remote: conj(exp(i*30deg)) = exp(-i*30deg). */
static int test_case2_lsb_conjugation()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_ManualPolPhaseCorrection op;
    op.SetStationIdentifier("F");
    op.SetPolarization("Y");
    op.SetPCPhaseOffset(30.0);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-9;
    const std::complex<double> exp_neg30(
        std::cos(MHO_Constants::deg_to_rad * 30.0),
       -std::sin(MHO_Constants::deg_to_rad * 30.0));

    // YY (pp=1), ch1 LSB remote: exp(-i*30deg)
    for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
        for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
            REQUIRE(cclose(vis(1, 1, ap, sp), exp_neg30, tol));

    return 0;
}

/* CASE 3 - Reference station conjugation.
   Ref "G", pol "X", 30deg.
   XX (pp=0), ch0 USB ref: conj(exp(i*30)) = exp(-i*30).
   XX (pp=0), ch1 LSB ref: conj(conj(exp(i*30))) = exp(i*30). */
static int test_case3_ref_station_conjugation()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_ManualPolPhaseCorrection op;
    op.SetStationIdentifier("G");
    op.SetPolarization("X");
    op.SetPCPhaseOffset(30.0);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-9;
    const std::complex<double> exp30(
        std::cos(MHO_Constants::deg_to_rad * 30.0),
        std::sin(MHO_Constants::deg_to_rad * 30.0));
    const std::complex<double> exp_neg30(
        std::cos(MHO_Constants::deg_to_rad * 30.0),
       -std::sin(MHO_Constants::deg_to_rad * 30.0));

    // XX (pp=0), ch0 USB ref: conj(exp(i*30)) = exp(-i*30)
    for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
        for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
            REQUIRE(cclose(vis(0, 0, ap, sp), exp_neg30, tol));

    // XX (pp=0), ch1 LSB ref: conj(conj(exp(i*30))) = exp(i*30)
    for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
        for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
            REQUIRE(cclose(vis(0, 1, ap, sp), exp30, tol));

    // YY (pp=1) unchanged
    const std::complex<double> one(1.0, 0.0);
    for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
        for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
            for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                REQUIRE(cclose(vis(1, ch, ap, sp), one, tol));

    return 0;
}

/* CASE 4 - Pol mismatch: no change.
   pol "L" matches neither X nor Y. All elements unchanged. */
static int test_case4_pol_mismatch_no_change()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_ManualPolPhaseCorrection op;
    op.SetStationIdentifier("F");
    op.SetPolarization("L");
    op.SetPCPhaseOffset(45.0);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-12;
    const std::complex<double> one(1.0, 0.0);
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    REQUIRE(cclose(vis(pp, ch, ap, sp), one, tol));

    return 0;
}

/* CASE 5 - Station mismatch: no change.
   station "Z" matches nothing. All elements unchanged. */
static int test_case5_station_mismatch_no_change()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_ManualPolPhaseCorrection op;
    op.SetStationIdentifier("Z");
    op.SetPolarization("X");
    op.SetPCPhaseOffset(45.0);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-12;
    const std::complex<double> one(1.0, 0.0);
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    REQUIRE(cclose(vis(pp, ch, ap, sp), one, tol));

    return 0;
}

/* CASE 6 - Wildcard pol "?" applies to all pol-products.
   Both XX and YY get the remote, sideband-dependent phasor. */
static int test_case6_wildcard_pol()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_ManualPolPhaseCorrection op;
    op.SetStationIdentifier("F");
    op.SetPolarization("?");
    op.SetPCPhaseOffset(10.0);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-9;
    const std::complex<double> exp10(
        std::cos(MHO_Constants::deg_to_rad * 10.0),
        std::sin(MHO_Constants::deg_to_rad * 10.0));
    const std::complex<double> exp_neg10(
        std::cos(MHO_Constants::deg_to_rad * 10.0),
       -std::sin(MHO_Constants::deg_to_rad * 10.0));

    // Both XX (pp=0) and YY (pp=1), ch0 USB: exp(i*10deg)
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
            for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                REQUIRE(cclose(vis(pp, 0, ap, sp), exp10, tol));

    // Both XX and YY, ch1 LSB: exp(-i*10deg)
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
            for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                REQUIRE(cclose(vis(pp, 1, ap, sp), exp_neg10, tol));

    return 0;
}

/* CASE 7 - Axis label recorded.
   After Case 1 run, retrieve pol-product label "rem_pcphase_offset_Y"
   from pol-axis index 1 (YY). Expected: 30*deg_to_rad. */
static int test_case7_axis_label_recorded()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_ManualPolPhaseCorrection op;
    op.SetStationIdentifier("F");
    op.SetPolarization("Y");
    op.SetPCPhaseOffset(30.0);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto& pp_ax = std::get<POLPROD_AXIS>(vis);
    double val = 0.0;
    bool ok = pp_ax.RetrieveIndexLabelKeyValue(1, "rem_pcphase_offset_Y", val);
    REQUIRE(ok);

    const double expected = MHO_Constants::deg_to_rad * 30.0;
    CHECK_CLOSE(val, expected, 1e-9);

    return 0;
}

/* CASE 8 - Non-idempotency: re-running Execute applies phasor again.
   After Case 1, YY ch0 = exp(i*30deg). Running again with same settings
   multiplies by exp(i*30deg) again => exp(i*60deg). */
static int test_case8_non_idempotent_re_execute()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_ManualPolPhaseCorrection op;
    op.SetStationIdentifier("F");
    op.SetPolarization("Y");
    op.SetPCPhaseOffset(30.0);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // First run: YY ch0 = exp(i*30deg)
    const double tol = 1e-9;
    const std::complex<double> exp30(
        std::cos(MHO_Constants::deg_to_rad * 30.0),
        std::sin(MHO_Constants::deg_to_rad * 30.0));
    for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
        for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
            REQUIRE(cclose(vis(1, 0, ap, sp), exp30, tol));

    // Second run: YY ch0 = exp(i*30deg) * exp(i*30deg) = exp(i*60deg)
    REQUIRE(op.Execute());

    const std::complex<double> exp60(
        std::cos(MHO_Constants::deg_to_rad * 60.0),
        std::sin(MHO_Constants::deg_to_rad * 60.0));
    for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
        for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
            REQUIRE(cclose(vis(1, 0, ap, sp), exp60, tol));

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_case1_remote_usb_polY()) return 1;
    if (test_case2_lsb_conjugation()) return 1;
    if (test_case3_ref_station_conjugation()) return 1;
    if (test_case4_pol_mismatch_no_change()) return 1;
    if (test_case5_station_mismatch_no_change()) return 1;
    if (test_case6_wildcard_pol()) return 1;
    if (test_case7_axis_label_recorded()) return 1;
    if (test_case8_non_idempotent_re_execute()) return 1;

    return 0;
}
