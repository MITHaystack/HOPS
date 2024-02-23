#ifndef MHO_SingleToneSignal_HH__
#define MHO_SingleToneSignal_HH__

/*!
*@file MHO_SingleToneSignal.hh
*@class MHO_SingleToneSignal
*@date
*@brief
*@author J. Barrett - barrettj@mit.edu
*/

#include "MHO_SimulatedSignalGenerator.hh"

namespace hops
{

class MHO_SingleToneSignal: public MHO_SimulatedSignalGenerator
{
    public:
        MHO_SingleToneSignal();
        ~MHO_SingleToneSignal();

        void SetPhaseOffset(double phase_offset){fPhaseOffset = phase_offset;} //radians
        void SetToneFrequency(double tone_freq){fToneFrequency = tone_freq;};

        //implementation specific
        virtual void Initialize(){};

    protected:

        virtual bool GenerateSample(const double& /*!sample_time*/, double& sample) const override;

        double fPhaseOffset;
        double fToneFrequency;
};


}

#endif /*! end of include guard: MHO_SingleToneSignal_HH__ */
