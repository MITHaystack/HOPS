#ifndef MHO_Serializable_HH__
#define MHO_Serializable_HH__

#include "MHO_ClassIdentity.hh"
#include "MHO_FileStreamer.hh"
#include "MHO_Types.hh"

#include "MHO_UUID.hh"
#include "MHO_UUIDGenerator.hh"

namespace hops
{

/*!
 *@file MHO_Serializable.hh
 *@class MHO_Serializable
 *@author J. Barrett - barrettj@mit.edu
 *@date Wed Apr 21 13:40:18 2021 -0400
 *@brief
 */

class MHO_Serializable
{
    public:
        MHO_Serializable()
        {
            MHO_UUIDGenerator gen;
            fObjectUUID = gen.GenerateUUID();
        };

        MHO_Serializable(const MHO_UUID& uuid) { fObjectUUID = uuid; };

        MHO_Serializable(std::size_t n): fObjectUUID(){};

        virtual ~MHO_Serializable(){};

        virtual MHO_ClassVersion GetVersion() const { return 0; };

        virtual uint64_t GetSerializedSize() const = 0;

        MHO_UUID GetObjectUUID() const { return fObjectUUID; };

        void SetObjectUUID(const MHO_UUID& uuid) { fObjectUUID = uuid; };

        virtual MHO_UUID GetTypeUUID() const
        {
            if(fTypeUUID.is_empty())
            {
                fTypeUUID = DetermineTypeUUID();
            }
            return fTypeUUID;
        }

    private:
        virtual MHO_UUID DetermineTypeUUID() const = 0;

        MHO_UUID fObjectUUID;
        mutable MHO_UUID fTypeUUID;
};

} // namespace hops

#endif /*! end of include guard: MHO_Serializable */
