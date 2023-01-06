#include "MHO_ManualChannelPhaseCorrection.hh"

namespace hops
{


MHO_ManualChannelPhaseCorrection::MHO_ManualChannelPhaseCorrection(){};

MHO_ManualChannelPhaseCorrection::~MHO_ManualChannelPhaseCorrection(){};


bool
MHO_ManualChannelPhaseCorrection::InitializeImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out)
{
    //TODO check that dimensions of in1 and out are the same 

    //check that the p-cal data is tagged with a station-id that is a member of this baseline

    return true;
}


bool
MHO_ManualChannelPhaseCorrection::ExecuteImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out)
{
    //first we need to determine if the p-cal corrects are being applied to the remote or reference station
    


    // 
    // determine the index map of p-cal pol to data pol-product, need to conjugate for remote station
    // 
    // determine the index map of p-cal index -> channel index 
    // (should be fairly straight forward), at the moment this is simply i=i;
    // 


    // if(fInitialized)
    // {
    //     std::size_t n_pols = 0;
    //     std::size_t n_chans = 0
    // 
    //     //loop over p-cal pols
    //     for(std::size_t pol_idx=0; pol_idx)
    //     {
    //         //loop over the p-cal channels
    //         for()
    //         {
    // 
    //         }
    //     }
    // 
    // 
    // }

    return true;
};


}//end of namespace