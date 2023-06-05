#include "MHO_ContainerStore.hh"

namespace hops 
{

MHO_ContainerStore::MHO_ContainerStore(){};

MHO_ContainerStore::~MHO_ContainerStore()
{
    Clear();
};

void 
MHO_ContainerStore::Clear()
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
MHO_ContainerStore::AddContainerObject(MHO_Serializable* obj, const MHO_FileKey& key)
{
    MHO_UUID type_id = key.fTypeId;
    MHO_UUID obj_id = key.fObjectId;
    std::string name = key.fName;
    uint32_t label = key.fLabel;
    return AddContainerObject(obj, type_id, obj_id, name, label);
};

bool 
MHO_ContainerStore::AddContainerObject(MHO_Serializable* obj, 
                        const std::string& type_uuid, 
                        const std::string& object_uuid,
                        std::string shortname,
                        uint32_t label)
{
    MHO_UUID type_id;
    bool ok1 = type_id.from_string(type_uuid);
    MHO_UUID obj_id;
    bool ok2 = obj_id.from_string(object_uuid);
    if(ok1 && ok2){return AddContainerObject(obj, type_id, obj_id, shortname, label);}
    return false;
};

bool 
MHO_ContainerStore::AddContainerObject(MHO_Serializable* obj, 
                        const MHO_UUID& type_uuid, 
                        const MHO_UUID& object_uuid,
                        std::string shortname,
                        uint32_t label)
{
    if( !IsObjectPresent(type_uuid, object_uuid) )
    {
        fObjects[type_uuid][object_uuid] = obj;
        fObjectsNameLabels[type_uuid][object_uuid] = std::make_pair(shortname,label);
        return true;
    }
    return false;
};


bool 
MHO_ContainerStore::IsObjectPresent(const MHO_FileKey& key) const
{
    MHO_UUID type_id = key.fTypeId;
    MHO_UUID obj_id = key.fObjectId;
    return IsObjectPresent(type_id, obj_id);
}

bool 
MHO_ContainerStore::IsObjectPresent(const std::string& type_uuid, const std::string& object_uuid) const
{
    MHO_UUID type_id;
    bool ok1 = type_id.from_string(type_uuid);
    MHO_UUID obj_id;
    bool ok2 = obj_id.from_string(object_uuid);
    if(ok1 && ok2){return IsObjectPresent(type_id, obj_id);}
    return false;
}

bool 
MHO_ContainerStore::IsObjectPresent(const MHO_UUID& type_uuid, const MHO_UUID& object_uuid) const
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




//if retrieval fails, returns nullptr
MHO_Serializable* 
MHO_ContainerStore::RetrieveObject(const MHO_FileKey& key)
{
    MHO_UUID type_id = key.fTypeId;
    MHO_UUID obj_id = key.fObjectId;
    return RetrieveObject(type_id, obj_id);
};

MHO_Serializable* 
MHO_ContainerStore::RetrieveObject(const std::string& type_uuid, const std::string& object_uuid)
{
    MHO_UUID type_id;
    bool ok1 = type_id.from_string(type_uuid);
    MHO_UUID obj_id;
    bool ok2 = obj_id.from_string(object_uuid);
    if(ok1 && ok2){return RetrieveObject(type_id, obj_id);}
    return nullptr;
};

MHO_Serializable* 
MHO_ContainerStore::RetrieveObject(const MHO_UUID& type_uuid, const MHO_UUID& object_uuid)
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
MHO_ContainerStore::RetrieveFirstObjectMatchingType(const MHO_FileKey& key)
{
    MHO_UUID type_id = key.fTypeId;
    return RetrieveFirstObjectMatchingType(type_id);
}

MHO_Serializable* 
MHO_ContainerStore::RetrieveFirstObjectMatchingType(const std::string& type_uuid)
{
    MHO_UUID type_id;
    bool ok = type_id.from_string(type_uuid);
    if(ok){return RetrieveFirstObjectMatchingType(type_id);}
    return nullptr;
}

MHO_Serializable* 
MHO_ContainerStore::RetrieveFirstObjectMatchingType(const MHO_UUID& type_uuid)
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
MHO_ContainerStore::GetNObjects() const
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

std::size_t 
MHO_ContainerStore::GetNObjectsOfType(const MHO_UUID& type_id) const
{
    std::size_t count = 0;
    auto it = fObjects.find(type_id);
    if(it != fObjects.end())
    {
        for(auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            count++;
        }
    }
    return count;
}

void 
MHO_ContainerStore::GetAllTypeUUIDs(std::vector<MHO_UUID>& type_ids) const
{
    type_ids.clear();
    for(auto it = fObjects.begin(); it != fObjects.end(); it++)
    {
        type_ids.push_back(it->first);
    }
}

void 
MHO_ContainerStore::GetAllObjectUUIDsOfType(const MHO_UUID& type_id, std::vector<MHO_UUID>& obj_ids) const
{
    obj_ids.clear();
    auto it = fObjects.find(type_id);
    if(it != fObjects.end())
    {
        for(auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            obj_ids.push_back(it2->first);
        }
    }
}


std::pair<std::string, uint32_t> 
MHO_ContainerStore::GetObjectNameLabel(const MHO_UUID& type_uuid, const MHO_UUID& object_uuid) const
{
    auto it = fObjectsNameLabels.find(type_uuid);
    if( it != fObjectsNameLabels.end() )
    {
        auto it2 = it->second.find(object_uuid);
        if( it2 != it->second.end() ){return it2->second;}
        else{return std::make_pair(std::string(""),0); }//no name/lable associated with this object
    }
    return std::make_pair(std::string(""),0);
}


void MHO_ContainerStore::DeleteObject(const MHO_FileKey& key)
{
    MHO_UUID type_id = key.fTypeId;
    MHO_UUID obj_id = key.fObjectId;
    DeleteObject(type_id, obj_id);
}

void 
MHO_ContainerStore::DeleteObject(const std::string& type_uuid, const std::string& object_uuid)
{
    MHO_UUID type_id;
    bool ok1 = type_id.from_string(type_uuid);
    MHO_UUID obj_id;
    bool ok2 = obj_id.from_string(object_uuid);
    if(ok1 && ok2){DeleteObject(type_id, obj_id);}

}

void 
MHO_ContainerStore::DeleteObject(const MHO_UUID& type_uuid, const MHO_UUID& object_uuid)
{
    auto it = fObjects.find(type_uuid);
    if( it != fObjects.end() )
    {
        auto it2 = it->second.find(object_uuid);
        if( it2 != it->second.end() )
        {
            delete it2->second;
            it->second.erase(it2);
        }
    }
}


} //end of namespace
