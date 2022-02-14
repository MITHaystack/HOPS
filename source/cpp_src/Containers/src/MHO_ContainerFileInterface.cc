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
MHO_ContainerFileInterface::PopulateLibraryFromFile(MHO_ContainerLibrary& lib)
{
    lib.Clear();
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

    //open file and read each object, and stuff it in the library
    ok = fFileInterface.OpenToRead(fFilename);
    if(!ok)
    {
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
                lib.AddContainerObject(obj,key);
            }
            else 
            {
                msg_warn("container", "factory failed to build object from file with type: "<< fUUID2ClassName[type_id] << eom );
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
MHO_ContainerFileInterface::WriteLibraryToFile(MHO_ContainerLibrary& lib)
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

    // for(auto it = ikeys.begin(); it != ikeys.end(); it++)
    // {
    //     MHO_FileKey key = *it;
    //     MHO_UUID type_id = key.fTypeId;
    //     auto factory = fFactoryMap.find(type_id);
    //     if(factory != fFactoryMap.end())
    //     {
    //         MHO_Serializable* obj = factory->second->BuildFromFileInterface(fFileInterface);
    //         if(obj != nullptr)
    //         {
    //             lib.AddContainerObject(obj,key);
    //         }
    //         else 
    //         {
    //             msg_warn("container", "factory failed to build object from file with type: "<< fUUID2ClassName[type_id] << eom );
    //         }
    //     }
    //     else 
    //     {
    //         msg_warn("containers", "unrecognized object in file with type uuid: " << type_id.as_string() << eom );
    //     }
    // }

    fFileInterface.Close();
}



}//end namespace