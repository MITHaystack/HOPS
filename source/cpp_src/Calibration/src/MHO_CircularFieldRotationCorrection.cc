#include "MHO_CircularFieldRotationCorrection.hh"
#include "MHO_Constants.hh"

#include <math.h>
#include <stdlib.h>
#include <complex>
#include <cmath>

// mount_type
#define NO_MOUNT_TYPE   0
#define CASSEGRAIN      1
#define NASMYTHLEFT     2
#define NASMYTHRIGHT    3

//pol-product
#define POL_LL 0
#define POL_RR 1
#define POL_LR 2
#define POL_RL 3


void
compute_field_rotations_fixed(std::complex<double> cpolvalue[4], double *par_angle, double *elevation, int *mount_type)
{
    /*
    * This function computes the field rotation angles
    * gbc, sep 14, 2021
    *
    * This version uses fields from the param struct which are indexed by
    * ref and rem, but it would be a minor change to add indexing to the
    * station coefficients to allow this to be truly a function of ap.
    *
    * See parallactic-angle-correction.txt for discussion.
    */

    double elmult[4];
    double radangle;
    int pp;
    elmult[NO_MOUNT_TYPE] = 0.0;
    elmult[CASSEGRAIN]    = 0.0;
    elmult[NASMYTHLEFT]   = - 1.0;
    elmult[NASMYTHRIGHT]  = + 1.0;
    for(pp = 0; pp < 4; pp++)
    {
        cpolvalue[pp] = 0.0;
        switch(pp)
        {
            case POL_LL: radangle =
                (+1) * (par_angle[0] + elmult[mount_type[0]]*elevation[0]) +
                (-1) * (par_angle[1] + elmult[mount_type[1]]*elevation[1]) ;
                break;
            case POL_RR: radangle =
                (-1) * (par_angle[0] + elmult[mount_type[0]]*elevation[0]) +
                (+1) * (par_angle[1] + elmult[mount_type[1]]*elevation[1]) ;
                break;
            case POL_LR: radangle =
                (+1) * (par_angle[0] + elmult[mount_type[0]]*elevation[0]) +
                (+1) * (par_angle[1] + elmult[mount_type[1]]*elevation[1]) ;
                break;
            case POL_RL: radangle =
                (-1) * (par_angle[0] + elmult[mount_type[0]]*elevation[0]) +
                (-1) * (par_angle[1] + elmult[mount_type[1]]*elevation[1]) ;
                break;
            default:
                msg_error("calibration", "error in field rotation correction while determining polarization product, this should not happen" << eom);
        }
        cpolvalue[pp] = std::exp( -1.0 * MHO_Constants::imag_unit * radangle );
    }
}

namespace hops
{


MHO_CircularFieldRotationCorrection::MHO_CircularFieldRotationCorrection()
{
    fRefParAngle = 0;
    fRemParAngle = 0;
    fRefElevation = 0;
    fRemElevation = 0;
    fFourfitRefTimeString = "";
    fRefMountType = "";
    fRemMountType = "";
    fRefData = nullptr;
    fRemData = nullptr;
};

MHO_CircularFieldRotationCorrection::~MHO_CircularFieldRotationCorrection(){};


bool
MHO_CircularFieldRotationCorrection::ExecuteInPlace(visibility_type* in)
{
    PreMultiply(in);
    return true;
}


bool
MHO_CircularFieldRotationCorrection::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    PreMultiply(out);
    return true;
}

bool
MHO_CircularFieldRotationCorrection::InitializeInPlace(visibility_type* /*in*/)
{


    return true;
}

bool
MHO_CircularFieldRotationCorrection::InitializeOutOfPlace(const visibility_type* /*in*/, visibility_type* /*out*/)
{
    return true;
}


void
MHO_CircularFieldRotationCorrection::PreMultiply(visibility_type* in)
{
    //evaluate the elevation angles
    fRefModel.SetEvaluationTimeVexString(fFourfitRefTimeString);
    fRefModel.SetStationData(fRefData);
    fRefModel.ComputeModel();
    fRefElevation = fRefModel.GetElevation();

    fRemModel.SetEvaluationTimeVexString(fFourfitRefTimeString);
    fRemModel.SetStationData(fRemDatadata);
    fRemModel.ComputeModel();
    fRemElevation = fRemModel.GetElevation();

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


int DetermineMountCode(const std::string& mount) const
{
    return 0;
}

std::complex<double>
MHO_CircularFieldRotationCorrection::GetPrefactor(std::string pp_label)
{
    //this is not super efficient, since we recalculate 4 factors for each pol-product
    //could reorganize this, but its probably not that critical right now
    std::complex<double> factor = 0;

    //if we cannot find this label in the set, return zero
    if( std::find( fPolProductSet.begin(), fPolProductSet.end(), pp_label) == fPolProductSet.end()  ){return factor;}

    std::complex<double> pp_factors[4];
    double par_angle[2];
    double elevation[2];
    int mount_type[2];

    par_angle[0] = fRefParAngle;
    par_angle[1] = fRemParAngle;

    //TODO FILL IN VALUES from station coords!!
    elevation[0] = 0.0;
    elevation[1] = 0.0;

    mount_type[0] = DetermineMountCode(fRefMountType);
    mount_type[1] = DetermineMountCode(fRemMountType);

    //call Geoff's routine
    compute_field_rotations_fixed(pp_factors, par_angle, elevation, mount_type);

    //return the value
    if(pp_label == "LL"){factor = pp_factors[POL_LL];}
    if(pp_label == "RR"){factor = pp_factors[POL_RR];}
    if(pp_label == "LR"){factor = pp_factors[POL_LR];}
    if(pp_label == "RL"){factor = pp_factors[POL_RL];}

    return factor;
}

}//end of namespace
