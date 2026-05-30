#include <cmath>
#include <complex>
#include <iostream>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_FringeRotation.hh"
#include "MHO_InterpolateFringePeak.hh"
#include "MHO_InterpolateFringePeakOptimized.hh"
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

static const double REF_FREQ     = 8000.0;   // MHz
static const double CHAN_SPACING = 8.0;      // MHz
static const double AP_DELTA     = 1.0;      // seconds
static const double SBD_DELTA    = 1.0 / (NSBD * CHAN_SPACING); // ~0.015625 s
static const double MBD_DELTA    = 1e-6;     // 1 us
static const double DR_DELTA     = 1e-6;     // 1 us/s
static const double FRT_OFFSET   = (NAP * AP_DELTA) / 2.0; // 4.0 s

static const int SBD_MAX = 3;   // interior SBD bin
static const int MBD_MAX = 2;   // centre MBD bin (value 0)
static const int DR_MAX  = 2;   // centre DR  bin (value 0)


/* Cross-check tolerances                                              */


static const double ATOL_SBD   = 1e-12;
static const double ATOL_MBD   = 1e-12;
static const double ATOL_DR    = 1e-12;
static const double ATOL_FR    = 1e-9;
static const double ATOL_AMP   = 1e-9;


/* Build synthetic fixture (identical to TestInterpolateFringePeak)    */


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

    auto& pol_ax  = std::get< POLPROD_AXIS >(sbd);
    pol_ax.at(0)  = "XX";

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


/* Helper: configure and run an operator                               */


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
    if (!op.Initialize()) return 1;
    if (!op.Execute())    return 1;
    return 0;
}

static int configure_and_run(MHO_InterpolateFringePeakOptimized& op,
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
    if (!op.Initialize()) return 1;
    if (!op.Execute())    return 1;
    return 0;
}


/* Cross-check helper: compare ref vs opt outputs                      */


static int cross_check(MHO_InterpolateFringePeak& ref,
                       MHO_InterpolateFringePeakOptimized& opt)
{
    REQUIRE(close(ref.GetSBDelay(),        opt.GetSBDelay(),        ATOL_SBD));
    REQUIRE(close(ref.GetMBDelay(),        opt.GetMBDelay(),        ATOL_MBD));
    REQUIRE(close(ref.GetDelayRate(),      opt.GetDelayRate(),      ATOL_DR));
    REQUIRE(close(ref.GetFringeRate(),     opt.GetFringeRate(),     ATOL_FR));
    REQUIRE(close(ref.GetFringeAmplitude(), opt.GetFringeAmplitude(), ATOL_AMP));
    return 0;
}


/* Test cases                                                          */


/* Case 1 - Elementwise equivalence (centered peak, dr0=0, mbd0=0) */
static int test_elementwise_centered_peak()
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

    MHO_InterpolateFringePeak ref_op;
    MHO_InterpolateFringePeakOptimized opt_op;

    if (configure_and_run(ref_op, sbd, w, mbd_ax, dr_ax,
                          sbd_max, mbd_max, dr_max,
                          ref_freq, frt_offset)) return 1;
    if (configure_and_run(opt_op, sbd, w, mbd_ax, dr_ax,
                          sbd_max, mbd_max, dr_max,
                          ref_freq, frt_offset)) return 1;

    return cross_check(ref_op, opt_op);
}

/* Case 2 - Elementwise equivalence, off-bin injected peak */
static int test_elementwise_offbin_peak()
{
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

    MHO_InterpolateFringePeak ref_op;
    MHO_InterpolateFringePeakOptimized opt_op;

    if (configure_and_run(ref_op, sbd, w, mbd_ax, dr_ax,
                          sbd_max, mbd_max, dr_max,
                          ref_freq, frt_offset)) return 1;
    if (configure_and_run(opt_op, sbd, w, mbd_ax, dr_ax,
                          sbd_max, mbd_max, dr_max,
                          ref_freq, frt_offset)) return 1;

    return cross_check(ref_op, opt_op);
}

/* Case 3 - Initialize() guard parity */
static int test_initialize_guards()
{
    MHO_InterpolateFringePeakOptimized op;
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

/* Case 4 - Idempotency */
static int test_idempotency()
{
    double mbd_base = (MBD_MAX - NMBD / 2) * MBD_DELTA;
    double dr_base  = (DR_MAX  - NDR  / 2) * DR_DELTA;
    double mbd0 = mbd_base + 0.3 * MBD_DELTA;
    double dr0  = dr_base  + 0.2 * DR_DELTA;

    visibility_type sbd;
    weight_type w;
    time_axis_type mbd_ax;
    delay_rate_axis_type dr_ax;
    int sbd_max, mbd_max, dr_max;
    double ref_freq, frt_offset;

    build_fixture(sbd, w, mbd_ax, dr_ax,
                  sbd_max, mbd_max, dr_max,
                  ref_freq, frt_offset,
                  dr0, mbd0, 0.0, 1.0, (double)(NCH * NAP));

    MHO_InterpolateFringePeakOptimized op;
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

/* Case 5 - Edge-bin modulo wrap (sbd_max = 0) - both finite + agree */
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

    /* Move the injected fringe from bin SBD_MAX(3) to bin 0 so the
       wrapped stencil (bins 6,7,0,1,2) actually sees the signal, giving a
       non-trivial peak for the ref-vs-opt cross-check.  (The all-zero drf
       cube that the un-moved fringe would produce is exercised separately
       in test_allzero_cube below.) */
    auto& chan_ax = std::get< CHANNEL_AXIS >(sbd);
    auto& ap_ax   = std::get< TIME_AXIS >(sbd);
    std::size_t nchan = chan_ax.GetSize();
    std::size_t nap   = ap_ax.GetSize();
    double ap_delta   = ap_ax.at(1) - ap_ax.at(0);
    MHO_FringeRotation rot;
    rot.SetOptimizeClosureFalse();
    for (std::size_t fr = 0; fr < nchan; fr++) {
        double freq = chan_ax.at(fr);
        for (std::size_t ap = 0; ap < nap; ap++) {
            /* clear old injection at SBD_MAX */
            sbd(0, fr, ap, SBD_MAX) = visibility_element_type(0.0, 0.0);
            double tdelta = ap_ax.at(ap) + ap_delta / 2.0 - FRT_OFFSET;
            std::complex<double> vr = rot.vrot(tdelta, freq, REF_FREQ, 0.0, 0.0);
            sbd(0, fr, ap, 0) = 1.0 * std::conj(vr);
        }
    }

    int edge_sbd_max = 0;

    MHO_InterpolateFringePeak ref_op;
    MHO_InterpolateFringePeakOptimized opt_op;

    if (configure_and_run(ref_op, sbd, w, mbd_ax, dr_ax,
                          edge_sbd_max, mbd_max, dr_max,
                          ref_freq, frt_offset)) return 1;
    if (configure_and_run(opt_op, sbd, w, mbd_ax, dr_ax,
                          edge_sbd_max, mbd_max, dr_max,
                          ref_freq, frt_offset)) return 1;

    /* Both must be finite */
    REQUIRE(std::isfinite(ref_op.GetSBDelay()));
    REQUIRE(std::isfinite(ref_op.GetMBDelay()));
    REQUIRE(std::isfinite(ref_op.GetDelayRate()));
    REQUIRE(std::isfinite(ref_op.GetFringeRate()));
    REQUIRE(std::isfinite(ref_op.GetFringeAmplitude()));

    REQUIRE(std::isfinite(opt_op.GetSBDelay()));
    REQUIRE(std::isfinite(opt_op.GetMBDelay()));
    REQUIRE(std::isfinite(opt_op.GetDelayRate()));
    REQUIRE(std::isfinite(opt_op.GetFringeRate()));
    REQUIRE(std::isfinite(opt_op.GetFringeAmplitude()));

    /* And they must agree */
    return cross_check(ref_op, opt_op);
}

/* Case 6 - All-zero drf cube - max555 must return a finite result
 *
 * Regression test for the xbest[3] fix in max555(): the fringe is injected at
 * bin SBD_MAX(3) but the search runs at edge bin 0, whose wrapped stencil
 * (bins 6,7,0,1,2) never includes bin 3.  The resulting drf cube is therefore
 * all zeros, so no interpolated value ever exceeds the initial bestval of 0.0.
 * Previously xbest was left uninitialized and the function returned garbage;
 * now xbest is zero-initialized so the coordinates default to the cube center
 * and the outputs are finite. */
static int test_allzero_cube()
{
    visibility_type sbd;
    weight_type w;
    time_axis_type mbd_ax;
    delay_rate_axis_type dr_ax;
    int sbd_max, mbd_max, dr_max;
    double ref_freq, frt_offset;

    /* fringe injected at SBD_MAX(3) by build_fixture */
    build_fixture(sbd, w, mbd_ax, dr_ax,
                  sbd_max, mbd_max, dr_max,
                  ref_freq, frt_offset,
                  0.0, 0.0, 0.0, 1.0, (double)(NCH * NAP));

    /* search at edge bin 0 -> wrapped stencil misses bin 3 -> all-zero cube */
    int edge_sbd_max = 0;

    MHO_InterpolateFringePeakOptimized opt_op;
    if (configure_and_run(opt_op, sbd, w, mbd_ax, dr_ax,
                          edge_sbd_max, mbd_max, dr_max,
                          ref_freq, frt_offset)) return 1;

    REQUIRE(std::isfinite(opt_op.GetSBDelay()));
    REQUIRE(std::isfinite(opt_op.GetMBDelay()));
    REQUIRE(std::isfinite(opt_op.GetDelayRate()));
    REQUIRE(std::isfinite(opt_op.GetFringeRate()));
    REQUIRE(std::isfinite(opt_op.GetFringeAmplitude()));

    /* an all-zero cube has no peak above zero */
    REQUIRE(close(opt_op.GetFringeAmplitude(), 0.0, 1e-9));

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_elementwise_centered_peak())  return 1;
    if (test_elementwise_offbin_peak())    return 1;
    if (test_initialize_guards())          return 1;
    if (test_idempotency())                return 1;
    if (test_edgebin_wrap())               return 1;
    if (test_allzero_cube())               return 1;

    return 0;
}
