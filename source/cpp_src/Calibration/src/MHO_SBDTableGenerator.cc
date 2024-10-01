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
    //out is the single-band-delay table (workspace/output for normfx)
    //we need to check the dimensions of the 'out' object (sbd_type)
    //if it has not been properly sized at this point, we need to resize it
    fInitialized = false;
    if(in != nullptr && out != nullptr)
    {
        //check if there are DSB channel pairs (these channels will be merged)
        std::vector< mho_json > dsb_labels = std::get<CHANNEL_AXIS>(*in).GetMatchingIntervalLabels("double_sideband");
        std::size_t n_dsb_chan = dsb_labels.size();

        if(n_dsb_chan == 0){ConditionallyResizeOutput(in,out);}
        else
        {
            ConditionallyResizeOutputDSB(in,out);
            return false; //not yet implemented
        }
        fInitialized = true;
    }
    return fInitialized;
}


bool
MHO_SBDTableGenerator::ExecuteImpl(const XArgType1* in, XArgType2* out)
{
    //exec is a no-op for this class
    if(fInitialized){return true;}
    return false;
};


void 
MHO_SBDTableGenerator::ConditionallyResizeOutput(const XArgType1* in, XArgType2* out)
{
    //first we need to check the dimensions of the 'out' object (sbd_type)
    //if it has not been properly sized at this point, we need to resize it
    //this function assumes we have only USB/LSB data (and no double-sideband channels!)
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
    }

    //no need to resize sbd array, all set
    if(!need_to_resize){return;} 

    msg_debug("calibration", "resizing single-band-delay table container frequency axis"); 

    //allocate the SBD space (expand the freq axis by the padding factor)
    in->GetDimensions(sbd_dim); //get input dimensions
    sbd_dim[FREQ_AXIS] = PADDING_FACTOR*vis_dim[FREQ_AXIS]; //normfx implementation demands this
    out->Resize(sbd_dim);
    out->ZeroArray();

    //copy the pol-product and time axes (as they are the same size as input table)
    std::get<POLPROD_AXIS>(*out).Copy( std::get<POLPROD_AXIS>(*in) );
    std::get<CHANNEL_AXIS>(*out).Copy( std::get<CHANNEL_AXIS>(*in) );
    std::get<TIME_AXIS>(*out).Copy( std::get<TIME_AXIS>(*in) );
    //we don't copy the FREQ_AXIS...as that gets filled in by the FFT in NormFX
}


void 
MHO_SBDTableGenerator::ConditionallyResizeOutputDSB(const XArgType1* in, XArgType2* out)
{
    msg_error("calibration", "MHO_NormFX discovered 1 or more double-sideband channels, this data type is not yet supported" <<eom );
    ConditionallyResizeOutput(in,out);

    TODO_FIXME_MSG("implemented double sideband SBD table resizing");


    // 
    // 
    // //first we need to check the dimensions of the 'out' object (sbd_type)
    // //if it has not been properly sized at this point, we need to resize it
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


}




}//end of namespace
