#ifndef MHO_VisibilityChannelizer_HH__
#define MHO_VisibilityChannelizer_HH__

/*
*File: MHO_VisibilityChannelizer.hh
*Class: MHO_VisibilityChannelizer
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

class MHO_VisibilityChannelizer: public MHO_TransformingOperator< uch_visibility_store_type, visibility_store_type>
{
    public:
        MHO_VisibilityChannelizer();
        virtual ~MHO_VisibilityChannelizer();

    protected:

        virtual bool InitializeImpl(const uch_visibility_store_type* in, visibility_store_type* out);
        virtual bool ExecuteImpl(const uch_visibility_store_type* in, visibility_store_type* out);

    private:

        bool fInitialized;

};

}

#endif /* end of include guard: MHO_VisibilityChannelizer */
