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
 *@brief Abstract base class for all serializable objects
 */

/**
 * @brief Class MHO_Serializable
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

        /**
         * @brief Getter for version
         *
         * @return MHO_ClassVersion version number.
         * @note This is a virtual function.
         */
        virtual MHO_ClassVersion GetVersion() const { return 0; };

        /**
         * @brief Getter for serialized size
         *
         * @return Return value (uint64_t)
         * @note This is a virtual function.
         */
        virtual uint64_t GetSerializedSize() const = 0;

        /**
         * @brief Getter for object uuid
         *
         * @return MHO_UUID: The unique identifier (UUID) of the object.
         */
        MHO_UUID GetObjectUUID() const { return fObjectUUID; };

        /**
         * @brief Setter for object uuid
         *
         * @param uuid The new UUID value to set for the object.
         */
        void SetObjectUUID(const MHO_UUID& uuid) { fObjectUUID = uuid; };

        /**
         * @brief Getter for type uuid
         *
         * @return MHO_UUID representing the type's universally unique identifier.
         * @note This is a virtual function.
         */
        virtual MHO_UUID GetTypeUUID() const
        {
            if(fTypeUUID.is_empty())
            {
                fTypeUUID = DetermineTypeUUID();
            }
            return fTypeUUID;
        }

    private:
        /**
         * @brief Function DetermineTypeUUID
         *
         * @return Return value (MHO_UUID)
         * @note This is a virtual function.
         */
        virtual MHO_UUID DetermineTypeUUID() const = 0;

        MHO_UUID fObjectUUID;
        mutable MHO_UUID fTypeUUID;
};

} // namespace hops

#endif /*! end of include guard: MHO_Serializable */
