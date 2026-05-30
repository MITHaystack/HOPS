#include <cmath>
#include <complex>
#include <iostream>
#include <string>

#include "MHO_ManualPolDelayCorrection.hh"
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
   Channel 0: center_freq=1000 MHz, label="a", net_sideband="U" (USB).
   Channel 1: center_freq=1500 MHz, label="b", net_sideband="L" (LSB).
   Station tags: ref mk4id="G", code="Gs"; rem mk4id="F", code="Wf". */
static void build_fixture(visibility_type& vis)
{
    vis.Resize(2, 2, 2, 2);

    auto& pp_ax = std::get<POLPROD_AXIS>(vis);
    pp_ax.at(0) = "XX";
    pp_ax.at(1) = "YY";

    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    chan_ax.at(0) = 1000.0;  // MHz
    chan_ax.at(1) = 1500.0;  // MHz
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

/* CASE 1 - Remote, pol Y, delay applies as channel-freq phase.
   ref_freq=1000, delay=2.0ns.
   ch0 (1000 MHz): deltaf=0 => theta=0 => phasor=(1,0) (no change).
   ch1 (1500 MHz): deltaf=5e8; theta=2*pi*5e8*2e-9 = 2*pi*1.0 = 2*pi
                   => exp(i*2*pi)=(1,0); conj for LSB => still (1,0).
   This mainly checks that ref-freq=channel gives no-op. */
static int test_case1_remote_polY_ref_freq_equals_channel()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_ManualPolDelayCorrection op;
    op.SetReferenceFrequency(1000.0);
    op.SetStationIdentifier("F");
    op.SetPolarization("Y");
    op.SetPCDelayOffset(2.0);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-9;
    const std::complex<double> one(1.0, 0.0);

    // YY (pp=1): both channels should be (1,0) - no effective change
    for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
        for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
            for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                REQUIRE(cclose(vis(1, ch, ap, sp), one, tol));

    // XX (pp=0) untouched
    for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
        for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
            for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                REQUIRE(cclose(vis(0, ch, ap, sp), one, tol));

    return 0;
}

/* CASE 2 - Non-trivial phase (quarter turn) and sideband sign.
   ref_freq=1000, delay=0.5ns.
   ch0: deltaf=0 => (1,0).
   ch1 LSB remote: deltaf=5e8; theta=2*pi*5e8*0.5e-9 = pi/2.
     exp(i*pi/2)=(0,1); conj for LSB => (0,-1). */
static int test_case2_quarter_turn_and_lsb_sign()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_ManualPolDelayCorrection op;
    op.SetReferenceFrequency(1000.0);
    op.SetStationIdentifier("F");
    op.SetPolarization("Y");
    op.SetPCDelayOffset(0.5);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-9;

    // YY (pp=1), ch0 USB: deltaf=0 => (1,0)
    for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
        for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
            REQUIRE(cclose(vis(1, 0, ap, sp), std::complex<double>(1.0, 0.0), tol));

    // YY (pp=1), ch1 LSB remote: conj(exp(i*pi/2)) = (0,-1)
    for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
        for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
            REQUIRE(cclose(vis(1, 1, ap, sp), std::complex<double>(0.0, -1.0), tol));

    // XX (pp=0) untouched
    for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
        for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
            for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                REQUIRE(cclose(vis(0, ch, ap, sp), std::complex<double>(1.0, 0.0), tol));

    return 0;
}

/* CASE 3 - Reference station conjugation.
   ref_freq=1000, station="G" (ref mk4id), pol X, delay=0.5ns.
   XX (pp=0), ch1 LSB ref: conj(LSB) then conj(ref) => exp(+i*pi/2) = (0,1).
   ch0: deltaf=0 => (1,0). */
static int test_case3_ref_station_conjugation()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_ManualPolDelayCorrection op;
    op.SetReferenceFrequency(1000.0);
    op.SetStationIdentifier("G");
    op.SetPolarization("X");
    op.SetPCDelayOffset(0.5);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-9;

    // XX (pp=0), ch0: deltaf=0 => (1,0)
    for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
        for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
            REQUIRE(cclose(vis(0, 0, ap, sp), std::complex<double>(1.0, 0.0), tol));

    // XX (pp=0), ch1 LSB ref: conj(conj(exp(i*pi/2))) = exp(i*pi/2) = (0,1)
    for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
        for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
            REQUIRE(cclose(vis(0, 1, ap, sp), std::complex<double>(0.0, 1.0), tol));

    return 0;
}

/* CASE 4 - Pol/station mismatch -> no change.
   station "Z" doesn't match any station. All elements remain (1,0). */
static int test_case4_station_mismatch_no_change()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_ManualPolDelayCorrection op;
    op.SetReferenceFrequency(1000.0);
    op.SetStationIdentifier("Z");
    op.SetPolarization("X");
    op.SetPCDelayOffset(5.0);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-12;
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    REQUIRE(cclose(vis(pp, ch, ap, sp), std::complex<double>(1.0, 0.0), tol));

    return 0;
}

/* CASE 5 - Recorded axis label.
   After Case 2 setup, retrieve "rem_delayoff_Y" from pol-axis index 1 (YY).
   Expected value: 0.5 ns. */
static int test_case5_recorded_axis_label()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_ManualPolDelayCorrection op;
    op.SetReferenceFrequency(1000.0);
    op.SetStationIdentifier("F");
    op.SetPolarization("Y");
    op.SetPCDelayOffset(0.5);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto& pp_ax = std::get<POLPROD_AXIS>(vis);
    double val = 0.0;
    bool ok = pp_ax.RetrieveIndexLabelKeyValue(1, "rem_delayoff_Y", val);
    REQUIRE(ok);
    CHECK_CLOSE(val, 0.5, 1e-12);

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_case1_remote_polY_ref_freq_equals_channel()) return 1;
    if (test_case2_quarter_turn_and_lsb_sign()) return 1;
    if (test_case3_ref_station_conjugation()) return 1;
    if (test_case4_station_mismatch_no_change()) return 1;
    if (test_case5_recorded_axis_label()) return 1;

    return 0;
}
