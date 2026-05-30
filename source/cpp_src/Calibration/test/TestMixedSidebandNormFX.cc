#include <cmath>
#include <iostream>
#include <string>
#include <complex>
#include <vector>
#include <algorithm>

#include "MHO_MixedSidebandNormFX.hh"
#include "MHO_SingleSidebandNormFX.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

/* Build a flat-spectrum USB fixture: npol=1, nchan=1, nap=1, nfreq=8.
   Channel 0: net_sideband="U", sky_freq=8000.0, bandwidth=32.0.
   All visibilities = (1.0, 0.0). */
static void build_flat_fixture(visibility_type& vis)
{
    vis.Resize(1, 1, 1, 8);

    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    chan_ax.at(0) = 8000.0;
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("U"));
    chan_ax.InsertIndexLabelKeyValue(0, "sky_freq", 8000.0);
    chan_ax.InsertIndexLabelKeyValue(0, "bandwidth", 32.0);

    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t fr = 0; fr < vis.GetDimension(FREQ_AXIS); fr++)
                    vis(pp, ch, ap, fr) = std::complex<double>(1.0, 0.0);
}

/* Build a tone fixture: npol=1, nchan=1, nap=1, nfreq=8.
   Channel 0: net_sideband="U", sky_freq=8000.0.
   vis(0,0,0,fr) = exp(+i * 2*pi * k0 * fr / Nfreq). */
static void build_tone_fixture(visibility_type& vis, int k0)
{
    vis.Resize(1, 1, 1, 8);

    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    chan_ax.at(0) = 8000.0;
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("U"));
    chan_ax.InsertIndexLabelKeyValue(0, "sky_freq", 8000.0);

    double Nfreq = (double)vis.GetDimension(FREQ_AXIS);
    for (std::size_t fr = 0; fr < vis.GetDimension(FREQ_AXIS); fr++)
        vis(0, 0, 0, fr) = std::polar(1.0, 2.0 * M_PI * (double)k0 * (double)fr / Nfreq);
}

// Find the bin index of the maximum magnitude in the output's freq axis.
static std::size_t find_peak_bin(const visibility_type& out)
{
    std::size_t nfreq = out.GetDimension(FREQ_AXIS);
    std::size_t best  = 0;
    double best_mag   = std::abs(out(0, 0, 0, 0));
    for (std::size_t fr = 1; fr < nfreq; fr++) {
        double mag = std::abs(out(0, 0, 0, fr));
        if (mag > best_mag) {
            best_mag = mag;
            best     = fr;
        }
    }
    return best;
}

// Test cases

// CASE 1 - Output dimensions == 4*Nfreq
static int test_output_dimensions()
{
    visibility_type vis;
    build_flat_fixture(vis);
    visibility_type out;

    MHO_MixedSidebandNormFX nf;
    nf.SetArgs(&vis, &out);
    REQUIRE(nf.Initialize());
    REQUIRE(nf.Execute());

    // FREQ_AXIS should be 4 * 8 = 32; others unchanged
    REQUIRE(out.GetDimension(POLPROD_AXIS) == 1);
    REQUIRE(out.GetDimension(CHANNEL_AXIS) == 1);
    REQUIRE(out.GetDimension(TIME_AXIS)    == 1);
    REQUIRE(out.GetDimension(FREQ_AXIS)    == 32);

    return 0;
}

// CASE 2 - Dimension-consistency: 4 * Nfreq_in == Nfreq_out
static int test_dimension_consistency()
{
    visibility_type vis;
    build_flat_fixture(vis);
    visibility_type out;

    MHO_MixedSidebandNormFX nf;
    nf.SetArgs(&vis, &out);
    REQUIRE(nf.Initialize());

    std::size_t nf_in  = vis.GetDimension(FREQ_AXIS);
    std::size_t nf_out = out.GetDimension(FREQ_AXIS);
    REQUIRE(nf_out == 4 * nf_in);

    return 0;
}

// CASE 3 - Flat spectrum -> single centered peak at bin 16 (Nout/2)
static int test_flat_spectrum_centered_peak()
{
    visibility_type vis;
    build_flat_fixture(vis);
    visibility_type out;

    MHO_MixedSidebandNormFX nf;
    nf.SetArgs(&vis, &out);
    REQUIRE(nf.Initialize());
    REQUIRE(nf.Execute());

    std::size_t peak = find_peak_bin(out);
    REQUIRE(peak == 16);  // Nout/2 = 32/2

    /* The peak must be the strict global maximum. Check that the next-highest
       bin has strictly smaller magnitude. */
    double peak_mag = std::abs(out(0, 0, 0, peak));
    double max_off  = 0.0;
    for (std::size_t fr = 0; fr < out.GetDimension(FREQ_AXIS); fr++) {
        if (fr == peak) continue;
        double mag = std::abs(out(0, 0, 0, fr));
        if (mag > max_off) max_off = mag;
    }
    REQUIRE(peak_mag > max_off);

    return 0;
}

// CASE 4 - Normalization sanity: peak magnitude == 1.0 for flat input
static int test_normalization()
{
    visibility_type vis;
    build_flat_fixture(vis);
    visibility_type out;

    MHO_MixedSidebandNormFX nf;
    nf.SetArgs(&vis, &out);
    REQUIRE(nf.Initialize());
    REQUIRE(nf.Execute());

    /* Numpy reference: 8x pad -> FFT -> /2 subsample -> roll -> *1/N
       For flat unit input, |peak| = 1.0 exactly. */
    std::size_t peak     = find_peak_bin(out);
    double peak_mag      = std::abs(out(0, 0, 0, peak));
    double reference_mag = 1.0;
    CHECK_CLOSE(peak_mag, reference_mag, 1e-4);

    return 0;
}

// CASE 5 - Cross-check vs SingleSidebandNormFX (regression anchor)
static int test_cross_check_ssb()
{
    // Build identical input for both operators
    visibility_type vis;
    build_flat_fixture(vis);

    visibility_type out_mixed;
    visibility_type out_ssb;

    MHO_MixedSidebandNormFX mixed_op;
    mixed_op.SetArgs(&vis, &out_mixed);
    REQUIRE(mixed_op.Initialize());
    REQUIRE(mixed_op.Execute());

    MHO_SingleSidebandNormFX ssb_op;
    ssb_op.SetArgs(&vis, &out_ssb);
    REQUIRE(ssb_op.Initialize());
    REQUIRE(ssb_op.Execute());

    // Both should produce 32-point output
    REQUIRE(out_mixed.GetDimension(FREQ_AXIS) == 32);
    REQUIRE(out_ssb.GetDimension(FREQ_AXIS)   == 32);

    // Peak bins must be at the same index
    std::size_t peak_mixed = find_peak_bin(out_mixed);
    std::size_t peak_ssb   = find_peak_bin(out_ssb);
    REQUIRE(peak_mixed == peak_ssb);

    // Peak magnitudes must agree within 1e-3 relative
    double mag_mixed = std::abs(out_mixed(0, 0, 0, peak_mixed));
    double mag_ssb   = std::abs(out_ssb(0, 0, 0, peak_ssb));
    if (mag_mixed > 0.0) {
        double rel_err = std::fabs(mag_mixed - mag_ssb) / mag_mixed;
        CHECK_CLOSE(rel_err, 0.0, 1e-3);
    }

    return 0;
}

// CASE 6 - Re-use / idempotency: Initialize+Execute twice, compare
static int test_reuse_idempotency()
{
    visibility_type vis;
    build_flat_fixture(vis);
    visibility_type out1, out2;

    MHO_MixedSidebandNormFX nf;

    // First run
    nf.SetArgs(&vis, &out1);
    REQUIRE(nf.Initialize());
    REQUIRE(nf.Execute());

    // Second run with same input
    nf.SetArgs(&vis, &out2);
    REQUIRE(nf.Initialize());
    REQUIRE(nf.Execute());

    // Compare a few bins
    for (std::size_t fr = 0; fr < out1.GetDimension(FREQ_AXIS); fr += 4) {
        REQUIRE_CLOSE_CPLX(out1(0, 0, 0, fr), out2(0, 0, 0, fr), 1e-9);
    }

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_output_dimensions())       return 1;
    if (test_dimension_consistency())   return 1;
    if (test_flat_spectrum_centered_peak()) return 1;
    if (test_normalization())           return 1;
    if (test_cross_check_ssb())        return 1;
    if (test_reuse_idempotency())       return 1;

    return 0;
}
