#include <cmath>
#include <iostream>
#include <string>
#include <complex>
#include <vector>

#include "MHO_LSBOffset.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Fixture

/* Build shared synthetic visibility fixture:
   npol=1, nchan=2, nap=2, nspec=4 (16 total elements).
   Channel 0: USB + dsb_partner=1
   Channel 1: LSB + dsb_partner=0
   All elements filled with (1.0, 0.0).
   Station tags: ref mk4id="G", ref code="Gs",
                 rem mk4id="E", rem code="Ef". */
static void build_fixture(visibility_type& vis)
{
    vis.Resize(1, 2, 2, 4);

    auto& pp_ax   = std::get<POLPROD_AXIS>(vis);
    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);

    pp_ax.at(0) = 0.0;
    pp_ax.InsertIndexLabelKeyValue(0, "polarization_product", std::string("XX"));

    chan_ax.at(0) = 8000.0;
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("U"));
    chan_ax.InsertIndexLabelKeyValue(0, "dsb_partner", (int)1);

    chan_ax.at(1) = 8000.0;
    chan_ax.InsertIndexLabelKeyValue(1, "net_sideband", std::string("L"));
    chan_ax.InsertIndexLabelKeyValue(1, "dsb_partner", (int)0);

    vis.Insert(std::string("reference_station_mk4id"), std::string("G"));
    vis.Insert(std::string("reference_station"),       std::string("Gs"));
    vis.Insert(std::string("remote_station_mk4id"),    std::string("E"));
    vis.Insert(std::string("remote_station"),          std::string("Ef"));

    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    vis(pp, ch, ap, sp) = std::complex<double>(1.0, 0.0);
}

// Test cases

static const double PHASOR_30_REAL = 0.8660254037844387;
static const double PHASOR_30_IMAG = 0.5;
static const double TOL = 1e-12;

// CASE 1 - Reference station selected: LSB channel rotated, USB untouched
static int test_reference_station_lsb_rotated()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_LSBOffset op;
    op.SetStationIdentifier("G");
    op.SetLSBPhaseOffset(30.0);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto npol = vis.GetDimension(POLPROD_AXIS);
    auto nap  = vis.GetDimension(TIME_AXIS);
    auto nspec = vis.GetDimension(FREQ_AXIS);

    // Channel 0 (USB): all elements unchanged == (1,0)
    for (std::size_t pp = 0; pp < npol; pp++)
        for (std::size_t ap = 0; ap < nap; ap++)
            for (std::size_t sp = 0; sp < nspec; sp++) {
                CHECK_CLOSE(vis(pp, 0, ap, sp).real(), 1.0, TOL);
                CHECK_CLOSE(vis(pp, 0, ap, sp).imag(), 0.0, TOL);
            }

    // Channel 1 (LSB): every element == phasor(30 deg)
    for (std::size_t pp = 0; pp < npol; pp++)
        for (std::size_t ap = 0; ap < nap; ap++)
            for (std::size_t sp = 0; sp < nspec; sp++) {
                CHECK_CLOSE(vis(pp, 1, ap, sp).real(), PHASOR_30_REAL, TOL);
                CHECK_CLOSE(vis(pp, 1, ap, sp).imag(), PHASOR_30_IMAG, TOL);
            }

    return 0;
}

// CASE 2 - Remote station selected: conjugate phasor applied to LSB
static int test_remote_station_conjugate()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_LSBOffset op;
    op.SetStationIdentifier("E");
    op.SetLSBPhaseOffset(30.0);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto npol = vis.GetDimension(POLPROD_AXIS);
    auto nap  = vis.GetDimension(TIME_AXIS);
    auto nspec = vis.GetDimension(FREQ_AXIS);

    // Channel 0 (USB): unchanged
    for (std::size_t pp = 0; pp < npol; pp++)
        for (std::size_t ap = 0; ap < nap; ap++)
            for (std::size_t sp = 0; sp < nspec; sp++) {
                CHECK_CLOSE(vis(pp, 0, ap, sp).real(), 1.0, TOL);
                CHECK_CLOSE(vis(pp, 0, ap, sp).imag(), 0.0, TOL);
            }

    // Channel 1 (LSB): conj(phasor) = (0.866..., -0.5)
    for (std::size_t pp = 0; pp < npol; pp++)
        for (std::size_t ap = 0; ap < nap; ap++)
            for (std::size_t sp = 0; sp < nspec; sp++) {
                CHECK_CLOSE(vis(pp, 1, ap, sp).real(), PHASOR_30_REAL, TOL);
                CHECK_CLOSE(vis(pp, 1, ap, sp).imag(), -PHASOR_30_IMAG, TOL);
            }

    return 0;
}

// CASE 3 - 2-char station code selection matches reference
static int test_station_code_selection()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_LSBOffset op;
    op.SetStationIdentifier("Gs");
    op.SetLSBPhaseOffset(30.0);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto npol = vis.GetDimension(POLPROD_AXIS);
    auto nap  = vis.GetDimension(TIME_AXIS);
    auto nspec = vis.GetDimension(FREQ_AXIS);

    // Channel 0 (USB): unchanged
    for (std::size_t pp = 0; pp < npol; pp++)
        for (std::size_t ap = 0; ap < nap; ap++)
            for (std::size_t sp = 0; sp < nspec; sp++) {
                CHECK_CLOSE(vis(pp, 0, ap, sp).real(), 1.0, TOL);
                CHECK_CLOSE(vis(pp, 0, ap, sp).imag(), 0.0, TOL);
            }

    // Channel 1 (LSB): phasor applied (reference station matched)
    for (std::size_t pp = 0; pp < npol; pp++)
        for (std::size_t ap = 0; ap < nap; ap++)
            for (std::size_t sp = 0; sp < nspec; sp++) {
                CHECK_CLOSE(vis(pp, 1, ap, sp).real(), PHASOR_30_REAL, TOL);
                CHECK_CLOSE(vis(pp, 1, ap, sp).imag(), PHASOR_30_IMAG, TOL);
            }

    return 0;
}

/* CASE 4 - Wildcard "?" matches both ref and remote:
   double application gives phasor*conj(phasor) = |phasor|^2 = 1.0 */
static int test_wildcard_single_char()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_LSBOffset op;
    op.SetStationIdentifier("?");
    op.SetLSBPhaseOffset(30.0);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto npol = vis.GetDimension(POLPROD_AXIS);
    auto nap  = vis.GetDimension(TIME_AXIS);
    auto nspec = vis.GetDimension(FREQ_AXIS);

    /* Both channels should be (1,0):
       Channel 0 (USB) is never touched (not LSB).
       Channel 1 (LSB) gets phasor * conj(phasor) = 1.0 from double match. */
    for (std::size_t ch = 0; ch < 2; ch++)
        for (std::size_t pp = 0; pp < npol; pp++)
            for (std::size_t ap = 0; ap < nap; ap++)
                for (std::size_t sp = 0; sp < nspec; sp++) {
                    CHECK_CLOSE(vis(pp, ch, ap, sp).real(), 1.0, TOL);
                    CHECK_CLOSE(vis(pp, ch, ap, sp).imag(), 0.0, TOL);
                }

    return 0;
}

// CASE 5 - Non-matching station: no-op
static int test_non_matching_station()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_LSBOffset op;
    op.SetStationIdentifier("Z");
    op.SetLSBPhaseOffset(30.0);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto npol  = vis.GetDimension(POLPROD_AXIS);
    auto nchan = vis.GetDimension(CHANNEL_AXIS);
    auto nap   = vis.GetDimension(TIME_AXIS);
    auto nspec = vis.GetDimension(FREQ_AXIS);

    // All elements unchanged
    for (std::size_t pp = 0; pp < npol; pp++)
        for (std::size_t ch = 0; ch < nchan; ch++)
            for (std::size_t ap = 0; ap < nap; ap++)
                for (std::size_t sp = 0; sp < nspec; sp++) {
                    CHECK_CLOSE(vis(pp, ch, ap, sp).real(), 1.0, TOL);
                    CHECK_CLOSE(vis(pp, ch, ap, sp).imag(), 0.0, TOL);
                }

    return 0;
}

// CASE 6 - Zero offset: identity (phasor == 1)
static int test_zero_offset_identity()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_LSBOffset op;
    op.SetStationIdentifier("G");
    op.SetLSBPhaseOffset(0.0);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto npol  = vis.GetDimension(POLPROD_AXIS);
    auto nchan = vis.GetDimension(CHANNEL_AXIS);
    auto nap   = vis.GetDimension(TIME_AXIS);
    auto nspec = vis.GetDimension(FREQ_AXIS);

    // All elements unchanged
    for (std::size_t pp = 0; pp < npol; pp++)
        for (std::size_t ch = 0; ch < nchan; ch++)
            for (std::size_t ap = 0; ap < nap; ap++)
                for (std::size_t sp = 0; sp < nspec; sp++) {
                    CHECK_CLOSE(vis(pp, ch, ap, sp).real(), 1.0, TOL);
                    CHECK_CLOSE(vis(pp, ch, ap, sp).imag(), 0.0, TOL);
                }

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_reference_station_lsb_rotated())  return 1;
    if (test_remote_station_conjugate())       return 1;
    if (test_station_code_selection())         return 1;
    if (test_wildcard_single_char())           return 1;
    if (test_non_matching_station())           return 1;
    if (test_zero_offset_identity())           return 1;

    return 0;
}
