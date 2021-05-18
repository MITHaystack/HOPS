#include <getopt.h>
#include "MHO_Message.hh"

#include "MHO_BinaryFileInterface.hh"
#include "MHO_ContainerDictionary.hh"


using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "DumpFileObjectKeys -f <index file>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string index_filename;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"index file", required_argument, 0, 'f'}};

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
                index_filename = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    //lets extract all of the object keys in the index file just for inspection
    std::vector< MHO_FileKey > ikeys;

    MHO_ContainerDictionary cdict;

    MHO_BinaryFileInterface inter;
    bool result = inter.ExtractObjectKeys(index_filename, ikeys);

    for(auto it = ikeys.begin(); it != ikeys.end(); it++)
    {
        std::cout<<"key:"<<std::endl;
        std::cout<<"sync: "<<it->fSync<<std::endl;
        std::cout<<"label: "<<it->fLabel<<std::endl;
        std::cout<<"object uuid: "<<it->fObjectId.as_string()<<std::endl;
        std::cout<<"type uuid: "<<it->fTypeId.as_string()<<std::endl;
        std::string class_name = cdict.GetClassNameFromUUID(it->fTypeId);
        std::cout<<"class name = "<<class_name<<std::endl;
        std::cout<<"size (bytes): "<<it->fSize<<std::endl;
        std::cout<<"----"<<std::endl;
    }

    return 0;
}
