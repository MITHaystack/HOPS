#include "MHO_VisibilityPrecisionUpCaster.hh"


namespace hops
{


MHO_VisibilityPrecisionUpCaster::MHO_VisibilityPrecisionUpCaster(){};
MHO_VisibilityPrecisionUpCaster::~MHO_VisibilityPrecisionUpCaster(){}

bool
MHO_VisibilityPrecisionUpCaster::ExecuteImpl(const visibility_store_type* in, visibility_type* out)
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
