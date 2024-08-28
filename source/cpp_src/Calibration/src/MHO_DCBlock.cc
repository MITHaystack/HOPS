#include "MHO_DCBlock.hh"

namespace hops
{


MHO_DCBlock::MHO_DCBlock()
{
    fSidebandLabelKey = "net_sideband";
    fUpperSideband = "U";
    fLowerSideband = "L";
};

MHO_DCBlock::~MHO_DCBlock(){};


bool
MHO_DCBlock::ExecuteInPlace(visibility_type* in)
{
    auto chan_ax = &(std::get<CHANNEL_AXIS>(*in) );
    auto freq_ax = &(std::get<FREQ_AXIS>(*in) );

    msg_debug("calibration", "zero-ing out DC spectral points for all channels" << eom);

    //loop over all channels (application depends on USB/LSB)
    for(std::size_t ch=0; ch < chan_ax->GetSize(); ch++) 
    {
        //get channel's frequency info
        double sky_freq = (*chan_ax)(ch); 
        std::string net_sideband;
        bool key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, fSidebandLabelKey, net_sideband);
        if(!key_present){msg_error("calibration", "missing net_sideband label for channel "<< ch << ", with sky_freq: "<<sky_freq << eom); }

        //figure out which spectral point we should zero out
        std::size_t dc_index = 0;
        if(net_sideband == fLowerSideband)
        {
            dc_index = freq_ax->GetSize() - 1;
        }
        //technically speaking we ought correct the data weights 
        //and bandwidth fraction, but this is typically a very small effect on SNR 
        in->SliceView(":", ch, ":", dc_index) *= 0.0;
    }

    return true;
}


bool
MHO_DCBlock::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}


bool
MHO_DCBlock::InitializeInPlace(visibility_type* /*in*/){ return true;}

bool
MHO_DCBlock::InitializeOutOfPlace(const visibility_type* /*in*/, visibility_type* /*out*/){return true;}




}//end of namespace
