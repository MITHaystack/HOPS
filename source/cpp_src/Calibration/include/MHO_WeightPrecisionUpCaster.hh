#ifndef MHO_WeightPrecisionUpCaster_HH__
#define MHO_WeightPrecisionUpCaster_HH__

/*
*File: MHO_WeightPrecisionUpCaster.hh
*Class: MHO_WeightPrecisionUpCaster
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: Class which up-cast the float-precision storage class to double-precision
*/

#include "MHO_NDArrayWrapper.hh"
#include "MHO_TransformingOperator.hh"

#include "MHO_ContainerDefinitions.hh"

namespace hops
{

class MHO_WeightPrecisionUpCaster: public MHO_TransformingOperator< ch_weight_store_type, ch_weight_type>
{
    public:
        MHO_WeightPrecisionUpCaster();
        virtual ~MHO_WeightPrecisionUpCaster();

    protected:

        virtual bool InitializeImpl(const ch_weight_store_type* /*in*/, ch_weight_type* /*out*/){return true;}; //no op
        virtual bool ExecuteImpl(const ch_weight_store_type* in, ch_weight_type* out);

    private:


};

}

#endif /* end of include guard: MHO_WeightPrecisionUpCaster */
