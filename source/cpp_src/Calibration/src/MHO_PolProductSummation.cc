#include "MHO_PolProductSummation.hh"


namespace hops
{


MHO_PolProductSummation::MHO_PolProductSummation()
{
    fSummedPolProdLabel = "XX";
};

MHO_PolProductSummation::~MHO_PolProductSummation(){};


bool
MHO_PolProductSummation::ExecuteInPlace(visibility_type* in)
{
    PreMultiply(in);
    bool ok = fReducer.Execute();
    FixLabels(in);
    return ok;
}


bool
MHO_PolProductSummation::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    PreMultiply(out);
    bool ok = fReducer.Execute();
    FixLabels(out);
    return ok;
}

bool
MHO_PolProductSummation::InitializeInPlace(visibility_type* in)
{
    fReducer.SetArgs(in);
    fReducer.ReduceAxis(POLPROD_AXIS);
    return fReducer.Initialize();
}

bool
MHO_PolProductSummation::InitializeOutOfPlace(const visibility_type* in, visibility_type* out)
{
    fReducer.SetArgs(out);
    fReducer.ReduceAxis(POLPROD_AXIS);
    return fReducer.Initialize();
}


void 
MHO_PolProductSummation::PreMultiply(visibility_type* in)
{
    auto pp_ax = &(std::get<POLPROD_AXIS>(*in) );

    for(std::size_t i=0; i < pp_ax->GetSize(); i++)
    {
        std::complex<double> prefac = GetPrefactor(pp_ax->at(i));
        in->SubView(i) *= prefac;
    }
}


std::complex<double> 
MHO_PolProductSummation::GetPrefactor(std::string pp_label)
{
    return 1.0;

    //if we cannot find this label in the set, return zero
    if( std::find( fPolProductSet.begin(), fPolProductSet.end(), pp_label) == fPolProductSet.end()  ){return std::complex<double>(0.0, 0.0);} 

    //this needs to compute the pol-product dependent scaling/rotation factor 
    //for the given pol products
    //depending on the telescope mount type, this may have varied dependance 
    //on (delta) parallactic angle
    
    #pragma message("FIXME TODO -- implement prefactor calculations for linear and circular pol-products (XX, YY, RR, etc).")

    //for now we just return 1 
    return std::complex<double>(1.0, 0.0);
}

void 
MHO_PolProductSummation::FixLabels(visibility_type* in)
{

    std::cout<<"post reduction meta data dump = "<<in->GetMetaDataAsJSON().dump(4)<<std::endl;
    std::size_t n = in->GetRank();
    for(std::size_t i=0; i<n; i++)
    {
        std::cout<<"dim @ "<<i<<" = "<< in->GetDimension(i)<<std::endl;
    }

    ( &(std::get<POLPROD_AXIS>(*in)) )->at(0) = fSummedPolProdLabel;
}

}//end of namespace
