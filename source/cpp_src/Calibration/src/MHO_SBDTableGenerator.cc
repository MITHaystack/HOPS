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
        //no resize the SBD table container  if needed
        ConditionallyResizeOutput(in,out);
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
    //we don't copy the FREQ_AXIS...as that gets filled in by the zero-padder & FFT in NormFX
}

// 
// void 
// MHO_SBDTableGenerator::ConditionallyResizeOutputDSB(const XArgType1* in, XArgType2* out)
// {
//     //first we need to check the dimensions of the 'out' object (sbd_type)
//     //if it has not been properly sized at this point, we need to resize it
//     //this function assumes we have only USB/LSB data (and no double-sideband channels!)
//     std::size_t vis_dim[visibility_type::rank::value];
//     std::size_t sbd_dim[visibility_type::rank::value];
//     in->GetDimensions(vis_dim);
//     out->GetDimensions(sbd_dim);
// 
//     //figure out the channel set-up...we are expecting at least one DSB channel
//     auto channel_axis = &(std::get<CHANNEL_AXIS>( *(in) ) );
//     std::size_t n_chan = channel_axis->GetSize();
// 
//     std::string sb_key = "net_sideband";
//     std::string usb_flag = "U";
//     std::string lsb_flag = "L";
//     auto usb_chan = channel_axis->GetMatchingIndexes(sb_key, usb_flag);
//     auto lsb_chan = channel_axis->GetMatchingIndexes(sb_key, lsb_flag);
// 
//     std::size_t n_usb_chan = usb_chan.size();
//     std::size_t n_lsb_chan = lsb_chan.size();
//     if(n_lsb_chan != 0){msg_debug("calibration", "MHO_SBDTableGenerator operating on LSB data, N LSB channels: " << n_lsb_chan <<eom );}
//     if(n_usb_chan != 0){msg_debug("calibration", "MHO_SBDTableGenerator operating on USB data, N USB channels: " << n_usb_chan <<eom );}
// 
//     std::vector< mho_json > dsb_labels = channel_axis->GetMatchingIntervalLabels("double_sideband");
//     std::size_t n_dsb_chan = dsb_labels.size();
//     if(n_usb_chan != 0){msg_debug("calibration", "MHO_NormFX operating on DSB data, N DSB channel pairs: " << n_dsb_chan <<eom );}
// 
//     //figure out the number of output channels, after we merge the DSB channels 
//     std::size_t n_out_chan = n_chan - n_dsb_chan; //each DSB combines 2 channel into one 
// 
//     //check to see if the output dimensions are properly sized
//     bool need_to_resize = false;
//     for(std::size_t i=0; i<visibility_type::rank::value; i++)
//     {
//         if(i == FREQ_AXIS) //only freq axis is padded 
//         {
//             if( PADDING_FACTOR*vis_dim[i] != sbd_dim[i] ){need_to_resize = true;}
//         }
//         else if(i == CHANNEL_AXIS)
//         {
//             if( sbd_dim[i] != n_out_chan ){need_to_resize = true;}
//         }
//         else if( vis_dim[i] != sbd_dim[i] ){need_to_resize = true;} 
//     }
// 
//     //no need to resize sbd array, all set
//     if(!need_to_resize){return;} 
// 
//     msg_debug("calibration", "resizing single-band-delay table container frequency axis"); 
// 
//     //allocate the SBD space (expand the freq axis by the padding factor)
//     in->GetDimensions(sbd_dim); //get input dimensions
//     sbd_dim[FREQ_AXIS] = PADDING_FACTOR*vis_dim[FREQ_AXIS]; //normfx implementation demands this
//     sbd_dim[CHANNEL_AXIS] = n_out_chan;//size after merging DSB channels
//     out->Resize(sbd_dim);
//     out->ZeroArray();
// 
//     //copy the pol-product and time axes (as they are the same size as input table)
//     std::get<POLPROD_AXIS>(*out).Copy( std::get<POLPROD_AXIS>(*in) );
//     std::get<TIME_AXIS>(*out).Copy( std::get<TIME_AXIS>(*in) );
//     //we have to populate the CHANNEL_AXIS and the FREQ_AXIS manually
// 
//     //now deal with the channel axis...these have a different sizes, because we 
//     //need to merge any DSB channels 
//     auto in_chan_ax = &(std::get<CHANNEL_AXIS>(*in));
//     auto out_chan_ax = &(std::get<CHANNEL_AXIS>(*out));
//     std::size_t out_chan_count = 0;
//     for(std::size_t i=0; i<in_chan_ax->GetSize(); i++)
//     {
//         mho_json index_label_obj = in_chan_ax->GetLabelObject(i);
//         //check if this channel is a member of a double side-band pair 
//         if(index_label_obj.contains("dsb_partner"))
//         {
//             //this is a double sideband channel, get the partner's index 
//             //this should be the USB partner
//             int pindex = index_label_obj["dsb_partner"].get<int>();
//             if(pindex != i+1)
//             {
//                 //something has gone seriously wrong, this data can't be processed
//                 msg_fatal("calibration", "non-adjacent double-sideband pair, cannot merge LSB/USB channels of this form " << eom);
//                 std::exit(1);
//             }
//             //copy the sky frequency value
//             out_chan_ax->at(out_chan_count) = in_chan_ax->at(i);
//             //copy the label, but modify the net_sideband 
//             index_label_obj["net_sideband"] = "D";
//             out_chan_ax->SetLabelObject(index_label_obj, out_chan_count);
//             i = pindex; //jump to adjacent USB channel
//             out_chan_count++;
//         }
//         else 
//         {
//             //just a regular channel 
//             out_chan_ax->at(out_chan_count) = in_chan_ax->at(i); //copy sky frequency
//             out_chan_ax->SetLabelObject(index_label_obj, out_chan_count); //copy labels
//             out_chan_count++;
//         }
//     }
// 
//     //now deal with the frequency axis -- the exact values are not as critical, 
//     //only the spacing is important
//     //since the FFT in NormFX will transform this axis to delay space
//     auto in_freq_ax = &(std::get<FREQ_AXIS>(*in));
//     auto out_freq_ax = &(std::get<FREQ_AXIS>(*out));
//     //figure out the frequency delta 
//     double fstart = in_freq_ax->at(0);
//     double fdelta = in_freq_ax->at(1) - in_freq_ax->at(0); 
//     for(std::size_t i=0; i<out_freq_ax->GetSize(); i++)
//     {
//         out_freq_ax->at(i) = fstart + i*fdelta;
//     }
// 
// }




}//end of namespace
