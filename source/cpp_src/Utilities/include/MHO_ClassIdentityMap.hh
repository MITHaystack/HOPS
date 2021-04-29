#ifndef MHO_ClassIdentityMap_HH__
#define MHO_ClassIdentityMap_HH__

/*
*File: MHO_ClassIdentityMap.hh
*Class: MHO_ClassIdentityMap
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include "MHO_ClassIdentity.hh"
#include "MHO_MD5HashGenerator.hh"

#include <map>
#include <string>
#include <iostream>

namespace hops
{

class MHO_ClassIdentityMap
{
    public:

        MHO_ClassIdentityMap(){};
        virtual ~MHO_ClassIdentityMap(){};

        template<typename XClassType>
        void AddClassType()
        {
            fMD5Generator.Initialize();
            std::string name = MHO_ClassIdentity::ClassName<XClassType>();
            fMD5Generator << name;
            fMD5Generator.Finalize();
            MHO_UUID type_uuid = fMD5Generator.GetDigestAsUUID();
            AddToMap(type_uuid, name);
        };

        template<typename XClassType>
        void AddClassType(const XClassType& obj)
        {
            fMD5Generator.Initialize();
            std::string name = MHO_ClassIdentity::ClassName(obj);
            fMD5Generator << name;
            fMD5Generator.Finalize();
            MHO_UUID type_uuid = fMD5Generator.GetDigestAsUUID();
            AddToMap(type_uuid, name);
        };

        std::string GetClassNameFromUUID(const MHO_UUID& uuid) const
        {
            // const auto it = fUUID2ClassName.cend();

            std::map< MHO_UUID, std::string >::const_iterator it = fUUID2ClassName.find(uuid);
            if( it != fUUID2ClassName.end() )
            {
                return std::string(it->second);
            }
            else
            {
                return std::string("unknown");
            }
        };

        MHO_UUID GetUUIDFromClassName(const std::string& name) const
        {
            MHO_UUID tmp;
            const auto it = fClassName2UUID.find(name);
            if( it != fClassName2UUID.end() )
            {
                tmp = it->second;
            }
            return tmp;
        };


    protected:

        void AddToMap(const MHO_UUID& type_uuid, const std::string& name)
        {
            fUUID2ClassName[type_uuid] = name;
            fClassName2UUID[name] = type_uuid;
        }

        MHO_MD5HashGenerator fMD5Generator;
        std::map< MHO_UUID, std::string > fUUID2ClassName;
        std::map< std::string, MHO_UUID > fClassName2UUID;


};

}

#endif /* end of include guard: MHO_ClassIdentityMap */
