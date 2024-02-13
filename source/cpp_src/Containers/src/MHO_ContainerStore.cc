#include "MHO_ContainerStore.hh"

namespace hops 
{

    

void 
MHO_ContainerStore::Clear()
{
    for(auto it = fObjectsToIds.begin(); it != fObjectsToIds.end(); it++)
    {
        MHO_Serializable* ptr = it->first;
        delete ptr;
    }
    fObjectsToIds.clear();
    fIdsToObjects.clear();
}


// bool 
// MHO_ContainerStore::AddObject(MHO_FileKey fkey, MHO_Serializable* obj)
// {
//     return AddObject(fkey.fTypeId, obj);
// }

// //add an object with type described by type_id, and generate/assign an object uuid for it
// bool 
// MHO_ContainerStore::AddObject(MHO_UUID type_id, MHO_Serializable* obj)
// {
//     if(obj == nullptr){return false;}
//     if( type_id.is_empty() ){return false;}
// 
//     MHO_UUID obj_id = obj->GetObjectUUID();
//     key_pair kp;
//     kp.first = type_id;
//     kp.second = obj_id;
// 
//     fIdsToObjects[kp] = obj;
//     fObjectsToIds[obj] = kp;
//     return true;
// }

//check if any object with the give object id is in the store
bool 
MHO_ContainerStore::IsObjectPresent(const MHO_UUID& obj_id) const
{
    for(auto it = fIdsToObjects.begin(); it != fIdsToObjects.end(); it++)
    {
        key_pair item_ids = it->first;
        MHO_UUID item_object_id = item_ids.second;
        if(obj_id == item_object_id){return true;}
    }
    return false;
}

//get an object via uuid (returns nullptr if not present)
MHO_Serializable* 
MHO_ContainerStore::GetObject(const MHO_UUID& obj_id)
{
    //this is a slow  O(N) way to take care of this
    MHO_Serializable* ptr = nullptr;
    for(auto it = fIdsToObjects.begin(); it != fIdsToObjects.end(); it++)
    {
        key_pair item_ids = it->first;
        MHO_UUID item_object_id = item_ids.second;
        if(obj_id == item_object_id)
        {
            ptr = it->second;
            break;
        }
    }
    return ptr;
}

bool 
MHO_ContainerStore::DeleteObject(const MHO_UUID& obj_id)
{
    MHO_Serializable* ptr = GetObject(obj_id);
    if(ptr == nullptr){return false;}
    DeleteObject<MHO_Serializable>(ptr);
    return true;
}


void 
MHO_ContainerStore::GetAllTypeUUIDs(std::vector< MHO_UUID >& type_ids)
{
    type_ids.clear();
    std::set< MHO_UUID > type_id_set;
    for(auto it = fIdsToObjects.begin(); it != fIdsToObjects.end(); it++)
    {
        key_pair item_ids = it->first;
        MHO_UUID item_type_id = item_ids.first;
        type_id_set.insert(item_type_id);
    }

    for(auto it = type_id_set.begin(); it != type_id_set.end(); it++)
    {
        type_ids.push_back(*it);
    }
}

void 
MHO_ContainerStore::GetAllObjectUUIDsOfType(MHO_UUID type_id, std::vector< MHO_UUID >& obj_ids)
{
    obj_ids.clear();
    for(auto it = fIdsToObjects.begin(); it != fIdsToObjects.end(); it++)
    {
        key_pair item_ids = it->first;
        MHO_UUID item_type_id = item_ids.first;
        MHO_UUID item_object_id = item_ids.second;
        if(type_id == item_type_id)
        {
            obj_ids.push_back(item_object_id);
        }
    }
}


bool 
MHO_ContainerStore::SetShortName(const MHO_UUID& obj_id, const std::string& shortname)
{
    if(shortname.size() != 0 && IsObjectPresent(obj_id))
    {
        auto it_status = fShortNameSet.insert(shortname);
        if(it_status.second) //if insertion was successful
        {
            fShortNameToIds[shortname] = obj_id;
            return true;
        }
    }
    return false;
}

MHO_UUID 
MHO_ContainerStore::GetObjectUUID(const std::string& shortname)
{
    MHO_UUID obj_id;
    auto it = fShortNameToIds.find(shortname);
    if(it != fShortNameToIds.end()){obj_id = it->second;}
    return obj_id;
}


//provide retrival of object short name from uuid
std::string 
MHO_ContainerStore::GetShortName(const MHO_UUID& obj_id)
{
    //brute force search 
    std::string value = "";
    for(auto it = fShortNameToIds.begin(); it != fShortNameToIds.end(); it++)
    {
        if(it->second == obj_id){value = it->first; break;}
    }
    return value;
}


void 
MHO_ContainerStore::GetAllShortNames(std::vector< std::string >& shortnames)
{
    shortnames.clear();
    for(auto it = fShortNameToIds.begin(); it != fShortNameToIds.end(); it++)
    {
        shortnames.push_back(it->first);
    }
}


MHO_UUID 
MHO_ContainerStore::GetObjectTypeUUID(const MHO_UUID& obj_id)
{
    MHO_UUID ret_val;
    for(auto it = fIdsToObjects.begin(); it != fIdsToObjects.end(); it++)
    {
        key_pair item_ids = it->first;
        MHO_UUID item_type_id = item_ids.first;
        MHO_UUID item_object_id = item_ids.second;
        if(obj_id == item_object_id)
        {
            ret_val = item_type_id;
            break;
        }
    }
    return ret_val;

}

std::vector< std::tuple< std::string, std::string, std::string > > 
MHO_ContainerStore::GetAllObjectInfo()
{
    std::vector< std::tuple< std::string, std::string, std::string > >  info;
    for(auto it = fIdsToObjects.begin(); it != fIdsToObjects.end(); it++)
    {
        std::tuple< std::string, std::string, std::string > obj_info;
        key_pair item_ids = it->first;
        MHO_UUID item_type_id = item_ids.first;
        MHO_UUID item_object_id = item_ids.second;
        std::string shortname = GetShortName(item_object_id);
        std::get<0>(obj_info) = item_type_id.as_string();
        std::get<1>(obj_info) = item_object_id.as_string();
        std::get<2>(obj_info) = shortname;
        info.push_back(obj_info);
    }
    return info;
}


}
