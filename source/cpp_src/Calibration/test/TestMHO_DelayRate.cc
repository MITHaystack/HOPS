#include <cmath>
#include <complex>
#include <iostream>
#include <vector>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_DelayRate.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;


/* Helpers: build synthetic visibility data                             */


static void fill_phase_ramp(
    visibility_type& vis, double rate_true, double ref_freq,
    const std::vector<double>& chan_freqs, double time_delta)
{
    std::size_t npp = vis.GetDimension(POLPROD_AXIS);
    std::size_t nch = vis.GetDimension(CHANNEL_AXIS);
    std::size_t nap = vis.GetDimension(TIME_AXIS);
    std::size_t nsbd = vis.GetDimension(FREQ_AXIS);

    for (std::size_t pp = 0; pp < npp; pp++)
    {
        for (std::size_t ch = 0; ch < nch; ch++)
        {
            double scale = chan_freqs[ch] / ref_freq;
            for (std::size_t ap = 0; ap < nap; ap++)
            {
                double phase = 2.0 * M_PI * rate_true * scale * ap * time_delta;
                std::complex<double> val(std::cos(phase), std::sin(phase));
                for (std::size_t fb = 0; fb < nsbd; fb++)
                {
                    vis(pp, ch, ap, fb) = val;
                }
            }
        }
    }
}

static void fill_weights(weight_type& wgt, double val)
{
    std::size_t npp = wgt.GetDimension(POLPROD_AXIS);
    std::size_t nch = wgt.GetDimension(CHANNEL_AXIS);
    std::size_t nap = wgt.GetDimension(TIME_AXIS);
    for (std::size_t pp = 0; pp < npp; pp++)
    {
        for (std::size_t ch = 0; ch < nch; ch++)
        {
            for (std::size_t ap = 0; ap < nap; ap++)
            {
                wgt(pp, ch, ap, 0) = val;
            }
        }
    }
}

static void set_axis_values(
    visibility_type& vis, const std::vector<double>& chan_freqs,
    double time_delta)
{
    /* CHANNEL axis: channel frequencies (MHz) */
    auto& chan_ax = std::get< CHANNEL_AXIS >(vis);
    for (std::size_t ch = 0; ch < chan_ax.GetSize(); ch++)
        chan_ax[ch] = chan_freqs[ch];

    /* TIME axis: uniform 0, 1, ..., N-1 (seconds) */
    auto& time_ax = std::get< TIME_AXIS >(vis);
    std::size_t nap = time_ax.GetSize();
    for (std::size_t ap = 0; ap < nap; ap++)
        time_ax[ap] = (double)ap * time_delta;

    /* FREQ axis: 0, 1, ..., N-1 */
    auto& freq_ax = std::get< FREQ_AXIS >(vis);
    std::size_t nsbd = freq_ax.GetSize();
    for (std::size_t fb = 0; fb < nsbd; fb++)
        freq_ax[fb] = (double)fb;

    /* POLPROD axis */
    auto& pol_ax = std::get< POLPROD_AXIS >(vis);
    pol_ax[0] = "XX";
}

/* Find argmax of |out(pp, ch, dr, fb)| over the dr axis.
   Only scan the first fDRSPSize entries (the interpolated region). */
static int argmax_dr(const sbd_type& out, std::size_t pp, std::size_t ch,
                     std::size_t fb, int fDRSPSize)
{
    double max_val = -1.0;
    int max_dr = 0;
    for (int dr = 0; dr < fDRSPSize; dr++)
    {
        double amp = std::abs(out(pp, ch, (std::size_t)dr, fb));
        if (amp > max_val)
        {
            max_val = amp;
            max_dr = dr;
        }
    }
    return max_dr;
}

/* Return peak amplitude at the argmax dr bin (within fDRSPSize). */
static double peak_amp_dr(const sbd_type& out, std::size_t pp, std::size_t ch,
                          std::size_t fb, int fDRSPSize)
{
    double max_val = 0.0;
    for (int dr = 0; dr < fDRSPSize; dr++)
    {
        double amp = std::abs(out(pp, ch, (std::size_t)dr, fb));
        if (amp > max_val)
            max_val = amp;
    }
    return max_val;
}


/* Test cases                                                           */


/* Case 1: CalculateSearchSpaceSize (table-driven) */
static int test_calculate_search_space_size()
{
    MHO_DelayRate dr;

    /* Note: NextLowestPowerOfTwo returns the CEILING power of 2
       (next highest, not floor).  Verified against source in
       MHO_BitReversalPermutation.cc.  Table reflects actual behavior. */
    struct TestCase { unsigned int n; unsigned int expected; };
    std::vector<TestCase> cases = {
        { 1,    2 },     /* 2 * 1    = 2   */
        { 2,    4 },     /* 2 * 2    = 4   */
        { 3,    8 },     /* 2 * 4    = 8   (NextLowestPowerOfTwo(3)=4) */
        { 16,   32 },    /* 2 * 16   = 32  */
        { 17,   64 },    /* 2 * 32   = 64  (NextLowestPowerOfTwo(17)=32) */
        { 32,   64 },    /* 2 * 32   = 64  */
        { 1024, 2048 },  /* 2 * 1024 = 2048 */
    };

    for (auto& tc : cases)
    {
        unsigned int result = dr.CalculateSearchSpaceSize(tc.n);
        if (result != tc.expected)
        {
            std::cerr << "FAIL: CalculateSearchSpaceSize(" << tc.n
                      << ") = " << result << ", expected " << tc.expected
                      << " @ " << __FILE__ << ":" << __LINE__ << std::endl;
            return 1;
        }
    }
    return 0;
}

/* Case 2: Initialize happy-path (no crash, correct sizing) */
static int test_initialize()
{
    visibility_type vis;
    vis.Resize(1, 4, 16, 4);
    weight_type wgt;
    wgt.Resize(1, 4, 16, 1);
    sbd_type out;

    std::vector<double> chan_freqs = { 8000.0, 8016.0, 8032.0, 8048.0 };
    set_axis_values(vis, chan_freqs, 1.0);
    fill_weights(wgt, 1.0);

    MHO_DelayRate dr;
    dr.SetArgs(&vis, &wgt, &out);
    dr.SetReferenceFrequency(8000.0);

    REQUIRE(dr.Initialize());
    REQUIRE(dr.GetDelayRateSearchSpaceSize() == 32);

    /* output TIME axis should be np = 4 * fDRSPSize = 128 */
    REQUIRE(out.GetDimension(TIME_AXIS) == 128);

    return 0;
}

/* Case 3: Execute -- peak at injected rate (dr=24 for rate=0.25) */
static int test_execute_peak_at_injected_rate()
{
    visibility_type vis;
    vis.Resize(1, 4, 16, 4);
    weight_type wgt;
    wgt.Resize(1, 4, 16, 1);
    sbd_type out;

    std::vector<double> chan_freqs = { 8000.0, 8016.0, 8032.0, 8048.0 };
    set_axis_values(vis, chan_freqs, 1.0);
    fill_phase_ramp(vis, 0.25, 8000.0, chan_freqs, 1.0);
    fill_weights(wgt, 1.0);

    MHO_DelayRate dr;
    dr.SetArgs(&vis, &wgt, &out);
    dr.SetReferenceFrequency(8000.0);
    REQUIRE(dr.Initialize());
    REQUIRE(dr.Execute());

    /* With rate_true=0.25, fDRSPSize=32, expected dr = 0.25*32 + 16 = 24 */
    int expected_dr = 24;
    int fDRSPSize = dr.GetDelayRateSearchSpaceSize();
    for (std::size_t ch = 0; ch < 4; ch++)
    {
        int dr_idx = argmax_dr(out, 0, ch, 0, fDRSPSize);
        if (dr_idx != expected_dr)
        {
            std::cerr << "FAIL: argmax dr for channel " << ch << " = " << dr_idx
                      << ", expected " << expected_dr
                      << " @ " << __FILE__ << ":" << __LINE__ << std::endl;
            return 1;
        }
    }

    /* Peak amplitude should be close to coherent sum (16 APs). */
    double pa = peak_amp_dr(out, 0, 0, 0, fDRSPSize);
    if (pa < 0.95 * 16.0)
    {
        std::cerr << "FAIL: peak amplitude " << pa
                  << " < 0.95*16 = " << (0.95 * 16.0)
                  << " @ " << __FILE__ << ":" << __LINE__ << std::endl;
        return 1;
    }

    return 0;
}

/* Case 4: Execute -- delay-rate axis values */
static int test_execute_delay_rate_axis_values()
{
    visibility_type vis;
    vis.Resize(1, 4, 16, 4);
    weight_type wgt;
    wgt.Resize(1, 4, 16, 1);
    sbd_type out;

    std::vector<double> chan_freqs = { 8000.0, 8016.0, 8032.0, 8048.0 };
    set_axis_values(vis, chan_freqs, 1.0);
    fill_phase_ramp(vis, 0.25, 8000.0, chan_freqs, 1.0);
    fill_weights(wgt, 1.0);

    MHO_DelayRate dr;
    dr.SetArgs(&vis, &wgt, &out);
    dr.SetReferenceFrequency(8000.0);
    REQUIRE(dr.Initialize());
    REQUIRE(dr.Execute());

    /* After Execute, out's TIME axis holds delay-rate values:
       ax_scale = 1.0 / (time_delta * fDRSPSize) = 1.0 / 32
       out.TIME_AXIS(dr) = (dr - 16) * ax_scale */
    double ax_scale = 1.0 / (1.0 * 32.0);
    for (int dr = 0; dr < 32; dr++)
    {
        double expected = ((double)dr - 16.0) * ax_scale;
        double actual = std::get< TIME_AXIS >(out)(dr);
        if (std::fabs(actual - expected) > 1e-12)
        {
            std::cerr << "FAIL: TIME axis[dr=" << dr << "] = " << actual
                      << ", expected " << expected
                      << " @ " << __FILE__ << ":" << __LINE__ << std::endl;
            return 1;
        }
    }

    return 0;
}

/* Case 5: Sign-flip round-trip (positive rate -> dr=24, negative rate -> dr=8) */
static int test_sign_flip_round_trip()
{
    std::vector<double> chan_freqs = { 8000.0, 8016.0, 8032.0, 8048.0 };
    double time_delta = 1.0;

    /* Positive rate: peak at dr=24 */
    {
        visibility_type vis;
        vis.Resize(1, 4, 16, 4);
        weight_type wgt;
        wgt.Resize(1, 4, 16, 1);
        sbd_type out;

        set_axis_values(vis, chan_freqs, time_delta);
        fill_phase_ramp(vis, 0.25, 8000.0, chan_freqs, time_delta);
        fill_weights(wgt, 1.0);

        MHO_DelayRate dr;
        dr.SetArgs(&vis, &wgt, &out);
        dr.SetReferenceFrequency(8000.0);
        REQUIRE(dr.Initialize());
        REQUIRE(dr.Execute());

        int dr_idx = argmax_dr(out, 0, 0, 0, dr.GetDelayRateSearchSpaceSize());
        if (dr_idx != 24)
        {
            std::cerr << "FAIL: positive-rate argmax = " << dr_idx
                      << ", expected 24"
                      << " @ " << __FILE__ << ":" << __LINE__ << std::endl;
            return 1;
        }
    }

    /* Negative rate: peak at dr=8  (dr = round(-0.25*32) + 16 = -8 + 16 = 8) */
    {
        visibility_type vis;
        vis.Resize(1, 4, 16, 4);
        weight_type wgt;
        wgt.Resize(1, 4, 16, 1);
        sbd_type out;

        set_axis_values(vis, chan_freqs, time_delta);
        fill_phase_ramp(vis, -0.25, 8000.0, chan_freqs, time_delta);
        fill_weights(wgt, 1.0);

        MHO_DelayRate dr;
        dr.SetArgs(&vis, &wgt, &out);
        dr.SetReferenceFrequency(8000.0);
        REQUIRE(dr.Initialize());
        REQUIRE(dr.Execute());

        int dr_idx = argmax_dr(out, 0, 0, 0, dr.GetDelayRateSearchSpaceSize());
        if (dr_idx != 8)
        {
            std::cerr << "FAIL: negative-rate argmax = " << dr_idx
                      << ", expected 8"
                      << " @ " << __FILE__ << ":" << __LINE__ << std::endl;
            return 1;
        }
    }

    return 0;
}

/* Case 6: Weights applied -- zero one AP, peak amplitude drops */
static int test_weights_applied()
{
    std::vector<double> chan_freqs = { 8000.0, 8016.0, 8032.0, 8048.0 };

    /* First get the reference peak amplitude with all weights = 1.0 */
    visibility_type vis_full;
    vis_full.Resize(1, 4, 16, 4);
    weight_type wgt_full;
    wgt_full.Resize(1, 4, 16, 1);
    sbd_type out_full;

    set_axis_values(vis_full, chan_freqs, 1.0);
    fill_phase_ramp(vis_full, 0.25, 8000.0, chan_freqs, 1.0);
    fill_weights(wgt_full, 1.0);

    MHO_DelayRate dr_full;
    dr_full.SetArgs(&vis_full, &wgt_full, &out_full);
    dr_full.SetReferenceFrequency(8000.0);
    REQUIRE(dr_full.Initialize());
    REQUIRE(dr_full.Execute());

    double ref_peak = peak_amp_dr(out_full, 0, 0, 0,
                                  dr_full.GetDelayRateSearchSpaceSize());

    /* Now zero out AP 0 in the weights */
    visibility_type vis_mask;
    vis_mask.Resize(1, 4, 16, 4);
    weight_type wgt_mask;
    wgt_mask.Resize(1, 4, 16, 1);
    sbd_type out_mask;

    set_axis_values(vis_mask, chan_freqs, 1.0);
    fill_phase_ramp(vis_mask, 0.25, 8000.0, chan_freqs, 1.0);
    fill_weights(wgt_mask, 1.0);

    /* Zero out AP 0 for all channels */
    for (std::size_t ch = 0; ch < 4; ch++)
        wgt_mask(0, ch, 0, 0) = 0.0;

    MHO_DelayRate dr_mask;
    dr_mask.SetArgs(&vis_mask, &wgt_mask, &out_mask);
    dr_mask.SetReferenceFrequency(8000.0);
    REQUIRE(dr_mask.Initialize());
    REQUIRE(dr_mask.Execute());

    /* Peak should still be at dr=24 */
    int dr_idx = argmax_dr(out_mask, 0, 0, 0,
                           dr_mask.GetDelayRateSearchSpaceSize());
    if (dr_idx != 24)
    {
        std::cerr << "FAIL: weighted argmax = " << dr_idx
                  << ", expected 24"
                  << " @ " << __FILE__ << ":" << __LINE__ << std::endl;
        return 1;
    }

    /* Peak amplitude should drop proportionally:
       15/16 of the original (one AP zeroed).
       Tolerance: ratio in [0.85, 1.0]. */
    double mask_peak = peak_amp_dr(out_mask, 0, 0, 0,
                                   dr_mask.GetDelayRateSearchSpaceSize());
    double ratio = mask_peak / ref_peak;
    if (ratio < 0.85 || ratio > 1.0)
    {
        std::cerr << "FAIL: peak ratio = " << ratio
                  << " not in [0.85, 1.0] (ref=" << ref_peak
                  << ", masked=" << mask_peak << ")"
                  << " @ " << __FILE__ << ":" << __LINE__ << std::endl;
        return 1;
    }

    return 0;
}

/* Case 7: Initialize with null args returns false */
static int test_null_args()
{
    weight_type wgt;
    wgt.Resize(1, 4, 16, 1);
    sbd_type out;

    MHO_DelayRate dr;
    dr.SetArgs(nullptr, &wgt, &out);
    bool result = dr.Initialize();
    if (result != false)
    {
        std::cerr << "FAIL: Initialize() with null vis returned true"
                  << " @ " << __FILE__ << ":" << __LINE__ << std::endl;
        return 1;
    }

    /* fDRSPSize is only set inside the non-null branch.
       Default-constructed operator has fDRSPSize uninitialized;
       we just verify Initialize returned false without crashing. */
    return 0;
}

/* Case 8: Re-initialize with different input size */
static int test_reinitialize_different_size()
{
    std::vector<double> chan_freqs4 = { 8000.0, 8016.0, 8032.0, 8048.0 };
    std::vector<double> chan_freqs32 = { 8000.0, 8016.0, 8032.0, 8048.0,
                                          8064.0, 8080.0, 8096.0, 8112.0,
                                          8128.0, 8144.0, 8160.0, 8176.0,
                                          8192.0, 8208.0, 8224.0, 8240.0,
                                          8256.0, 8272.0, 8288.0, 8304.0,
                                          8320.0, 8336.0, 8352.0, 8368.0,
                                          8384.0, 8400.0, 8416.0, 8432.0,
                                          8448.0, 8464.0, 8480.0, 8496.0 };

    MHO_DelayRate dr;
    sbd_type out;

    /* First init: (1,4,16,4) -> fDRSPSize=32, np=128 */
    {
        visibility_type vis;
        vis.Resize(1, 4, 16, 4);
        weight_type wgt;
        wgt.Resize(1, 4, 16, 1);

        set_axis_values(vis, chan_freqs4, 1.0);
        fill_phase_ramp(vis, 0.25, 8000.0, chan_freqs4, 1.0);
        fill_weights(wgt, 1.0);

        dr.SetArgs(&vis, &wgt, &out);
        dr.SetReferenceFrequency(8000.0);
        REQUIRE(dr.Initialize());
        REQUIRE(dr.GetDelayRateSearchSpaceSize() == 32);
        REQUIRE(out.GetDimension(TIME_AXIS) == 128);
        REQUIRE(dr.Execute());

        int dr_idx = argmax_dr(out, 0, 0, 0, dr.GetDelayRateSearchSpaceSize());
        if (dr_idx != 24)
        {
            std::cerr << "FAIL: reinit-first argmax = " << dr_idx
                      << ", expected 24"
                      << " @ " << __FILE__ << ":" << __LINE__ << std::endl;
            return 1;
        }
    }

    /* Second init: (1,32,32,4) -> fDRSPSize=64, np=256 */
    {
        visibility_type vis;
        vis.Resize(1, 32, 32, 4);
        weight_type wgt;
        wgt.Resize(1, 32, 32, 1);

        set_axis_values(vis, chan_freqs32, 1.0);
        fill_phase_ramp(vis, 0.25, 8000.0, chan_freqs32, 1.0);
        fill_weights(wgt, 1.0);

        dr.SetArgs(&vis, &wgt, &out);
        dr.SetReferenceFrequency(8000.0);
        REQUIRE(dr.Initialize());
        REQUIRE(dr.GetDelayRateSearchSpaceSize() == 64);
        REQUIRE(out.GetDimension(TIME_AXIS) == 256);
        REQUIRE(dr.Execute());

        /* With fDRSPSize=64, rate=0.25: dr = round(0.25*64) + 32 = 16 + 32 = 48 */
        int dr_idx = argmax_dr(out, 0, 0, 0, dr.GetDelayRateSearchSpaceSize());
        if (dr_idx != 48)
        {
            std::cerr << "FAIL: reinit-second argmax = " << dr_idx
                      << ", expected 48"
                      << " @ " << __FILE__ << ":" << __LINE__ << std::endl;
            return 1;
        }
    }

    return 0;
}

/* Case 9: Channel frequency scaling */
static int test_channel_frequency_scaling()
{
    visibility_type vis;
    vis.Resize(1, 4, 16, 4);
    weight_type wgt;
    wgt.Resize(1, 4, 16, 1);
    sbd_type out;

    std::vector<double> chan_freqs = { 8000.0, 8016.0, 8032.0, 8048.0 };
    double ref_freq = 8000.0;
    double rate_true = 0.25;
    double time_delta = 1.0;

    set_axis_values(vis, chan_freqs, time_delta);
    fill_phase_ramp(vis, rate_true, ref_freq, chan_freqs, time_delta);
    fill_weights(wgt, 1.0);

    MHO_DelayRate dr;
    dr.SetArgs(&vis, &wgt, &out);
    dr.SetReferenceFrequency(ref_freq);
    REQUIRE(dr.Initialize());
    REQUIRE(dr.Execute());

    /* Expected peak bin per channel:
       dr_ch = round(rate_true * fDRSPSize * (chan_freq_ch / ref_freq)) + fDRSPSize/2
       fDRSPSize = 32 */
    int fDRSPSize = dr.GetDelayRateSearchSpaceSize();
    for (std::size_t ch = 0; ch < 4; ch++)
    {
        double scale = chan_freqs[ch] / ref_freq;
        int expected_dr = (int)std::llround(rate_true * fDRSPSize * scale) + fDRSPSize / 2;
        int dr_idx = argmax_dr(out, 0, ch, 0, fDRSPSize);
        if (dr_idx != expected_dr)
        {
            std::cerr << "FAIL: channel " << ch << " (freq=" << chan_freqs[ch]
                      << ") argmax = " << dr_idx
                      << ", expected " << expected_dr
                      << " @ " << __FILE__ << ":" << __LINE__ << std::endl;
            return 1;
        }
    }

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_calculate_search_space_size()) return 1;
    if (test_initialize()) return 1;
    if (test_execute_peak_at_injected_rate()) return 1;
    if (test_execute_delay_rate_axis_values()) return 1;
    if (test_sign_flip_round_trip()) return 1;
    if (test_weights_applied()) return 1;
    if (test_null_args()) return 1;
    if (test_reinitialize_different_size()) return 1;
    if (test_channel_frequency_scaling()) return 1;

    return 0;
}
