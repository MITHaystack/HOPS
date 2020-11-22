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

#include "MHOArrayFunctor.hh"
#include "MHOTensorContainer.hh"
#include "MHOChannelizedVisibilities.hh"




class MHOVChannelizedRotationFunctor: public MHOArrayFunctor< ch_baseline_data_type, ch_baseline_data_type >
{
    public:

        MHOChannelizedRotationFunctor()
        {

        }

        virtual ~MHOChannelizedRotationFunctor(){};


        void SetReferenceFrequency(double ref_freq){fRefFrequency = ref_freq;};

        void SetAxisPack(ch_baseline_axis_pack* axes){fAxes = axis;};

        virtual void operator( input_iterator& input, output_iterator& output) override
        {
            //first get the indices of the input iterator
            const std::size_t* in_loc = input.GetIndices();

            //then retrieve the time/frequency values of the axis
            double time = (std:get<CH_TIME_AXIS>(fAxes))[in_loc[CH_TIME_AXIS]];
            double freq = (std::get<CH_FREQ_AXIS>(fAxes))[in_loc[CH)CH_FREQ_AXIS]];

            //then calculate the complex rotation (a la vrot.c, though more primitive)

            //then multiply the input by rotation and set the output


        }

    private:

        double fRefFrequency;
        ch_baseline_axis_pack* fAxes;


};

#endif /* end of include guard: MHOChannelizedRotationFunctor */
