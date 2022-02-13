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
        MHO_Serializable* tmp = AttemptToRead(key);
        if(tmp != nullptr)
        {
            lib.AddContainerObject(tmp,key);
        }
    }

    fFileInterface.Close();
};

MHO_Serializable*
MHO_ContainerFileInterface::AttemptToRead(MHO_FileKey& object_key)
{
        MHO_UUID type_uuid = object_key.fTypeId;
        MHO_Serializable* obj = fFactoryMap[type_uuid]();
        MHO_FileKey read_key;
        //must figure out what object type we should attempt to read
        fFileInterface.Read(obj, read_key);
}



















}//end namespace