#ifndef MHO_VisibilityChannelizer_HH__
#define MHO_VisibilityChannelizer_HH__



#include "MHO_NDArrayWrapper.hh"
#include "MHO_TransformingOperator.hh"

#include "MHO_ContainerDefinitions.hh"

namespace hops
{

/*!
*@file MHO_VisibilityChannelizer.hh
*@class MHO_VisibilityChannelizer
*@author J. Barrett - barrettj@mit.edu
*@date
*@brief  collects unchannelized (3d) visibility data and groups by channel (but all must be of equal size) into 4d object
*/

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

#endif /*! end of include guard: MHO_VisibilityChannelizer */
