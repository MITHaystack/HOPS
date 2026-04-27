#include "MHO_Constants.hh"
#include "MHO_Message.hh"

#include "MHO_CircularFieldRotationCorrection.hh"

#include <cmath>
#include <complex>
#include <math.h>
#include <stdlib.h>
#include <unordered_map>

namespace hops
{

enum class MountType : int
{
    None = 0,
    Cassegrain = 1,
    NasmythLeft = 2,
    NasmythRight = 3
};

enum class PolProduct : int
{
    LL = 0,
    RR = 1,
    LR = 2,
    RL = 3
};

void compute_field_rotations_fixed(std::complex< double > cpolvalue[4], double par_angle[2], double elevation[2],
                                   MountType mount_type[2])
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

    // elevation multiplier indexed by MountType: None=0, Cassegrain=0, NasmythLeft=-1, NasmythRight=+1
    const double elmult[4] = {0.0, 0.0, -1.0, +1.0};
    double radangle = 0.0;
    const double em0 = elmult[static_cast< int >(mount_type[0])];
    const double em1 = elmult[static_cast< int >(mount_type[1])];
    for(int pp = 0; pp < 4; pp++)
    {
        cpolvalue[pp] = 0.0;
        switch(static_cast< PolProduct >(pp))
        {
            case PolProduct::LL:
                radangle = (+1) * (par_angle[0] + em0 * elevation[0]) + (-1) * (par_angle[1] + em1 * elevation[1]);
                break;
            case PolProduct::RR:
                radangle = (-1) * (par_angle[0] + em0 * elevation[0]) + (+1) * (par_angle[1] + em1 * elevation[1]);
                break;
            case PolProduct::LR:
                radangle = (+1) * (par_angle[0] + em0 * elevation[0]) + (+1) * (par_angle[1] + em1 * elevation[1]);
                break;
            case PolProduct::RL:
                radangle = (-1) * (par_angle[0] + em0 * elevation[0]) + (-1) * (par_angle[1] + em1 * elevation[1]);
                break;
            default:
                msg_error("calibration",
                          "error in field rotation correction while determining polarization product, this should not happen"
                              << eom);
        }
        std::complex< double > iunit = MHO_Constants::imag_unit;
        cpolvalue[pp] = std::exp(-1.0 * iunit * radangle);
    }
}

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

bool MHO_CircularFieldRotationCorrection::ExecuteInPlace(visibility_type* in)
{
    PreMultiply(in);
    return true;
}

bool MHO_CircularFieldRotationCorrection::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    PreMultiply(out);
    return true;
}

bool MHO_CircularFieldRotationCorrection::InitializeInPlace(visibility_type* /*in*/)
{
    return true;
}

bool MHO_CircularFieldRotationCorrection::InitializeOutOfPlace(const visibility_type* /*in*/, visibility_type* /*out*/)
{
    return true;
}

void MHO_CircularFieldRotationCorrection::PreMultiply(visibility_type* in)
{
    //evaluate the elevation angles
    fRefModel.SetEvaluationTimeVexString(fFourfitRefTimeString);
    fRefModel.SetStationData(fRefData);
    fRefModel.ComputeModel();
    fRefElevation = fRefModel.GetElevation();
    fRefParAngle = fRefModel.GetParallacticAngle();

    fRemModel.SetEvaluationTimeVexString(fFourfitRefTimeString);
    fRemModel.SetStationData(fRemData);
    fRemModel.ComputeModel();
    fRemElevation = fRemModel.GetElevation();
    fRemParAngle = fRemModel.GetParallacticAngle();

    //TODO this is an extremely basic implementation (single pre-factor per-pol product)
    //it is entirely possible to imagine a time dependent pre-factor for each pol-product
    //(e.g if parallactic angle is changing substantiall)
    //or other more complex pre-multiplication
    auto pp_ax = &(std::get< POLPROD_AXIS >(*in));
    for(std::size_t i = 0; i < pp_ax->GetSize(); i++)
    {
        auto prefac = GetPrefactor(pp_ax->at(i));
        in->SubView(i) *= prefac;
    }
}

int MHO_CircularFieldRotationCorrection::DetermineMountCode(const std::string& mount) const
{
    static const std::unordered_map< std::string, MountType > kMountMap = {
        {"", MountType::None},
        {"no_mount", MountType::None},
        {"cassegrain", MountType::Cassegrain},
        {"nasmythleft", MountType::NasmythLeft},
        {"nasmythright", MountType::NasmythRight},
    };
    auto it = kMountMap.find(mount);
    if(it != kMountMap.end())
    {
        return static_cast< int >(it->second);
    }
    msg_error("calibration", "could not determine antenna mount type from: " << mount << eom);
    return static_cast< int >(MountType::None);
}

std::complex< double > MHO_CircularFieldRotationCorrection::GetPrefactor(std::string pp_label)
{
    //this is not super efficient, since we recalculate 4 factors for each pol-product
    //could reorganize this, but its probably not that critical right now
    std::complex< double > factor = 0;

    //if we cannot find this label in the set, return zero
    if(std::find(fPolProductSet.begin(), fPolProductSet.end(), pp_label) == fPolProductSet.end())
    {
        return factor;
    }

    std::complex< double > pp_factors[4];
    double par_angle[2];
    double elevation[2];
    MountType mount_type[2];

    //fill in station coords
    par_angle[0] = fRefParAngle * MHO_Constants::deg_to_rad;
    par_angle[1] = fRemParAngle * MHO_Constants::deg_to_rad;
    elevation[0] = fRefElevation * MHO_Constants::deg_to_rad;
    elevation[1] = fRemElevation * MHO_Constants::deg_to_rad;

    mount_type[0] = static_cast< MountType >(DetermineMountCode(fRefMountType));
    mount_type[1] = static_cast< MountType >(DetermineMountCode(fRemMountType));

    //call Geoff's routine
    compute_field_rotations_fixed(pp_factors, par_angle, elevation, mount_type);

    //return the value
    if(pp_label == "LL")
    {
        factor = pp_factors[static_cast< int >(PolProduct::LL)];
    }
    if(pp_label == "RR")
    {
        factor = pp_factors[static_cast< int >(PolProduct::RR)];
    }
    if(pp_label == "LR")
    {
        factor = pp_factors[static_cast< int >(PolProduct::LR)];
    }
    if(pp_label == "RL")
    {
        factor = pp_factors[static_cast< int >(PolProduct::RL)];
    }

    return factor;
}

} // namespace hops
