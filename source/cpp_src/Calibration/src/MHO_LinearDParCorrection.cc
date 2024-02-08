#include "MHO_LinearDParCorrection.hh"

#define signum(a) (a>=0 ? 1.0 : -1.0)

namespace hops
{


MHO_LinearDParCorrection::MHO_LinearDParCorrection(){};

MHO_LinearDParCorrection::~MHO_LinearDParCorrection(){};


bool
MHO_LinearDParCorrection::ExecuteInPlace(visibility_type* in)
{
    PreMultiply(in);
    return true;
}


bool
MHO_LinearDParCorrection::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    PreMultiply(out);
    return true;
}

bool
MHO_LinearDParCorrection::InitializeInPlace(visibility_type* /*in*/){return true;}

bool
MHO_LinearDParCorrection::InitializeOutOfPlace(const visibility_type* /*in*/, visibility_type* /*out*/)
{
    return true;
}


void 
MHO_LinearDParCorrection::PreMultiply(visibility_type* in)
{
    //TODO this is an extremely basic implementation (single pre-factor per-pol product)
    //it is entirely possible to imagine a time dependent pre-factor for each pol-product
    //(e.g if parallactic angle is changing substantiall)
    //or other more complex pre-multiplication
    auto pp_ax = &(std::get<POLPROD_AXIS>(*in) );
    for(std::size_t i=0; i < pp_ax->GetSize(); i++)
    {
        auto prefac = GetPrefactor(pp_ax->at(i));
        in->SubView(i) *= prefac;
    }
}


std::complex<double> 
MHO_LinearDParCorrection::GetPrefactor(std::string pp_label)
{
    #pragma message("TODO FIXME...we need to implement proper treatment of X/Y vs H/V pols. Fourfit has the convention inverted." )
    std::complex<double> factor = 0;
    //if we cannot find this label in the set, return zero
    if( std::find( fPolProductSet.begin(), fPolProductSet.end(), pp_label) == fPolProductSet.end()  ){return factor;}

    //calculate the parallactic angle difference 
    double dpar = (fRemParAngle - fRefParAngle)*(M_PI/180.);
    if(pp_label == "XX"){factor = signum( std::cos(dpar) ); }
    if(pp_label == "YY"){factor = signum( std::cos(dpar) ); }
    if(pp_label == "YX"){factor = signum( std::sin(dpar) ); }
    if(pp_label == "XY"){factor = signum( std::sin(-1.*dpar) ); }

    //purely real
    return factor;
}

}//end of namespace
