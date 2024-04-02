#include "MHO_Passband.hh"


namespace hops
{


MHO_Passband::MHO_Passband()
{
    fLow = 0.0;
    fHigh =0.0;
};

MHO_Passband::~MHO_Passband(){};


bool
MHO_Passband::ExecuteInPlace(visibility_type* in)
{

    return true;
}


bool
MHO_Passband::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}


bool
MHO_Passband::InitializeInPlace(visibility_type* /*in*/){ return true;}

bool
MHO_Passband::InitializeOutOfPlace(const visibility_type* /*in*/, visibility_type* /*out*/){return true;}


}//end of namespace
