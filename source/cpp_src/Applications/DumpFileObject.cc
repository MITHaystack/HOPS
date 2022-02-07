#include <getopt.h>
#include "MHO_Message.hh"

#include "MHO_BinaryFileInterface.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_Visibilities.hh"


using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "DumpFileObject -f <file> -t <type> -u <uuid>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    //TODO extend this to other container types
    msg_warn("main", "currenly only implemented for the visibility container type." << eom);

    std::string filename;
    std::string uuid;
    std::string type;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"file", required_argument, 0, 'f'},
                                          {"type", required_argument, 0, 't'},
                                          {"uuid", required_argument, 0, 'u'}
    };

    static const char* optString = "hf:t:u:";

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
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    //the container dictionay lets us look up the names of (known) file objects
    MHO_ContainerDictionary cdict;

    //only support for 
    ch_baseline_data_type ch_vis;
    std::string ch_vis_classname = cdict.GetClassNameFromObject(ch_vis);
    MHO_UUID ch_vis_classuuid = cdict.GetUUIDFromClassName(ch_vis_classname);

    MHO_UUID classuuid;
    bool success = classuuid.from_string(type);
    if(!success){std::cout<<"type uuid could not be converted"<<std::endl;}

    std::cout<<"type uuids: "<<classuuid.as_string()<<", "<<ch_vis_classuuid.as_string()<<", "<<type<<std::endl;

    //pull the file object keys for inspection 
    std::vector< MHO_FileKey > ikeys;

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
            msg_info("main", "found a visibility object with:")
            std::cout<<"key:"<<std::endl;
            std::stringstream ss1; 
            ss1 << std::hex << it->fSync;
            std::cout<<"    sync: "<<ss1.str()<<std::endl;
            std::stringstream ss2;
            ss2 << std::hex << it->fLabel;
            std::cout<<"    label: "<<ss2.str()<<std::endl;
            std::cout<<"    object uuid: "<<it->fObjectId.as_string()<<std::endl;
            std::cout<<"    type uuid: "<<it->fTypeId.as_string()<<std::endl;
            std::string class_name = cdict.GetClassNameFromUUID(it->fTypeId);
            std::cout<<"    class name: "<<class_name<<std::endl;
            std::cout<<"    object name: "<<it->fName<<std::endl;
            std::cout<<"    size (bytes): "<<it->fSize<<std::endl;
            found_obj = true;
            break;
        }
    }

    if(found_obj)
    {
        //now compute the offset to this objects 
        uint64_t offset = 0;
        for(auto it2 = ikeys.begin(); it2 != it; it2++)
        {
            offset += sizeof(MHO_FileKey);
            offset += it2->fSize;
        }

        //now open and read the (channelized) baseline visibility data
        bool status = inter.OpenToReadAtOffset(filename, offset);
        if(status)
        {
            MHO_FileKey key;
            inter.Read(ch_vis, key);
            //std::cout<<"baseline object label = "<<blabel<<std::endl;
            std::cout<<"Total size of baseline data = "<<ch_vis.GetSerializedSize()<<std::endl;
        }
        else
        {
            std::cout<<" error opening file to read"<<std::endl;
            inter.Close();
            std::exit(1);
        }
        inter.Close();
    }
    else 
    {
        msg_error("main", "could not locate object with class type uuid: " << uuid << eom );
    }

    return 0;
}
