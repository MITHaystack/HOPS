#include <cmath>
#include <iostream>
#include <string>
#include <complex>
#include <vector>

#include "MHO_SingleSidebandNormFX.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Helpers

/* Build a flat-spectrum fixture: npol=1, nchan=1, nap=1, nfreq=8.
   Channel 0: net_sideband="U" (USB), sky_freq=8000.0, bandwidth=32.0.
   All visibilities = (1.0, 0.0). */
static void build_flat_usb_fixture(visibility_type& vis)
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

// Build a flat-spectrum LSB fixture: same as above but net_sideband="L".
static void build_flat_lsb_fixture(visibility_type& vis)
{
    vis.Resize(1, 1, 1, 8);

    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    chan_ax.at(0) = 8000.0;
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("L"));
    chan_ax.InsertIndexLabelKeyValue(0, "sky_freq", 8000.0);
    chan_ax.InsertIndexLabelKeyValue(0, "bandwidth", 32.0);

    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t fr = 0; fr < vis.GetDimension(FREQ_AXIS); fr++)
                    vis(pp, ch, ap, fr) = std::complex<double>(1.0, 0.0);
}

/* Build a tone fixture (USB): npol=1, nchan=1, nap=1, nfreq=8.
   vis(0,0,0,fr) = exp(+i * 2*pi * k0 * fr / Nfreq). */
static void build_tone_usb_fixture(visibility_type& vis, int k0)
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

// Build a tone fixture (LSB): same as above but net_sideband="L".
static void build_tone_lsb_fixture(visibility_type& vis, int k0)
{
    vis.Resize(1, 1, 1, 8);

    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    chan_ax.at(0) = 8000.0;
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("L"));
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

// CASE 1 - Output dimensions / padding factor == 4
static int test_output_dimensions()
{
    visibility_type vis;
    build_flat_usb_fixture(vis);
    visibility_type out;

    MHO_SingleSidebandNormFX nf;
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

// CASE 2 - Flat spectrum (USB) -> single dominant peak at center bin 16
static int test_flat_spectrum_centered_peak()
{
    visibility_type vis;
    build_flat_usb_fixture(vis);
    visibility_type out;

    MHO_SingleSidebandNormFX nf;
    nf.SetArgs(&vis, &out);
    REQUIRE(nf.Initialize());
    REQUIRE(nf.Execute());

    std::size_t peak = find_peak_bin(out);
    REQUIRE(peak == 16);  // Nout/2 = 32/2

    // Peak must be strict global maximum
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

// CASE 3 - Normalization: peak magnitude == 1.0 for flat unit input
static int test_normalization()
{
    visibility_type vis;
    build_flat_usb_fixture(vis);
    visibility_type out;

    MHO_SingleSidebandNormFX nf;
    nf.SetArgs(&vis, &out);
    REQUIRE(nf.Initialize());
    REQUIRE(nf.Execute());

    /* With all-ones length-8 input: DFT DC component is 8, scaled by 1/Nfreq
       (1/8) gives peak magnitude 1.0. Padding does not change DC value. */
    std::size_t peak     = find_peak_bin(out);
    double peak_mag      = std::abs(out(0, 0, 0, peak));
    double reference_mag = 1.0;
    CHECK_CLOSE(peak_mag, reference_mag, 1e-5);

    return 0;
}

// CASE 4 - LSB conjugation: out_LSB == conj(out_USB) element-wise
static int test_lsb_conjugation()
{
    // Build USB output
    visibility_type vis_usb;
    build_tone_usb_fixture(vis_usb, 1);
    visibility_type out_usb;

    MHO_SingleSidebandNormFX nf_usb;
    nf_usb.SetArgs(&vis_usb, &out_usb);
    REQUIRE(nf_usb.Initialize());
    REQUIRE(nf_usb.Execute());

    // Build LSB output
    visibility_type vis_lsb;
    build_tone_lsb_fixture(vis_lsb, 1);
    visibility_type out_lsb;

    MHO_SingleSidebandNormFX nf_lsb;
    nf_lsb.SetArgs(&vis_lsb, &out_lsb);
    REQUIRE(nf_lsb.Initialize());
    REQUIRE(nf_lsb.Execute());

    // Element-wise: out_LSB(f) == conj(out_USB(f))
    std::size_t nfreq = out_usb.GetDimension(FREQ_AXIS);
    for (std::size_t fr = 0; fr < nfreq; fr++) {
        REQUIRE_CLOSE_CPLX(out_lsb(0, 0, 0, fr), std::conj(out_usb(0, 0, 0, fr)), 1e-6);
    }

    return 0;
}

// CASE 5 - Re-use / idempotency: same operator, two runs, identical output
static int test_reuse_idempotency()
{
    visibility_type vis;
    build_flat_usb_fixture(vis);

    MHO_SingleSidebandNormFX nf;

    // First run
    visibility_type out1;
    nf.SetArgs(&vis, &out1);
    REQUIRE(nf.Initialize());
    REQUIRE(nf.Execute());

    // Second run with same input
    visibility_type out2;
    nf.SetArgs(&vis, &out2);
    REQUIRE(nf.Initialize());
    REQUIRE(nf.Execute());

    // Compare all bins
    std::size_t nfreq = out1.GetDimension(FREQ_AXIS);
    for (std::size_t fr = 0; fr < nfreq; fr++) {
        REQUIRE_CLOSE_CPLX(out1(0, 0, 0, fr), out2(0, 0, 0, fr), 1e-9);
    }

    return 0;
}

// CASE 6 - Pure-tone (k0=1, USB) peak location == bin 20
static int test_tone_peak_location()
{
    visibility_type vis;
    build_tone_usb_fixture(vis, 1);
    visibility_type out;

    MHO_SingleSidebandNormFX nf;
    nf.SetArgs(&vis, &out);
    REQUIRE(nf.Initialize());
    REQUIRE(nf.Execute());

    // Numpy reference: peak at bin 20 (offset +4 from center bin 16)
    std::size_t peak = find_peak_bin(out);
    REQUIRE(peak == 20);

    // Peak magnitude should also be ~1.0 (same normalization)
    double peak_mag = std::abs(out(0, 0, 0, peak));
    CHECK_CLOSE(peak_mag, 1.0, 1e-5);

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_output_dimensions())          return 1;
    if (test_flat_spectrum_centered_peak()) return 1;
    if (test_normalization())              return 1;
    if (test_lsb_conjugation())            return 1;
    if (test_reuse_idempotency())          return 1;
    if (test_tone_peak_location())         return 1;

    return 0;
}
