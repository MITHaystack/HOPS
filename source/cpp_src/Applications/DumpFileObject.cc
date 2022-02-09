#include <getopt.h>
#include "MHO_Message.hh"

#include "MHO_BinaryFileInterface.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_Visibilities.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_ContainerJSON.hh"


using namespace hops;


template< typename XObjectType>
void ReadAndDump(MHO_FileKey& object_key, uint64_t offset, std::string filename)
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
        //TODO -- figure out what rule in the c++ spec demands we use the 'template' keyword here
        obj.template MakeExtension< MHO_ContainerJSON< XObjectType > >();
        json_obj = obj.template AsExtension< MHO_ContainerJSON< XObjectType > >()->GetJSON();

        //dump the json to terminal -- TODO, replace this with a visitor which can handle multiple verbosity levels
        if(json_obj){std::cout<<json_obj->dump(2)<<std::endl; }
    }
    else
    {
        msg_error("main", "error opening file with byte offset: " << offset << eom );
    }
    inter.Close();
}


void DumpToJSON(MHO_FileKey& object_key, uint64_t offset, std::string filename)
{
    MHO_ContainerDictionary cdict;
    //switch off of the type id and cast to the underlying type 
    uint64_t tid = object_key.fTypeId.as_long();

    if(tid == cdict.GetUUIDFor<ch_baseline_data_type>().as_long() )
    {
        ReadAndDump<ch_baseline_data_type>(object_key, offset, filename);
        return;
    }

    if(tid == cdict.GetUUIDFor<ch_baseline_weight_type>().as_long() )
    {
        ReadAndDump<ch_baseline_weight_type>(object_key, offset, filename);
        return;
    }

    msg_error("main", "cannot identify an object with type id: "<< object_key.fTypeId.as_string() << eom);

}



    // //should we dump file-key information??
    // json_obj["name"] = key.fName; 
    // std::stringstream ss2;
    // ss2 << std::hex << key.fLabel;
    // json_obj["label"] = ss2.str();
    // json_obj["type_uuid"] = key.fTypeId.as_string(); 
    // json_obj["object_uuid"] = key.fObjectId.as_string(); 
    // json_obj["size_bytes"] = key.fSize; 
    // 
    // 
    // 
    // //object data
    // json_obj["rank"] = ch_vis.GetRank();
    // json_obj["total_size"] = ch_vis.GetSize();
    // json dim_array = ch_vis.GetDimensionArray();
    // json stride_array = ch_vis.GetStrideArray();
    // json_obj["dimensions"] = dim_array;
    // json_obj["strides"] = stride_array;
    // 
    // json ax0;
    // auto polprod_axis = &(std::get<CH_POLPROD_AXIS>(ch_vis));
    // for(auto it = polprod_axis->cbegin(); it != polprod_axis->cend(); it++)
    // {
    //     ax0.push_back(*it);
    // }
    // json_obj["axis_0"] = ax0;
    // 
    // 
    // json ax1;
    // auto ch_axis = &(std::get<CH_CHANNEL_AXIS>(ch_vis));
    // for(auto it = ch_axis->cbegin(); it != ch_axis->cend(); it++)
    // {
    //     ax1.push_back(*it);
    // }
    // json_obj["axis_1"] = ax1;
    // 
    // 
    // json ax2;
    // auto ap_axis = &(std::get<CH_TIME_AXIS>(ch_vis));
    // for(auto it = ap_axis->cbegin(); it != ap_axis->cend(); it++)
    // {
    //     ax2.push_back(*it);
    // }
    // json_obj["axis_2"] = ax2;
    // 
    // 
    // json ax3;
    // auto sp_axis = &(std::get<CH_FREQ_AXIS>(ch_vis));
    // for(auto it = sp_axis->cbegin(); it != sp_axis->cend(); it++)
    // {
    //     ax3.push_back(*it);
    // }
    // json_obj["axis_3"] = ax3;
    // 
    // 
    // json ch_vis_data;
    // for(auto it = ch_vis.cbegin(); it != ch_vis.cend(); it++)
    // {
    // 
    //     ch_vis_data.push_back({it->real(), it->imag()});
    // }
    // json_obj["data"] = ch_vis_data;



int main(int argc, char** argv)
{
    std::string usage = "DumpFileObject -f <file> -t <type> -u <uuid>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eWarning);

    // //TODO extend this to other container types
    // msg_warn("main", "currenly only implemented for the visibility container type." << eom);

    std::string filename = "";
    std::string uuid = "";
    std::string type = "";

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

    MHO_UUID classuuid;
    bool success = classuuid.from_string(type);
    if(!success){msg_error("main", "could not convert object (-u) string to uuid" << eom);}

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
        offset += sizeof(MHO_FileKey);
        offset += it2->fSize;
    }

    std::cout<<"offset of size: "<<offset<<std::endl;

    DumpToJSON(object_key, offset, filename);

    return 0;
}
