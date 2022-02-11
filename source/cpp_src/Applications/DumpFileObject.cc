#include <getopt.h>
#include "MHO_Message.hh"

#include "MHO_BinaryFileInterface.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_Visibilities.hh"
#include "MHO_StationCoordinates.hh"

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_ContainerJSON.hh"


using namespace hops;

template< typename XObjectType>
void ReadAndDump(MHO_FileKey& object_key, uint64_t offset, std::string filename, int detail)
{
    MHO_BinaryFileInterface inter;
    json* json_obj = nullptr;
    //now open skip ahead to the object we want
    bool status = inter.OpenToReadAtOffset(filename, offset);
    if(status)
    {
        XObjectType obj;
        MHO_FileKey read_key;
        inter.Read(obj, read_key);
        obj.template MakeExtension< MHO_ContainerJSON< XObjectType > >();
        obj.template AsExtension< MHO_ContainerJSON< XObjectType > >()->SetLevelOfDetail(detail);
        obj.template AsExtension< MHO_ContainerJSON< XObjectType > >()->ConstructJSONRepresentation();
        json_obj = obj.template AsExtension< MHO_ContainerJSON< XObjectType > >()->GetJSON();
        //dump the json to terminal
        if(json_obj){std::cout<<json_obj->dump(2)<<std::endl; }
    }
    else
    {
        msg_error("main", "error opening file with byte offset: " << offset << eom );
    }
    inter.Close();
}


void DumpToJSON(MHO_FileKey& object_key, uint64_t offset, std::string filename, int detail)
{
    MHO_ContainerDictionary cdict;
    MHO_UUID tid = object_key.fTypeId;

    if(tid == cdict.GetUUIDFor<ch_baseline_data_type>())
    {
        ReadAndDump<ch_baseline_data_type>(object_key, offset, filename, detail);
        return;
    }

    if(tid == cdict.GetUUIDFor<ch_baseline_weight_type>())
    {
        ReadAndDump<ch_baseline_weight_type>(object_key, offset, filename, detail);
        return;
    }

    if(tid == cdict.GetUUIDFor<station_coord_data_type>())
    {
        ReadAndDump<station_coord_data_type>(object_key, offset, filename, detail);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    //TODO FIXME -- ADD IF() statements for the rest of the data container types
    ////////////////////////////////////////////////////////////////////////////

    msg_error("main", "cannot identify an object with type id: "<< object_key.fTypeId.as_string() << eom);

}


int main(int argc, char** argv)
{
    std::string usage = "DumpFileObject -f <file> -t <type> -u <uuid> -d <detail level 0-3>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    // //TODO extend this to other container types
    // msg_warn("main", "currenly only implemented for the visibility container type." << eom);

    std::string filename = "";
    std::string uuid = "";
    std::string type = "";
    int detail = eJSONAll;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"file", required_argument, 0, 'f'},
                                          {"type", required_argument, 0, 't'},
                                          {"uuid", required_argument, 0, 'u'},
                                          {"detail", required_argument, 0, 'd'}
    };

    static const char* optString = "hf:t:u:d:";

    while(true)
    {
        char optId = getopt_long(argc, argv, optString, longOptions, NULL);
        if (optId == -1)
            break;
        switch(optId)
        {
            case ('h'):  // help
                std::cout << usage << std::endl;
                return 0;
            case ('f'):
                filename = std::string(optarg);
                break;
            case ('u'):
                uuid = std::string(optarg);
                break;
            case ('t'):
                type = std::string(optarg);
                break;
            case ('d'):
                detail = std::atoi(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    MHO_UUID classuuid;
    bool success = classuuid.from_string(type);
    if(!success){msg_error("main", "could not convert object type string (-t) to uuid" << eom);}

    //pull the file object keys for inspection 
    std::vector< MHO_FileKey > ikeys;
    MHO_FileKey object_key;

    MHO_BinaryFileInterface inter;
    bool result = false;
    result = inter.ExtractFileObjectKeys(filename, ikeys);
    if(!result){msg_fatal("main", "could not extract file object keys" << eom); std::exit(1);}

    bool found_obj = false;
    auto it = ikeys.begin();
    for(it = ikeys.begin(); it != ikeys.end(); it++)
    {
        if( it->fTypeId.as_string() == classuuid.as_string() )
        {
            //if no object uuid given, just grab the first one
            if(uuid == "" || it->fObjectId.as_string() == uuid ) 
            {
                found_obj = true;
                object_key = *it;
                break;
            }
        }
    }

    if(!found_obj) 
    {
        msg_error("main", "could not locate object with class type uuid: " << uuid << eom );
        return 1;
    }

    //now compute the offset to this object
    uint64_t offset = 0;
    for(auto it2 = ikeys.begin(); it2 != it; it2++)
    {
        offset += MHO_FileKey::ByteSize(); //DO NOT USE sizeof(MHO_FileKey);
        offset += it2->fSize;
    }

    DumpToJSON(object_key, offset, filename, detail);

    return 0;
}
