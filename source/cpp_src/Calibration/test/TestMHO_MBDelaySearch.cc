#include <cmath>
#include <complex>
#include <iostream>
#include <vector>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_MBDelaySearch.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

static void set_axes(visibility_type& vis, const std::vector< double >& chan_freqs, double time_delta)
{
    auto& chan_ax = std::get< CHANNEL_AXIS >(vis);
    for(std::size_t ch = 0; ch < chan_ax.GetSize(); ch++)
    {
        chan_ax[ch] = chan_freqs[ch];
    }
    auto& time_ax = std::get< TIME_AXIS >(vis);
    for(std::size_t ap = 0; ap < time_ax.GetSize(); ap++)
    {
        time_ax[ap] = static_cast< double >(ap) * time_delta;
    }
    auto& freq_ax = std::get< FREQ_AXIS >(vis); // SBD-lag axis
    for(std::size_t fb = 0; fb < freq_ax.GetSize(); fb++)
    {
        freq_ax[fb] = static_cast< double >(fb);
    }
    std::get< POLPROD_AXIS >(vis)[0] = "XX";
}

static void fill_constant(visibility_type& vis, std::complex< double > v)
{
    std::size_t npp = vis.GetDimension(POLPROD_AXIS);
    std::size_t nch = vis.GetDimension(CHANNEL_AXIS);
    std::size_t nap = vis.GetDimension(TIME_AXIS);
    std::size_t nsbd = vis.GetDimension(FREQ_AXIS);
    for(std::size_t pp = 0; pp < npp; pp++)
        for(std::size_t ch = 0; ch < nch; ch++)
            for(std::size_t ap = 0; ap < nap; ap++)
                for(std::size_t fb = 0; fb < nsbd; fb++)
                    vis(pp, ch, ap, fb) = v;
}

// phase ramp in time -> pure delay-rate signal (zero MBD delay at ap=0)
static void fill_rate_ramp(visibility_type& vis, double rate_true, double ref_freq, const std::vector< double >& chan_freqs,
                           double time_delta)
{
    std::size_t npp = vis.GetDimension(POLPROD_AXIS);
    std::size_t nch = vis.GetDimension(CHANNEL_AXIS);
    std::size_t nap = vis.GetDimension(TIME_AXIS);
    std::size_t nsbd = vis.GetDimension(FREQ_AXIS);
    for(std::size_t pp = 0; pp < npp; pp++)
        for(std::size_t ch = 0; ch < nch; ch++)
        {
            double scale = chan_freqs[ch] / ref_freq;
            for(std::size_t ap = 0; ap < nap; ap++)
            {
                double phase = 2.0 * M_PI * rate_true * scale * ap * time_delta;
                std::complex< double > val(std::cos(phase), std::sin(phase));
                for(std::size_t fb = 0; fb < nsbd; fb++)
                    vis(pp, ch, ap, fb) = val;
            }
        }
}

static void fill_weights(weight_type& wgt, double val)
{
    std::size_t npp = wgt.GetDimension(POLPROD_AXIS);
    std::size_t nch = wgt.GetDimension(CHANNEL_AXIS);
    std::size_t nap = wgt.GetDimension(TIME_AXIS);
    for(std::size_t pp = 0; pp < npp; pp++)
        for(std::size_t ch = 0; ch < nch; ch++)
            for(std::size_t ap = 0; ap < nap; ap++)
                wgt(pp, ch, ap, 0) = val;
}

// index of the axis bin whose value is closest to zero
template< typename AxisT > static int argmin_abs(AxisT* ax)
{
    int best = 0;
    double best_val = 1e300;
    for(std::size_t i = 0; i < ax->GetSize(); i++)
    {
        double a = std::fabs((*ax)(i));
        if(a < best_val)
        {
            best_val = a;
            best = static_cast< int >(i);
        }
    }
    return best;
}

static const std::vector< double > CHAN_FREQS = {8000.0, 8016.0, 8032.0, 8048.0, 8064.0, 8080.0, 8096.0, 8112.0};
static const double REF_FREQ = 8000.0;

// Case 1: Initialize happy path + sizing getters
static int test_initialize()
{
    visibility_type vis;
    vis.Resize(1, 8, 16, 4);
    weight_type wgt;
    wgt.Resize(1, 8, 16, 1);
    set_axes(vis, CHAN_FREQS, 1.0);
    fill_constant(vis, std::complex< double >(1.0, 0.0));
    fill_weights(wgt, 1.0);

    MHO_MBDelaySearch search;
    search.SetArgs(&vis);
    search.SetWeights(&wgt);
    search.SetReferenceFrequency(REF_FREQ);

    REQUIRE(search.Initialize());
    REQUIRE(search.GetNSBDBins() == 4);                  // FREQ_AXIS size
    REQUIRE(search.GetNDRBins() == 16);                  // TIME_AXIS size
    REQUIRE(search.GetNMBDBins() >= 8);                  // grid must cover all channels
    CHECK_CLOSE(search.GetFrequencySpacing(), 16.0, 1e-6);
    REQUIRE(search.GetAverageFrequency() >= 8000.0);
    REQUIRE(search.GetAverageFrequency() <= 8112.0);
    return 0;
}

// Case 2: constant (zero-delay, zero-rate) signal -> peak at zero-closest MBD/DR bin, SBD bin 0
static int test_constant_peak_at_zero()
{
    visibility_type vis;
    vis.Resize(1, 8, 16, 4);
    weight_type wgt;
    wgt.Resize(1, 8, 16, 1);
    set_axes(vis, CHAN_FREQS, 1.0);
    fill_constant(vis, std::complex< double >(1.0, 0.0));
    fill_weights(wgt, 1.0);

    MHO_MBDelaySearch search;
    search.SetArgs(&vis);
    search.SetWeights(&wgt);
    search.SetReferenceFrequency(REF_FREQ);
    REQUIRE(search.Initialize());
    REQUIRE(search.Execute());

    // SBD: all lag-slices identical -> first one (bin 0) wins, axis value 0
    REQUIRE(search.GetSBDMaxBin() == 0);
    CHECK_CLOSE(search.GetCoarseSBD(), 0.0, 1e-12);

    // MBD/DR: peak must land on the zero-closest bin of each axis
    int exp_mbd = argmin_abs(search.GetMBDAxis());
    int exp_dr = argmin_abs(search.GetDRAxis());
    REQUIRE(search.GetMBDMaxBin() == exp_mbd);
    REQUIRE(search.GetDRMaxBin() == exp_dr);

    // coarse values are consistent with the reported max bins
    CHECK_CLOSE(search.GetCoarseMBD(), (*search.GetMBDAxis())(search.GetMBDMaxBin()), 1e-12);
    CHECK_CLOSE(search.GetCoarseDR(), (*search.GetDRAxis())(search.GetDRMaxBin()), 1e-12);

    REQUIRE(search.GetSearchMaximumAmplitude() > 0.0);
    REQUIRE(search.GetNPointsSearched() > 0.0);
    return 0;
}

// Case 3: injected delay-rate ramp -> coarse DR takes the sign of the injected rate
static int test_rate_sign()
{
    for(int sgn = -1; sgn <= 1; sgn += 2)
    {
        visibility_type vis;
        vis.Resize(1, 8, 16, 4);
        weight_type wgt;
        wgt.Resize(1, 8, 16, 1);
        set_axes(vis, CHAN_FREQS, 1.0);
        fill_rate_ramp(vis, sgn * 0.25, REF_FREQ, CHAN_FREQS, 1.0);
        fill_weights(wgt, 1.0);

        MHO_MBDelaySearch search;
        search.SetArgs(&vis);
        search.SetWeights(&wgt);
        search.SetReferenceFrequency(REF_FREQ);
        REQUIRE(search.Initialize());
        REQUIRE(search.Execute());

        double dr = search.GetCoarseDR();
        if(sgn > 0)
        {
            REQUIRE(dr > 0.0);
        }
        else
        {
            REQUIRE(dr < 0.0);
        }
        // an on-axis (zero MBD delay) ramp still peaks near zero MBD
        REQUIRE(search.GetMBDMaxBin() == argmin_abs(search.GetMBDAxis()));
    }
    return 0;
}

// Case 4: search-window set/get + clamping logic
static int test_windows()
{
    visibility_type vis;
    vis.Resize(1, 8, 16, 4);
    weight_type wgt;
    wgt.Resize(1, 8, 16, 1);
    set_axes(vis, CHAN_FREQS, 1.0);
    fill_constant(vis, std::complex< double >(1.0, 0.0));
    fill_weights(wgt, 1.0);

    MHO_MBDelaySearch search;
    search.SetArgs(&vis);
    search.SetWeights(&wgt);
    search.SetReferenceFrequency(REF_FREQ);
    REQUIRE(search.Initialize());
    REQUIRE(search.Execute()); // populates the SBD/MBD/DR axes used by GetWindow

    // SBD axis is {0,1,2,3} with bin separation 1 -> full (unset) window is [0, 4]
    double lo = -1.0, hi = -1.0;
    search.GetSBDWindow(lo, hi);
    CHECK_CLOSE(lo, 0.0, 1e-12);
    CHECK_CLOSE(hi, 4.0, 1e-12);

    // an interior window is returned unchanged (clamped to itself)
    search.SetSBDWindow(1.0, 2.5);
    search.GetSBDWindow(lo, hi);
    CHECK_CLOSE(lo, 1.0, 1e-12);
    CHECK_CLOSE(hi, 2.5, 1e-12);

    // reversed limits are sorted by SetWindow
    search.SetSBDWindow(2.5, 1.0);
    search.GetSBDWindow(lo, hi);
    CHECK_CLOSE(lo, 1.0, 1e-12);
    CHECK_CLOSE(hi, 2.5, 1e-12);

    // a window wider than the axis is clamped down to the axis extent
    search.SetSBDWindow(-100.0, 100.0);
    search.GetSBDWindow(lo, hi);
    CHECK_CLOSE(lo, 0.0, 1e-12);
    CHECK_CLOSE(hi, 4.0, 1e-12);

    // MBD/DR: an interior window (strictly inside the axis range) round-trips exactly
    int mlast = static_cast< int >(search.GetMBDAxis()->GetSize()) - 1;
    double mlo = (*search.GetMBDAxis())(0);
    double mhi = (*search.GetMBDAxis())(mlast);
    double span = mhi - mlo;
    search.SetMBDWindow(mlo + 0.25 * span, mhi - 0.25 * span);
    search.GetMBDWindow(lo, hi);
    CHECK_CLOSE(lo, mlo + 0.25 * span, 1e-9);
    CHECK_CLOSE(hi, mhi - 0.25 * span, 1e-9);

    int dlast = static_cast< int >(search.GetDRAxis()->GetSize()) - 1;
    double dlo = (*search.GetDRAxis())(0);
    double dhi = (*search.GetDRAxis())(dlast);
    double dspan = dhi - dlo;
    search.SetDRWindow(dlo + 0.25 * dspan, dhi - 0.25 * dspan);
    search.GetDRWindow(lo, hi);
    CHECK_CLOSE(lo, dlo + 0.25 * dspan, 1e-12);
    CHECK_CLOSE(hi, dhi - 0.25 * dspan, 1e-12);

    return 0;
}

// Case 5: null / not-initialized guards
static int test_guards()
{
    // null input -> Initialize fails
    MHO_MBDelaySearch search;
    search.SetArgs(nullptr);
    REQUIRE(search.Initialize() == false);

    // Execute without a successful Initialize -> fails (does not crash)
    REQUIRE(search.Execute() == false);
    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if(test_initialize())
    {
        return 1;
    }
    if(test_constant_peak_at_zero())
    {
        return 1;
    }
    if(test_rate_sign())
    {
        return 1;
    }
    if(test_windows())
    {
        return 1;
    }
    if(test_guards())
    {
        return 1;
    }
    return 0;
}
