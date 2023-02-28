#include "MHO_VisibilityPrecisionDownCaster.hh"


namespace hops
{


MHO_VisibilityPrecisionDownCaster::MHO_VisibilityPrecisionDownCaster(){};
MHO_VisibilityPrecisionDownCaster::~MHO_VisibilityPrecisionDownCaster(){}

bool
MHO_VisibilityPrecisionDownCaster::ExecuteImpl(const visibility_type* in, visibility_store_type* out)
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
