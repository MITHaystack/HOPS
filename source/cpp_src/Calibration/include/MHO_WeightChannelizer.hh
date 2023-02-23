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


class MHO_WeightChannelizer: public MHO_TransformingOperator< uch_weight_store_type, weight_store_type>
{
    public:
        MHO_WeightChannelizer();
        virtual ~MHO_WeightChannelizer();

    private:

        virtual bool InitializeImpl(const uch_weight_store_type* in, weight_store_type* out);
        virtual bool ExecuteImpl(const uch_weight_store_type* in, weight_store_type* out);

        bool fInitialized;

};

}

#endif /* end of include guard: MHO_WeightChannelizer */
