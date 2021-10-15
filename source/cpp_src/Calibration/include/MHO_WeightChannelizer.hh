#ifndef MHO_WeightChannelizer_HH__
#define MHO_WeightChannelizer_HH__

/*
*File: MHO_WeightChannelizer.hh
*Class: MHO_WeightChannelizer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include "MHO_NDArrayWrapper.hh"
#include "MHO_TransformingOperator.hh"

#include "MHO_Visibilities.hh"
#include "MHO_ChannelizedVisibilities.hh"

namespace hops
{


class MHO_WeightChannelizer: public MHO_TransformingOperator< baseline_weight_type, ch_baseline_weight_type>
{
    public:
        MHO_WeightChannelizer();
        virtual ~MHO_WeightChannelizer();

    private:

        virtual bool InitializeImpl(const baseline_weight_type* in, ch_baseline_weight_type* out);
        virtual bool ExecuteImpl(const baseline_weight_type* in, ch_baseline_weight_type* out);

        bool fInitialized;

};

}

#endif /* end of include guard: MHO_WeightChannelizer */
