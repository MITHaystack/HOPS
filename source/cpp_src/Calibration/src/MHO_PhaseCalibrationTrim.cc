#include "MHO_PhaseCalibrationTrim.hh"

namespace hops
{

MHO_PhaseCalibrationTrim::MHO_PhaseCalibrationTrim()
{

};

MHO_PhaseCalibrationTrim::~MHO_PhaseCalibrationTrim(){};

bool MHO_PhaseCalibrationTrim::ExecuteInPlace(multitone_pcal_type* in)
{
    



    return true;
}

bool MHO_PhaseCalibrationTrim::ExecuteOutOfPlace(const multitone_pcal_type* in, multitone_pcal_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}

bool MHO_PhaseCalibrationTrim::InitializeInPlace(multitone_pcal_type* /*in*/)
{
    return true;
}

bool MHO_PhaseCalibrationTrim::InitializeOutOfPlace(const multitone_pcal_type* /*in*/, multitone_pcal_type* /*out*/)
{
    return true;
}

} // namespace hops
