#include "MHO_MinWeight.hh"

namespace hops
{


MHO_MinWeight::MHO_MinWeight()
{
    fMinWeight = 0.0;
};

MHO_MinWeight::~MHO_MinWeight(){};


bool
MHO_MinWeight::ExecuteInPlace(weight_type* in)
{
    msg_debug("calibration", "zero-ing out weights for all values less than "<< fMinWeight << eom);
    for(auto it = in->begin(); it != in->end(); it++)
    {
        double w = *it;
        if( w < fMinWeight){ *it = 0.0;}
    }
    return true;
}


bool
MHO_MinWeight::ExecuteOutOfPlace(const weight_type* in, weight_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}


bool
MHO_MinWeight::InitializeInPlace(weight_type* /*in*/){ return true;}

bool
MHO_MinWeight::InitializeOutOfPlace(const weight_type* /*in*/, weight_type* /*out*/){return true;}




}//end of namespace
