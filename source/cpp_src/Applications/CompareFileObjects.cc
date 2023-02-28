#include <getopt.h>

#include "MHO_Message.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_ContainerFileInterface.hh"

using namespace hops;


int main(int argc, char** argv)
{
    std::string usage = "CompareFileObjects -a <file1> -b <file2> -x <uuid1> -y <uuid2>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string file1 = "";
    std::string file2 = "";
    std::string uuid1 = "";
    std::string uuid2 = "";
    int detail = eJSONAll;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"file1", required_argument, 0, 'a'},
                                          {"file2", required_argument, 0, 'b'},
                                          {"uuid1", required_argument, 0, 'x'},
                                          {"uuid2", required_argument, 0, 'y'}
    };

    static const char* optString = "ha:b:x:y:";

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
            case ('a'):
                file1 = std::string(optarg);
                break;
            case ('b'):
                file2 = std::atoi(optarg);
                break;
            case ('x'):
                uuid1 = std::string(optarg);
                break;
            case ('y'):
                uuid2 = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    //convert given uuid string to MHO_UUID object 
    MHO_UUID obj_uuid1;
    bool ok1 = obj_uuid1.from_string(uuid1);

    //convert given uuid string to MHO_UUID object 
    MHO_UUID obj_uuid2;
    bool ok2 = obj_uuid2.from_string(uuid2);

    if(!ok1 || !ok2)
    {
        msg_fatal("main", "could not convert given strings into UUID keys." eom);
        return 1;
    }

    MHO_ContainerStore conStore1;
    MHO_ContainerFileInterface conInter1;
    conInter1.SetFilename(file1);

    MHO_ContainerStore conStore2;
    MHO_ContainerFileInterface conInter2;
    conInter2.SetFilename(file2);

     //reads in all the objects in a file, this may not be super desireable for large files
    conInter1.PopulateStoreFromFile(conStore1);
    conInter2.PopulateStoreFromFile(conStore2);

    //retrieve the objects 

    //now do the comparison

    return 0;
}
