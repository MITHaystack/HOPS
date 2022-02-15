#ifndef MHO_ContainerLibrary_HH__
#define MHO_ContainerLibrary_HH__

/*
*@file: MHO_ContainerLibrary.hh
*@class: MHO_ContainerLibrary
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief: holds a collection of objects all pointed to by base class MHO_Serializable*
* retrival is through type/object ids
*/

#include <map>
#include <vector>

#include "MHO_Message.hh"
#include "MHO_Serializable.hh"
#include "MHO_FileKey.hh"
#include "MHO_UUID.hh"

namespace hops 
{

class MHO_ContainerLibrary
{
    public:

        MHO_ContainerLibrary();
        virtual ~MHO_ContainerLibrary();

        void Clear();

        //returns true if object successfully added, false if not added
        bool AddContainerObject(MHO_Serializable* obj, const MHO_FileKey& key);
        bool AddContainerObject(MHO_Serializable* obj, const std::string& type_uuid, const std::string& object_uuid);
        bool AddContainerObject(MHO_Serializable* obj, const MHO_UUID& type_uuid, const MHO_UUID& object_uuid);

        bool IsObjectPresent(const MHO_FileKey& key) const;
        bool IsObjectPresent(const std::string& type_uuid, const std::string& object_uuid) const;
        bool IsObjectPresent(const MHO_UUID& type_uuid, const MHO_UUID& object_uuid) const;

        //if retrieval fails, returns nullptr
        MHO_Serializable* RetrieveObject(const MHO_FileKey& key);
        MHO_Serializable* RetrieveObject(const std::string& type_uuid, const std::string& object_uuid);
        MHO_Serializable* RetrieveObject(const MHO_UUID& type_uuid, const MHO_UUID& object_uuid);

        MHO_Serializable* RetrieveFirstObjectMatchingType(const MHO_FileKey& key);
        MHO_Serializable* RetrieveFirstObjectMatchingType(const std::string& type_uuid);
        MHO_Serializable* RetrieveFirstObjectMatchingType(const MHO_UUID& type_uuid);

        std::size_t GetNObjects() const;
        std::size_t GetNObjectsOfType(const MHO_UUID& type_id) const;
        void GetAllTypeUUIDs(std::vector<MHO_UUID>& type_ids) const;
        void GetAllObjectUUIDsOfType(const MHO_UUID& type_id, std::vector<MHO_UUID>& obj_ids) const;


    protected:

        //first uuid key is for the object type, second uuid key is for the object itself
        std::map< MHO_UUID, std::map< MHO_UUID, MHO_Serializable* >  > fObjects;

};

} //end namespace

#endif /* end of include guard: MHO_ContainerLibrary */