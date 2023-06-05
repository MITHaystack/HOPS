#ifndef MHO_ContainerStore_HH__
#define MHO_ContainerStore_HH__

/*
*@file: MHO_ContainerStore.hh
*@class: MHO_ContainerStore
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief: holds a collection of objects all pointed to by base class MHO_Serializable*
* retrival is through type/object ids
*/

#include <map>
#include <utility>
#include <vector>

#include "MHO_Message.hh"
#include "MHO_Serializable.hh"
#include "MHO_FileKey.hh"
#include "MHO_UUID.hh"
#include "MHO_ContainerDictionary.hh"

namespace hops 
{

class MHO_ContainerStore
{
    public:

        MHO_ContainerStore();
        virtual ~MHO_ContainerStore();

        void Clear();

        //returns true if object successfully added, false if not added
        bool AddContainerObject(MHO_Serializable* obj, const MHO_FileKey& key);

        bool AddContainerObject(MHO_Serializable* obj, 
                                const std::string& type_uuid, 
                                const std::string& object_uuid,
                                std::string shortname = "",
                                uint32_t label = 0);

        bool AddContainerObject(MHO_Serializable* obj, 
                                const MHO_UUID& type_uuid, 
                                const MHO_UUID& object_uuid,
                                std::string shortname = "",
                                uint32_t label = 0);

        bool IsObjectPresent(const MHO_FileKey& key) const;
        bool IsObjectPresent(const std::string& type_uuid, const std::string& object_uuid) const;
        bool IsObjectPresent(const MHO_UUID& type_uuid, const MHO_UUID& object_uuid) const;

        //if retrieval fails, returns nullptr
        MHO_Serializable* RetrieveObject(const MHO_FileKey& key);
        MHO_Serializable* RetrieveObject(const std::string& type_uuid, const std::string& object_uuid);
        MHO_Serializable* RetrieveObject(const MHO_UUID& type_uuid, const MHO_UUID& object_uuid);

        template < typename XClassType > 
        XClassType* RetrieveObject(std::string object_uuid = "");
    
        //just grab the first object of a specific type, nullptr on fail
        MHO_Serializable* RetrieveFirstObjectMatchingType(const MHO_FileKey& key);
        MHO_Serializable* RetrieveFirstObjectMatchingType(const std::string& type_uuid);
        MHO_Serializable* RetrieveFirstObjectMatchingType(const MHO_UUID& type_uuid);

        std::pair<std::string, uint32_t> GetObjectNameLabel(const MHO_UUID& type_uuid, const MHO_UUID& object_uuid) const;

        std::size_t GetNObjects() const;
        std::size_t GetNObjectsOfType(const MHO_UUID& type_id) const;

        void GetAllTypeUUIDs(std::vector<MHO_UUID>& type_ids) const;
        void GetAllObjectUUIDsOfType(const MHO_UUID& type_id, std::vector<MHO_UUID>& obj_ids) const;

        //destroy an object in the store
        template < typename XClassType > void DeleteObject(XClassType* obj_ptr);
        void DeleteObject(const MHO_FileKey& key);
        void DeleteObject(const std::string& type_uuid, const std::string& object_uuid);
        void DeleteObject(const MHO_UUID& type_uuid, const MHO_UUID& object_uuid);


    protected:

        //object dictionary...currently we only have one dictionary implementation
        //however, we may in the future want to allow the user to pass a custom dictionary implementation 
        //if they have additional classes they want to serialize which are not already supported 
        MHO_ContainerDictionary fDictionary;

        //first uuid key is for the object type, second uuid key is for the object itself
        std::map< MHO_UUID, std::map< MHO_UUID, MHO_Serializable* >  > fObjects;

        //allow us to store short names and labels for the objects as well (for file output)
        std::map< MHO_UUID, std::map< MHO_UUID, std::pair<std::string, uint32_t> > > fObjectsNameLabels;

};


template < typename XClassType > 
XClassType* 
MHO_ContainerStore::RetrieveObject(std::string object_uuid)
{
    //get the type id for this object 
    std::string type_uuid = fDictionary.GetUUIDFor<XClassType>().as_string();
    MHO_Serializable* ser_obj = nullptr;
    XClassType* obj = nullptr;

    if(object_uuid != "")
    {
        ser_obj = RetrieveObject(type_uuid, object_uuid);
    }
    else 
    {
        //no specific uuid given, just grab the ;first object of this type
        ser_obj = RetrieveFirstObjectMatchingType(type_uuid);
    }

    if(ser_obj)
    {
        obj = dynamic_cast<XClassType*>(ser_obj);
    }
    
    return obj;
}


template < typename XClassType > 
void 
MHO_ContainerStore::DeleteObject(XClassType* obj_ptr)
{
    MHO_Serializable* base_ptr = static_cast<MHO_Serializable*>(obj_ptr);
    
    //get the type id for this object 
    MHO_UUID type_id = fDictionary.GetUUIDFor<XClassType>();
    auto it1 = fObjects.find(type_id);
    if( it1 != fObjects.end() )
    {
        for(auto it2 = it1->second.begin(); it2 != it1->second.end(); it2++)
        {
            if( base_ptr == it2->second )
            {
                delete it2->second;
                it1->second.erase(it2);
            }
        }
    }
}



} //end namespace

#endif /* end of include guard: MHO_ContainerStore */
