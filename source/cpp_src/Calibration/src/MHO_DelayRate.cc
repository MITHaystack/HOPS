#include "MHO_DelayRate.hh"
#include "MHO_BitReversalPermutation.hh"

#include <math.h>

namespace hops
{

namespace
{
    //Zero-padding factor applied to the time axis: the rate FFT runs on
    //kTimePadFactor*fDRSPSize points and is then resampled back down to fDRSPSize bins.
    const int kTimePadFactor = 4;
} // namespace

MHO_DelayRate::MHO_DelayRate(): fInitialized(false)
{
    fRefFreq = 1.0;
};

MHO_DelayRate::~MHO_DelayRate(){};

bool MHO_DelayRate::InitializeImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out)
{

    fInitialized = false;
    if(in1 != nullptr && in2 != nullptr && out != nullptr)
    {
        profiler_scope();
        bool ok = true;

        in1->GetDimensions(fInDims);

        // //copy the input data into the workspace
        out->CopyTags(*in1);

        fDRSPSize = CalculateSearchSpaceSize(fInDims[TIME_AXIS]);
        msg_debug("calibration", "delay rate search space size = " << fDRSPSize << eom);

        BuildResamplingTable(in1);

        std::size_t np = fDRSPSize * kTimePadFactor;
        ConditionallyResizeOutput(&(fInDims[0]), np, out);

        //pre-allocate workspace for ApplyInterpolation
        fInterpWorkspace.resize(fDRSPSize * fInDims[FREQ_AXIS]);

        fZeroPadder.SetArgs(in1, out);
        fZeroPadder.DeselectAllAxes();
        fZeroPadder.SelectAxis(TIME_AXIS); //only pad on the time (to delay rate) axis
        fZeroPadder.SetPaddedSize(np);
        fZeroPadder.SetEndPadded();
        fZeroPadder.PreserveWorkspace();
        fZeroPadder.DisableTagCopy();

        fFFTEngine.SetArgs(out);
        fFFTEngine.DeselectAllAxes();
        fFFTEngine.SelectAxis(TIME_AXIS); //only perform padded fft on time (to delay rate) axis
        fFFTEngine.SetForward();          //forward DFT

        ok = fZeroPadder.Initialize();
        if(!ok)
        {
            msg_error("operators", "Could not initialize zero padder in MHO_DelayRate" << eom);
            return false;
        }

        ok = fFFTEngine.Initialize();
        if(!ok)
        {
            msg_error("operators", "Could not initialize FFT in MHO_DelayRate" << eom);
            return false;
        }

        fInitialized = true;
    }

    return fInitialized;
}

bool MHO_DelayRate::ExecuteImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out)
{
    profiler_scope();

    if(fInitialized)
    {
        bool ok;

        ok = fZeroPadder.Execute();
        if(!ok)
        {
            msg_error("operators", "Could not execute zero padder in MHO_DelayRate" << eom);
            return false;
        }

        ApplyDataWeights(in2, out);

        ok = fFFTEngine.Execute();
        if(!ok)
        {
            msg_error("operators", "Could not execute FFT in MHO_DelayRate" << eom);
            return false;
        }

        //no explicit fftshift (MHO_CyclicRotator) pass is needed here: the lo/hi indices in fInterpTable already
        //carry the half-length shift, so the resampling reads the raw FFT output directly.
        ApplyInterpolation(in1, out);

        return true;
    }

    return false;
};

void MHO_DelayRate::ApplyDataWeights(const XArgType2* in2, XArgType3* out)
{
    profiler_scope();
    //apply the data weights to the data
    std::size_t pprod = out->GetDimension(POLPROD_AXIS);
    std::size_t nch = out->GetDimension(CHANNEL_AXIS);
    std::size_t nap = out->GetDimension(TIME_AXIS);

    std::size_t wnap = in2->GetDimension(TIME_AXIS);

    //make sure we don't over run the weight array bounds (since out array has been padded)
    std::size_t nap_range = std::min(nap, wnap);

    for(std::size_t pp = 0; pp < pprod; pp++)
    {
        for(std::size_t ch = 0; ch < nch; ch++)
        {
            for(std::size_t ap = 0; ap < nap_range; ap++)
            {
                auto val = (*in2)(pp, ch, ap, 0);
                out->SubView(pp, ch, ap) *= val; //apply the data weights
            }
        }
    }
}

void MHO_DelayRate::ConditionallyResizeOutput(const std::size_t* dims, std::size_t size, XArgType3* out)
{
    auto out_dim = out->GetDimensionArray();
    bool have_to_resize = false;
    for(std::size_t i = 0; i < XArgType3::rank::value; i++)
    {
        if(i == TIME_AXIS)
        {
            if(out_dim[i] != size)
            {
                have_to_resize = true;
                out_dim[i] = size;
            }
        }
        else
        {
            if(dims[i] != out_dim[i])
            {
                have_to_resize = true;
                out_dim[i] = dims[i];
            }
        }
    }
    if(have_to_resize)
    {
        out->Resize(&(out_dim[0]));
    }
}

unsigned int MHO_DelayRate::CalculateSearchSpaceSize(unsigned int input_size)
{
    //just make it as big as needed
    int drsp_size = 2 * MHO_BitReversalPermutation::NextLowestPowerOfTwo(input_size);
    //the legacy size calculation (see search_windows.c) is as follows:
    // unsigned int drsp_size = 8192;
    // while( input_size < (drsp_size / 4) )
    // {
    //     drsp_size /= 2;
    // };
    return drsp_size;
}

void MHO_DelayRate::BuildResamplingTable(const XArgType1* in1)
{
    const int n_dr = fDRSPSize;                     //output delay-rate bins
    const int n_fft = kTimePadFactor * fDRSPSize;   //padded FFT length
    const int half_dr = n_dr / 2;                   //zero-rate bin (n_dr is always even)
    const std::size_t nch = fInDims[CHANNEL_AXIS];

    fInterpTable.resize(nch * n_dr);
    for(std::size_t ch = 0; ch < nch; ch++)
    {
        //The fringe rate seen in this channel is (delay rate)*(sky freq), so a single physical
        //delay rate lands in a different fringe-rate bin in each channel. Stretching this
        //channel's rate spectrum by chan_freq/fRefFreq removes that channel dependence and puts
        //every channel on one shared delay-rate grid. Channels above fRefFreq compress, those
        //below stretch; a channel exactly at fRefFreq gives stretch == kTimePadFactor, i.e. a
        //plain 4:1 decimation of the padded spectrum.
        double chan_freq = std::get< CHANNEL_AXIS >(*in1)(ch);
        double stretch = kTimePadFactor * (chan_freq / fRefFreq);

        for(int dr = 0; dr < n_dr; dr++)
        {
            //Fractional source position in the padded spectrum for output bin 'dr':
            //  (dr - n_dr/2)   signed bin offset from zero rate
            //  * stretch       convert to a position on this channel's rate spectrum
            //  + n_fft/2       recentre onto the fftshift'ed (zero-rate-at-middle) ordering
            //  + n_fft         one extra period, so the fmod argument stays non-negative
            double pos = ((double)dr - (double)half_dr) * stretch + (double)n_fft * 1.5;
            double pos_wrapped = std::fmod(pos, (double)n_fft);

            int lo = (int)pos_wrapped;
            //guard only: pos goes negative when chan_freq/fRefFreq > 3, which no realistic
            //setup should reach TODO FIXME: if it ever does, then this will resample the wrong bin.
            if(lo < 0)
            {
                lo = 0;
            }
            //NOTE: the upper neighbour is clamped rather than wrapped to 0, so at the topmost
            //bin the blend degenerates to just spectrum[n_fft-1]. This matches legacy fourfit
            //(delay_rate.c): TODO check this behavior against the old hops code.
            int hi = lo + 1;
            if(hi > (n_fft - 1))
            {
                hi = n_fft - 1;
            }
            double frac = pos_wrapped - (double)lo;

            //Fold the fftshift into the indices so ApplyInterpolation can read the raw
            //post-FFT array: post-FFT position p lands at post-shift position
            //(p - n_fft/2 + n_fft) % n_fft, so to read post-shift index q we read
            //(q + n_fft/2) % n_fft.
            lo = (lo + n_fft / 2) % n_fft;
            hi = (hi + n_fft / 2) % n_fft;

            fInterpTable[ch * n_dr + dr] = {lo, hi, frac};
        }
    }
}

void MHO_DelayRate::ApplyInterpolation(const XArgType1* in1, XArgType3* out)
{
    profiler_scope();
    std::size_t pprod = in1->GetDimension(POLPROD_AXIS);
    std::size_t nch = in1->GetDimension(CHANNEL_AXIS);
    double time_delta = std::get< TIME_AXIS >(*in1)(1) - std::get< TIME_AXIS >(*in1)(0);

    std::size_t nsbd = out->GetDimension(FREQ_AXIS);

    //write the rate axis labels once, they depend only on dr, not on pp/ch/sbd,
    //these are fringe rates *at the reference frequency* (s^-1); the caller (MBDelaySearch) divides by
    //fRefFreq to obtain the delay rate that bin dr represents in every channel.
    const int half_dr = fDRSPSize / 2; //zero-rate bin (fDRSPSize is always even)
    double ax_scale = 1.0 / (time_delta * (double)fDRSPSize);
    for(int dr = 0; dr < fDRSPSize; dr++)
    {
        std::get< TIME_AXIS > (*out)(dr) = ((double)dr - (double)half_dr) * ax_scale;
    }

    using value_t = sbd_type::value_type;

    for(std::size_t pp = 0; pp < pprod; pp++)
    {
        for(std::size_t ch = 0; ch < nch; ch++)
        {
            const InterpEntry* tbl = &fInterpTable[ch * fDRSPSize];

            //Stage results into workspace: dr(outer)->sbd(inner).
            //For each dr, lo and hi are constants from the table, so we obtain raw pointers
            //to the two contiguous source sbd-rows once per dr and traverse them cheaply, that way
            //OffsetFromStrideIndex is called once per (pp,ch,dr) triple, not once per element.
            for(int dr = 0; dr < fDRSPSize; dr++)
            {
                const InterpEntry& e = tbl[dr];
                const double w1 = 1.0 - e.frac;
                const double w2 = e.frac;
                const value_t* src0 = &(*out)(pp, ch, e.lo, 0);
                const value_t* src1 = &(*out)(pp, ch, e.hi, 0);
                value_t* dst = &fInterpWorkspace[dr * nsbd];
                for(std::size_t sbd = 0; sbd < nsbd; sbd++)
                {
                    dst[sbd] = src0[sbd] * w1 + src1[sbd] * w2;
                }
            }

            //Copy staged results back into the output array row by row.
            //Both src and dst walks are contiguous.
            for(int dr = 0; dr < fDRSPSize; dr++)
            {
                value_t* out_row = &(*out)(pp, ch, dr, 0);
                const value_t* src = &fInterpWorkspace[dr * nsbd];
                std::copy(src, src + nsbd, out_row);
            }
        }
    }
}

} // namespace hops
