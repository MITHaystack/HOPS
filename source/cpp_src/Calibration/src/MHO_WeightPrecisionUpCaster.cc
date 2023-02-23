#include "MHO_WeightPrecisionUpCaster.hh"


namespace hops
{


MHO_WeightPrecisionUpCaster::MHO_WeightPrecisionUpCaster(){};
MHO_WeightPrecisionUpCaster::~MHO_WeightPrecisionUpCaster(){}

bool
MHO_WeightPrecisionUpCaster::ExecuteImpl(const ch_weight_store_type* in, ch_weight_type* out)
{
    if(in != nullptr && out != nullptr)
    {
        //resize the output array 
        out->Resize( in->GetDimensions() );
        
        //copy the contents
        std::size_t total_size = in->GetSize();
        for(std::size_t i=0; i<total_size; i++)
        {
            (*out)[i] = (*in)[i];
        }

        //copy the axes, their tags and labels 
        *( out->GetAxisPack() ) = *( in->GetAxisPack() );

        //copy the tags
        out->CopyTags(*in);
        
        return true;
    }
    return false;
}



}//end of namespace
