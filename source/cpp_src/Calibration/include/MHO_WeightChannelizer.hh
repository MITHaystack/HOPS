#ifndef MHO_WeightChannelizer_HH__
#define MHO_WeightChannelizer_HH__



#include "MHO_NDArrayWrapper.hh"
#include "MHO_TransformingOperator.hh"

#include "MHO_ContainerDefinitions.hh"

namespace hops
{

/*!
*@file MHO_WeightChannelizer.hh
*@class MHO_WeightChannelizer
*@author J. Barrett - barrettj@mit.edu
*@date
*@brief collects unchannelized (3d) weight data and groups by channel (but all must be of equal size) into 4d object
*/

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

#endif /*! end of include guard: MHO_WeightChannelizer */
