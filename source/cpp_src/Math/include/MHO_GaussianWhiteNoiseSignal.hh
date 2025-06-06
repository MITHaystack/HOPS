#ifndef MHO_GaussianWhiteNoiseSignal_HH__
#define MHO_GaussianWhiteNoiseSignal_HH__

#include "MHO_SimulatedSignalGenerator.hh"

#include <random>

namespace hops
{

/*!
 *@file MHO_GaussianWhiteNoiseSignal.hh
 *@class MHO_GaussianWhiteNoiseSignal
 *@date Mon Dec 19 16:33:05 2022 -0500
 *@brief
 *@author J. Barrett - barrettj@mit.edu
 */

class MHO_GaussianWhiteNoiseSignal: public MHO_SimulatedSignalGenerator
{
    public:
        MHO_GaussianWhiteNoiseSignal();
        ~MHO_GaussianWhiteNoiseSignal();

        void SetRandomSeed(unsigned int seed) { fSeed = std::mt19937::result_type(seed); }

        void SetMean(double mean) { fMean = mean; }

        void SetStandardDeviation(double std_dev) { fStandardDeviation = std::fabs(std_dev); };

        //implementation specific
        virtual void Initialize() override;

    protected:
        virtual bool GenerateSample(const double& /*!sample_time*/, double& sample) const override;

        double fMean;
        double fStandardDeviation;
        std::mt19937::result_type fSeed;
        std::mt19937* fGenerator;
        std::normal_distribution< double >* fDistribution;
};

} // namespace hops

#endif /*! end of include guard: MHO_GaussianWhiteNoiseSignal_HH__ */
