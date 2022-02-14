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
#include "MHO_UUID.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_SerializableObjectFactory.hh"

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
            //add a factory for these types of objects
            auto it = fFactoryMap.find(type_uuid);
            if( it == fFactoryMap.end())
            {
                fFactoryMap.emplace(type_uuid, new MHO_SerializableObjectFactorySpecific<XClassType>() );
            }
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
            //add a factory for these types of objects
            auto it = fFactoryMap.find(type_uuid);
            if( it == fFactoryMap.end())
            {
                fFactoryMap.emplace(type_uuid, new MHO_SerializableObjectFactorySpecific<XClassType>() );
            }
        };


        template<typename XClassType>
        std::string GetClassNameFromObject(const XClassType& obj)
        {
            std::string name = MHO_ClassIdentity::ClassName(obj);
            return name;
        };

        std::string GetClassNameFromUUID(const MHO_UUID& uuid) const
        {
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

        template<typename XClassType>
        MHO_UUID GetUUIDFor() const
        {
            std::string name = MHO_ClassIdentity::ClassName<XClassType>();
            return GetUUIDFromClassName(name);
        }

        bool IsTypePresent(const MHO_UUID& uuid) const
        {
            auto it = fUUID2ClassName.find(uuid);
            if(it != fUUID2ClassName.end()){return true;}
            return false;
        }

        //returns a ptr to base class MHO_Serializable, but the underlying type 
        //is that which is associated with the uuid, if the uuid is not in the
        //factory map then nullptr is returned, memory managment is delegated to
        //the caller
        MHO_Serializable* GenerateContainerFromUUID(const MHO_UUID& uuid)
        {
            auto it = fFactoryMap.find(uuid);
            if( it != fFactoryMap.end() )
            {
                return fFactoryMap[uuid]->Build();
            }
            else{return nullptr;}
        }


    protected:

        void AddToMap(const MHO_UUID& type_uuid, const std::string& name)
        {
            fUUID2ClassName[type_uuid] = name;
            fClassName2UUID[name] = type_uuid;
        };

        MHO_MD5HashGenerator fMD5Generator;
        std::map< MHO_UUID, std::string > fUUID2ClassName;
        std::map< std::string, MHO_UUID > fClassName2UUID;
        std::map< MHO_UUID, MHO_SerializableObjectFactory* > fFactoryMap;

};

}

#endif /* end of include guard: MHO_ClassIdentityMap */
