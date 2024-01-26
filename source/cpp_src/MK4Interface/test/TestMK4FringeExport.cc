#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <utility>
#include <map>
#include <getopt.h>

#include "MHO_Tokenizer.hh"
#include "MHO_MK4VexInterface.hh"
#include "MHO_MK4CorelInterface.hh"
#include "MHO_MK4FringeExport.hh"


using namespace hops;


int main(int argc, char** argv)
{

    MHO_MK4FringeExport fexporter;

    

    // std::string usage = "TestMK4FringeExport -r <root_filename> -f <corel_filename>";
    // 
    // std::string root_filename;
    // std::string corel_filename;
    // 
    // static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
    //                                       {"root (vex) file", required_argument, 0, 'r'},
    //                                       {"corel file", required_argument, 0, 'f'}};
    // 
    // static const char* optString = "hr:f:";
    // 
    // while(true)
    // {
    //     char optId = getopt_long(argc, argv, optString, longOptions, NULL);
    //     if (optId == -1)
    //         break;
    //     switch(optId)
    //     {
    //         case ('h'):  // help
    //             std::cout << usage << std::endl;
    //             return 0;
    //         case ('r'):
    //             root_filename = std::string(optarg);
    //             break;
    //         case ('f'):
    //             corel_filename = std::string(optarg);
    //             break;
    //         default:
    //             std::cout << usage << std::endl;
    //             return 1;
    //     }
    // }
    // 
    // MHO_Message::GetInstance().AcceptAllKeys();
    // MHO_Message::GetInstance().SetMessageLevel(eDebug);
    // 
    // MHO_MK4CorelInterface mk4inter;
    // 
    // mk4inter.SetCorelFile(corel_filename);
    // mk4inter.SetVexFile(root_filename);
    // mk4inter.ExtractCorelFile();
    // 
    // uch_visibility_store_type* bl_data = mk4inter.GetExtractedVisibilities();
    // uch_weight_store_type* bl_wdata = mk4inter.GetExtractedWeights(); 
    // 
    // if(bl_data == nullptr)
    // {
    //     msg_fatal("main", "Failed to extract mk4corel data." << eom);
    //     std::exit(1);
    // }

    return 0;
}
