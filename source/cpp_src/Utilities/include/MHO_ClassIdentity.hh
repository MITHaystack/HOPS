#ifndef MHO_ClassIdentity_HH__
#define MHO_ClassIdentity_HH__

#include "MHO_MD5HashGenerator.hh"
#include "MHO_Message.hh"
#include "MHO_TemplateTypenameDeduction.hh"
#include "MHO_Types.hh"
#include "MHO_UUID.hh"
#include <string>

namespace hops
{

/*!
 *@file MHO_ClassIdentity.hh
 *@class MHO_ClassIdentity
 *@author J. Barrett - barrettj@mit.edu
 *@date Wed Apr 21 13:40:18 2021 -0400
 *@brief template class to produce the name of XClassType at runtime
 */

typedef uint32_t MHO_ClassVersion;

/**
 * @brief Class MHO_ClassIdentity
 */
struct MHO_ClassIdentity
{

        /**
         * @brief Returns the class name as a string.
         *
         * @tparam XClassType The type of the class for which the name is requested.
         * @return std::string The name of the class.
         */
        template< typename XClassType > static std::string ClassName() { return MHO_ClassName< XClassType >(); };

        /**
         * @brief Returns the class name as a string.
         *
         * @tparam XClassType The type of the class for which the name is requested.
         * @param obj An instance of the class (unused in function).
         * @return std::string The name of the class.
         */
        template< typename XClassType > static std::string ClassName(const XClassType&)
        {
            return MHO_ClassName< XClassType >();
        };

        /**
         * @brief Returns the class name as a string.
         *
         * @tparam XClassType The type of the class for which the name is requested.
         * @param obj An instance of the class (unused in function).
         * @return std::string The name of the class.
         */
        template< typename XClassType > static MHO_UUID GetUUIDFromClass()
        {
            std::string name = ClassName< XClassType >();
            return GetUUIDFromClassName(ClassName< XClassType >());
        }

        /**
         * @brief Returns the UUID associated with a class type.
         *
         * @tparam XClassType The type of the class for which the UUID is requested.
         * @param obj An instance of the class (unused in function).
         * @return MHO_UUID The UUID of the class.
         */
        template< typename XClassType > static MHO_UUID GetUUIDFromClass(const XClassType&)
        {
            std::string name = ClassName< XClassType >();
            return GetUUIDFromClassName(ClassName< XClassType >());
        }

        /**
         * @brief Generates a UUID from the class name.
         *
         * @param name The name of the class.
         * @return MHO_UUID The generated UUID for the class name.
         */
        static MHO_UUID GetUUIDFromClassName(std::string name)
        {
            MHO_MD5HashGenerator gen;
            gen.Initialize();
            gen << name;
            gen.Finalize();
            MHO_UUID type_uuid = gen.GetDigestAsUUID();
            return type_uuid;
        }

        /**
         * @brief Generates an error message for when an unknown or unsupported class version is encountered
         *
         * @tparam XClassType The type of the class for which the error message is generated.
         * @param obj An instance of the class.
         * @param version The unsupported class version.
         */
        template< typename XClassType > static void ClassVersionErrorMsg(const XClassType& obj, MHO_ClassVersion version)
        {
            msg_error("file", "Failed to stream object data for: " << MHO_ClassIdentity::ClassName(obj)
                                                                   << ", version: " << version << " is not recognized." << eom);
        };
};

} // namespace hops

#endif /*! end of include guard:MHO_ClassIdentity */
