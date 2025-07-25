#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <utility>
#include <map>
#include <getopt.h>

#include "MHO_ContainerStore.hh"
#include "MHO_ContainerFileInterface.hh"

#include "MHO_Tokenizer.hh"
#include "MHO_MK4VexInterface.hh"
#include "MHO_MK4StationInterface.hh"
#include "MHO_MK4StationInterfaceReversed.hh"



#ifndef HOPS3_USE_CXX
extern "C"
{
#endif
    #include "msg.h"
#ifndef HOPS3_USE_CXX
}
#endif


using namespace hops;


int main(int argc, char** argv)
{

    set_msglev(-2);

    std::string usage = "TestHOPS2StationData -r <root_file> -f <hops .sta file>";

    std::string sta_filename;
    std::string root_filename;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"hops root file", required_argument, 0, 'r'},
                                          {"hops .sta file", required_argument, 0, 'f'}};

    static const char* optString = "hr:f:";

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
            case ('r'):
                root_filename = std::string(optarg);
                break;
            case ('f'):
                sta_filename = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);


    MHO_ContainerStore conStore;
    MHO_ContainerFileInterface conInter;
    conInter.SetFilename(sta_filename);
    conInter.PopulateStoreFromFile(conStore); //reads in all the objects in a file    

    //evidently there are no double precision objects, so we look for the single-precision 'storage types'
    std::size_t n_stc = conStore.GetNObjects< station_coord_type >();
    std::size_t n_pcal = conStore.GetNObjects< multitone_pcal_type >();

    //retrieve the (first) visibility and weight objects
    //(currently assuming there is only one object per type)
    station_coord_type* sta_data = nullptr;
    sta_data = conStore.GetObject< station_coord_type >(0);

    multitone_pcal_type* pcal_data = nullptr;
    if(n_pcal){pcal_data = conStore.GetObject< multitone_pcal_type >(0);}

    MHO_MK4StationInterfaceReversed mk4inter;

    //mk4inter.SetRootFileName(root_filename);
    mk4inter.SetStationCoordData(sta_data);
    mk4inter.SetPCalData(pcal_data);
    mk4inter.SetOutputDirectory(".");
    mk4inter.GenerateStationStructure();
    mk4inter.WriteStationFile();
    mk4inter.FreeAllocated();

    return 0;
}
