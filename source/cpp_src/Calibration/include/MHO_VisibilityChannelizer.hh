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
 *@date Thu Nov 5 13:11:33 2020 -0500
 *@brief  collects unchannelized (3d) visibility data and groups by channel (but all channels must be of equal size) into 4d object
 */

/**
 * @brief Class MHO_VisibilityChannelizer
 */
class MHO_VisibilityChannelizer: public MHO_TransformingOperator< uch_visibility_store_type, visibility_store_type >
{
    public:
        MHO_VisibilityChannelizer();
        virtual ~MHO_VisibilityChannelizer();

    protected:
        /**
         * @brief Reorganizes input visibility data into output array by channel and resizes it accordingly.
         * 
         * @param in Input visibility store data (rank = 3)
         * @param out Output visibility store data (rank = 4)
         * @return True if initialization is successful, false otherwise
         * @note This is a virtual function.
         */
        virtual bool InitializeImpl(const uch_visibility_store_type* in, visibility_store_type* out);
        /**
         * @brief Reorganizes visibility data into channelized format and updates channel labels.
         * 
         * @param in Input visibility store data (rank = 3)
         * @param out Output channelized visibility store data
         * @return True if execution is successful, false otherwise
         * @note This is a virtual function.
         */
        virtual bool ExecuteImpl(const uch_visibility_store_type* in, visibility_store_type* out);

    private:
        bool fInitialized;
};

} // namespace hops

#endif /*! end of include guard: MHO_VisibilityChannelizer */
