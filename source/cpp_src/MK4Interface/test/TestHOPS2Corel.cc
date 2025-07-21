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
#include "MHO_MK4CorelInterface.hh"
#include "MHO_MK4CorelInterfaceReversed.hh"


using namespace hops;


int main(int argc, char** argv)
{
    std::string usage = "TestHOPS2Corel -f <hops .cor file>";

    std::string cor_filename;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"hops .cor file", required_argument, 0, 'f'}};

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
                cor_filename = std::string(optarg);
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
    conInter.SetFilename(cor_filename);
    conInter.PopulateStoreFromFile(conStore); //reads in all the objects in a file    

    //evidently there are no double precision objects, so we look for the single-precision 'storage types'
    std::size_t n_vis = conStore.GetNObjects< visibility_store_type >();
    std::size_t n_wt = conStore.GetNObjects< weight_store_type >();

    //retrieve the (first) visibility and weight objects
    //(currently assuming there is only one object per type)
    visibility_store_type* vis_store_data = nullptr;
    weight_store_type* wt_store_data = nullptr;

    vis_store_data = conStore.GetObject< visibility_store_type >(0);
    wt_store_data = conStore.GetObject< weight_store_type >(0);

    MHO_MK4CorelInterfaceReversed mk4inter;

    mk4inter.SetVisibilityData(vis_store_data);
    mk4inter.SetWeightData(wt_store_data);

    struct mk4_corel* mk4c = mk4inter.GenerateCorelStructure();

    mk4inter.SetOutputFile("GE..0VSI1M");
    mk4inter.WriteCorelFile();

    return 0;
}
