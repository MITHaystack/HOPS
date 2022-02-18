#include <getopt.h>
#include "MHO_Message.hh"

#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_ContainerFileInterface.hh"

using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "TestContainerStore -f <file>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string filename = "";

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"file", required_argument, 0, 'f'}
    };

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

    MHO_ContainerDictionary conDict;
    MHO_ContainerStore conStore;
    MHO_ContainerFileInterface conInter;
    conInter.SetFilename(filename);
    conInter.PopulateStoreFromFile(conStore); //reads in all the objects in a file

    //all file objects are now in memory
    std::cout<<"library has: "<<conStore.GetNObjects()<<" objects."<<std::endl;

    //here's how we would retrieve a 'visibility' object from the library
    std::string vis_classname = MHO_ClassIdentity::ClassName<ch_baseline_data_type>();
    MHO_UUID vis_id = conDict.GetUUIDFromClassName(vis_classname);
    MHO_Serializable* generic = conStore.RetrieveFirstObjectMatchingType(vis_id);
    if(generic != nullptr)
    {
        ch_baseline_data_type* vis = dynamic_cast<ch_baseline_data_type*>(generic);
        if(vis != nullptr)
        {
            std::cout<<"success, we have located a visibility object" <<std::endl;
            /* we can now do some data manipulation with the vis object... */
        }
    }

    //convert the entire store to json 
    json root;
    conInter.ConvertStoreToJSON(conStore,root,eJSONTags);

    std::cout<< root.dump(2) <<std::endl;

    return 0;
}
