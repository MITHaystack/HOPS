#include "MHO_ContainerFileInterface.hh"

namespace hops
{

MHO_ContainerFileInterface::MHO_ContainerFileInterface():
    fFilename(""),
    fIndexFilename("")
{}

MHO_ContainerFileInterface::~MHO_ContainerFileInterface(){};

void 
MHO_ContainerFileInterface::SetFilename(std::string filename)
{
    fFilename = filename;
}

void 
MHO_ContainerFileInterface::SetIndexFileName(std::string index_filename)
{
    fIndexFilename = index_filename;
}

void 
MHO_ContainerFileInterface::PopulateStoreFromFile(MHO_ContainerStore& store)
{
    store.Clear();
    bool ok = false;

    //pull the file object keys for inspection 
    std::vector< MHO_FileKey > ikeys;
    MHO_FileKey object_key;

    //extract object keys
    if(fIndexFilename != "" )
    {
        ok = fFileInterface.ExtractIndexFileObjectKeys(fIndexFilename, ikeys);
        if(!ok){msg_error("containers", "could not extract index-file object keys" << eom); return;}
    }
    else 
    {
        ok = fFileInterface.ExtractFileObjectKeys(fFilename, ikeys);
        if(!ok){msg_error("containers", "could not extract file object keys" << eom); return;}
    }

    //open file and read each object, and stuff it in the storerary
    ok = fFileInterface.OpenToRead(fFilename);
    if(!ok)
    {
        msg_error("containers", "failed to open file interface to read: "<<fFilename<<eom);
        fFileInterface.Close();
        return;
    }

    for(auto it = ikeys.begin(); it != ikeys.end(); it++)
    {
        MHO_FileKey key = *it;
        MHO_UUID type_id = key.fTypeId;
        auto factory = fFactoryMap.find(type_id);
        if(factory != fFactoryMap.end())
        {
            MHO_Serializable* obj = factory->second->BuildFromFileInterface(fFileInterface);
            if(obj != nullptr)
            {
                store.AddContainerObject(obj,key);
            }
            else 
            {
                msg_warn("containers", "factory failed to build object from file with type: "<< fUUID2ClassName[type_id] << eom );
            }
        }
        else 
        {
            msg_warn("containers", "unrecognized object in file with type uuid: " << type_id.as_string() << eom );
        }
    }

    fFileInterface.Close();
};

void 
MHO_ContainerFileInterface::WriteStoreToFile(MHO_ContainerStore& store)
{
    bool ok = false;
    //open up the file we want to write to
    if(fIndexFilename != "" )
    {
        ok = fFileInterface.OpenToWrite(fFilename,fIndexFilename);
        if(!ok){msg_error("containers", "could not open file: " <<fFilename << " to write." << eom); return;}
    }
    else 
    {
        ok = fFileInterface.OpenToWrite(fFilename);
        if(!ok){msg_error("containers", "could not open file: " <<fFilename << " to write." << eom); return;}
    }

    std::vector< MHO_UUID > type_ids;
    store.GetAllTypeUUIDs(type_ids);

    for(auto it = type_ids.begin(); it != type_ids.end(); it++)
    {
        auto factory = fFactoryMap.find(*it);
        if(factory != fFactoryMap.end())
        {
            std::vector< MHO_UUID > obj_ids;
            store.GetAllObjectUUIDsOfType(*it, obj_ids);
            for(auto it2 = obj_ids.begin(); it2 != obj_ids.end(); it2++)
            {
                MHO_Serializable* obj = store.RetrieveObject(*it, *it2);
                std::pair<std::string, uint32_t> name_label = store.GetObjectNameLabel(*it,*it2);
                bool ok = factory->second->WriteToFileInterface(fFileInterface, obj, name_label.first, name_label.second);
                if(!ok)
                {
                    msg_warn("containers", "factory failed to write object to file with type: "<< fUUID2ClassName[*it] << eom );
                }
            }
        }
    }

    fFileInterface.Close();
}


void 
MHO_ContainerFileInterface::ConvertStoreToJSON(MHO_ContainerStore& store, json& json_obj, int level_of_detail)
{
    std::vector< MHO_UUID > type_ids;
    store.GetAllTypeUUIDs(type_ids);

    for(auto it = type_ids.begin(); it != type_ids.end(); it++)
    {
        auto converter = fJSONConverterMap.find(*it);
        if(converter != fJSONConverterMap.end())
        {
            std::vector< MHO_UUID > obj_ids;
            store.GetAllObjectUUIDsOfType(*it, obj_ids);
            for(auto it2 = obj_ids.begin(); it2 != obj_ids.end(); it2++)
            {
                MHO_Serializable* obj = store.RetrieveObject(*it, *it2);
                if(obj != nullptr)
                {
                    converter->second->SetObjectToConvert(obj);
                    converter->second->SetLevelOfDetail(level_of_detail);
                    converter->second->ConstructJSONRepresentation();
                    json j = *(converter->second->GetJSON());
                    std::string object_uuid = it2->as_string();
                    json_obj[object_uuid] = j;
                }
            }
        }
    }
};


}//end namespace