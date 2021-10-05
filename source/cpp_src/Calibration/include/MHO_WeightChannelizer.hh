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
#include "MHO_NDArrayOperator.hh"

#include "MHO_Visibilities.hh"
#include "MHO_ChannelizedVisibilities.hh"

namespace hops
{

class MHO_WeightChannelizer: public MHO_NDArrayOperator< baseline_weight_type, ch_baseline_weight_type>
{
    public:
        MHO_WeightChannelizer();
        virtual ~MHO_WeightChannelizer();

        virtual bool Initialize() override;
        virtual bool Execute() override;

    private:

        bool fInitialized;

};

}

#endif /* end of include guard: MHO_WeightChannelizer */