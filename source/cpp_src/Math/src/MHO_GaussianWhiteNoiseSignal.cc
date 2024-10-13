#include "MHO_GaussianWhiteNoiseSignal.hh"

#include <limits>

namespace hops
{

MHO_GaussianWhiteNoiseSignal::MHO_GaussianWhiteNoiseSignal()
    : MHO_SimulatedSignalGenerator(), fMean(0.0), fStandardDeviation(1.0), fSeed(0), fGenerator(nullptr),
      fDistribution(nullptr){};

MHO_GaussianWhiteNoiseSignal::~MHO_GaussianWhiteNoiseSignal()
{
    delete fGenerator;
    delete fDistribution;
};

void MHO_GaussianWhiteNoiseSignal::Initialize()
{
    if(fGenerator != nullptr)
    {
        delete fGenerator;
    }
    if(fDistribution != nullptr)
    {
        delete fDistribution;
    }
    fGenerator = new std::mt19937(fSeed);
    fDistribution = new std::normal_distribution< double >(fMean, fStandardDeviation);
}

bool MHO_GaussianWhiteNoiseSignal::GenerateSample(const double& /*sample_time*/, double& sample) const
{
    sample = (*fDistribution)(*fGenerator);
    return true;
}

} // namespace hops
