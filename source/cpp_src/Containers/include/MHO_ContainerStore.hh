#ifndef MHO_ContainerStore_HH__
#define MHO_ContainerStore_HH__

#include <map>
#include <set>
#include <utility>
#include <vector>

#include "MHO_ContainerDictionary.hh"
#include "MHO_Message.hh"
#include "MHO_Serializable.hh"
#include "MHO_Types.hh"
#include "MHO_UUID.hh"
#include "MHO_UUIDGenerator.hh"

namespace hops
{

/*!
 *@file  MHO_ContainerStore.hh
 *@class  MHO_ContainerStore
 *@author  J. Barrett - barrettj@mit.edu
 *@date Sat Feb 12 17:54:26 2022 -0500
 *@brief  holds a collection of objects all pointed to by base class MHO_Serializable*
 * retrieval is through type/object ids
 */

/**
 * @brief Class MHO_ContainerStore
 */
class MHO_ContainerStore
{
    public:
        MHO_ContainerStore(){};

        virtual ~MHO_ContainerStore() { Clear(); };

        //deletes all objects in the store
        /**
         * @brief Deletes all objects in the store.
         */
        void Clear();

        //add an object with specific type
        /**
         * @brief Adds an object to the container store if it's non-null and can be cast to MHO_Serializable.
         * 
         * @param obj Pointer to the object to add, must not be nullptr
         * @return True if object was successfully added, false otherwise
         */
        template< typename XClassType > bool AddObject(XClassType* obj);
        //get an object of a specific type via object uuid (returns nullptr if not present)
        /**
         * @brief Getter for object
         * 
         * @tparam XClassType Template parameter XClassType
         * @param obj_id UUID of the object to retrieve
         * @return Pointer to the object of type XClassType if found, nullptr otherwise
         */
        template< typename XClassType > XClassType* GetObject(const MHO_UUID& obj_id);
        //provides retrieval of an object via nickname, returns nullptr on failure
        /**
         * @brief Getter for object
         * 
         * @tparam XClassType Template parameter XClassType
         * @param obj_id UUID of the desired object
         * @return Pointer to the object if found, nullptr otherwise
         */
        template< typename XClassType > XClassType* GetObject(const std::string& shortname);
        //get an object of a specific type via index (returns nullptr if not present)
        /**
         * @brief Getter for object
         * 
         * @tparam XClassType Template parameter XClassType
         * @param shortname (const std::string&)
         * @return Pointer to the retrieved object cast to XClassType, or nullptr if not found
         */
        template< typename XClassType > XClassType* GetObject(std::size_t index);
        //destroy an object in the store, returns true if successful
        /**
         * @brief Deletes an object and removes associated entries from containers.
         * 
         * @tparam XClassType Template parameter XClassType
         * @param obj_ptr Pointer to the object of type XClassType to delete.
         * @return True if deletion was successful, false otherwise.
         */
        template< typename XClassType > bool DeleteObject(XClassType* obj_ptr);
        //get the type uuid for a specific type (if it is supported) - if unsupported uuid will be zero
        /**
         * @brief Getter for type uuid
         * 
         * @tparam XClassType Template parameter XClassType
         * @return MHO_UUID representing the type's UUID.
         */
        template< typename XClassType > MHO_UUID GetTypeUUID();
        //get the number of objects of a specific type
        /**
         * @brief Checks if an object with a given UUID is present in the container store.
         * 
         * @tparam XClassType Template parameter XClassType
         * @param obj_id The UUID of the object to search for.
         * @return True if the object is found, false otherwise.
         */
        /**
         * @brief Getter for nobjects
         * 
         * @tparam XClassType Template parameter XClassType
         * @return Size of fObjectsToIds vector as std::size_t
         */
        template< typename XClassType > std::size_t GetNObjects() const;

        //check if any object with the give object id is in the store
        bool IsObjectPresent(const MHO_UUID& obj_id) const;
        //get an object via uuid (returns nullptr if not present)
        /**
         * @brief Getter for object
         * 
         * @param obj_id UUID of the object to retrieve
         * @return Pointer to the object if found, nullptr otherwise
         */
        MHO_Serializable* GetObject(const MHO_UUID& obj_id);
        //destroy an object in the store by uuid, returns true if successful
        /**
         * @brief Removes an object from the container and related mappings by its UUID.
         * 
         * @param obj_id UUID of the object to delete.
         * @return True if deletion was successful, false otherwise.
         */
        bool DeleteObject(const MHO_UUID& obj_id);
        //get every type uuid present
        /**
         * @brief Getter for all type uuids
         * 
         * @param type_ids Output parameter: vector to hold retrieved UUIDs.
         */
        void GetAllTypeUUIDs(std::vector< MHO_UUID >& type_ids);
        //get every object uuid associated with the type
        /**
         * @brief Getter for all object uuids of type
         * 
         * @param type_id Input type ID of MHO_UUID to filter objects
         * @param obj_ids (std::vector< MHO_UUID )&
         */
        void GetAllObjectUUIDsOfType(MHO_UUID type_id, std::vector< MHO_UUID >& obj_ids);

        //get total number of objects in store
        /**
         * @brief Getter for nobjects
         * 
         * @return Number of objects as std::size_t
         */
        std::size_t GetNObjects() const { return fObjectsToIds.size(); }

        //provide the ability to attach a nicknames to object uuids
        //all nicknames must be unique
        //returns false if unsuccessful (object not present, or shortname already in use)
        /**
         * @brief Setter for short name
         * 
         * @param obj_id Object UUID to associate with the short name
         * @param shortname Unique short name to assign to the object
         * @return True if successful, false otherwise
         */
        bool SetShortName(const MHO_UUID& obj_id, const std::string& shortname);

        //provide retrieval of an object uuid via nickname
        //returns zero'd uuid if none exist
        /**
         * @brief Getter for object uuid
         * 
         * @param shortname Short name of the object
         * @return Object UUID; zero'd if none exist
         */
        MHO_UUID GetObjectUUID(const std::string& shortname);

        //returns the type uuid of the object with obj_id (if it exists)
        /**
         * @brief Getter for object type uuid
         * 
         * @param obj_id The ID of the object whose type's UUID is to be retrieved.
         * @return The UUID of the object type if found, otherwise an empty UUID.
         */
        MHO_UUID GetObjectTypeUUID(const MHO_UUID& obj_id);

        //provide retrival of object short name from uuid
        /**
         * @brief Getter for short name
         * 
         * @param obj_id The UUID for which to retrieve the short name.
         * @return The short name as a string if found, otherwise an empty string.
         */
        std::string GetShortName(const MHO_UUID& obj_id);

        //get all short names currently in use
        void GetAllShortNames(std::vector< std::string >& shortnames);

        //this is primarily here to provide a object look-up table for the python interface
        //returns a list of (object_type_uuid, object_item_uuid, shortname)
        std::vector< std::tuple< std::string, std::string, std::string > > GetAllObjectInfo();

        //provides a way in which we can replace the nickname of an object
        void RenameObject(const std::string& current_shortname, const std::string& new_shortname);

        void DumpShortNamesToIds()
        {
            for(auto it = fShortNameToIds.begin(); it != fShortNameToIds.end(); it++)
            {
                std::cout << it->first << " : " << it->second.as_string() << std::endl;
            }
        }

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
};

/**
 * @brief Function MHO_ContainerStore::AddObject
 * 
 * @param obj (XClassType*)
 * @return Return value (template< typename XClassType > bool)
 */
template< typename XClassType > bool MHO_ContainerStore::AddObject(XClassType* obj)
{
    if(obj == nullptr)
    {
        return false;
    }

    //attempt to cast to our storage/serialization type
    auto ptr = static_cast< MHO_Serializable* >(obj);
    if(ptr == nullptr)
    {
        return false;
    }

    MHO_UUID obj_id = obj->GetObjectUUID();
    MHO_UUID type_id = obj->GetTypeUUID();

    key_pair kp;
    kp.first = type_id;
    kp.second = obj_id;

    fIdsToObjects[kp] = ptr;
    fObjectsToIds[ptr] = kp;
    return true;
}

/**
 * @brief Retrieves an object of type XClassType from the container store using its UUID.
 * 
 * @param obj_id The unique identifier (UUID) of the desired object
 * @return A pointer to the object of type XClassType, or nullptr if not found
 */
template< typename XClassType > XClassType* MHO_ContainerStore::GetObject(const MHO_UUID& obj_id)
{
    MHO_UUID type_id = fDictionary.GetUUIDFor< XClassType >();

    key_pair kp;
    kp.first = type_id;
    kp.second = obj_id;

    XClassType* ptr = nullptr;
    auto it = fIdsToObjects.find(kp);
    if(it != fIdsToObjects.end())
    {
        MHO_Serializable* obj = it->second;
        ptr = dynamic_cast< XClassType* >(obj);
    }
    return ptr;
}

/**
 * @brief Retrieves an object of type XClassType from the container store using its UUID.
 * 
 * @param shortname (const std::string&)
 * @return Pointer to the retrieved object of type XClassType, or nullptr if not found
 */
template< typename XClassType > XClassType* MHO_ContainerStore::GetObject(const std::string& shortname)
{
    XClassType* ptr = nullptr;
    MHO_UUID obj_id = GetObjectUUID(shortname);
    if(!obj_id.is_empty())
    {
        ptr = GetObject< XClassType >(obj_id);
    }
    return ptr;
}

/**
 * @brief Retrieves an object of type XClassType at a specific index in the container.
 * 
 * @param index Index of the desired object
 * @return Pointer to the object of type XClassType if found, nullptr otherwise
 */
template< typename XClassType > XClassType* MHO_ContainerStore::GetObject(std::size_t index)
{
    XClassType* ptr = nullptr;
    std::size_t n_objects = GetNObjects< XClassType >();
    std::size_t count = 0;
    if(index < n_objects)
    {
        MHO_UUID type_id = fDictionary.GetUUIDFor< XClassType >();
        for(auto it = fIdsToObjects.begin(); it != fIdsToObjects.end(); it++)
        {
            key_pair item_ids = it->first;
            MHO_UUID item_type_id = item_ids.first;
            if(type_id == item_type_id)
            {
                if(count == index)
                {
                    MHO_Serializable* obj = it->second;
                    ptr = dynamic_cast< XClassType* >(obj);
                    break;
                }
                count++;
            }
        }
    }

    return ptr;
}

/**
 * @brief Function MHO_ContainerStore::DeleteObject
 * 
 * @param obj_ptr (XClassType*)
 * @return Return value (template< typename XClassType > bool)
 */
template< typename XClassType > bool MHO_ContainerStore::DeleteObject(XClassType* obj_ptr)
{
    MHO_Serializable* ptr = static_cast< MHO_Serializable* >(obj_ptr);
    if(ptr == nullptr)
    {
        return false;
    }

    auto it = fObjectsToIds.find(ptr);
    if(it == fObjectsToIds.end())
    {
        return false;
    }

    key_pair kp = it->second;
    MHO_UUID obj_id = kp.second;
    auto it2 = fIdsToObjects.find(kp);
    if(it2 == fIdsToObjects.end())
    {
        return false;
    }

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
    return true;
}

/**
 * @brief Retrieves the UUID for a given class type from the internal dictionary.
 * 
 * @return MHO_UUID representing the type's UUID.
 */
template< typename XClassType > MHO_UUID MHO_ContainerStore::GetTypeUUID()
{
    MHO_UUID type_id = fDictionary.GetUUIDFor< XClassType >();
    return type_id;
}

/**
 * @brief Counts and returns the number of objects of type XClassType stored in MHO_ContainerStore.
 * 
 * @return Number of objects of type XClassType found
 */
template< typename XClassType > std::size_t MHO_ContainerStore::GetNObjects() const
{
    MHO_UUID type_id = fDictionary.GetUUIDFor< XClassType >();
    std::size_t count = 0;
    for(auto it = fIdsToObjects.begin(); it != fIdsToObjects.end(); it++)
    {
        key_pair item_ids = it->first;
        MHO_UUID item_type_id = item_ids.first;
        if(type_id == item_type_id)
        {
            count++;
        }
    }
    return count;
}

/**
 * @brief Renames an object in MHO_ContainerStore by updating its short name and associated UUID.
 * 
 * @param current_shortname Current short name of the object to be renamed
 * @param new_shortname New short name for the object
 */
inline void MHO_ContainerStore::RenameObject(const std::string& current_shortname, const std::string& new_shortname)
{
    MHO_UUID obj_uuid = GetObjectUUID(current_shortname);
    if(obj_uuid.as_long() != 0)
    {
        fShortNameSet.erase(current_shortname);
        fShortNameToIds.erase(current_shortname);
        SetShortName(obj_uuid, new_shortname);
    }
}

} // namespace hops

#endif /*! end of include guard: MHO_ContainerStore */
