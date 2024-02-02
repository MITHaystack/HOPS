#include <getopt.h>

#include "MHO_Message.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_ContainerFileInterface.hh"

using namespace hops;


int main(int argc, char** argv)
{
    std::string usage = "hops2json -f <file> -d <detail level: 0 (low) to 4 (high)> -o <output_file>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string filename = "";
    std::string output_file = "";
    int detail = eJSONAll;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"file", required_argument, 0, 'f'},
                                          {"detail", required_argument, 0, 'd'},
                                          {"output", required_argument, 0, 'o'}
    };

    static const char* optString = "hf:d:o:";

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
            case ('o'):
                output_file = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    MHO_ContainerStore conStore;
    MHO_ContainerFileInterface conInter;
    conInter.SetFilename(filename);
    conInter.PopulateStoreFromFile(conStore); //reads in all the objects in a file

    //all file objects are now in memory
    std::cout<<"Converting "<<conStore.GetNObjects()<<" objects."<<std::endl;

    //convert the entire store to json 
    mho_json root;
    conInter.ConvertStoreToJSON(conStore,root,detail);

    //open and dump to file 
    std::ofstream outFile(output_file.c_str(), std::ofstream::out);
    outFile << root;
    outFile.close();

    std::cout<<"Done."<<std::endl;

    return 0;
}
