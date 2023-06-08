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

#include <set>
#include <map>
#include <utility>
#include <vector>
#include <cstdint>

#include "MHO_Message.hh"
#include "MHO_Serializable.hh"
#include "MHO_UUID.hh"
#include "MHO_UUIDGenerator.hh"
#include "MHO_ContainerDictionary.hh"

namespace hops 
{

class MHO_ContainerStore
{
    public:

        MHO_ContainerStore(){};
        virtual ~MHO_ContainerStore(){Clear();};

        //deletes all objects in the store 
        void Clear();
        
        //add an object with specific type
        template<typename XClassType> bool AddObject(XClassType* obj);
        //get an object of a specific type via object uuid (returns nullptr if not present)
        template < typename XClassType > XClassType* GetObject(const MHO_UUID& obj_id);
        //get an object of a specific type via index (returns nullptr if not present)
        template < typename XClassType > XClassType* GetObject(std::size_t index);
        //destroy an object in the store, returns true if successful
        template < typename XClassType > bool DeleteObject(XClassType* obj_ptr);
        //get the type uuid for a specific type (if it is supported) - if unsupported uuid will be zero
        template < typename XClassType > MHO_UUID GetTypeUUID();
        //get the number of objects of a specific type
        template < typename XClassType > std::size_t GetNObjects() const;
        
        //check if any object with the give object id is in the store
        bool IsObjectPresent(const MHO_UUID& obj_id) const;
        //get an object via uuid (returns nullptr if not present)
        MHO_Serializable* GetObject(const MHO_UUID& obj_id);
        //destroy an object in the store by uuid, returns true if successful
        bool DeleteObject(const MHO_UUID& obj_id);
        //get every type uuid present
        void GetAllTypeUUIDs(std::vector< MHO_UUID >& type_ids);
        //get every object uuid associated with the type
        void GetAllObjectUUIDsOfType(MHO_UUID type_id, std::vector< MHO_UUID >& obj_ids);
        //get total number of objects in store
        std::size_t GetNObjects() const {return fObjectsToIds.size();}
        
        //provide the ability to attach a nicknames to object uuids
        //all nicknames must be unique
        //returns false if unsuccessful (object not present, or shortname already in use)
        bool SetShortName(const MHO_UUID& obj_id, const std::string& shortname);
        
        //provide retrieval of an object uuid via nickname
        //returns zero'd uuid if none exist
        MHO_UUID GetObjectUUID(const std::string& shortname);
        
        //provide retrival of object short name from uuid
        std::string GetShortName(const MHO_UUID& obj_id);
        
        //get all short names currently in use
        void GetAllShortNames(std::vector< std::string >& shortnames);
        
        //provide the ability to attach integer labels to object uuids
        //only a single integer label can be assigned per object uuid
        //returns false if object not present
        bool SetObjectLabel(const MHO_UUID& obj_id, uint32_t label);

        //returns zero if object or label is not present
        uint32_t GetObjectLabel(const MHO_UUID& obj_id);
        

    protected:
        
        using key_pair = std::pair< MHO_UUID, MHO_UUID >;

        //object dictionary...currently we only have one dictionary implementation
        //however, we may in the future want to allow the user to pass a custom dictionary implementation 
        //if they have additional classes they want to serialize which are not already supported 
        MHO_ContainerDictionary fDictionary;
        
        //all objects are stored as pointers to the base-class MHO_Serializable
        //they are cast to the underlying type specified by the type id upon retrieval
        
        //the key pair is <type_uuid, obj_id>, and the value is a pointer to the object
        std::map< key_pair, MHO_Serializable* > fIdsToObjects;
        
        //the key is a pointer to an object, and the value is a pair of <type_uuid, obj_id>
        std::map< MHO_Serializable*, key_pair > fObjectsToIds;
        
        //maps string names to object uuids 
        std::map< std::string, MHO_UUID > fShortNameToIds;
        std::set< std::string > fShortNameSet;
        //maps uuid to integer label 
        std::map< MHO_UUID, uint32_t > fIdsToLabels;
        
        
        
        
};


template<typename XClassType>
bool 
MHO_ContainerStore::AddObject(XClassType* obj)
{
    if(obj == nullptr){return false;}

    //attempt to cast to our storage/serialization type
    auto ptr = static_cast< MHO_Serializable* >(obj);
    if(ptr == nullptr){return false;}

    MHO_UUID obj_id = obj->GetObjectUUID();
    MHO_UUID type_id = obj->GetTypeUUID();
    
    key_pair kp;
    kp.first = type_id;
    kp.second = obj_id;
    
    fIdsToObjects[kp] = ptr;
    fObjectsToIds[ptr] = kp;
    return true;
}
 
template < typename XClassType >
XClassType*
MHO_ContainerStore::GetObject(const MHO_UUID& obj_id)
{
    MHO_UUID type_id = fDictionary.GetUUIDFor<XClassType>();
    
    key_pair kp;
    kp.first = type_id;
    kp.second = obj_id;

    XClassType* ptr = nullptr;
    auto it = fIdsToObjects.find(kp);
    if(it != fIdsToObjects.end())
    {
        MHO_Serializable* obj = it->second;
        ptr = dynamic_cast<XClassType*>(obj);
    }
    return ptr;
}

template < typename XClassType >
XClassType*
MHO_ContainerStore::GetObject(std::size_t index)
{
    XClassType* ptr = nullptr;
    std::size_t n_objects = GetNObjects<XClassType>();
    std::size_t count = 0;
    if(index < n_objects)
    {
        MHO_UUID type_id = fDictionary.GetUUIDFor<XClassType>();
        for(auto it = fIdsToObjects.begin(); it != fIdsToObjects.end(); it++)
        {
            key_pair item_ids = it->first;
            MHO_UUID item_type_id = item_ids.first;
            if(type_id == item_type_id)
            {
                if(count == index)
                {
                    MHO_Serializable* obj = it->second;
                    ptr = dynamic_cast<XClassType*>(obj);
                    break;
                }
                count++;
            }
        }
    }

    return ptr;
}

template < typename XClassType >
bool
MHO_ContainerStore::DeleteObject(XClassType* obj_ptr)
{
    MHO_Serializable* ptr = static_cast<MHO_Serializable*>(obj_ptr);
    if(ptr == nullptr){return false;}
    
    auto it = fObjectsToIds.find(ptr);
    if(it == fObjectsToIds.end()){return false;}
    
    key_pair kp = it->second;
    MHO_UUID obj_id = kp.second;
    auto it2 = fIdsToObjects.find(kp);
    if(it2 == fIdsToObjects.end()){return false;}
    
    //remove entries related to this object
    fObjectsToIds.erase(it);
    fIdsToObjects.erase(it2);
    delete obj_ptr;
    
    //remove labels and short name associated with this object 
    std::string shortname = "";
    for(auto it = fShortNameToIds.begin(); it != fShortNameToIds.end(); it++)
    {
        if(it->second == obj_id)
        {
            shortname = it->first;
            fShortNameToIds.erase(it);
            break;
        }
    }
    fShortNameSet.erase(shortname);

    auto label_it = fIdsToLabels.find(obj_id);
    if(label_it != fIdsToLabels.end()){fIdsToLabels.erase(label_it);}
    
    return true;
}

template < typename XClassType >
MHO_UUID
MHO_ContainerStore::GetTypeUUID()
{
    MHO_UUID type_id = fDictionary.GetUUIDFor<XClassType>();
    return type_id;
}

template < typename XClassType >
std::size_t
MHO_ContainerStore::GetNObjects() const
{
    MHO_UUID type_id = fDictionary.GetUUIDFor<XClassType>();
    std::size_t count = 0;
    for(auto it = fIdsToObjects.begin(); it != fIdsToObjects.end(); it++)
    {
        key_pair item_ids = it->first;
        MHO_UUID item_type_id = item_ids.first;
        if(type_id == item_type_id){count++;}
    }
    return count;
}









} //end namespace

#endif /* end of include guard: MHO_ContainerStore */
