#include <getopt.h>
#include "MHO_Message.hh"

#include "MHO_BinaryFileInterface.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_Visibilities.hh"


using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "DumpFileObjectKeys -f <file>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string filename;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"file", required_argument, 0, 'f'}};

    static const char* optString = "hf:";

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
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    //the container dictionay lets us look the names of (known) file objects
    MHO_ContainerDictionary cdict;

    //now lets extract all of the object keys in file for inspection
    std::vector< MHO_FileKey > ikeys;

    MHO_BinaryFileInterface inter;
    bool result = false;
    bool is_index_file = false;
    std::string index_ext = ".index";
    if(filename.size() > 6 )
    {
        auto pos = filename.find_last_of(index_ext) - (index_ext.size()-1);
        std::string tail = filename.substr(pos);
        if(tail == index_ext){is_index_file = true;}
    }

    if(is_index_file )
    {
        //key extraction from an index file is different b/c there are no objects to skip over
        result = inter.ExtractIndexFileObjectKeys(filename, ikeys);
    }
    else 
    {
        //regular key extraction skips over the objects, just pulling keys
        result = inter.ExtractFileObjectKeys(filename, ikeys);
    }


    for(auto it = ikeys.begin(); it != ikeys.end(); it++)
    {
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
        std::cout<<"------------------------------------------------------------"<<std::endl;
    }

    return 0;
}
