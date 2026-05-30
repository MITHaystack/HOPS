#include <cmath>
#include <complex>
#include <iostream>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_FringeRotation.hh"
#include "MHO_InterpolateFringePeak.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

static bool close(double a, double b, double atol)
{ return std::fabs(a - b) <= atol; }


/* Constants                                                           */


static const int    NCH   = 4;
static const int    NAP   = 8;
static const int    NSBD  = 8;
static const int    NMBD  = 5;
static const int    NDR   = 5;

static const double REF_FREQ    = 8000.0;   // MHz
static const double CHAN_SPACING = 8.0;     // MHz
static const double AP_DELTA    = 1.0;      // seconds
static const double SBD_DELTA   = 1.0 / (NSBD * CHAN_SPACING); // ~0.015625 s
static const double MBD_DELTA   = 1e-6;     // 1 us
static const double DR_DELTA    = 1e-6;     // 1 us/s
static const double FRT_OFFSET  = (NAP * AP_DELTA) / 2.0; // 4.0 s

static const int SBD_MAX = 3;   // interior SBD bin
static const int MBD_MAX = 2;   // centre MBD bin (value 0)
static const int DR_MAX  = 2;   // centre DR  bin (value 0)


/* Build synthetic fixture                                             */


/* Inject a fringe signal into sbd at the given coarse (sbd,mbd,dr).
   amp0 is the raw visibility amplitude (same for every AP/channel). */
static void inject_fringe(visibility_type& sbd, int sbd_bin,
                          double dr0, double mbd0, double amp0,
                          double phase0,
                          const channel_axis_type& chan_ax,
                          const time_axis_type& ap_ax,
                          double ref_freq, double frt_offset)
{
    MHO_FringeRotation rot;
    rot.SetOptimizeClosureFalse();

    std::size_t nchan = chan_ax.GetSize();
    std::size_t nap   = ap_ax.GetSize();
    double ap_delta   = ap_ax.at(1) - ap_ax.at(0);

    for (std::size_t fr = 0; fr < nchan; fr++)
    {
        double freq = chan_ax.at(fr);
        for (std::size_t ap = 0; ap < nap; ap++)
        {
            double tdelta = ap_ax.at(ap) + ap_delta / 2.0 - frt_offset;
            std::complex<double> vr = rot.vrot(tdelta, freq, ref_freq, dr0, mbd0);
            sbd(0, fr, ap, sbd_bin) = amp0 * std::conj(vr)
                                     * std::complex<double>(std::cos(phase0),
                                                            std::sin(phase0));
        }
    }
}

/* Build a complete fixture: visibility + weights + axes + max bins.
   dr0, mbd0 are the *true* injected values (0 for centred peak).
   phase0 and amp0 control the injected signal. */
static void build_fixture(visibility_type& sbd, weight_type& w,
                          time_axis_type& mbd_ax,
                          delay_rate_axis_type& dr_ax,
                          int& sbd_max, int& mbd_max, int& dr_max,
                          double& ref_freq, double& frt_offset,
                          double dr0, double mbd0, double phase0,
                          double amp0, double total_sw)
{
    /* --- visibility container --- */
    sbd.Resize(1, NCH, NAP, NSBD);

    auto& chan_ax = std::get< CHANNEL_AXIS >(sbd);
    for (int fr = 0; fr < NCH; fr++)
        chan_ax.at(fr) = REF_FREQ + fr * CHAN_SPACING;

    auto& ap_ax   = std::get< TIME_AXIS >(sbd);
    for (int ap = 0; ap < NAP; ap++)
        ap_ax.at(ap) = ap * AP_DELTA;

    auto& sbd_ax  = std::get< FREQ_AXIS >(sbd);
    for (int s = 0; s < NSBD; s++)
        sbd_ax.at(s) = (s - NSBD / 2) * SBD_DELTA;

    /* POLPROD axis (required by Resize but unused beyond index 0) */
    auto& pol_ax  = std::get< POLPROD_AXIS >(sbd);
    pol_ax.at(0)  = "XX";

    /* Inject fringe (zeroes all other bins first via Resize) */
    inject_fringe(sbd, SBD_MAX, dr0, mbd0, amp0, phase0,
                  chan_ax, ap_ax, REF_FREQ, FRT_OFFSET);

    /* --- weights container --- */
    w.Resize(1, NCH, NAP, NSBD);
    for (int fr = 0; fr < NCH; fr++)
        for (int ap = 0; ap < NAP; ap++)
            w(0, fr, ap, 0) = 1.0;

    w.Insert("total_summed_weights", total_sw);

    /* --- MBD axis --- */
    mbd_ax.Resize(NMBD);
    for (int i = 0; i < NMBD; i++)
        mbd_ax.at(i) = (i - NMBD / 2) * MBD_DELTA;

    /* --- DR axis --- */
    dr_ax.Resize(NDR);
    for (int i = 0; i < NDR; i++)
        dr_ax.at(i) = (i - NDR / 2) * DR_DELTA;

    /* --- output scalars --- */
    sbd_max    = SBD_MAX;
    mbd_max    = MBD_MAX;
    dr_max     = DR_MAX;
    ref_freq   = REF_FREQ;
    frt_offset = FRT_OFFSET;
}

/* Helper: construct and configure an operator, then Execute().
   Returns 0 on success, 1 on failure. */
static int configure_and_run(MHO_InterpolateFringePeak& op,
        visibility_type& sbd, weight_type& w,
        time_axis_type& mbd_ax, delay_rate_axis_type& dr_ax,
        int sbd_max, int mbd_max, int dr_max,
        double ref_freq, double frt_offset)
{
    op.SetSBDArray(&sbd);
    op.SetWeights(&w);
    op.SetMBDAxis(&mbd_ax);
    op.SetDRAxis(&dr_ax);
    op.SetReferenceFrequency(ref_freq);
    op.SetReferenceTimeOffset(frt_offset);
    op.SetMaxBins(sbd_max, mbd_max, dr_max);
    op.DisableOptimizeClosure();
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
    MHO_InterpolateFringePeak op;
    visibility_type sbd; sbd.Resize(1, NCH, NAP, NSBD);
    weight_type w;       w.Resize(1, NCH, NAP, NSBD);

    /* (a) No SBD array */
    op.SetSBDArray(nullptr);
    REQUIRE(op.Initialize() == false);

    /* (b) SBD set, no weights */
    op.SetSBDArray(&sbd);
    op.SetWeights(nullptr);
    REQUIRE(op.Initialize() == false);

    /* (c) SBD+weights set, MBD axis size == 1 (default) */
    for (int fr = 0; fr < NCH; fr++)
        for (int ap = 0; ap < NAP; ap++)
            w(0, fr, ap, 0) = 1.0;
    w.Insert("total_summed_weights", (double)(NCH * NAP));
    op.SetSBDArray(&sbd);
    op.SetWeights(&w);
    /* fMBDAxis default size is 1 */
    REQUIRE(op.Initialize() == false);

    /* (d) MBD ok, DR axis size == 1 */
    time_axis_type mbd_ax; mbd_ax.Resize(NMBD);
    op.SetMBDAxis(&mbd_ax);
    /* fDRAxis default size is 1 */
    REQUIRE(op.Initialize() == false);

    /* (e) Axes ok, but "total_summed_weights" tag missing */
    delay_rate_axis_type dr_ax; dr_ax.Resize(NDR);
    op.SetDRAxis(&dr_ax);
    weight_type w_no_tag; w_no_tag.Resize(1, NCH, NAP, NSBD);
    for (int fr = 0; fr < NCH; fr++)
        for (int ap = 0; ap < NAP; ap++)
            w_no_tag(0, fr, ap, 0) = 1.0;
    /* no Insert of total_summed_weights */
    op.SetWeights(&w_no_tag);
    REQUIRE(op.Initialize() == false);

    return 0;
}

/* Case 1 - Centred peak, zero rate / mbd */
static int test_centered_peak()
{
    visibility_type sbd;
    weight_type w;
    time_axis_type mbd_ax;
    delay_rate_axis_type dr_ax;
    int sbd_max, mbd_max, dr_max;
    double ref_freq, frt_offset;

    build_fixture(sbd, w, mbd_ax, dr_ax,
                  sbd_max, mbd_max, dr_max,
                  ref_freq, frt_offset,
                  0.0, 0.0, 0.0, 1.0, (double)(NCH * NAP));

    MHO_InterpolateFringePeak op;
    if (configure_and_run(op, sbd, w, mbd_ax, dr_ax,
                          sbd_max, mbd_max, dr_max,
                          ref_freq, frt_offset)) return 1;

    auto& sbd_ax = std::get< FREQ_AXIS >(sbd);

    double expected_sbd = sbd_ax.at(sbd_max);
    double expected_mbd = mbd_ax.at(mbd_max);
    double expected_dr  = dr_ax.at(dr_max);

    REQUIRE(close(op.GetSBDelay(),   expected_sbd,   0.5 * SBD_DELTA));
    REQUIRE(close(op.GetMBDelay(),   expected_mbd,   0.5 * MBD_DELTA));
    REQUIRE(close(op.GetDelayRate(), expected_dr,    0.5 * DR_DELTA));
    REQUIRE(close(op.GetFringeRate(),
                  expected_dr * ref_freq,
                  0.5 * DR_DELTA * ref_freq));
    REQUIRE(std::isfinite(op.GetFringeAmplitude()));
    REQUIRE(op.GetFringeAmplitude() > 0.0);

    return 0;
}

/* Case 2 - Off-bin injected peak (sub-bin recovery) */
static int test_offbin_peak()
{
    /* Axis value at the coarse max bin, then offset by fraction of delta */
    double mbd_base = (MBD_MAX - NMBD / 2) * MBD_DELTA;  /* = 0 */
    double dr_base  = (DR_MAX  - NDR  / 2) * DR_DELTA;   /* = 0 */
    double mbd0 = mbd_base + 0.3 * MBD_DELTA; /* 0.3 us */
    double dr0  = dr_base  + 0.2 * DR_DELTA;  /* 0.2 us/s */
    double amp0 = 1.0;

    visibility_type sbd;
    weight_type w;
    time_axis_type mbd_ax;
    delay_rate_axis_type dr_ax;
    int sbd_max, mbd_max, dr_max;
    double ref_freq, frt_offset;

    build_fixture(sbd, w, mbd_ax, dr_ax,
                  sbd_max, mbd_max, dr_max,
                  ref_freq, frt_offset,
                  dr0, mbd0, 0.0, amp0, (double)(NCH * NAP));

    MHO_InterpolateFringePeak op;
    if (configure_and_run(op, sbd, w, mbd_ax, dr_ax,
                          sbd_max, mbd_max, dr_max,
                          ref_freq, frt_offset)) return 1;

    auto& sbd_ax = std::get< FREQ_AXIS >(sbd);
    double expected_sbd = sbd_ax.at(sbd_max);

    REQUIRE(close(op.GetMBDelay(),   mbd0,   0.1 * MBD_DELTA));
    REQUIRE(close(op.GetDelayRate(), dr0,    0.1 * DR_DELTA));
    REQUIRE(close(op.GetSBDelay(),   expected_sbd, 0.1 * SBD_DELTA));

    /* amplitude: relative tolerance 5 % */
    double amp = op.GetFringeAmplitude();
    REQUIRE(std::isfinite(amp) && amp > 0.0);
    REQUIRE(std::fabs(amp - amp0) / amp0 < 0.05);

    return 0;
}

/* Case 3 - Amplitude scales with total_summed_weights */
static int test_amplitude_weight_scaling()
{
    /* First run: total_sw = NCH*NAP */
    visibility_type sbd1;
    weight_type w1;
    time_axis_type mbd_ax1;
    delay_rate_axis_type dr_ax1;
    int sbd_max, mbd_max, dr_max;
    double ref_freq, frt_offset;

    build_fixture(sbd1, w1, mbd_ax1, dr_ax1,
                  sbd_max, mbd_max, dr_max,
                  ref_freq, frt_offset,
                  0.0, 0.0, 0.0, 1.0, (double)(NCH * NAP));

    MHO_InterpolateFringePeak op1;
    if (configure_and_run(op1, sbd1, w1, mbd_ax1, dr_ax1,
                          sbd_max, mbd_max, dr_max,
                          ref_freq, frt_offset)) return 1;
    double amp1 = op1.GetFringeAmplitude();

    /* Second run: total_sw doubled */
    visibility_type sbd2;
    weight_type w2;
    time_axis_type mbd_ax2;
    delay_rate_axis_type dr_ax2;

    build_fixture(sbd2, w2, mbd_ax2, dr_ax2,
                  sbd_max, mbd_max, dr_max,
                  ref_freq, frt_offset,
                  0.0, 0.0, 0.0, 1.0, 2.0 * (double)(NCH * NAP));

    MHO_InterpolateFringePeak op2;
    if (configure_and_run(op2, sbd2, w2, mbd_ax2, dr_ax2,
                          sbd_max, mbd_max, dr_max,
                          ref_freq, frt_offset)) return 1;
    double amp2 = op2.GetFringeAmplitude();

    REQUIRE(close(amp2, amp1 / 2.0, 1e-9));

    return 0;
}

/* Case 4 - Re-use / idempotency */
static int test_idempotency()
{
    visibility_type sbd;
    weight_type w;
    time_axis_type mbd_ax;
    delay_rate_axis_type dr_ax;
    int sbd_max, mbd_max, dr_max;
    double ref_freq, frt_offset;

    build_fixture(sbd, w, mbd_ax, dr_ax,
                  sbd_max, mbd_max, dr_max,
                  ref_freq, frt_offset,
                  0.0, 0.0, 0.0, 1.0, (double)(NCH * NAP));

    MHO_InterpolateFringePeak op;
    if (configure_and_run(op, sbd, w, mbd_ax, dr_ax,
                          sbd_max, mbd_max, dr_max,
                          ref_freq, frt_offset)) return 1;

    double sbd1  = op.GetSBDelay();
    double mbd1  = op.GetMBDelay();
    double dr1   = op.GetDelayRate();
    double fr1   = op.GetFringeRate();
    double amp1  = op.GetFringeAmplitude();

    /* Second execute */
    REQUIRE(op.Execute());

    REQUIRE(op.GetSBDelay()   == sbd1);
    REQUIRE(op.GetMBDelay()   == mbd1);
    REQUIRE(op.GetDelayRate() == dr1);
    REQUIRE(op.GetFringeRate() == fr1);
    REQUIRE(op.GetFringeAmplitude() == amp1);

    return 0;
}

/* Case 5 - Edge-bin modulo wrap (sbd_max = 0) */
static int test_edgebin_wrap()
{
    visibility_type sbd;
    weight_type w;
    time_axis_type mbd_ax;
    delay_rate_axis_type dr_ax;
    int sbd_max, mbd_max, dr_max;
    double ref_freq, frt_offset;

    build_fixture(sbd, w, mbd_ax, dr_ax,
                  sbd_max, mbd_max, dr_max,
                  ref_freq, frt_offset,
                  0.0, 0.0, 0.0, 1.0, (double)(NCH * NAP));

    int edge_sbd_max = 0;

    MHO_InterpolateFringePeak op;
    if (configure_and_run(op, sbd, w, mbd_ax, dr_ax,
                          edge_sbd_max, mbd_max, dr_max,
                          ref_freq, frt_offset)) return 1;

    REQUIRE(std::isfinite(op.GetSBDelay()));
    REQUIRE(std::isfinite(op.GetMBDelay()));
    REQUIRE(std::isfinite(op.GetDelayRate()));
    REQUIRE(std::isfinite(op.GetFringeRate()));
    REQUIRE(std::isfinite(op.GetFringeAmplitude()));

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_initialize_guards())       return 1;
    if (test_centered_peak())           return 1;
    if (test_offbin_peak())             return 1;
    if (test_amplitude_weight_scaling()) return 1;
    if (test_idempotency())             return 1;
    if (test_edgebin_wrap())            return 1;

    return 0;
}
