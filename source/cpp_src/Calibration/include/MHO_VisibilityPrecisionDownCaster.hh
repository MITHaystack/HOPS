#ifndef MHO_VisibilityPrecisionDownCaster_HH__
#define MHO_VisibilityPrecisionDownCaster_HH__

/*
*File: MHO_VisibilityPrecisionDownCaster.hh
*Class: MHO_VisibilityPrecisionDownCaster
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

class MHO_VisibilityPrecisionDownCaster: public MHO_TransformingOperator< visibility_type, visibility_store_type>
{
    public:
        MHO_VisibilityPrecisionDownCaster();
        virtual ~MHO_VisibilityPrecisionDownCaster();

    protected:

        virtual bool InitializeImpl(const visibility_type* /*in*/, visibility_store_type* /*out*/){return true;}; //no op
        virtual bool ExecuteImpl(const visibility_type* in, visibility_store_type* out);

    private:


};

}

#endif /* end of include guard: MHO_VisibilityPrecisionDownCaster */
