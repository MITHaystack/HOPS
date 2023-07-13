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

    std::string filename = "./test-container-names.bin";

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
    std::string vis_classname = MHO_ClassIdentity::ClassName<visibility_type>();
    MHO_UUID vis_id = conDict.GetUUIDFromClassName(vis_classname);
    //MHO_Serializable* generic

    visibility_type* vis = conStore.GetObject<visibility_type>(0); //get the first object with this type
    if(vis != nullptr)
    {
        std::cout<<"success, we have located a visibility object" <<std::endl;
        /* we can now do some data manipulation with the vis object... */
    }


    //convert the entire store to mho_json 
    mho_json root;
    conInter.ConvertStoreToJSON(conStore,root,eJSONTags);

    std::cout<< root.dump(2) <<std::endl;

    conStore.Clear();

    return 0;
}
