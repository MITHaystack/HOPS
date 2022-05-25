#ifndef MHO_ChannelizedRotationFunctor_HH__
#define MHO_ChannelizedRotationFunctor_HH__

/*
*File: MHO_ChannelizedRotationFunctor.hh
*Class: MHO_ChannelizedRotationFunctor
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <cmath>
#include <complex>


#include "MHO_NDArrayFunctor.hh"
#include "MHO_TableContainer.hh"
#include "MHO_ChannelizedVisibilities.hh"

namespace hops
{


class MHO_ChannelizedRotationFunctor: public MHO_NDArrayFunctor< ch_visibility_type, ch_visibility_type >
{
    public:

        using input_iterator = MHO_NDArrayFunctor< ch_visibility_type, ch_visibility_type >::input_iterator;
        using output_iterator = MHO_NDArrayFunctor< ch_visibility_type, ch_visibility_type >::output_iterator;


        MHO_ChannelizedRotationFunctor();
        virtual ~MHO_ChannelizedRotationFunctor();

        //set time/freq axes for the data
        void SetAxisPack(ch_baseline_axis_pack* axes);

        //set time/freq refernce points
        void SetReferenceFrequency(double ref_freq){fRefFrequency = ref_freq;};
        void SetReferenceTime(double ref_time){fRefTime = ref_time;};

        //set the delay-rate and the single band-delay for computing the rotation
        //FIXME => UNITS??!
        void SetDelayRate(const double& dr){fDelayRate = dr;};
        void SetSingleBandDelay(const double& sbd){fSingleBandDelay = sbd;}

        //rotation operator
        virtual void operator() ( input_iterator& input, output_iterator& output) override
        {
            //first get the indices of the input iterator
            const std::size_t* in_loc = input.GetIndices();

            //then retrieve the time/frequency values of the axis
            double time = (*fTimeAxis)[in_loc[CH_TIME_AXIS]];
            double freq = (*fFreqAxis)[in_loc[CH_FREQ_AXIS]];

            //then calculate the complex rotation (a la vrot.c, though more primitive)
            //TODO --- think more carefully about this....
            double dT = time - fRefTime;
            double dF = freq - fRefFrequency;
            double theta = -2.0*M_PI*dF*(fSingleBandDelay + fDelayRate*dT);
            std::complex<double> rot = std::exp(cI*theta);

            //then multiply the input by rotation and set the output
            *output = (*input)*rot;
        }

    private:

        static const std::complex<double> cI;

        double fRefFrequency;
        double fRefTime;
        double fDelayRate;
        double fSingleBandDelay;

        MHO_Axis<double>* fTimeAxis;
        MHO_Axis<double>* fFreqAxis;



};


}//end of hops namespace

#endif /* end of include guard: MHO_ChannelizedRotationFunctor */
