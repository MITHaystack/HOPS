#ifndef MHO_WeightPrecisionDownCaster_HH__
#define MHO_WeightPrecisionDownCaster_HH__

/*
*File: MHO_WeightPrecisionDownCaster.hh
*Class: MHO_WeightPrecisionDownCaster
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

class MHO_WeightPrecisionDownCaster: public MHO_TransformingOperator< weight_type, weight_store_type>
{
    public:
        MHO_WeightPrecisionDownCaster();
        virtual ~MHO_WeightPrecisionDownCaster();

    protected:

        virtual bool InitializeImpl(const weight_type* /*in*/, weight_store_type* /*out*/){return true;}; //no op
        virtual bool ExecuteImpl(const weight_type* in, weight_store_type* out);

    private:


};

}

#endif /* end of include guard: MHO_WeightPrecisionDownCaster */
