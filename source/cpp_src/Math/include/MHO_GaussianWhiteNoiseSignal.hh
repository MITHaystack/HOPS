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

        /**
         * @brief Setter for random seed
         *
         * @param seed New seed value to initialize the random number generator.
         */
        void SetRandomSeed(unsigned int seed) { fSeed = std::mt19937::result_type(seed); }

        /**
         * @brief Setter for mean
         *
         * @param mean New mean value to set
         */
        void SetMean(double mean) { fMean = mean; }

        /**
         * @brief Setter for standard deviation
         *
         * @param std_dev Input standard deviation value.
         */
        void SetStandardDeviation(double std_dev) { fStandardDeviation = std::fabs(std_dev); };

        //implementation specific
        /**
         * @brief Initializes random number generator and normal distribution for Gaussian white noise signal.
         * @note This is a virtual function.
         */
        virtual void Initialize() override;

    protected:
        /**
         * @brief Generates a sample from Gaussian white noise distribution.
         *
         * @param !sample_time Parameter description
         * @param sample Output sampled value (reference).
         * @return True if sample generation is successful.
         * @note This is a virtual function.
         */
        virtual bool GenerateSample(const double& /*!sample_time*/, double& sample) const override;

        double fMean;
        double fStandardDeviation;
        std::mt19937::result_type fSeed;
        std::mt19937* fGenerator;
        std::normal_distribution< double >* fDistribution;
};

} // namespace hops

#endif /*! end of include guard: MHO_GaussianWhiteNoiseSignal_HH__ */
