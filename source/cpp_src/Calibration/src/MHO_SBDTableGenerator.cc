#include "MHO_SBDTableGenerator.hh"

//original normfx.c implementation/algorithm demands this size be 8
//we later (after FFT) sub-sample the array by a factor of 2, for effective factor of 4
// #define PADDING_FACTOR 8
#define PADDING_FACTOR 4

namespace hops
{

MHO_SBDTableGenerator::MHO_SBDTableGenerator():
    fInitialized(false)
{};

MHO_SBDTableGenerator::~MHO_SBDTableGenerator(){};

bool
MHO_SBDTableGenerator::InitializeImpl(const XArgType1* in, XArgType2* out)
{
    //in is raw visibilities 
    //out is the single-band-delay table
    fInitialized = false;
    if(in != nullptr && out != nullptr)
    {
        bool status = true;

        //first we need to check the dimensions of the 'out' object (sbd_type)
        //if it has not been properly sized at this point, we need to resize it
        std::vector< mho_json > dsb_labels = channel_axis->GetMatchingIntervalLabels("double_sideband");
        std::size_t n_dsb_chan = dsb_labels.size();

        if(n_dsb_chan == 0){ConditionallyResizeOutput(in,out);}
        else{ConditionallyResizeOutputDSB(in,out);}

        // 
        // 
        // //figure out if we have USB, LSB data, a mixture, and/or double-sideband data
        // auto channel_axis = &(std::get<CHANNEL_AXIS>( *(in) ) );
        // 
        // std::string sb_key = "net_sideband";
        // std::string usb_flag = "U";
        // std::string lsb_flag = "L";
        // auto usb_chan = channel_axis->GetMatchingIndexes(sb_key, usb_flag);
        // auto lsb_chan = channel_axis->GetMatchingIndexes(sb_key, lsb_flag);
        // 
        // std::size_t n_usb_chan = usb_chan.size();
        // std::size_t n_lsb_chan = lsb_chan.size();
        // if(n_lsb_chan != 0){msg_debug("calibration", "MHO_SBDTableGenerator operating on LSB data, N LSB channels: " << n_lsb_chan <<eom );}
        // if(n_usb_chan != 0){msg_debug("calibration", "MHO_SBDTableGenerator operating on USB data, N USB channels: " << n_usb_chan <<eom );}
        // 
        // // if(n_usb_chan != 0 && n_lsb_chan != 0)
        // // {
        // //     msg_error("calibration", "problem initializing MHO_SBDTableGenerator, mixed USB/LSB data not yet supported." << eom);
        // //     return false;
        // // }
        // 
        // std::vector< mho_json > dsb_labels = channel_axis->GetMatchingIntervalLabels("double_sideband");
        // std::size_t n_dsb_chan = dsb_labels.size();
        // if(n_usb_chan != 0){msg_debug("calibration", "MHO_NormFX operating on DSB data, N DSB channel pairs: " << n_dsb_chan <<eom );}
        // 
        // //allocate the SBD space (expand the freq axis by 4x)
        // std::size_t sbd_dim[visibility_type::rank::value];
        // in->GetDimensions(sbd_dim);
        // sbd_dim[FREQ_AXIS] *= 4; //normfx implementation demands this
        // 
        // //DSB channels combine the LSB and USB halves...so the number of 'channels' 
        // //is reduced by 1 for every DSB pair we have 
        // std::size_t n_sbd_chan = sbd_dim[CHANNEL_AXIS] - n_dsb_chan;
        // sbd_dim[CHANNEL_AXIS] = n_sbd_chan;
        // 
        // out->Resize(sbd_dim);
        // out->ZeroArray();
        // 
        // //copy the pol-product and time axes (as they are the same)
        // std::get<POLPROD_AXIS>(*out).Copy( std::get<POLPROD_AXIS>(*in) );
        // 
        // std::get<TIME_AXIS>(*out).Copy( std::get<TIME_AXIS>(*in) );
        // 
        // //now deal with the channel axis...this has a different size, because we 
        // //need to merge any DSB channels 
        // in_chan_ax = &(std::get<CHANNEL_AXIS>(*in));
        // out_chan_ax =&(std::get<CHANNEL_AXIS>(*out));
        // //loop, copying info over
        // 
        // 
        // 
        // 
        // //allocate the SBD space
        // std::size_t sbd_dim[visibility_type::rank::value];
        // in->GetDimensions(sbd_dim);
        // sbd_dim[FREQ_AXIS] *= 4; //normfx implementation demands this
        // out->Resize(sbd_dim);
        // out->ZeroArray();
        // 
        // //copy all axes but sub-channel frequency
        // std::get<POLPROD_AXIS>(*out).Copy( std::get<POLPROD_AXIS>(*in) );
        // std::get<CHANNEL_AXIS>(*out).Copy( std::get<CHANNEL_AXIS>(*in) );
        // std::get<TIME_AXIS>(*out).Copy( std::get<TIME_AXIS>(*in) );
        // 
        // 
        // in->GetDimensions(fInDims);
        // out->GetDimensions(fOutDims);
        // 
        // //check that the output dimensions are correct
        // if(fInDims[POLPROD_AXIS] != fOutDims[POLPROD_AXIS]){status = false;}
        // if(fInDims[CHANNEL_AXIS] != fOutDims[CHANNEL_AXIS]){status = false;}
        // if(fInDims[TIME_AXIS] != fOutDims[TIME_AXIS]){status = false;}
        // if(4*fInDims[FREQ_AXIS] != fOutDims[FREQ_AXIS]){status = false;}
        // if(!status){msg_error("calibration", "Could not initialize MHO_SBDTableGenerator, in/out dimension mis-match." << eom); return false;}
        // 
        // std::size_t nlags = fInDims[FREQ_AXIS]; //in the original norm_fx, nlags is 2x this number
        // 
        // //temp fWorkspace
        // out->GetDimensions(fWorkDims);
        // fWorkDims[FREQ_AXIS] *= 2;
        // fWorkspace.Resize(fWorkDims);
        // fWorkspace.SetArray(std::complex<double>(0.0,0.0));
        // 
        // TODO_FIXME_MSG("TODO FIXME, the following line casts away const-ness:")
        // fNaNBroadcaster.SetArgs( const_cast<XArgType*>(in) );
        // status = fNaNBroadcaster.Initialize();
        // if(!status){msg_error("calibration", "Could not initialize NaN mask broadcast in MHO_SBDTableGenerator." << eom); return false;}
        // 
        // 
        // //double it
        // nlags *= 2;

        fInitialized = true;
    }

    return fInitialized;

}


bool
MHO_SBDTableGenerator::ExecuteImpl(const XArgType1* in, XArgType2* out)
{
    if(fInitialized)
    {
        bool status;

        //all we do here is copy the visibility data into the SBD array 
        //keeping in mind that:
        //we need to (end) zero-pad the data for LSB/USB 
        //we need to merge double-sideband pairs into a single channel 
        //with the USB/LSB chunks at either end



        // //first thing we do is filter out any NaNs
        // //(ADHOC flagging would likely also be implemented in a similar fashion)
        // status = fNaNBroadcaster.Execute();
        // if(!status){msg_error("calibration", "Could not execute NaN masker MHO_SBDTableGenerator." << eom); return false;}
        // 
        // status = fZeroPadder.Execute();
        // if(!status){msg_error("calibration", "Could not execute zero padder in MHO_SBDTableGenerator." << eom); return false;}
        // 
        // status = fFFTEngine.Execute();
        // if(!status){msg_error("calibration", "Could not execute FFT in MHO_SBDTableGenerator." << eom); return false;}
        // 
        // status = fSubSampler.Execute();
        // if(!status){msg_error("calibration", "Could not execute sub-sampler in MHO_SBDTableGenerator." << eom); return false;}
        // 
        // status = fCyclicRotator.Execute();
        // if(!status){msg_error("calibration", "Could not execute cyclic-rotation MHO_SBDTableGenerator." << eom); return false;}
        // 
        // //for lower sideband we complex conjugate the data
        // auto chan_ax = &(std::get<CHANNEL_AXIS>(*out));
        // for(std::size_t ch=0; ch<chan_ax->GetSize(); ch++)
        // {
        //     std::string net_sideband;
        //     bool key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", net_sideband);
        //     if(!key_present){msg_error("calibration", "norm_fx missing net_sideband label for channel "<< ch << eom); }
        //     if(net_sideband == "L")
        //     {
        //         //just the slice that matches this channel
        //         auto slice = out->SliceView(":", ch, ":", ":");
        //         for(auto it = slice.begin(); it != slice.end(); it++){*it = std::conj(*it);}
        //     }
        // }
        // 
        // //normalize the array (due to FFT)
        // double norm =  1.0/(double)fInDims[FREQ_AXIS];
        // *(out) *= norm;

        return true;
    }

    return false;
};


void 
MHO_SBDTableGenerator::ConditionallyResizeOutput(const XArgType1* in, XArgType2* out)
{
    //first we need to check the dimensions of the 'out' object (sbd_type)
    //if it has not been properly sized at this point, we need to resize it
    //this function assumes we have only USB/LSB data (and no double-sideband channels)
    std::size_t vis_dim[visibility_type::rank::value];
    std::size_t sbd_dim[visibility_type::rank::value];
    in->GetDimensions(vis_dim);
    out->GetDimensions(sbd_dim);

    //check to see if the output dimensions are properly sized
    bool need_to_resize = false;
    for(std::size_t i=0; i<visibility_type::rank::value; i++)
    {
        if(i == FREQ_AXIS) //only freq axis is padded 
        {
            if( PADDING_FACTOR*vis_dim[i] != sbd_dim[i] ){need_to_resize = true;}
        }
        else if( vis_dim[i] != sbd_dim[i] ){need_to_resize = true;} 

        if(fInDims[POLPROD_AXIS] != fOutDims[POLPROD_AXIS]){need_to_resize = false;}
        if(fInDims[CHANNEL_AXIS] != fOutDims[CHANNEL_AXIS]){need_to_resize = false;}
        if(fInDims[TIME_AXIS] != fOutDims[TIME_AXIS]){need_to_resize = false;}
        if(4*fInDims[FREQ_AXIS] != fOutDims[FREQ_AXIS]){need_to_resize = false;}
        if(!need_to_resize){msg_error("calibration", "Could not initialize MHO_NormFX, in/out dimension mis-match." << eom); return false;}

    }

    std::size_t nlags = fInDims[FREQ_AXIS]; //in the original norm_fx, nlags is 2x this number

    //temp fWorkspace
    out->GetDimensions(fWorkDims);
    fWorkDims[FREQ_AXIS] *= 2;
    fWorkspace.Resize(fWorkDims);
    fWorkspace.SetArray(std::complex<double>(0.0,0.0));



    //figure out if we have USB, LSB data, a mixture, and/or double-sideband data
    auto channel_axis = &(std::get<CHANNEL_AXIS>( *(in) ) );

    std::string sb_key = "net_sideband";
    std::string usb_flag = "U";
    std::string lsb_flag = "L";
    auto usb_chan = channel_axis->GetMatchingIndexes(sb_key, usb_flag);
    auto lsb_chan = channel_axis->GetMatchingIndexes(sb_key, lsb_flag);

    std::size_t n_usb_chan = usb_chan.size();
    std::size_t n_lsb_chan = lsb_chan.size();
    if(n_lsb_chan != 0){msg_debug("calibration", "MHO_SBDTableGenerator operating on LSB data, N LSB channels: " << n_lsb_chan <<eom );}
    if(n_usb_chan != 0){msg_debug("calibration", "MHO_SBDTableGenerator operating on USB data, N USB channels: " << n_usb_chan <<eom );}

    //allocate the SBD space (expand the freq axis by the padding factor)
    std::size_t sbd_dim[visibility_type::rank::value];
    in->GetDimensions(sbd_dim);
    sbd_dim[FREQ_AXIS] *= PADDING_FACTOR; //normfx implementation demands this

    out->Resize(sbd_dim);
    out->ZeroArray();

    //copy the pol-product and time axes (as they are the same)
    std::get<POLPROD_AXIS>(*out).Copy( std::get<POLPROD_AXIS>(*in) );
    std::get<CHANNEL_AXIS>(*out).Copy( std::get<CHANNEL_AXIS>(*in) );
    std::get<TIME_AXIS>(*out).Copy( std::get<TIME_AXIS>(*in) );
}


void 
MHO_SBDTableGenerator::ConditionallyResizeOutputDSB(const XArgType1* in, XArgType2* out)
{



        //first we need to check the dimensions of the 'out' object (sbd_type)
        //if it has not been properly sized at this point, we need to resize it

        //figure out if we have USB, LSB data, a mixture, and/or double-sideband data
        auto channel_axis = &(std::get<CHANNEL_AXIS>( *(in) ) );

        std::string sb_key = "net_sideband";
        std::string usb_flag = "U";
        std::string lsb_flag = "L";
        auto usb_chan = channel_axis->GetMatchingIndexes(sb_key, usb_flag);
        auto lsb_chan = channel_axis->GetMatchingIndexes(sb_key, lsb_flag);

        std::size_t n_usb_chan = usb_chan.size();
        std::size_t n_lsb_chan = lsb_chan.size();
        if(n_lsb_chan != 0){msg_debug("calibration", "MHO_SBDTableGenerator operating on LSB data, N LSB channels: " << n_lsb_chan <<eom );}
        if(n_usb_chan != 0){msg_debug("calibration", "MHO_SBDTableGenerator operating on USB data, N USB channels: " << n_usb_chan <<eom );}

        // if(n_usb_chan != 0 && n_lsb_chan != 0)
        // {
        //     msg_error("calibration", "problem initializing MHO_SBDTableGenerator, mixed USB/LSB data not yet supported." << eom);
        //     return false;
        // }

        std::vector< mho_json > dsb_labels = channel_axis->GetMatchingIntervalLabels("double_sideband");
        std::size_t n_dsb_chan = dsb_labels.size();
        if(n_usb_chan != 0){msg_debug("calibration", "MHO_NormFX operating on DSB data, N DSB channel pairs: " << n_dsb_chan <<eom );}

        //allocate the SBD space (expand the freq axis by 4x)
        std::size_t sbd_dim[visibility_type::rank::value];
        in->GetDimensions(sbd_dim);
        sbd_dim[FREQ_AXIS] *= 4; //normfx implementation demands this

        //DSB channels combine the LSB and USB halves...so the number of 'channels' 
        //is reduced by 1 for every DSB pair we have 
        std::size_t n_sbd_chan = sbd_dim[CHANNEL_AXIS] - n_dsb_chan;
        sbd_dim[CHANNEL_AXIS] = n_sbd_chan;

        out->Resize(sbd_dim);
        out->ZeroArray();

        //copy the pol-product and time axes (as they are the same)
        std::get<POLPROD_AXIS>(*out).Copy( std::get<POLPROD_AXIS>(*in) );

        std::get<TIME_AXIS>(*out).Copy( std::get<TIME_AXIS>(*in) );

        //now deal with the channel axis...this has a different size, because we 
        //need to merge any DSB channels 
        in_chan_ax = &(std::get<CHANNEL_AXIS>(*in));
        out_chan_ax =&(std::get<CHANNEL_AXIS>(*out));
        //loop, copying info over


}




}//end of namespace
