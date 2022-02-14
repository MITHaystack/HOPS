#include "MHO_ContainerLibrary.hh"

namespace hops 
{

MHO_ContainerLibrary::MHO_ContainerLibrary(){};

MHO_ContainerLibrary::~MHO_ContainerLibrary()
{
    Clear();
};

void 
MHO_ContainerLibrary::Clear()
{
    for(auto it = fObjects.begin(); it != fObjects.end(); it++)
    {
        for(auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            MHO_Serializable* ptr = it2->second;
            delete ptr;
        }
    }
    fObjects.clear();
}

//returns true if object successfully added, false if not added
bool 
MHO_ContainerLibrary::AddContainerObject(MHO_Serializable* obj, const MHO_FileKey& key)
{
    MHO_UUID type_id = key.fTypeId;
    MHO_UUID obj_id = key.fObjectId;
    return AddContainerObject(obj, type_id, obj_id);
};

bool 
MHO_ContainerLibrary::AddContainerObject(MHO_Serializable* obj, const std::string& type_uuid, const std::string& object_uuid)
{
    MHO_UUID type_id;
    bool ok1 = type_id.from_string(type_uuid);
    MHO_UUID obj_id;
    bool ok2 = obj_id.from_string(object_uuid);
    if(ok1 && ok2){return AddContainerObject(obj, type_id, obj_id);}
    return false;
};


bool 
MHO_ContainerLibrary::IsObjectPresent(const MHO_FileKey& key) const
{
    MHO_UUID type_id = key.fTypeId;
    MHO_UUID obj_id = key.fObjectId;
    return IsObjectPresent(type_id, obj_id);
}

bool 
MHO_ContainerLibrary::IsObjectPresent(const std::string& type_uuid, const std::string& object_uuid) const
{
    MHO_UUID type_id;
    bool ok1 = type_id.from_string(type_uuid);
    MHO_UUID obj_id;
    bool ok2 = obj_id.from_string(object_uuid);
    if(ok1 && ok2){return IsObjectPresent(type_id, obj_id);}
    return false;
}

bool 
MHO_ContainerLibrary::IsObjectPresent(const MHO_UUID& type_uuid, const MHO_UUID& object_uuid) const
{
    auto it = fObjects.find(type_uuid);
    if( it != fObjects.end() )
    {
        auto it2 = it->second.find(object_uuid);
        if( it2 != it->second.end() ){return true;}
        else{return false;}//no object with this id present
    }
    return false;//no type of this kind present
}

bool 
MHO_ContainerLibrary::AddContainerObject(MHO_Serializable* obj, const MHO_UUID& type_uuid, const MHO_UUID& object_uuid)
{
    if( !IsObjectPresent(type_uuid, object_uuid) )
    {
        fObjects[type_uuid][object_uuid] = obj;
        return true;
    }
    return false;
};


//if retrieval fails, returns nullptr
MHO_Serializable* 
MHO_ContainerLibrary::RetrieveObject(const MHO_FileKey& key)
{
    MHO_UUID type_id = key.fTypeId;
    MHO_UUID obj_id = key.fObjectId;
    return RetrieveObject(type_id, obj_id);
};

MHO_Serializable* 
MHO_ContainerLibrary::RetrieveObject(const std::string& type_uuid, const std::string& object_uuid)
{
    MHO_UUID type_id;
    bool ok1 = type_id.from_string(type_uuid);
    MHO_UUID obj_id;
    bool ok2 = obj_id.from_string(object_uuid);
    if(ok1 && ok2){return RetrieveObject(type_id, obj_id);}
    return nullptr;
};

MHO_Serializable* 
MHO_ContainerLibrary::RetrieveObject(const MHO_UUID& type_uuid, const MHO_UUID& object_uuid)
{
    auto it = fObjects.find(type_uuid);
    if( it != fObjects.end() )
    {
        auto it2 = it->second.find(object_uuid);
        if( it2 != it->second.end() ){return it2->second;}
        else{return nullptr;}//no object with this id present
    }
    return nullptr;//no type of this kind present
};

MHO_Serializable* 
MHO_ContainerLibrary::RetrieveFirstObjectMatchingType(const MHO_FileKey& key)
{
    MHO_UUID type_id = key.fTypeId;
    return RetrieveFirstObjectMatchingType(type_id);
}

MHO_Serializable* 
MHO_ContainerLibrary::RetrieveFirstObjectMatchingType(const std::string& type_uuid)
{
    MHO_UUID type_id;
    bool ok = type_id.from_string(type_uuid);
    if(ok){return RetrieveFirstObjectMatchingType(type_id);}
    return nullptr;
}

MHO_Serializable* 
MHO_ContainerLibrary::RetrieveFirstObjectMatchingType(const MHO_UUID& type_uuid)
{
    auto it = fObjects.find(type_uuid);
    if( it != fObjects.end() )
    {
        if(it->second.size() != 0)
        {
            auto it2 = it->second.begin();
            return it2->second;
        }
    }
    return nullptr;//no type of this kind present
}


std::size_t 
MHO_ContainerLibrary::GetNObjects()
{
    std::size_t count = 0;
    for(auto it = fObjects.begin(); it != fObjects.end(); it++)
    {
        for(auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            count++;
        }
    }
    return count;
}



}