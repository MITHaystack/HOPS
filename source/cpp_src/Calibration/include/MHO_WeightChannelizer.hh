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

#include "MHO_ContainerDefinitions.hh"

namespace hops
{


class MHO_WeightChannelizer: public MHO_TransformingOperator< weight_store_type, ch_weight_store_type>
{
    public:
        MHO_WeightChannelizer();
        virtual ~MHO_WeightChannelizer();

    private:

        virtual bool InitializeImpl(const weight_store_type* in, ch_weight_store_type* out);
        virtual bool ExecuteImpl(const weight_store_type* in, ch_weight_store_type* out);

        bool fInitialized;

};

}

#endif /* end of include guard: MHO_WeightChannelizer */
