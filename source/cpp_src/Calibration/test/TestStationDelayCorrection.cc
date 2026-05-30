#include <cmath>
#include <complex>
#include <iostream>
#include <string>

#include "MHO_StationDelayCorrection.hh"
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

/* CASE 1 - Remote station applies to BOTH pol-products.
   ref_freq=1000, station="F" (remote), delay=0.5ns.
   ch0 (1000 MHz): deltaf=0 => phasor=(1,0).
   ch1 (1500 MHz) LSB remote: deltaf=5e8; theta=pi/2.
     exp(i*pi/2)=(0,1); conj for LSB => (0,-1).
   Both XX and YY get the same rotation (no pol selection). */
static int test_case1_remote_all_pols()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_StationDelayCorrection op;
    op.SetReferenceFrequency(1000.0);
    op.SetStationIdentifier("F");
    op.SetPCDelayOffset(0.5);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-9;

    // Both pol-products, ch0 USB: deltaf=0 => (1,0)
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
            for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                REQUIRE(cclose(vis(pp, 0, ap, sp), std::complex<double>(1.0, 0.0), tol));

    // Both pol-products, ch1 LSB remote: conj(exp(i*pi/2)) = (0,-1)
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
            for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                REQUIRE(cclose(vis(pp, 1, ap, sp), std::complex<double>(0.0, -1.0), tol));

    return 0;
}

/* CASE 2 - Reference station conjugation, all pols.
   ref_freq=1000, station="G" (ref mk4id), delay=0.5ns.
   ch1 LSB ref: conj(LSB) then conj(ref) => exp(+i*pi/2) = (0,1).
   ch0: deltaf=0 => (1,0). */
static int test_case2_ref_station_all_pols()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_StationDelayCorrection op;
    op.SetReferenceFrequency(1000.0);
    op.SetStationIdentifier("G");
    op.SetPCDelayOffset(0.5);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-9;

    // Both pols, ch0: deltaf=0 => (1,0)
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
            for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                REQUIRE(cclose(vis(pp, 0, ap, sp), std::complex<double>(1.0, 0.0), tol));

    // Both pols, ch1 LSB ref: conj(conj(exp(i*pi/2))) = exp(i*pi/2) = (0,1)
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
            for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                REQUIRE(cclose(vis(pp, 1, ap, sp), std::complex<double>(0.0, 1.0), tol));

    return 0;
}

/* CASE 3 - 2-char station code selection.
   station="Wf" (matches remote_station "Wf"), same math as Case 1. */
static int test_case3_two_char_station_code()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_StationDelayCorrection op;
    op.SetStationIdentifier("Wf");
    op.SetReferenceFrequency(1000.0);
    op.SetPCDelayOffset(0.5);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-9;

    // Both pols, ch0: (1,0)
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
            for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                REQUIRE(cclose(vis(pp, 0, ap, sp), std::complex<double>(1.0, 0.0), tol));

    // Both pols, ch1 LSB remote: (0,-1)
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
            for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                REQUIRE(cclose(vis(pp, 1, ap, sp), std::complex<double>(0.0, -1.0), tol));

    return 0;
}

/* CASE 4 - Wildcard "?" matches both stations by single-char mk4id.
   Both ref (st_idx=0) and rem (st_idx=1) are applicable.
   Corrections accumulate as successive multiplications on the same sub-view:
     after ref (st_idx=0): ch1 LSB phasor = conj(conj(exp(i*pi/2))) = exp(i*pi/2)
     after rem (st_idx=1): ch1 LSB phasor *= conj(exp(i*pi/2)) = exp(-i*pi/2)
     net: exp(i*pi/2) * exp(-i*pi/2) = (1,0)
   ch0 stays (1,0) since deltaf=0. */
static int test_case4_wildcard_both_stations()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_StationDelayCorrection op;
    op.SetStationIdentifier("?");
    op.SetReferenceFrequency(1000.0);
    op.SetPCDelayOffset(0.5);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-9;

    // Both pols, both channels should return to (1,0)
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    REQUIRE(cclose(vis(pp, ch, ap, sp), std::complex<double>(1.0, 0.0), tol));

    return 0;
}

/* CASE 5 - Station mismatch -> no change.
   station="Z" matches nothing. All elements remain (1,0). */
static int test_case5_station_mismatch()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_StationDelayCorrection op;
    op.SetStationIdentifier("Z");
    op.SetReferenceFrequency(1000.0);
    op.SetPCDelayOffset(5.0);

    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    const double tol = 1e-9;
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    REQUIRE(cclose(vis(pp, ch, ap, sp), std::complex<double>(1.0, 0.0), tol));

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_case1_remote_all_pols()) return 1;
    if (test_case2_ref_station_all_pols()) return 1;
    if (test_case3_two_char_station_code()) return 1;
    if (test_case4_wildcard_both_stations()) return 1;
    if (test_case5_station_mismatch()) return 1;

    return 0;
}
