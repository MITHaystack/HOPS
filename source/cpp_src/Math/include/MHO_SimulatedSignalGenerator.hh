#ifndef MHO_SimulatedSignalGenerator_HH__
#define MHO_SimulatedSignalGenerator_HH__

#include <cmath>

namespace hops
{

/*!
 *@file MHO_SimulatedSignalGenerator.hh
 *@class MHO_SimulatedSignalGenerator
 *@date Mon Dec 19 16:33:05 2022 -0500
 *@brief
 *@author J. Barrett - barrettj@mit.edu
 */

class MHO_SimulatedSignalGenerator
{
    public:
        MHO_SimulatedSignalGenerator(): fSamplingFrequency(0.0){};
        virtual ~MHO_SimulatedSignalGenerator(){};

        //tells the sample generation implementation what the expected the sampling frequency is
        virtual void SetSamplingFrequency(double sample_freq) { fSamplingFrequency = std::fabs(sample_freq); };

        virtual double GetSamplingFrequency() const { return fSamplingFrequency; };

        //implementation specific
        virtual void Initialize() = 0;

        bool GetSample(const double& sample_time, double& sample) const { return GenerateSample(sample_time, sample); }

    protected:
        virtual bool GenerateSample(const double& sample_time, double& sample) const = 0;

        //some specific distributions and their implementations (e.g. colored noise)
        //need to know the expected sampling frequency in advance
        double fSamplingFrequency;
};

} // namespace hops

#endif /*! MHO_SimulatedSignalGenerator_H__ */
