#include "MHO_DelayRate.hh"
#include "MHO_BitReversalPermutation.hh"

#include <iomanip>

#include <math.h>

namespace hops
{

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
        bool ok = true;

        in1->GetDimensions(fInDims);

        // //copy the input data into the workspace
        out->CopyTags(*in1);

        fDRSPSize = CalculateSearchSpaceSize(fInDims[TIME_AXIS]);
        msg_debug("calibration", "delay rate search space size = " << fDRSPSize << eom);

        std::size_t np = fDRSPSize * 4;
        ConditionallyResizeOutput(&(fInDims[0]), np, out);

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

        fCyclicRotator.SetOffset(TIME_AXIS, np / 2);
        fCyclicRotator.SetArgs(out);
        ok = fCyclicRotator.Initialize();
        if(!ok)
        {
            msg_error("operators", "Could not initialize cyclic rotation in MHO_DelayRate." << eom);
            return false;
        }

        fInitialized = true;
    }

    return fInitialized;
}

bool MHO_DelayRate::ExecuteImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out)
{

    if(fInitialized)
    {
        out->ZeroArray();
        bool ok;

        ok = fZeroPadder.Execute();
        if(!ok)
        {
            msg_error("operators", "Could not execute zero padder in MHO_DelayRate" << eom);
            return false;
        }

        //ApplyDataWeights(in2, out);

        ok = fFFTEngine.Execute();
        if(!ok)
        {
            msg_error("operators", "Could not execute FFT in MHO_DelayRate" << eom);
            return false;
        }

        ok = fCyclicRotator.Execute();
        check_step_fatal(ok, "fringe", "cyclic rotation execution." << eom);

        //apply the legacy linear interpolation step -- TODO determine if this is strictly needed
        ApplyInterpolation(in1, out);

        return true;
    }

    return false;
};

void MHO_DelayRate::ApplyDataWeights(const XArgType2* in2, XArgType3* out)
{
    //apply the data weights to the data
    std::size_t pprod = out->GetDimension(POLPROD_AXIS);
    std::size_t nch = out->GetDimension(CHANNEL_AXIS);
    std::size_t nap = out->GetDimension(TIME_AXIS);
    std::size_t nsbd = out->GetDimension(FREQ_AXIS);

    std::size_t wpprod = in2->GetDimension(POLPROD_AXIS);
    std::size_t wnch = in2->GetDimension(CHANNEL_AXIS);
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

void MHO_DelayRate::ApplyInterpolation(const XArgType1* in1, XArgType3* out)
{
    std::size_t pprod = in1->GetDimension(POLPROD_AXIS);
    std::size_t nch = in1->GetDimension(CHANNEL_AXIS);
    std::size_t nap = in1->GetDimension(TIME_AXIS);
    double time_delta = std::get< TIME_AXIS >(*in1)(1) - std::get< TIME_AXIS >(*in1)(0);

    //linear interpolation and modifcation of delay rate axis (see delay_rate.c line 81)
    int sz = 4 * fDRSPSize;
    std::size_t nsbd = out->GetDimension(FREQ_AXIS);

    std::vector< sbd_type::value_type > workspace;
    workspace.resize(fDRSPSize);
    for(std::size_t pp = 0; pp < pprod; pp++)
    {
        for(std::size_t ch = 0; ch < nch; ch++)
        {
            double chan_freq = (std::get< CHANNEL_AXIS >(*in1))(ch);
            double b = ((chan_freq / fRefFreq) * sz) / fDRSPSize;

            for(std::size_t sbd = 0; sbd < nsbd; sbd++)
            {
                for(std::size_t dr = 0; dr < fDRSPSize; dr++)
                {
                    double num = ((double)dr - (double)(fDRSPSize / 2)) * b + ((double)sz * 1.5);
                    double l_fp = fmod(num, (double)sz);
                    int l_int = (int)l_fp;
                    if(l_int < 0)
                    {
                        l_int = 0;
                    }
                    int l_int2 = l_int + 1;
                    if(l_int2 > (sz - 1))
                    {
                        l_int2 = sz - 1;
                    }
                    sbd_type::value_type interp_val =
                        (*out)(pp, ch, l_int, sbd) * (1.0 - l_fp + l_int) + (*out)(pp, ch, l_int2, sbd) * (l_fp - l_int);
                    workspace[dr] = interp_val;

                    double ax_val = ((double)dr - (double)(fDRSPSize / 2)) * (1.0 / (time_delta * (double)fDRSPSize));
                    std::get< TIME_AXIS > (*out)(dr) = ax_val;
                }
                for(std::size_t dr = 0; dr < fDRSPSize; dr++)
                {
                    (*out)(pp, ch, dr, sbd) = workspace[dr];
                }
            }
        }
    }
}

} // namespace hops
