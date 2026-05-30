#include <cmath>
#include <complex>
#include <iostream>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_InterpolateSpectralLinePeak.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

static bool close(double a, double b, double atol)
{ return std::fabs(a - b) <= atol; }


/* Constants                                                           */


static const int    NCH   = 3;
static const int    NDR   = 5;
static const int    NFREQ = 5;

static const double REF_FREQ_MHZ = 8000.0;
static const double CHAN_SPACING = 8.0;    // MHz
static const double DR_BIN_W     = 1e-12;  // s/s
static const double FREQ_BIN_W   = CHAN_SPACING / NFREQ; // 1.6 MHz

static const int PEAK_CHAN   = 1;  // interior
static const int PEAK_DR_BIN = 2;  // centre (axis value 0)
static const int PEAK_FREQ_BIN = 2; // centre (axis value 0)


/* Build synthetic fixture                                             */


/* Build the amplitude surface for the peak channel.
   dr_amps[k] is the amplitude at DR index k (only k-1, k, k+1 around peak
   dr bin are read by the stencil, but we fill the whole array for clarity).
   freq_amps[j] similarly.
   The peak channel gets dr_amps * freq_amps * peak_vis_scale * exp(i*phi0).
   Non-peak channels are filled with tiny values so peak_chan is unambiguous.
   If total_sw has a value, the weights container is tagged with it. */
static void build_fixture(visibility_type& spec, weight_type& w,
                          delay_rate_axis_type& dr_ax,
                          const double* dr_amps,    // NDR elements
                          const double* freq_amps,  // NFREQ elements
                          double phi0,
                          double peak_vis_scale,
                          double* total_sw)  // output: actual W used
{
    /* --- visibility container --- */
    spec.Resize(1, NCH, NDR, NFREQ);

    auto& pol_ax  = std::get< POLPROD_AXIS >(spec);
    pol_ax(0) = "XX";

    auto& chan_ax = std::get< CHANNEL_AXIS >(spec);
    for (int c = 0; c < NCH; c++)
        chan_ax(c) = REF_FREQ_MHZ + c * CHAN_SPACING;

    /* FREQ axis: centered so bin NFREQ/2 has offset 0 */
    auto& freq_ax = std::get< FREQ_AXIS >(spec);
    for (int j = 0; j < NFREQ; j++)
        freq_ax(j) = (j - NFREQ / 2) * FREQ_BIN_W;

    /* DR axis: centered and uniform */
    dr_ax.Resize(NDR);
    for (int k = 0; k < NDR; k++)
        dr_ax(k) = (k - NDR / 2) * DR_BIN_W;

    /* Fill the complex amplitude surface */
    for (int c = 0; c < NCH; c++)
    {
        for (int k = 0; k < NDR; k++)
        {
            for (int j = 0; j < NFREQ; j++)
            {
                if (c == PEAK_CHAN)
                {
                    double A = dr_amps[k] * freq_amps[j] * peak_vis_scale;
                    spec(0, c, k, j) = std::polar(A, phi0);
                }
                else
                {
                    spec(0, c, k, j) = std::complex<double>(1e-15, 0.0);
                }
            }
        }
    }

    /* --- weights container --- */
    w.Resize(1, NCH, NDR, NFREQ);
    for (int c = 0; c < NCH; c++)
        for (int k = 0; k < NDR; k++)
            for (int j = 0; j < NFREQ; j++)
                w(0, c, k, j) = 1.0;
}

/* Configure and run the operator.  Returns 0 on success, 1 on failure. */
static int configure_and_run(MHO_InterpolateSpectralLinePeak& op,
        visibility_type& spec, weight_type& w,
        delay_rate_axis_type& dr_ax,
        int peak_chan, int peak_dr_bin, int peak_freq_bin,
        double ref_freq_mhz)
{
    op.SetSpecDRData(&spec);
    op.SetWeights(&w);
    op.SetDRAxis(&dr_ax);
    op.SetReferenceFrequency(ref_freq_mhz);
    op.SetMaxBins(peak_chan, peak_dr_bin, peak_freq_bin);
    if (!op.Initialize())
    {
        std::cerr << "FAIL: op.Initialize() @ " << __FILE__ << ":"
                  << __LINE__ << std::endl;
        return 1;
    }
    if (!op.Execute())
    {
        std::cerr << "FAIL: op.Execute() @ " << __FILE__ << ":"
                  << __LINE__ << std::endl;
        return 1;
    }
    return 0;
}


/* Test cases                                                          */


/* Case 0 - Initialize() guards */
static int test_initialize_guards()
{
    /* (a) SpecDRData unset */
    {
        MHO_InterpolateSpectralLinePeak op;
        REQUIRE(op.Initialize() == false);
    }

    /* (b) SpecDRData set, weights unset */
    {
        MHO_InterpolateSpectralLinePeak op;
        visibility_type spec; spec.Resize(1, NCH, NDR, NFREQ);
        op.SetSpecDRData(&spec);
        REQUIRE(op.Initialize() == false);
    }

    /* (c) DR axis size < 3 (default size 1) */
    {
        MHO_InterpolateSpectralLinePeak op;
        visibility_type spec; spec.Resize(1, NCH, NDR, NFREQ);
        weight_type w;      w.Resize(1, NCH, NDR, NFREQ);
        delay_rate_axis_type dr_ax; dr_ax.Resize(1);
        op.SetSpecDRData(&spec);
        op.SetWeights(&w);
        op.SetDRAxis(&dr_ax);
        REQUIRE(op.Initialize() == false);
    }

    /* (d) Missing "total_summed_weights" tag -> Initialize() true,
           fTotalSummedWeights falls back to 1.0.
           Verify via amplitude: amp == |peak_vis| / 1.0 = |peak_vis|. */
    {
        MHO_InterpolateSpectralLinePeak op;
        visibility_type spec;
        weight_type w;
        delay_rate_axis_type dr_ax;
        double dr_amps[NDR]   = {0, 1.0, 2.0, 1.0, 0};
        double freq_amps[NFREQ] = {0, 1.0, 2.0, 1.0, 0};
        double total_sw = 0;

        build_fixture(spec, w, dr_ax, dr_amps, freq_amps, 0.0, 0.5, &total_sw);
        /* no Insert of total_summed_weights */

        if (configure_and_run(op, spec, w, dr_ax,
                              PEAK_CHAN, PEAK_DR_BIN, PEAK_FREQ_BIN,
                              REF_FREQ_MHZ))
            return 1;

        /* |peak_vis| = dr_amps[2]*freq_amps[2]*0.5 = 2*2*0.5 = 2.0
           amplitude = |peak_vis| / 1.0 = 2.0 */
        double expected_amp = 2.0;
        REQUIRE(close(op.GetFringeAmplitude(), expected_amp, 1e-12));
    }

    /* (e) total_summed_weights <= 0 -> falls back to 1.0 */
    {
        MHO_InterpolateSpectralLinePeak op;
        visibility_type spec;
        weight_type w;
        delay_rate_axis_type dr_ax;
        double dr_amps[NDR]   = {0, 1.0, 2.0, 1.0, 0};
        double freq_amps[NFREQ] = {0, 1.0, 2.0, 1.0, 0};
        double total_sw = 0;

        build_fixture(spec, w, dr_ax, dr_amps, freq_amps, 0.0, 0.5, &total_sw);
        w.Insert("total_summed_weights", -5.0);

        if (configure_and_run(op, spec, w, dr_ax,
                              PEAK_CHAN, PEAK_DR_BIN, PEAK_FREQ_BIN,
                              REF_FREQ_MHZ))
            return 1;

        /* same as (d): fallback to 1.0 */
        double expected_amp = 2.0;
        REQUIRE(close(op.GetFringeAmplitude(), expected_amp, 1e-12));
    }

    /* Also test total_summed_weights == 0 */
    {
        MHO_InterpolateSpectralLinePeak op;
        visibility_type spec;
        weight_type w;
        delay_rate_axis_type dr_ax;
        double dr_amps[NDR]   = {0, 1.0, 2.0, 1.0, 0};
        double freq_amps[NFREQ] = {0, 1.0, 2.0, 1.0, 0};
        double total_sw = 0;

        build_fixture(spec, w, dr_ax, dr_amps, freq_amps, 0.0, 0.5, &total_sw);
        w.Insert("total_summed_weights", 0.0);

        if (configure_and_run(op, spec, w, dr_ax,
                              PEAK_CHAN, PEAK_DR_BIN, PEAK_FREQ_BIN,
                              REF_FREQ_MHZ))
            return 1;

        double expected_amp = 2.0;
        REQUIRE(close(op.GetFringeAmplitude(), expected_amp, 1e-12));
    }

    return 0;
}

/* Case 1 - Symmetric peak -> zero offset in both DR and FREQ */
static int test_symmetric_peak()
{
    MHO_InterpolateSpectralLinePeak op;
    visibility_type spec;
    weight_type w;
    delay_rate_axis_type dr_ax;
    double dr_amps[NDR]      = {0, 1.0, 2.0, 1.0, 0};
    double freq_amps[NFREQ]  = {0, 1.0, 2.0, 1.0, 0};
    double total_sw = 0;

    build_fixture(spec, w, dr_ax, dr_amps, freq_amps, 0.0, 1.0, &total_sw);
    w.Insert("total_summed_weights", 10.0);

    if (configure_and_run(op, spec, w, dr_ax,
                          PEAK_CHAN, PEAK_DR_BIN, PEAK_FREQ_BIN,
                          REF_FREQ_MHZ))
        return 1;

    /* fine delay rate == dr_ax(peak_dr_bin) exactly */
    double expected_dr = dr_ax(static_cast<std::size_t>(PEAK_DR_BIN));
    REQUIRE(close(op.GetDelayRate(), expected_dr, 1e-18));

    /* peak sky freq == chan_centre + freq_ax(peak_freq_bin)
       chan_centre = 8000 + 1*8 = 8008; freq_ax(2) = 0 */
    auto& chan_ax = std::get< CHANNEL_AXIS >(spec);
    auto& freq_ax = std::get< FREQ_AXIS >(spec);
    double expected_freq = chan_ax(static_cast<std::size_t>(PEAK_CHAN))
                         + freq_ax(static_cast<std::size_t>(PEAK_FREQ_BIN));
    REQUIRE(close(op.GetPeakSkyFrequencyMHz(), expected_freq, 1e-9));

    return 0;
}

/* Case 2 - Known asymmetric DR offset */
/* We want offset = +0.25.
   Solve: 0.5*(fm1 - fp1)/(fm1 - 2*f0 + fp1) = 0.25
   with f0=2.0, fm1=1.0:
     2*(1.0 - fp1) = 1.0 - 4.0 + fp1
     2 - 2*fp1 = -3 + fp1
     5 = 3*fp1  ->  fp1 = 5/3 */
static int test_asymmetric_dr_offset()
{
    MHO_InterpolateSpectralLinePeak op;
    visibility_type spec;
    weight_type w;
    delay_rate_axis_type dr_ax;
    double dr_amps[NDR]      = {0, 1.0, 2.0, 5.0/3.0, 0};
    double freq_amps[NFREQ]  = {0, 1.0, 2.0, 1.0, 0};
    double total_sw = 0;

    build_fixture(spec, w, dr_ax, dr_amps, freq_amps, 0.0, 1.0, &total_sw);
    w.Insert("total_summed_weights", 10.0);

    if (configure_and_run(op, spec, w, dr_ax,
                          PEAK_CHAN, PEAK_DR_BIN, PEAK_FREQ_BIN,
                          REF_FREQ_MHZ))
        return 1;

    double target_offset = 0.25;
    double expected_dr = dr_ax(static_cast<std::size_t>(PEAK_DR_BIN))
                       + target_offset * DR_BIN_W;
    REQUIRE(close(op.GetDelayRate(), expected_dr, 1e-15));

    return 0;
}

/* Case 3 - Known asymmetric FREQ offset -> peak sky frequency */
/* Same offset = +0.25 in the FREQ dimension (fp1 = 5/3, same derivation as Case 2) */
static int test_asymmetric_freq_offset()
{
    MHO_InterpolateSpectralLinePeak op;
    visibility_type spec;
    weight_type w;
    delay_rate_axis_type dr_ax;
    double dr_amps[NDR]      = {0, 1.0, 2.0, 1.0, 0};
    double freq_amps[NFREQ]  = {0, 1.0, 2.0, 5.0/3.0, 0};
    double total_sw = 0;

    build_fixture(spec, w, dr_ax, dr_amps, freq_amps, 0.0, 1.0, &total_sw);
    w.Insert("total_summed_weights", 10.0);

    if (configure_and_run(op, spec, w, dr_ax,
                          PEAK_CHAN, PEAK_DR_BIN, PEAK_FREQ_BIN,
                          REF_FREQ_MHZ))
        return 1;

    auto& chan_ax = std::get< CHANNEL_AXIS >(spec);
    auto& freq_ax = std::get< FREQ_AXIS >(spec);
    double target_offset = 0.25;
    double expected_freq = chan_ax(static_cast<std::size_t>(PEAK_CHAN))
                         + (freq_ax(static_cast<std::size_t>(PEAK_FREQ_BIN))
                            + target_offset * FREQ_BIN_W);
    REQUIRE(close(op.GetPeakSkyFrequencyMHz(), expected_freq, 1e-9));

    return 0;
}

/* Case 4 - Fringe phase, amplitude, fringe rate, phase delay */
static int test_fringe_quantities()
{
    MHO_InterpolateSpectralLinePeak op;
    visibility_type spec;
    weight_type w;
    delay_rate_axis_type dr_ax;
    double dr_amps[NDR]      = {0, 1.0, 2.0, 1.0, 0};
    double freq_amps[NFREQ]  = {0, 1.0, 2.0, 1.0, 0};
    double total_sw = 0;

    /* peak_vis_scale = 0.5 so |peak_vis| = 0.5*2*2 = 2.0 */
    const double phi0 = 0.6;
    const double W = 10.0;

    build_fixture(spec, w, dr_ax, dr_amps, freq_amps, phi0, 0.5, &total_sw);
    w.Insert("total_summed_weights", W);

    if (configure_and_run(op, spec, w, dr_ax,
                          PEAK_CHAN, PEAK_DR_BIN, PEAK_FREQ_BIN,
                          REF_FREQ_MHZ))
        return 1;

    double peak_vis_mag = 2.0;  // 0.5 * 2 * 2

    /* Fringe phase */
    REQUIRE(close(op.GetFringePhase(), phi0, 1e-12));

    /* Fringe amplitude = |peak_vis| / W */
    REQUIRE(close(op.GetFringeAmplitude(), peak_vis_mag / W, 1e-12));

    /* Delay rate: symmetric so offset 0 */
    double expected_dr = dr_ax(static_cast<std::size_t>(PEAK_DR_BIN));
    REQUIRE(close(op.GetDelayRate(), expected_dr, 1e-18));

    /* Fringe rate = fine_dr * ref_freq_MHz * 1e6 */
    double expected_frate = expected_dr * REF_FREQ_MHZ * 1e6;
    REQUIRE(close(op.GetFringeRate(), expected_frate,
                  std::fabs(expected_frate) * 1e-12));

    /* Phase delay = phi0 / (2*pi*nu_peak_Hz) */
    double peak_sky_freq_hz = op.GetPeakSkyFrequencyMHz() * 1e6;
    double expected_pd = phi0 / (2.0 * M_PI * peak_sky_freq_hz);
    REQUIRE(close(op.GetPhaseDelay(), expected_pd,
                  std::fabs(expected_pd) * 1e-12));

    return 0;
}

/* Case 5 - Clamping of parabolic offset to +/-0.5 */
/* We need raw offset > 0.5.
   With fm1=0.0, f0=0.5, fp1=0.4:
   denom = 0.0 - 1.0 + 0.4 = -0.6
   offset = 0.5*(0.0 - 0.4)/(-0.6) = 0.5*0.6667 = 0.3333 -- not > 0.5

   Need a more extreme triple.
   Let's use fm1=0.1, f0=1.0, fp1=0.001:
   denom = 0.1 - 2.0 + 0.001 = -1.899
   offset = 0.5*(0.1 - 0.001)/(-1.899) = 0.5*0.099/(-1.899) = -0.026 -- wrong sign

   We need fm1 < fp1 for positive offset and a small (negative) denominator.
   Let's try fm1=0.0, f0=2.0, fp1=3.9:
   denom = 0.0 - 4.0 + 3.9 = -0.1
   offset = 0.5*(0.0 - 3.9)/(-0.1) = 0.5*39 = 19.5 -> clamps to 0.5

   But fp1 > f0 means the peak is shifted right by more than a bin.
   Let's use: fm1=0.0, f0=1.0, fp1=1.9:
   denom = 0.0 - 2.0 + 1.9 = -0.1
   offset = 0.5*(0.0 - 1.9)/(-0.1) = 0.5*19 = 9.5 -> clamps to 0.5 */
static int test_clamp_offset()
{
    MHO_InterpolateSpectralLinePeak op;
    visibility_type spec;
    weight_type w;
    delay_rate_axis_type dr_ax;
    double dr_amps[NDR]      = {0, 0.0, 1.0, 1.9, 0};
    double freq_amps[NFREQ]  = {0, 1.0, 2.0, 1.0, 0};
    double total_sw = 0;

    build_fixture(spec, w, dr_ax, dr_amps, freq_amps, 0.0, 1.0, &total_sw);
    w.Insert("total_summed_weights", 10.0);

    if (configure_and_run(op, spec, w, dr_ax,
                          PEAK_CHAN, PEAK_DR_BIN, PEAK_FREQ_BIN,
                          REF_FREQ_MHZ))
        return 1;

    /* clamped offset = +0.5 */
    double expected_dr = dr_ax(static_cast<std::size_t>(PEAK_DR_BIN))
                       + 0.5 * DR_BIN_W;
    REQUIRE(close(op.GetDelayRate(), expected_dr, 1e-15));

    return 0;
}

/* Case 6 - Flat region -> zero offset (denominator guard) */
static int test_flat_region()
{
    MHO_InterpolateSpectralLinePeak op;
    visibility_type spec;
    weight_type w;
    delay_rate_axis_type dr_ax;
    double dr_amps[NDR]      = {1.0, 1.0, 1.0, 1.0, 1.0};
    double freq_amps[NFREQ]  = {1.0, 1.0, 1.0, 1.0, 1.0};
    double total_sw = 0;

    build_fixture(spec, w, dr_ax, dr_amps, freq_amps, 0.0, 1.0, &total_sw);
    w.Insert("total_summed_weights", 10.0);

    if (configure_and_run(op, spec, w, dr_ax,
                          PEAK_CHAN, PEAK_DR_BIN, PEAK_FREQ_BIN,
                          REF_FREQ_MHZ))
        return 1;

    double expected_dr = dr_ax(static_cast<std::size_t>(PEAK_DR_BIN));
    REQUIRE(close(op.GetDelayRate(), expected_dr, 1e-18));

    REQUIRE(std::isfinite(op.GetPeakSkyFrequencyMHz()));

    return 0;
}

/* Case 7 - Idempotency */
static int test_idempotency()
{
    MHO_InterpolateSpectralLinePeak op;
    visibility_type spec;
    weight_type w;
    delay_rate_axis_type dr_ax;
    double dr_amps[NDR]      = {0, 1.0, 2.0, 1.0, 0};
    double freq_amps[NFREQ]  = {0, 1.0, 2.0, 1.0, 0};
    double total_sw = 0;

    const double phi0 = 0.6;
    const double W = 10.0;

    build_fixture(spec, w, dr_ax, dr_amps, freq_amps, phi0, 0.5, &total_sw);
    w.Insert("total_summed_weights", W);

    if (configure_and_run(op, spec, w, dr_ax,
                          PEAK_CHAN, PEAK_DR_BIN, PEAK_FREQ_BIN,
                          REF_FREQ_MHZ))
        return 1;

    double dr1    = op.GetDelayRate();
    double fr1    = op.GetFringeRate();
    double amp1   = op.GetFringeAmplitude();
    double phase1 = op.GetFringePhase();
    double pd1    = op.GetPhaseDelay();
    double freq1  = op.GetPeakSkyFrequencyMHz();

    /* Second execute */
    REQUIRE(op.Execute());

    REQUIRE(op.GetDelayRate()       == dr1);
    REQUIRE(op.GetFringeRate()      == fr1);
    REQUIRE(op.GetFringeAmplitude() == amp1);
    REQUIRE(op.GetFringePhase()     == phase1);
    REQUIRE(op.GetPhaseDelay()      == pd1);
    REQUIRE(op.GetPeakSkyFrequencyMHz() == freq1);

    return 0;
}

/* Case 8 - Index wrap at edges does not crash */
static int test_edge_wrap()
{
    MHO_InterpolateSpectralLinePeak op;
    visibility_type spec;
    weight_type w;
    delay_rate_axis_type dr_ax;
    double dr_amps[NDR]      = {0, 1.0, 2.0, 1.0, 0};
    double freq_amps[NFREQ]  = {0, 1.0, 2.0, 1.0, 0};
    double total_sw = 0;

    build_fixture(spec, w, dr_ax, dr_amps, freq_amps, 0.0, 1.0, &total_sw);
    w.Insert("total_summed_weights", 10.0);

    /* peak at bin 0 for both DR and FREQ */
    if (configure_and_run(op, spec, w, dr_ax,
                          PEAK_CHAN, 0, 0,
                          REF_FREQ_MHZ))
        return 1;

    REQUIRE(std::isfinite(op.GetDelayRate()));
    REQUIRE(std::isfinite(op.GetFringeRate()));
    REQUIRE(std::isfinite(op.GetFringeAmplitude()));
    REQUIRE(std::isfinite(op.GetFringePhase()));
    REQUIRE(std::isfinite(op.GetPhaseDelay()));
    REQUIRE(std::isfinite(op.GetPeakSkyFrequencyMHz()));

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_initialize_guards())   return 1;
    if (test_symmetric_peak())      return 1;
    if (test_asymmetric_dr_offset()) return 1;
    if (test_asymmetric_freq_offset()) return 1;
    if (test_fringe_quantities())   return 1;
    if (test_clamp_offset())        return 1;
    if (test_flat_region())         return 1;
    if (test_idempotency())         return 1;
    if (test_edge_wrap())           return 1;

    return 0;
}
