#include "MHO_SingleToneSignal.hh"

#include <limits>

namespace hops
{

MHO_SingleToneSignal::MHO_SingleToneSignal():
    MHO_SimulatedSignalGenerator(),
    fPhaseOffset(0.0),
    fToneFrequency(1.0)
{};

MHO_SingleToneSignal::~MHO_SingleToneSignal(){};

bool
MHO_SingleToneSignal::GenerateSample(const double& sample_time, double& sample) const
{
    sample = std::sin(fPhaseOffset + 2.0*M_PI*fToneFrequency*sample_time);
    return true;
}


}//end namespace
