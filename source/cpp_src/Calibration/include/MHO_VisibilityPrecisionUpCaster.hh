#ifndef MHO_VisibilityPrecisionUpCaster_HH__
#define MHO_VisibilityPrecisionUpCaster_HH__

/*
*File: MHO_VisibilityPrecisionUpCaster.hh
*Class: MHO_VisibilityPrecisionUpCaster
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

class MHO_VisibilityPrecisionUpCaster: public MHO_TransformingOperator< visibility_store_type, visibility_type>
{
    public:
        MHO_VisibilityPrecisionUpCaster();
        virtual ~MHO_VisibilityPrecisionUpCaster();

    protected:

        virtual bool InitializeImpl(const visibility_store_type* /*in*/, visibility_type* /*out*/){return true;}; //no op
        virtual bool ExecuteImpl(const visibility_store_type* in, visibility_type* out);

    private:


};

}

#endif /* end of include guard: MHO_VisibilityPrecisionUpCaster */
