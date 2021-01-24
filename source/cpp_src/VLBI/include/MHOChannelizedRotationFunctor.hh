#ifndef MHOChannelizedRotationFunctor_HH__
#define MHOChannelizedRotationFunctor_HH__

/*
*File: MHOChannelizedRotationFunctor.hh
*Class: MHOChannelizedRotationFunctor
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <cmath>
#include <complex>


#include "MHONDArrayFunctor.hh"
#include "MHOTableContainer.hh"
#include "MHOChannelizedVisibilities.hh"

namespace hops
{


class MHOChannelizedRotationFunctor: public MHONDArrayFunctor< ch_baseline_data_type, ch_baseline_data_type >
{
    public:

        using input_iterator = MHONDArrayFunctor< ch_baseline_data_type, ch_baseline_data_type >::input_iterator;
        using output_iterator = MHONDArrayFunctor< ch_baseline_data_type, ch_baseline_data_type >::output_iterator;


        MHOChannelizedRotationFunctor();
        virtual ~MHOChannelizedRotationFunctor();

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

        MHOAxis<double>* fTimeAxis;
        MHOAxis<double>* fFreqAxis;



};


}//end of hops namespace

#endif /* end of include guard: MHOChannelizedRotationFunctor */
