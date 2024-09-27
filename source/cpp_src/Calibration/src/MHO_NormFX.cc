#include "MHO_NormFX.hh"

namespace hops
{


MHO_NormFX::MHO_NormFX():
    fInitialized(false)
{};

MHO_NormFX::~MHO_NormFX(){};

bool
MHO_NormFX::InitializeOutOfPlace(const XArgType* in1, XArgType* out)
{
    fInitialized = false;
    if(in1 != nullptr && out != nullptr)
    {
        bool status = true;
        //figure out if we have USB or LSB data (or a mixture)
        auto* channel_axis = &(std::get<CHANNEL_AXIS>( *(in1) ) );

        std::string sb_key = "net_sideband";
        std::string usb_flag = "U";
        std::string lsb_flag = "L";
        auto usb_chan = channel_axis->GetMatchingIndexes(sb_key, usb_flag);
        auto lsb_chan = channel_axis->GetMatchingIndexes(sb_key, lsb_flag);

        std::size_t n_usb_chan = usb_chan.size();
        std::size_t n_lsb_chan = lsb_chan.size();
        if(n_lsb_chan != 0){msg_debug("calibration", "MHO_NormFX operating on LSB data, N LSB channels: " << n_lsb_chan <<eom );}
        if(n_usb_chan != 0){msg_debug("calibration", "MHO_NormFX operating on USB data, N USB channels: " << n_usb_chan <<eom );}

        // if(n_usb_chan != 0 && n_lsb_chan != 0)
        // {
        //     msg_error("calibration", "problem initializing MHO_NormFX, mixed USB/LSB data not yet supported." << eom);
        //     //return false;
        // }

        //allocate the SBD space
        std::size_t sbd_dim[visibility_type::rank::value];
        in1->GetDimensions(sbd_dim);
        sbd_dim[FREQ_AXIS] *= 4; //normfx implementation demands this
        out->Resize(sbd_dim);
        out->ZeroArray();

        //copy all axes but sub-channel frequency
        std::get<POLPROD_AXIS>(*out).Copy( std::get<POLPROD_AXIS>(*in1) );
        std::get<CHANNEL_AXIS>(*out).Copy( std::get<CHANNEL_AXIS>(*in1) );
        std::get<TIME_AXIS>(*out).Copy( std::get<TIME_AXIS>(*in1) );


        in1->GetDimensions(fInDims);
        out->GetDimensions(fOutDims);

        //check that the output dimensions are correct
        if(fInDims[POLPROD_AXIS] != fOutDims[POLPROD_AXIS]){status = false;}
        if(fInDims[CHANNEL_AXIS] != fOutDims[CHANNEL_AXIS]){status = false;}
        if(fInDims[TIME_AXIS] != fOutDims[TIME_AXIS]){status = false;}
        if(4*fInDims[FREQ_AXIS] != fOutDims[FREQ_AXIS]){status = false;}
        if(!status){msg_error("calibration", "Could not initialize MHO_NormFX, in/out dimension mis-match." << eom); return false;}

        std::size_t nlags = fInDims[FREQ_AXIS]; //in the original norm_fx, nlags is 2x this number

        //temp fWorkspace
        out->GetDimensions(fWorkDims);
        fWorkDims[FREQ_AXIS] *= 2;
        fWorkspace.Resize(fWorkDims);
        fWorkspace.SetArray(std::complex<double>(0.0,0.0));

        TODO_FIXME_MSG("TODO FIXME, the following line casts away const-ness:")
        fNaNBroadcaster.SetArgs( const_cast<XArgType*>(in1) );
        status = fNaNBroadcaster.Initialize();
        if(!status){msg_error("calibration", "Could not initialize NaN mask broadcast in MHO_NormFX." << eom); return false;}

        fZeroPadder.SetArgs(in1, &fWorkspace);
        fZeroPadder.DeselectAllAxes();
        //fZeroPadder.EnableNormFXMode(); //doesnt seem to make any difference
        fZeroPadder.SelectAxis(FREQ_AXIS); //only pad on the frequency (to lag) axis
        fZeroPadder.SetPaddingFactor(8);
        fZeroPadder.SetEndPadded(); //for both LSB and USB (what about DSB?)

        fFFTEngine.SetArgs(&fWorkspace);
        fFFTEngine.DeselectAllAxes();
        fFFTEngine.SelectAxis(FREQ_AXIS); //only perform padded fft on frequency (to lag) axis
        fFFTEngine.SetForward();//forward DFT

        status = fZeroPadder.Initialize();
        if(!status){msg_error("calibration", "Could not initialize zero padder in MHO_NormFX." << eom); return false;}

        status = fFFTEngine.Initialize();
        if(!status){msg_error("calibration", "Could not initialize FFT in MHO_NormFX." << eom); return false;}

        fSubSampler.SetDimensionAndStride(FREQ_AXIS, 2);
        fSubSampler.SetArgs(&fWorkspace, out);
        status = fSubSampler.Initialize();
        if(!status){msg_error("calibration", "Could not initialize sub-sampler in MHO_NormFX." << eom); return false;}

        fCyclicRotator.SetOffset(FREQ_AXIS, 2*nlags);
        fCyclicRotator.SetArgs(out);
        status = fCyclicRotator.Initialize();
        if(!status){msg_error("calibration", "Could not initialize cyclic rotation in MHO_NormFX." << eom); return false;}

        //double it
        nlags *= 2;

        fInitialized = true;
    }

    return fInitialized;

}


bool
MHO_NormFX::ExecuteOutOfPlace(const XArgType* in1, XArgType* out)
{

    if(fInitialized)
    {
        bool status;

        //first thing we do is filter out any NaNs
        //(ADHOC flagging would likely also be implemented in a similar fashion)
        status = fNaNBroadcaster.Execute();
        if(!status){msg_error("calibration", "Could not execute NaN masker MHO_NormFX." << eom); return false;}

        status = fZeroPadder.Execute();
        if(!status){msg_error("calibration", "Could not execute zero padder in MHO_NormFX." << eom); return false;}

        status = fFFTEngine.Execute();
        if(!status){msg_error("calibration", "Could not execute FFT in MHO_NormFX." << eom); return false;}

        status = fSubSampler.Execute();
        if(!status){msg_error("calibration", "Could not execute sub-sampler in MHO_NormFX." << eom); return false;}

        status = fCyclicRotator.Execute();
        if(!status){msg_error("calibration", "Could not execute cyclic-rotation MHO_NormFX." << eom); return false;}

        //for lower sideband we complex conjugate the data
        auto chan_ax = &(std::get<CHANNEL_AXIS>(*out));
        for(std::size_t ch=0; ch<chan_ax->GetSize(); ch++)
        {
            std::string net_sideband;
            bool key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", net_sideband);
            if(!key_present){msg_error("calibration", "norm_fx missing net_sideband label for channel "<< ch << eom); }
            if(net_sideband == "L")
            {
                //just the slice that matches this channel
                auto slice = out->SliceView(":", ch, ":", ":");
                for(auto it = slice.begin(); it != slice.end(); it++){*it = std::conj(*it);}
            }
        }

        //normalize the array (due to FFT)
        double norm =  1.0/(double)fInDims[FREQ_AXIS];
        *(out) *= norm;

        return true;
    }

    return false;
};



bool 
MHO_NormFX::InitializeInPlace(XArgType* in)
{
    msg_error("calibration", "in place norm_fx not implemented" <<eom);
}

bool 
MHO_NormFX::ExecuteInPlace(XArgType* in)
{
    msg_error("calibration", "in place norm_fx not implemented" <<eom);
}


}//end of namespace
