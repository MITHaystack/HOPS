#ifndef MHO_ClassIdentity_HH__
#define MHO_ClassIdentity_HH__

/*
*File: MHO_ClassIdentity.hh
*Class: MHO_ClassIdentity
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: template class to produce the name of XClassType at runtime
*/


#include <string>
#include "MHO_Types.hh"
#include "MHO_TemplateTypenameDeduction.hh"
#include "MHO_Message.hh"
#include "MHO_UUID.hh"
#include "MHO_MD5HashGenerator.hh"

namespace hops
{

typedef uint32_t MHO_ClassVersion;

struct MHO_ClassIdentity
{
    template<typename XClassType>
    static std::string ClassName(){ return MHO_ClassName<XClassType>(); };

    template<typename XClassType>
    static std::string ClassName(const XClassType&){ return MHO_ClassName<XClassType>(); };

    template<typename XClassType>
    static MHO_UUID GetUUIDFromClass()
    {
        std::string name = ClassName<XClassType>();
        return GetUUIDFromClassName(ClassName<XClassType>());
    }

    template<typename XClassType>
    static MHO_UUID GetUUIDFromClass(const XClassType&)
    {
        std::string name = ClassName<XClassType>();
        return GetUUIDFromClassName(ClassName<XClassType>());
    }

    static MHO_UUID GetUUIDFromClassName(std::string name)
    {
        MHO_MD5HashGenerator gen;
        gen.Initialize();
        gen << name;
        gen.Finalize();
        MHO_UUID type_uuid = gen.GetDigestAsUUID();
        return type_uuid;
    }

    //unknown version error for when an unknown or unsupported class version
    //is encountered
    template<typename XClassType>
    static void ClassVersionErrorMsg(const XClassType& obj, MHO_ClassVersion version)
    {
        msg_error("file", "Failed to stream object data for: " <<
                   MHO_ClassIdentity::ClassName(obj) <<
                   ", version: " << version << " is not recognized." << eom);
    };

};



}


#endif /* end of include guard:MHO_ClassIdentity */
