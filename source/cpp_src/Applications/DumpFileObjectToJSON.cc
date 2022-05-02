#include <getopt.h>

#include "MHO_Message.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_ContainerFileInterface.hh"

using namespace hops;


int main(int argc, char** argv)
{
    std::string usage = "DumpFileObjectToJSON -f <file> -d <detail level 0-3> -o <output_file>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string filename = "";
    std::string output_file = "";
    std::string uuid_string = "";
    int detail = eJSONAll;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"file", required_argument, 0, 'f'},
                                          {"uuid", required_argument, 0, 'u'},
                                          {"detail", required_argument, 0, 'd'},
                                          {"output", required_argument, 0, 'o'}
    };

    static const char* optString = "hf:d:u:o:";

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
            case ('d'):
                detail = std::atoi(optarg);
                break;
            case ('u'):
                uuid_string = std::string(optarg);
                break;
            case ('o'):
                output_file = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    //convert given uuid string to MHO_UUID object 
    MHO_UUID obj_uuid;
    bool ok = obj_uuid.from_string(uuid_string);
    if(!ok)
    {
        msg_fatal("main", "Could not convert given string into UUID key: " << uuid_string << eom);
        return 1;
    }

    MHO_ContainerStore conStore;
    MHO_ContainerFileInterface conInter;
    conInter.SetFilename(filename);

     //reads in all the objects in a file, this may not be super desireable for large files
    conInter.PopulateStoreFromFile(conStore);

    //convert the selected object to json 
    json obj_json;
    conInter.ConvertObjectInStoreToJSON(conStore, obj_uuid, obj_json, detail);

    //open and dump to file 
    std::ofstream outFile(output_file.c_str(), std::ofstream::out);
    outFile << obj_json;
    outFile.close();

    return 0;
}
