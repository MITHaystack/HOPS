#ifndef MHO_SingleToneSignal_HH__
#define MHO_SingleToneSignal_HH__

#include "MHO_SimulatedSignalGenerator.hh"

namespace hops
{

/*!
 *@file MHO_SingleToneSignal.hh
 *@class MHO_SingleToneSignal
 *@date Mon Dec 19 16:33:05 2022 -0500
 *@brief
 *@author J. Barrett - barrettj@mit.edu
 */

class MHO_SingleToneSignal: public MHO_SimulatedSignalGenerator
{
    public:
        MHO_SingleToneSignal();
        ~MHO_SingleToneSignal();

        /**
         * @brief Setter for phase offset
         *
         * @param phase_offset New phase offset value to be applied
         */
        void SetPhaseOffset(double phase_offset) { fPhaseOffset = phase_offset; } //radians

        /**
         * @brief Setter for tone frequency
         *
         * @param tone_freq Frequency value in Hertz
         */
        void SetToneFrequency(double tone_freq) { fToneFrequency = tone_freq; };

        //implementation specific
        /**
         * @brief Function Initialize
         * @note This is a virtual function.
         */
        virtual void Initialize(){};

    protected:
        /**
         * @brief Generates a sample using sine wave formula based on input time and frequency.
         *
         * @param !sample_time Parameter description
         * @param sample (double&)
         * @return True indicating successful generation of the sample.
         * @note This is a virtual function.
         */
        virtual bool GenerateSample(const double& /*!sample_time*/, double& sample) const override;

        double fPhaseOffset;
        double fToneFrequency;
};

} // namespace hops

#endif /*! end of include guard: MHO_SingleToneSignal_HH__ */
