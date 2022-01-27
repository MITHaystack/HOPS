#include "MHO_ManualChannelPhaseCorrection.hh"

namespace hops
{


MHO_ManualChannelPhaseCorrection::MHO_ManualChannelPhaseCorrection()
{

}

MHO_ManualChannelPhaseCorrection::~MHO_ManualChannelPhaseCorrection()
{

}

bool 
MHO_ManualChannelPhaseCorrection::InitializeInPlace(XArrayType* in)
{
    //verify in is not null and retrieve dimensions of input 
    return false;
}

bool 
MHO_ManualChannelPhaseCorrection::ExecuteInPlace(XArrayType* in)
{
    //loop over each channel we have a phase correction for
    //and apply the corresponding complex rotation to all elements of this channel
    return false;
}


//ignore this for now
bool 
MHO_ManualChannelPhaseCorrection::InitializeOutOfPlace(const XArrayType* in, XArrayType* out)
{
    //just copy to output and do 'in-place'
    out->Copy(*in);
    return InitializeInPlace(out);
}

//ignore this for now
bool 
MHO_ManualChannelPhaseCorrection::ExecuteOutOfPlace(const XArrayType* in, XArrayType* out)
{
    //just execute 'in-place' on the copy
    return ExecuteInPlace(out);
}

}