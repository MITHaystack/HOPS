#include "MHO_PhaseCalibrationTrim.hh"

namespace hops
{

MHO_PhaseCalibrationTrim::MHO_PhaseCalibrationTrim()
{

};

MHO_PhaseCalibrationTrim::~MHO_PhaseCalibrationTrim(){};

bool MHO_PhaseCalibrationTrim::ExecuteInPlace(visibility_type* in)
{
    return true;
}

bool MHO_PhaseCalibrationTrim::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}

bool MHO_PhaseCalibrationTrim::InitializeInPlace(visibility_type* /*in*/)
{
    return true;
}

bool MHO_PhaseCalibrationTrim::InitializeOutOfPlace(const visibility_type* /*in*/, visibility_type* /*out*/)
{
    return true;
}

} // namespace hops
