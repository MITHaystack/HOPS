#include "MHO_NormFX.hh"

namespace hops
{


MHO_NormFX::MHO_NormFX():
    fInitialized(false)
{};

MHO_NormFX::~MHO_NormFX(){};

bool
MHO_NormFX::InitializeOutOfPlace(const XArgType* in, XArgType* out)
{
    fInitialized = false;
    if(in != nullptr && out != nullptr)
    {
        bool status = true;
        //figure out if we have USB or LSB data (or a mixture)
        auto channel_axis = &(std::get<CHANNEL_AXIS>( *(in) ) );

        std::string sb_key = "net_sideband";
        std::string usb_flag = "U";
        std::string lsb_flag = "L";
        auto usb_chan = channel_axis->GetMatchingIndexes(sb_key, usb_flag);
        auto lsb_chan = channel_axis->GetMatchingIndexes(sb_key, lsb_flag);

        std::size_t n_usb_chan = usb_chan.size();
        std::size_t n_lsb_chan = lsb_chan.size();
        if(n_lsb_chan != 0){msg_debug("calibration", "MHO_NormFX operating on LSB data, N LSB channels: " << n_lsb_chan <<eom );}
        if(n_usb_chan != 0){msg_debug("calibration", "MHO_NormFX operating on USB data, N USB channels: " << n_usb_chan <<eom );}

        //mixed sideband data should be ok, but warn user since it is not well tested
        if(n_usb_chan != 0 && n_lsb_chan != 0)
        {
            msg_warn("calibration", "support for data with mixed USB/LSB is experimental" << eom);
        }

        std::vector< mho_json > dsb_labels = channel_axis->GetMatchingIntervalLabels("double_sideband");
        std::size_t n_dsb_chan = dsb_labels.size();
        if(n_dsb_chan != 0)
        {
            msg_error("calibration", "MHO_NormFX discovered: "<< n_dsb_chan <<" double-sideband channels, this data type is not yet supported" <<eom );
            return false;
        }

        //allocate the SBD space
        std::size_t sbd_dim[visibility_type::rank::value];
        in->GetDimensions(sbd_dim);
        sbd_dim[FREQ_AXIS] *= 4; //normfx implementation demands this
        out->Resize(sbd_dim);
        out->ZeroArray();

        //copy all axes but sub-channel frequency
        std::get<POLPROD_AXIS>(*out).Copy( std::get<POLPROD_AXIS>(*in) );
        std::get<CHANNEL_AXIS>(*out).Copy( std::get<CHANNEL_AXIS>(*in) );
        std::get<TIME_AXIS>(*out).Copy( std::get<TIME_AXIS>(*in) );


        in->GetDimensions(fInDims);
        out->GetDimensions(fOutDims);

        //check that the output dimensions are correct
        if(fInDims[POLPROD_AXIS] != fOutDims[POLPROD_AXIS]){status = false;}
        if(fInDims[CHANNEL_AXIS] != fOutDims[CHANNEL_AXIS]){status = false;}
        if(fInDims[TIME_AXIS] != fOutDims[TIME_AXIS]){status = false;}
        if(4*fInDims[FREQ_AXIS] != fOutDims[FREQ_AXIS]){status = false;}
        if(!status){msg_error("calibration", "Could not initialize MHO_NormFX, in/out dimension mis-match." << eom); return false;}

        std::size_t nlags = fInDims[FREQ_AXIS]; //in the original norm_fx, nlags is 2x this number

        // fZeroPadder.SetArgs(in, &fWorkspace);
        fZeroPadder.SetArgs(in, out);
        fZeroPadder.DeselectAllAxes();
        //fZeroPadder.EnableNormFXMode(); //doesnt seem to make any difference
        fZeroPadder.SelectAxis(FREQ_AXIS); //only pad on the frequency (to lag) axis
        fZeroPadder.SetPaddingFactor(4); //original padding factor was 8...but then data was subsampled by factor of 2
        fZeroPadder.SetEndPadded(); //for both LSB and USB (what about DSB?)

        status = fZeroPadder.Initialize();
        if(!status){msg_error("calibration", "Could not initialize zero padder in MHO_NormFX." << eom); return false;}

        fNaNBroadcaster.SetArgs(out);
        status = fNaNBroadcaster.Initialize();
        if(!status){msg_error("calibration", "Could not initialize NaN mask broadcast in MHO_NormFX." << eom); return false;}

        fFFTEngine.SetArgs(out);
        fFFTEngine.DeselectAllAxes();
        fFFTEngine.SelectAxis(FREQ_AXIS); //only perform padded fft on frequency (to lag) axis
        fFFTEngine.SetForward();//forward DFT
        status = fFFTEngine.Initialize();
        if(!status){msg_error("calibration", "Could not initialize FFT in MHO_NormFX." << eom); return false;}

        fCyclicRotator.SetOffset(FREQ_AXIS, sbd_dim[FREQ_AXIS]/2);
        fCyclicRotator.SetArgs(out);
        status = fCyclicRotator.Initialize();
        if(!status){msg_error("calibration", "Could not initialize cyclic rotation in MHO_NormFX." << eom); return false;}

        fInitialized = true;
    }

    return fInitialized;

}


bool
MHO_NormFX::ExecuteOutOfPlace(const XArgType* in, XArgType* out)
{

    if(fInitialized)
    {
        bool status;
        //copy in the visibility data in a zero-padded fashion
        status = fZeroPadder.Execute();
        if(!status){msg_error("calibration", "Could not execute zero padder in MHO_NormFX." << eom); return false;}

        //filter out any NaNs
        status = fNaNBroadcaster.Execute();
        if(!status){msg_error("calibration", "Could not execute NaN masker MHO_NormFX." << eom); return false;}

        status = fFFTEngine.Execute();
        if(!status){msg_error("calibration", "Could not execute FFT in MHO_NormFX." << eom); return false;}

        status = fCyclicRotator.Execute();
        if(!status){msg_error("calibration", "Could not execute cyclic-rotation MHO_NormFX." << eom); return false;}

        //for lower sideband we complex conjugate the data (could do this before FFT)
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
    XArgType* tmp = new XArgType();
    bool status = InitializeOutOfPlace(in, tmp);
    in->Copy(*tmp);
    delete tmp;
    return status;
}

bool 
MHO_NormFX::ExecuteInPlace(XArgType* in)
{
    XArgType* tmp = new XArgType();
    bool status = ExecuteOutOfPlace(in, tmp);
    in->Copy(*tmp);
    delete tmp;
    return status;
}


}//end of namespace
