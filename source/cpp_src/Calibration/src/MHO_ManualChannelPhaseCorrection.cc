#include "MHO_ManualChannelPhaseCorrection.hh"

namespace hops
{


MHO_ManualChannelPhaseCorrection::MHO_ManualChannelPhaseCorrection():
    fInitialized(false),
    fConjugate(false)
{};

MHO_ManualChannelPhaseCorrection::~MHO_ManualChannelPhaseCorrection(){};


bool
MHO_ManualChannelPhaseCorrection::InitializeImpl(const XArgType1* in_vis, const XArgType2* pcal, XArgType3* out_vis)
{
    //TODO check that dimensions of in_vis and out_vis are the same 

    //check that the p-cal data is tagged with a station-id that is a member of this baseline

    //determine if the p-cal corrections are being applied to the remote or reference station

    //map the pcal polarization index to the visibility pol-product index 

    //map the pcal channel index to the visibility channel index



    return true;
}


bool
MHO_ManualChannelPhaseCorrection::ExecuteImpl(const XArgType1* in_vis, const XArgType2* pcal, XArgType3* out_vis)
{
    if(fInitialized)
    {
        //just copy in_vis into out_vis
        //TODO FIXME...there is no reason we can't do this operation in place applied to in_vs
        //but the operator interface needs to be different since we need a unary op 
        //with separate 2 input arguments (vis array, and pcal array)
        out_vis->Copy(*in_vis);
    
        //loop over p-cal pols
        for(auto pol_it = fPolIdxMap.begin(); pol_it != fPolIdxMap.end(); pol_it++)
        {
            //loop over the p-cal channels
            for(auto ch_it = fChanIdxMap.begin(); ch_it != fChanIdxMap.end(); ch_it++)
            {
                //retrieve the p-cal phasor (assume unit normal)
                pcal_phasor_type pc_val = (*pcal)(pol_it->first, ch_it->first);
                //conjugate if applied to remote station
                if(fConjugate){pc_val = std::conj(pc_val);} 
                
                //retrieve and multiply the appropriate sub view of the visibility array 
                auto chunk = out_vis->SubView(pol_it->second, ch_it->second);
                chunk *= pc_val;
            }
        }
    }

    return true;
};


}//end of namespace