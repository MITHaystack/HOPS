#include "MHO_SingleSidebandNormFX.hh"

#include <algorithm>

#define PADDING_FACTOR 4

namespace hops
{

MHO_SingleSidebandNormFX::MHO_SingleSidebandNormFX(): fInitialized(false){};

MHO_SingleSidebandNormFX::~MHO_SingleSidebandNormFX(){};

bool MHO_SingleSidebandNormFX::InitializeOutOfPlace(const XArgType* in, XArgType* out)
{
    fInitialized = false;
    if(in != nullptr && out != nullptr)
    {
        bool status = true;
        //first we initialize the SBD array, by resizing if necessary
        //all this does is expand the frequency axis by 4
        fSBDGen.SetArgs(in, out);
        fSBDGen.Initialize(); //re-size the sbd (out) array if needed
        fSBDGen.Execute();    //this is a no-op

        in->GetDimensions(fInDims);
        out->GetDimensions(fOutDims);
        // fInDims[FREQ_AXIS] -- in the original norm_fx, nlags is 2x this number

        //this operator fills the SBD table from the input visibilities
        //but is only used where there are no double-sideband channels
        fZeroPadder.SetArgs(in, out);
        fZeroPadder.DeselectAllAxes();
        fZeroPadder.SelectAxis(FREQ_AXIS);            //only pad on the frequency (to lag) axis
        fZeroPadder.SetPaddingFactor(PADDING_FACTOR); //original padding factor was 8, but then data was subsampled by 2
        fZeroPadder.SetEndPadded();
        status = fZeroPadder.Initialize();
        if(!status)
        {
            msg_error("calibration", "could not initialize zero padder in MHO_SingleSidebandNormFX" << eom);
            return false;
        }

        fNaNBroadcaster.SetArgs(out);
        status = fNaNBroadcaster.Initialize();
        if(!status)
        {
            msg_error("calibration", "could not initialize NaN mask broadcast in MHO_SingleSidebandNormFX" << eom);
            return false;
        }

        fFFTEngine.SetArgs(out);
        fFFTEngine.DeselectAllAxes();
        fFFTEngine.SelectAxis(FREQ_AXIS); //only perform padded fft on frequency (to lag) axis
        fFFTEngine.SetForward();          //forward DFT
        status = fFFTEngine.Initialize();
        if(!status)
        {
            msg_error("calibration", "could not initialize FFT in MHO_SingleSidebandNormFX" << eom);
            return false;
        }

        fCyclicRotator.SetOffset(FREQ_AXIS, fOutDims[FREQ_AXIS] / 2);
        fCyclicRotator.SetArgs(out);
        status = fCyclicRotator.Initialize();
        if(!status)
        {
            msg_error("calibration", "could not initialize cyclic rotation in MHO_SingleSidebandNormFX" << eom);
            return false;
        }

        fInitialized = true;
    }

    return fInitialized;
}

bool MHO_SingleSidebandNormFX::ExecuteOutOfPlace(const XArgType* in, XArgType* out)
{

    if(fInitialized)
    {
        bool status;

        //copy in the visibility data in a zero-padded fashion (zeros go on the end here)
        out->ZeroArray();
        status = fZeroPadder.Execute(); //use pre-configured zero-padder operator
        if(!status)
        {
            msg_error("calibration", "could not execute zero padder in MHO_SingleSidebandNormFX" << eom);
            return false;
        }

        //filter out any NaNs
        status = fNaNBroadcaster.Execute();
        if(!status)
        {
            msg_error("calibration", "could not execute NaN masker in MHO_SingleSidebandNormFX" << eom);
            return false;
        }

        //apply the data weights if set
        if(this->fWeights != nullptr)
        {
            status = ApplyWeights(out, this->fWeights);
            if(!status)
            {
                msg_error("calibration", "could not apply weights in MHO_SingleSidebandNormFX" << eom);
                return false;
            }
        }
        else 
        {
            msg_warn("calibration", "no visibility data weights available for MHO_SingleSidebandNormFX" << eom);
        }

        status = fFFTEngine.Execute();
        if(!status)
        {
            msg_error("calibration", "could not execute FFT in MHO_SingleSidebandNormFX" << eom);
            return false;
        }

        status = fCyclicRotator.Execute();
        if(!status)
        {
            msg_error("calibration", "could not execute cyclic-rotation in MHO_SingleSidebandNormFX" << eom);
            return false;
        }
        
        //for lower sideband channels we complex conjugate the data after the FFT
        auto chan_ax = &(std::get< CHANNEL_AXIS >(*out));
        for(std::size_t ch = 0; ch < chan_ax->GetSize(); ch++)
        {
            std::string net_sideband;
            bool key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", net_sideband);
            if(net_sideband == "L")
            {
                //just the slice that matches this channel
                auto slice = out->SliceView(":", ch, ":", ":");
                for(auto it = slice.begin(); it != slice.end(); it++)
                {
                    *it = std::conj(*it);
                }
            }
        }

        //normalize the array (due to FFT)
        double norm = 1.0 / (double)fInDims[FREQ_AXIS];
        *(out) *= norm;

        return true;
    }

    return false;
};


bool MHO_SingleSidebandNormFX::ApplyWeights(visibility_type* out, weight_type* w)
{
    std::size_t vis_dim[ visibility_type::rank::value];
    std::size_t wt_dim[ visibility_type::rank::value];

    out->GetDimensions(vis_dim);
    w->GetDimensions(wt_dim);

    //make sure the first 3 dimensions (polprod, channels, time) are the same!
    bool same_size = true;
    for(std::size_t i=0; i<3; i++)
    {
        if(vis_dim[i] != wt_dim[i]){same_size = false;}
    }
    
    if(!same_size)
    {
        msg_error("calibration", "could not apply weights in MHO_SingleSidebandNormFX, dimension mismatch " << eom );
        return false;
    }

    for(std::size_t pp = 0; pp < vis_dim[POLPROD_AXIS]; pp++)
    {
        for(std::size_t ch = 0; ch < vis_dim[CHANNEL_AXIS]; ch++)
        {
            for(std::size_t ap = 0; ap < vis_dim[TIME_AXIS]; ap++)
            {
                out->SubView(pp, ch, ap) *= (*w)(pp,ch,ap,0); //apply the data weights
            }
        }
    }

    return true;
}




//not used
bool MHO_SingleSidebandNormFX::InitializeInPlace(XArgType* in)
{
    XArgType* tmp = new XArgType();
    bool status = InitializeOutOfPlace(in, tmp);
    in->Copy(*tmp);
    delete tmp;
    return status;
}

//not used
bool MHO_SingleSidebandNormFX::ExecuteInPlace(XArgType* in)
{
    XArgType* tmp = new XArgType();
    bool status = ExecuteOutOfPlace(in, tmp);
    in->Copy(*tmp);
    delete tmp;
    return status;
}




} // namespace hops
