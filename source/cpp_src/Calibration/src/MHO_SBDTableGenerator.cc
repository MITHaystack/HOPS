#include "MHO_SBDTableGenerator.hh"

#define PADDING_FACTOR 4

namespace hops
{

MHO_SBDTableGenerator::MHO_SBDTableGenerator(): fInitialized(false){};

MHO_SBDTableGenerator::~MHO_SBDTableGenerator(){};

bool MHO_SBDTableGenerator::InitializeImpl(const XArgType1* in, XArgType2* out)
{
    //in is raw visibilities
    //out is the single-band-delay table (workspace/output for normfx)
    //we need to check the dimensions of the 'out' object (sbd_type)
    //if it has not been properly sized at this point, we need to resize it
    fInitialized = false;
    if(in != nullptr && out != nullptr)
    {
        //resize the SBD table container if needed
        ConditionallyResizeOutput(in, out);
        fInitialized = true;
    }
    return fInitialized;
}

bool MHO_SBDTableGenerator::ExecuteImpl(const XArgType1* in, XArgType2* out)
{
    //exec is a no-op for this class
    if(fInitialized)
    {
        return true;
    }
    return false;
};

void MHO_SBDTableGenerator::ConditionallyResizeOutput(const XArgType1* in, XArgType2* out)
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
    for(std::size_t i = 0; i < visibility_type::rank::value; i++)
    {
        if(i == FREQ_AXIS) //only freq axis is padded
        {
            if(PADDING_FACTOR * vis_dim[i] != sbd_dim[i])
            {
                need_to_resize = true;
            }
        }
        else if(vis_dim[i] != sbd_dim[i])
        {
            need_to_resize = true;
        }
    }

    //no need to resize sbd array, all set
    if(!need_to_resize)
    {
        return;
    }

    msg_debug("calibration", "resizing single-band-delay table container frequency axis" << eom);

    //allocate the SBD space (expand the freq axis by the padding factor)
    in->GetDimensions(sbd_dim);                               //get input dimensions
    sbd_dim[FREQ_AXIS] = PADDING_FACTOR * vis_dim[FREQ_AXIS]; //normfx implementation demands this
    out->Resize(sbd_dim);
    out->ZeroArray();

    //copy the pol-product and time axes (as they are the same size as input table)
    std::get< POLPROD_AXIS >(*out).Copy(std::get< POLPROD_AXIS >(*in));
    std::get< CHANNEL_AXIS >(*out).Copy(std::get< CHANNEL_AXIS >(*in));
    std::get< TIME_AXIS >(*out).Copy(std::get< TIME_AXIS >(*in));
    //we don't copy the FREQ_AXIS...as that gets filled in by the zero-padder & FFT in NormFX
}

} // namespace hops
