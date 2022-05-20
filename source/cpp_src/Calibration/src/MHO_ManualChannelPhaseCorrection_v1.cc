#include "MHO_ManualChannelPhaseCorrection_v1.hh"

namespace hops
{


MHO_ManualChannelPhaseCorrection_v1::MHO_ManualChannelPhaseCorrection_v1()
{

}

MHO_ManualChannelPhaseCorrection_v1::~MHO_ManualChannelPhaseCorrection_v1()
{

}

bool 
MHO_ManualChannelPhaseCorrection_v1::InitializeInPlace(XArrayType* in)
{
    //verify in is not null and retrieve dimensions of input 
    return false;
}

bool 
MHO_ManualChannelPhaseCorrection_v1::ExecuteInPlace(XArrayType* in)
{
    //loop over each channel we have a phase correction for
    //and apply the corresponding complex rotation to all elements of this channel
    return false;
}


//ignore the following functions for now, no real optimization to be done for now

bool 
MHO_ManualChannelPhaseCorrection_v1::InitializeOutOfPlace(const XArrayType* in, XArrayType* out)
{
    //just copy to output and do 'in-place'
    out->Copy(*in);
    return InitializeInPlace(out);
}

bool 
MHO_ManualChannelPhaseCorrection_v1::ExecuteOutOfPlace(const XArrayType* in, XArrayType* out)
{
    //just execute 'in-place' on the copy
    return ExecuteInPlace(out);
}

}//end of namespace