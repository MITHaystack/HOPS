#include <getopt.h>
#include "MHO_Message.hh"

#include "MHO_BinaryFileInterface.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_Visibilities.hh"
#include "MHO_JSONHeaderWrapper.hh"


using namespace hops;


void DumpToJSON(const ch_baseline_data_type& ch_vis, const MHO_FileKey& key, json& json_obj)
{
    json_obj["name"] = key.fName; 
    std::stringstream ss2;
    ss2 << std::hex << key.fLabel;
    json_obj["label"] = ss2.str();
    json_obj["type_uuid"] = key.fTypeId.as_string(); 
    json_obj["object_uuid"] = key.fObjectId.as_string(); 
    json_obj["size_bytes"] = key.fSize; 
    json_obj["rank"] = ch_vis.GetRank();
    json_obj["total_size"] = ch_vis.GetSize();
    json dim_array = ch_vis.GetDimensionArray();
    json stride_array = ch_vis.GetStrideArray();
    json_obj["dimensions"] = dim_array;
    json_obj["strides"] = stride_array;
    
    json ax0;
    auto polprod_axis = &(std::get<CH_POLPROD_AXIS>(ch_vis));
    for(auto it = polprod_axis->cbegin(); it != polprod_axis->cend(); it++)
    {
        ax0.push_back(*it);
    }
    json_obj["axis_0"] = ax0;


    json ax1;
    auto ch_axis = &(std::get<CH_CHANNEL_AXIS>(ch_vis));
    for(auto it = ch_axis->cbegin(); it != ch_axis->cend(); it++)
    {
        ax1.push_back(*it);
    }
    json_obj["axis_1"] = ax1;


    json ax2;
    auto ap_axis = &(std::get<CH_TIME_AXIS>(ch_vis));
    for(auto it = ap_axis->cbegin(); it != ap_axis->cend(); it++)
    {
        ax2.push_back(*it);
    }
    json_obj["axis_2"] = ax2;


    json ax3;
    auto sp_axis = &(std::get<CH_FREQ_AXIS>(ch_vis));
    for(auto it = sp_axis->cbegin(); it != sp_axis->cend(); it++)
    {
        ax3.push_back(*it);
    }
    json_obj["axis_3"] = ax3;


    json ch_vis_data;
    for(auto it = ch_vis.cbegin(); it != ch_vis.cend(); it++)
    {
        
        ch_vis_data.push_back({it->real(), it->imag()});
    }
    json_obj["data"] = ch_vis_data;


    std::cout<<json_obj.dump(2)<<std::endl; //dump the json to terminal

}




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
        //now compute the offset to this object
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
            std::cout<<"Total size of baseline data = "<<ch_vis.GetSerializedSize()<<std::endl;
            json json_obj;
            DumpToJSON(ch_vis, key, json_obj);
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
