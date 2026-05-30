#include <cmath>
#include <iostream>
#include <string>
#include <complex>

#include "MHO_MixedPolYShift.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

/* Build synthetic visibility fixture:
   npol=3 (YR, RY, RR), nchan=2 (USB, LSB), nap=2, nspec=2.
   All elements filled with (1.0, 0.0).
   Station tags: ref="Gs", rem="Wf".
   Returns a copy as pristine for comparison. */
static void build_fixture(visibility_type& vis, visibility_type& pristine)
{
    vis.Resize(3, 2, 2, 2);

    auto& pp_ax   = std::get<POLPROD_AXIS>(vis);
    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);

    pp_ax.at(0) = std::string("YR");

    pp_ax.at(1) = std::string("RY");

    pp_ax.at(2) = std::string("RR");

    chan_ax.at(0) = 8000.0;
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("U"));

    chan_ax.at(1) = 8000.0;
    chan_ax.InsertIndexLabelKeyValue(1, "net_sideband", std::string("L"));

    vis.Insert(std::string("reference_station"), std::string("Gs"));
    vis.Insert(std::string("remote_station"),    std::string("Wf"));

    auto npol  = vis.GetDimension(POLPROD_AXIS);
    auto nchan = vis.GetDimension(CHANNEL_AXIS);
    auto nap   = vis.GetDimension(TIME_AXIS);
    auto nspec = vis.GetDimension(FREQ_AXIS);

    for (std::size_t pp = 0; pp < npol; pp++)
        for (std::size_t ch = 0; ch < nchan; ch++)
            for (std::size_t ap = 0; ap < nap; ap++)
                for (std::size_t sp = 0; sp < nspec; sp++)
                    vis(pp, ch, ap, sp) = std::complex<double>(1.0, 0.0);

    // Make pristine copy
    pristine.Copy(vis);
}

// Test cases

static const double TOL = 1e-12;

/* CASE 1 - YR on reference station, USB and LSB.
   base phasor = exp(i * -90 deg) = (0, -1).
   Ref station => conj => (0, +1).
   USB: conj(base) = (0, +1).
   LSB: conj(conj(base)) = base = (0, -1). */
static int test_yr_reference_station()
{
    visibility_type vis;
    visibility_type pristine;
    build_fixture(vis, pristine);

    MHO_MixedPolYShift op;
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto nap  = vis.GetDimension(TIME_AXIS);
    auto nspec = vis.GetDimension(FREQ_AXIS);

    // pp=0 (YR): reference station Y-pol mixed with R circular
    // ch=0 (USB): should be conj(base) = (0, +1)
    for (std::size_t ap = 0; ap < nap; ap++)
        for (std::size_t sp = 0; sp < nspec; sp++) {
            CHECK_CLOSE(vis(0, 0, ap, sp).real(), 0.0, TOL);
            CHECK_CLOSE(vis(0, 0, ap, sp).imag(), 1.0, TOL);
        }

    // ch=1 (LSB): should be conj(conj(base)) = base = (0, -1)
    for (std::size_t ap = 0; ap < nap; ap++)
        for (std::size_t sp = 0; sp < nspec; sp++) {
            CHECK_CLOSE(vis(0, 1, ap, sp).real(), 0.0, TOL);
            CHECK_CLOSE(vis(0, 1, ap, sp).imag(), -1.0, TOL);
        }

    return 0;
}

/* CASE 2 - RY on remote station, USB and LSB.
   base phasor = exp(i * -90 deg) = (0, -1).
   Remote station => no conj => (0, -1).
   USB: base = (0, -1).
   LSB: conj(base) = (0, +1). */
static int test_ry_remote_station()
{
    visibility_type vis;
    visibility_type pristine;
    build_fixture(vis, pristine);

    MHO_MixedPolYShift op;
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto nap  = vis.GetDimension(TIME_AXIS);
    auto nspec = vis.GetDimension(FREQ_AXIS);

    // pp=1 (RY): remote station Y-pol mixed with R circular
    // ch=0 (USB): should be base = (0, -1)
    for (std::size_t ap = 0; ap < nap; ap++)
        for (std::size_t sp = 0; sp < nspec; sp++) {
            CHECK_CLOSE(vis(1, 0, ap, sp).real(), 0.0, TOL);
            CHECK_CLOSE(vis(1, 0, ap, sp).imag(), -1.0, TOL);
        }

    // ch=1 (LSB): should be conj(base) = (0, +1)
    for (std::size_t ap = 0; ap < nap; ap++)
        for (std::size_t sp = 0; sp < nspec; sp++) {
            CHECK_CLOSE(vis(1, 1, ap, sp).real(), 0.0, TOL);
            CHECK_CLOSE(vis(1, 1, ap, sp).imag(), 1.0, TOL);
        }

    return 0;
}

// CASE 3 - RR (non-mixed) pol-product is untouched.
static int test_rr_untouched()
{
    visibility_type vis;
    visibility_type pristine;
    build_fixture(vis, pristine);

    MHO_MixedPolYShift op;
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto nchan = vis.GetDimension(CHANNEL_AXIS);
    auto nap   = vis.GetDimension(TIME_AXIS);
    auto nspec = vis.GetDimension(FREQ_AXIS);

    // pp=2 (RR): not mixed, should remain (1, 0) everywhere
    for (std::size_t ch = 0; ch < nchan; ch++)
        for (std::size_t ap = 0; ap < nap; ap++)
            for (std::size_t sp = 0; sp < nspec; sp++) {
                CHECK_CLOSE(vis(2, ch, ap, sp).real(), 1.0, TOL);
                CHECK_CLOSE(vis(2, ch, ap, sp).imag(), 0.0, TOL);
            }

    return 0;
}

/* CASE 4 - Custom phase offset of 0.0 via SetPhaseOffset.
   base = exp(0) = (1, 0); conj = (1, 0).
   All elements should be unchanged (identity phasor). */
static int test_custom_phase_offset_zero()
{
    visibility_type vis;
    visibility_type pristine;
    build_fixture(vis, pristine);

    MHO_MixedPolYShift op;
    op.SetPhaseOffset(0.0);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto npol  = vis.GetDimension(POLPROD_AXIS);
    auto nchan = vis.GetDimension(CHANNEL_AXIS);
    auto nap   = vis.GetDimension(TIME_AXIS);
    auto nspec = vis.GetDimension(FREQ_AXIS);

    // All elements should be (1, 0) - same as pristine
    for (std::size_t pp = 0; pp < npol; pp++)
        for (std::size_t ch = 0; ch < nchan; ch++)
            for (std::size_t ap = 0; ap < nap; ap++)
                for (std::size_t sp = 0; sp < nspec; sp++) {
                    CHECK_CLOSE(vis(pp, ch, ap, sp).real(), 1.0, TOL);
                    CHECK_CLOSE(vis(pp, ch, ap, sp).imag(), 0.0, TOL);
                }

    return 0;
}

/* CASE 5 - nullptr input returns false.
   We call ExecuteInPlace via the public Execute() after
   SetArgs(nullptr) to exercise the nullptr guard. */
static int test_nullptr_input()
{
    MHO_MixedPolYShift op;
    op.SetArgs((visibility_type*)nullptr);

    bool result = op.Execute();
    REQUIRE(result == false);

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_yr_reference_station())     return 1;
    if (test_ry_remote_station())        return 1;
    if (test_rr_untouched())             return 1;
    if (test_custom_phase_offset_zero()) return 1;
    if (test_nullptr_input())            return 1;

    return 0;
}
