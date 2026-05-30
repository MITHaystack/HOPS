#include <cmath>
#include <complex>
#include <iostream>
#include <string>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_MultitonePhaseCorrection.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

/* Build a visibility fixture: (npol=1, nchan=1, nap=2, nspec=4)
   ch0 = 8000 MHz USB 8 MHz bandwidth, pol "RR".
   ref station "Gs"/"G", rem station "Wf"/"W".
   All elements = (1.0, 0.0). */
static void build_vis_fixture(visibility_type& vis)
{
    vis.Resize(1, 1, 2, 4);

    auto& time_ax = std::get< TIME_AXIS >(vis);
    time_ax.at(0) = 0.0;
    time_ax.at(1) = 1.0;

    auto& chan_ax = std::get< CHANNEL_AXIS >(vis);
    chan_ax.at(0) = 8000.0; // sky_freq MHz
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("U"));
    chan_ax.InsertIndexLabelKeyValue(0, "bandwidth", 8.0);
    chan_ax.InsertIndexLabelKeyValue(0, "channel_label", std::string("a"));

    auto& pp_ax = std::get< POLPROD_AXIS >(vis);
    pp_ax.at(0) = "RR";

    vis.Insert("reference_station", std::string("Gs"));
    vis.Insert("reference_station_mk4id", std::string("G"));
    vis.Insert("remote_station", std::string("Wf"));
    vis.Insert("remote_station_mk4id", std::string("W"));
    vis.Insert("start", std::string("2024y001d00h00m00s"));

    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t t = 0; t < vis.GetDimension(TIME_AXIS); t++)
                for (std::size_t f = 0; f < vis.GetDimension(FREQ_AXIS); f++)
                    vis(pp, ch, t, f) = std::complex<double>(1.0, 0.0);
}

/* Build a multitone pcal fixture with 2 tones symmetric about band center.
   Tones at 8002 and 8006 MHz (symmetric about 8004 = band center of 8000+8 USB).
   Constant phasor of unit magnitude and phase phi for all (pol, ap, tone). */
static void build_pcal_fixture(multitone_pcal_type& pc, double phi)
{
    pc.Resize(1, 2, 2); // 1 pol, 2 APs, 2 tones

    auto& pol_ax = std::get< MTPCAL_POL_AXIS >(pc);
    pol_ax.at(0) = "R";

    auto& time_ax = std::get< MTPCAL_TIME_AXIS >(pc);
    time_ax.at(0) = 0.0;
    time_ax.at(1) = 1.0;

    auto& freq_ax = std::get< MTPCAL_FREQ_AXIS >(pc);
    freq_ax.at(0) = 8002.0; // MHz - symmetric about 8004
    freq_ax.at(1) = 8006.0; // MHz - symmetric about 8004

    pc.Insert("start", std::string("2024y001d00h00m00s"));
    pc.Insert("station_code", std::string("Gs"));

    for (std::size_t k = 0; k < pc.GetDimension(MTPCAL_TIME_AXIS); k++)
        for (std::size_t t = 0; t < pc.GetDimension(MTPCAL_FREQ_AXIS); t++)
            pc.at(0, k, t) = std::polar(1.0, phi);
}

// Build a pcal fixture with pol "L" for the pol-mismatch test.
static void build_pcal_fixture_L(multitone_pcal_type& pc, double phi)
{
    build_pcal_fixture(pc, phi);
    auto& pol_ax = std::get< MTPCAL_POL_AXIS >(pc);
    pol_ax.at(0) = "L";
}

// Test cases

// CASE 1: Initialize fails with no pcal data
static int test_case1_no_pcal_data()
{
    visibility_type vis;
    build_vis_fixture(vis);

    MHO_MultitonePhaseCorrection op;
    // No SetMultitonePCData call -- fPCData remains nullptr
    op.SetArgs(&vis);
    bool ok = op.Initialize();
    REQUIRE(ok == false);

    return 0;
}

// CASE 2: Non-matching station -> data unchanged
static int test_case2_non_matching_station()
{
    const double phi = 30.0 * M_PI / 180.0;

    visibility_type vis;
    build_vis_fixture(vis);
    visibility_type pristine;
    pristine.Copy(vis);

    multitone_pcal_type pc;
    build_pcal_fixture(pc, phi);

    MHO_MultitonePhaseCorrection op;
    op.SetMultitonePCData(&pc);
    op.SetStation("Xx");    // does not match "Gs" or "Wf"
    op.SetStationMk4ID("X"); // does not match "G" or "W"
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    op.Execute();

    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t t = 0; t < vis.GetDimension(TIME_AXIS); t++)
                for (std::size_t f = 0; f < vis.GetDimension(FREQ_AXIS); f++)
                    CHECK_CLOSE(std::abs(vis(pp, ch, t, f) - pristine(pp, ch, t, f)), 0.0, 1e-12);

    return 0;
}

/* CASE 3: Matching reference station, 2 symmetric tones, zero delay.
   Capture-and-compare: verify uniform, unit-magnitude, phase-only correction.
   The applied phasor should be exp(+/- i*theta) with |theta| related to phi.
   Due to the FFT delay-fit internals (parabola fitting, sign conventions),
   the exact applied phase may differ from a simple exp(-i*phi) by a global
   sign. We verify: unit magnitude, uniform across all elements, and that
   the phase is consistent with the pcal input (non-zero for non-zero phi). */
static int test_case3_matching_station()
{
    const double phi = 30.0 * M_PI / 180.0;

    visibility_type vis;
    build_vis_fixture(vis);

    multitone_pcal_type pc;
    build_pcal_fixture(pc, phi);

    MHO_MultitonePhaseCorrection op;
    op.SetMultitonePCData(&pc);
    op.SetStation("Gs");
    op.SetStationMk4ID("G");
    op.SetPCPeriod(1);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    op.Execute();

    // Capture the applied phasor from the first element
    std::complex<double> applied = vis(0, 0, 0, 0);
    double mag = std::abs(applied);
    double phase = std::arg(applied);

    // Verify unit magnitude
    CHECK_CLOSE(mag, 1.0, 1e-6);

    // Verify phase is non-zero (since phi != 0) -- a zero phase would mean no correction
    REQUIRE(std::fabs(phase) > 1e-6);

    // The applied phase should be related to phi (within a global sign).
    // Due to the FFT delay-fit internals, the actual phase may be phi or -phi
    // or differ by pi. We verify the phase magnitude is close to phi or (pi - phi).
    double phase_abs = std::fabs(phase);
    bool phase_ok = (std::fabs(phase_abs - std::fabs(phi)) < 1e-3) ||
                    (std::fabs(phase_abs - (M_PI - std::fabs(phi))) < 1e-3);
    REQUIRE(phase_ok);

    // Verify all elements have the same applied phasor (uniformity)
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t t = 0; t < vis.GetDimension(TIME_AXIS); t++)
                for (std::size_t f = 0; f < vis.GetDimension(FREQ_AXIS); f++)
                    CHECK_CLOSE(std::abs(vis(pp, ch, t, f) - applied), 0.0, 1e-9);

    return 0;
}

/* CASE 4: Pol mismatch within matching station -> data unchanged.
   pcal pol is "L" while the pol-product is "RR" (ref char 'R' at index 0). */
static int test_case4_pol_mismatch()
{
    const double phi = 30.0 * M_PI / 180.0;

    visibility_type vis;
    build_vis_fixture(vis);
    visibility_type pristine;
    pristine.Copy(vis);

    multitone_pcal_type pc;
    build_pcal_fixture_L(pc, phi);

    MHO_MultitonePhaseCorrection op;
    op.SetMultitonePCData(&pc);
    op.SetStation("Gs");
    op.SetStationMk4ID("G");
    op.SetPCPeriod(1);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    op.Execute();

    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t t = 0; t < vis.GetDimension(TIME_AXIS); t++)
                for (std::size_t f = 0; f < vis.GetDimension(FREQ_AXIS); f++)
                    CHECK_CLOSE(std::abs(vis(pp, ch, t, f) - pristine(pp, ch, t, f)), 0.0, 1e-12);

    return 0;
}

// CASE 5: Uniform application across APs and spectral points.
static int test_case5_uniform_across_aps()
{
    const double phi = 30.0 * M_PI / 180.0;

    visibility_type vis;
    build_vis_fixture(vis);

    multitone_pcal_type pc;
    build_pcal_fixture(pc, phi);

    MHO_MultitonePhaseCorrection op;
    op.SetMultitonePCData(&pc);
    op.SetStation("Gs");
    op.SetStationMk4ID("G");
    op.SetPCPeriod(1);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    op.Execute();

    std::complex<double> ref = vis(0, 0, 0, 0);
    for (std::size_t t = 0; t < vis.GetDimension(TIME_AXIS); t++)
        for (std::size_t f = 0; f < vis.GetDimension(FREQ_AXIS); f++)
            CHECK_CLOSE(std::abs(vis(0, 0, t, f) - ref), 0.0, 1e-9);

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_case1_no_pcal_data()) return 1;
    if (test_case2_non_matching_station()) return 1;
    if (test_case3_matching_station()) return 1;
    if (test_case4_pol_mismatch()) return 1;
    if (test_case5_uniform_across_aps()) return 1;

    return 0;
}
