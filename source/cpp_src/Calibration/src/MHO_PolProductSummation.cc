#include "MHO_PolProductSummation.hh"

#define signum(a) (a >= 0 ? 1.0 : -1.0)

namespace hops
{

MHO_PolProductSummation::MHO_PolProductSummation()
{
    fWeights = nullptr;
    fSummedPolProdLabel = "??";
};

MHO_PolProductSummation::~MHO_PolProductSummation(){};

bool MHO_PolProductSummation::ExecuteInPlace(visibility_type* in)
{

    PreMultiply(in);
    bool ok = fReducer.Execute();
    FixLabels(in);

    TODO_FIXME_MSG("TODO FIXME -- is this treatment (averaging) of the pol-product weights correct?")
    //average the weights across all summed pol-products
    bool wok = fWReducer.Execute();
    double n_polprod = fPolProductSet.size();
    // (*fWeights) *= 1.0/n_polprod;
    fWeights->Insert("n_summed_polprod", n_polprod);

    msg_debug("calibration", "executed pol-product summation on: " << n_polprod << " pol-products" << eom);

    return ok && wok;
}

bool MHO_PolProductSummation::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    PreMultiply(out);
    bool ok = fReducer.Execute();
    FixLabels(out);

    TODO_FIXME_MSG("TODO FIXME -- is this treatment (averaging) of the pol-product weights correct?")
    //average the weights across all 4 pol-products
    bool wok = fWReducer.Execute();
    double n_polprod = fPolProductSet.size();
    // (*fWeights) *= 1.0/n_polprod;

    fWeights->Insert("n_summed_polprod", n_polprod);

    return ok && wok;
}

bool MHO_PolProductSummation::InitializeInPlace(visibility_type* in)
{
    fWReducer.SetArgs(fWeights);
    fWReducer.ReduceAxis(POLPROD_AXIS);

    fReducer.SetArgs(in);
    fReducer.ReduceAxis(POLPROD_AXIS);
    return (fReducer.Initialize() && fWReducer.Initialize());
}

bool MHO_PolProductSummation::InitializeOutOfPlace(const visibility_type* in, visibility_type* out)
{
    fWReducer.SetArgs(fWeights);
    fWReducer.ReduceAxis(POLPROD_AXIS);

    fReducer.SetArgs(out);
    fReducer.ReduceAxis(POLPROD_AXIS);
    return (fReducer.Initialize() && fWReducer.Initialize());
}

void MHO_PolProductSummation::PreMultiply(visibility_type* in)
{
    //TODO this is an extremely basic implementation (single pre-factor per-pol product)
    //it is entirely possible to imagine a time dependent pre-factor for each pol-product
    //(e.g if parallactic angle is changing substantially)
    //or other more complex pre-multiplication
    auto pp_ax = &(std::get< POLPROD_AXIS >(*in));

    double prefac_sum = 0.0;
    std::vector< std::complex< double > > prefac;
    prefac.resize(pp_ax->GetSize());
    for(std::size_t i = 0; i < pp_ax->GetSize(); i++)
    {
        prefac[i] = GetPrefactor(pp_ax->at(i));
        prefac_sum += std::abs(prefac[i]);
    }

    //specific to pseudo-Stokes-I (IXY) mode, (4 involved pol-products) explictly set to 2 (for 2 polarizations)
    if(fSummedPolProdLabel == "I")
    {
        prefac_sum = 2.0;
    }

    for(std::size_t i = 0; i < pp_ax->GetSize(); i++)
    {
        in->SubView(i) *= prefac[i] / prefac_sum;
    }
}

std::complex< double > MHO_PolProductSummation::GetPrefactor(std::string pp_label)
{
    std::complex< double > factor = 1.0;
    //if we cannot find this label in the set, return zero
    if(std::find(fPolProductSet.begin(), fPolProductSet.end(), pp_label) == fPolProductSet.end())
    {
        return factor;
    }

    //calculate the parallactic angle difference
    double dpar = (fRemParAngle - fRefParAngle) * (M_PI / 180.);

    if(pp_label == "XX")
    {
        if(fPolProductSet.size() > 1)
        {
            factor = std::cos(dpar);
        }
        else
        {
            factor = signum(std::cos(dpar));
        }
    }

    if(pp_label == "YY")
    {
        if(fPolProductSet.size() > 1)
        {
            factor = std::cos(dpar);
        }
        else
        {
            factor = signum(std::cos(dpar));
        }
    }

    if(pp_label == "YX")
    {
        if(fPolProductSet.size() > 1)
        {
            factor = std::sin(dpar);
        }
        else
        {
            factor = signum(std::sin(dpar));
        }
    }

    if(pp_label == "XY")
    {
        if(fPolProductSet.size() > 1)
        {
            factor = std::sin(-1. * dpar);
        }
        else
        {
            factor = signum(std::sin(-1. * dpar));
        }
    }

    //pol-product dependent scaling/rotation factor for the given pol products
    //depends on the telescope mount type and may have varied dependance
    //on (delta) parallactic angle...this is handled separately in MHO_CircularFieldRotationCorrection
    //for circular-circular pol-products, so we just set the factor to 1 here.

    if(pp_label == "RR")
    {
        factor = 1.0;
    }
    if(pp_label == "LL")
    {
        factor = 1.0;
    }
    if(pp_label == "RL")
    {
        factor = 1.0;
    }
    if(pp_label == "LR")
    {
        factor = 1.0;
    }

    //what about mixed linear-circular pol-products?
    TODO_FIXME_MSG(
        "FIXME TODO -- implement prefactor calculations for both mixed linear and circular pol-products (XX, RX, RR, etc).")

    return factor;
}

void MHO_PolProductSummation::FixLabels(visibility_type* in)
{
    msg_debug("calibration", "pol-product summation produced the combination: " << fSummedPolProdLabel << eom);
    (&(std::get< POLPROD_AXIS >(*in)))->at(0) = fSummedPolProdLabel;
}

} // namespace hops
