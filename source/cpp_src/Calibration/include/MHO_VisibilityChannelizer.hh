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

class MHO_VisibilityChannelizer: public MHO_TransformingOperator< visibility_type, ch_visibility_type>
{
    public:
        MHO_VisibilityChannelizer();
        virtual ~MHO_VisibilityChannelizer();

    protected:

        virtual bool InitializeImpl(const visibility_type* in, ch_visibility_type* out);
        virtual bool ExecuteImpl(const visibility_type* in, ch_visibility_type* out);

    private:

        bool fInitialized;

};

}

#endif /* end of include guard: MHO_VisibilityChannelizer */
