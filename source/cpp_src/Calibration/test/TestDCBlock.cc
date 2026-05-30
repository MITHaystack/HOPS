#include <cmath>
#include <iostream>
#include <string>
#include <complex>

#include "MHO_DCBlock.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Helpers

/* Build the shared fixture: npol=2, nchan=2, nap=3, nspec=4.
   Channel 0: USB (net_sideband="U"), Channel 1: LSB (net_sideband="L").
   All visibility elements filled with (1.0, 0.0). */
static void build_fixture(visibility_type& vis)
{
    vis.Resize(2, 2, 3, 4);

    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    chan_ax.at(0) = 8000.0;
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("U"));
    chan_ax.at(1) = 8200.0;
    chan_ax.InsertIndexLabelKeyValue(1, "net_sideband", std::string("L"));

    // Fill all elements with (1.0, 0.0)
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    vis(pp, ch, ap, sp) = std::complex<double>(1.0, 0.0);
}

// Build fixture but WITHOUT net_sideband label on channel 0.
static void build_fixture_no_sideband(visibility_type& vis)
{
    vis.Resize(2, 2, 3, 4);

    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    chan_ax.at(0) = 8000.0;
    // intentionally no net_sideband on channel 0
    chan_ax.at(1) = 8200.0;
    chan_ax.InsertIndexLabelKeyValue(1, "net_sideband", std::string("L"));

    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    vis(pp, ch, ap, sp) = std::complex<double>(1.0, 0.0);
}

// Test cases

// CASE 1 - USB channel zeroes spectral point 0 only
static int test_usb_dc_zero()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_DCBlock op;
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto npol = vis.GetDimension(POLPROD_AXIS);
    auto nap  = vis.GetDimension(TIME_AXIS);
    auto nspec = vis.GetDimension(FREQ_AXIS);

    // Channel 0 (USB): sp==0 must be zeroed, sp 1..3 must be (1,0)
    for (std::size_t pp = 0; pp < npol; pp++)
        for (std::size_t ap = 0; ap < nap; ap++) {
            CHECK_CLOSE(vis(pp, 0, ap, 0).real(), 0.0, 1e-15);
            CHECK_CLOSE(vis(pp, 0, ap, 0).imag(), 0.0, 1e-15);
            for (std::size_t sp = 1; sp < nspec; sp++) {
                CHECK_CLOSE(vis(pp, 0, ap, sp).real(), 1.0, 1e-15);
                CHECK_CLOSE(vis(pp, 0, ap, sp).imag(), 0.0, 1e-15);
            }
        }

    return 0;
}

// CASE 2 - LSB channel zeroes the LAST spectral point only
static int test_lsb_dc_zero()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_DCBlock op;
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto npol = vis.GetDimension(POLPROD_AXIS);
    auto nap  = vis.GetDimension(TIME_AXIS);
    auto nspec = vis.GetDimension(FREQ_AXIS);

    // Channel 1 (LSB): sp==(nspec-1)==3 must be zeroed, sp 0..2 must be (1,0)
    for (std::size_t pp = 0; pp < npol; pp++)
        for (std::size_t ap = 0; ap < nap; ap++) {
            CHECK_CLOSE(vis(pp, 1, ap, nspec - 1).real(), 0.0, 1e-15);
            CHECK_CLOSE(vis(pp, 1, ap, nspec - 1).imag(), 0.0, 1e-15);
            for (std::size_t sp = 0; sp < nspec - 1; sp++) {
                CHECK_CLOSE(vis(pp, 1, ap, sp).real(), 1.0, 1e-15);
                CHECK_CLOSE(vis(pp, 1, ap, sp).imag(), 0.0, 1e-15);
            }
        }

    return 0;
}

// CASE 3 - Coverage across all pol-products and APs (count zeroed elements)
static int test_all_pol_ap_coverage()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_DCBlock op;
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto npol = vis.GetDimension(POLPROD_AXIS);
    auto nap  = vis.GetDimension(TIME_AXIS);
    auto nspec = vis.GetDimension(FREQ_AXIS);

    // Each channel should have exactly npol * nap zeroed elements
    std::size_t expected_zeros = npol * nap;

    for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++) {
        std::size_t zero_count = 0;
        for (std::size_t pp = 0; pp < npol; pp++)
            for (std::size_t ap = 0; ap < nap; ap++)
                for (std::size_t sp = 0; sp < nspec; sp++)
                    if (vis(pp, ch, ap, sp) == std::complex<double>(0.0, 0.0))
                        zero_count++;
        REQUIRE(zero_count == expected_zeros);
    }

    return 0;
}

// CASE 4 - Missing net_sideband label defaults to point 0 (USB path)
static int test_missing_sideband_default()
{
    visibility_type vis;
    build_fixture_no_sideband(vis);

    MHO_DCBlock op;
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto npol = vis.GetDimension(POLPROD_AXIS);
    auto nap  = vis.GetDimension(TIME_AXIS);
    auto nspec = vis.GetDimension(FREQ_AXIS);

    // Channel 0 (no label, defaults to USB/dc_index=0): sp==0 zeroed
    for (std::size_t pp = 0; pp < npol; pp++)
        for (std::size_t ap = 0; ap < nap; ap++) {
            CHECK_CLOSE(vis(pp, 0, ap, 0).real(), 0.0, 1e-15);
            CHECK_CLOSE(vis(pp, 0, ap, 0).imag(), 0.0, 1e-15);
            for (std::size_t sp = 1; sp < nspec; sp++) {
                CHECK_CLOSE(vis(pp, 0, ap, sp).real(), 1.0, 1e-15);
                CHECK_CLOSE(vis(pp, 0, ap, sp).imag(), 0.0, 1e-15);
            }
        }

    // Channel 1 (LSB, label="L"): sp==(nspec-1) zeroed
    for (std::size_t pp = 0; pp < npol; pp++)
        for (std::size_t ap = 0; ap < nap; ap++) {
            CHECK_CLOSE(vis(pp, 1, ap, nspec - 1).real(), 0.0, 1e-15);
            CHECK_CLOSE(vis(pp, 1, ap, nspec - 1).imag(), 0.0, 1e-15);
        }

    return 0;
}

// CASE 5 - Idempotency: second Execute produces no change
static int test_idempotency()
{
    visibility_type vis;
    build_fixture(vis);

    MHO_DCBlock op;
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // Capture state after first execute
    visibility_type after_first;
    after_first.Copy(vis);

    // Second execute
    REQUIRE(op.Execute());

    auto npol = vis.GetDimension(POLPROD_AXIS);
    auto nchan = vis.GetDimension(CHANNEL_AXIS);
    auto nap  = vis.GetDimension(TIME_AXIS);
    auto nspec = vis.GetDimension(FREQ_AXIS);

    // Every element must be identical
    for (std::size_t pp = 0; pp < npol; pp++)
        for (std::size_t ch = 0; ch < nchan; ch++)
            for (std::size_t ap = 0; ap < nap; ap++)
                for (std::size_t sp = 0; sp < nspec; sp++) {
                    CHECK_CLOSE(vis(pp, ch, ap, sp).real(),
                                 after_first(pp, ch, ap, sp).real(), 1e-15);
                    CHECK_CLOSE(vis(pp, ch, ap, sp).imag(),
                                 after_first(pp, ch, ap, sp).imag(), 1e-15);
                }

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_usb_dc_zero())          return 1;
    if (test_lsb_dc_zero())          return 1;
    if (test_all_pol_ap_coverage())  return 1;
    if (test_missing_sideband_default()) return 1;
    if (test_idempotency())          return 1;

    return 0;
}
