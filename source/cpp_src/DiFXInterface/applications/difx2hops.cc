#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <utility>
#include <map>
#include <getopt.h>

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_VisibilityChannelizer.hh"
#include "MHO_WeightChannelizer.hh"

#include "MHO_BinaryFileStreamer.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_ClassIdentityMap.hh"

#include "MHO_DirectoryInterface.hh"
#include "MHO_DiFXInterface.hh"

#include "MHO_StationCodeMap.hh"



using namespace hops;




int main(int argc, char** argv)
{
    std::string usage = "difx2hops -i <input_directory> -c <station_codes_file> -o <output_directory>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string input_dir = "./";
    std::string output_dir = "./";
    std::string station_codes_file = "";

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"input_directory", required_argument, 0, 'i'},
                                          {"station_codes", required_argument, 0, 'c'},
                                          {"output_directory", required_argument, 0, 'o'}};

    static const char* optString = "hi:c:o:";

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
            case ('i'):
                input_dir = std::string(optarg);
                break;
            case ('c'):
                station_codes_file = std::string(optarg);
                break;
            case ('o'):
                output_dir = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    //if the output directory doesn't exist, then create it
    MHO_DirectoryInterface dirInterface;
    std::string output_directory = dirInterface.GetDirectoryFullPath(output_dir);
    bool ok = dirInterface.DoesDirectoryExist(output_directory);
    if(!ok){ ok = dirInterface.CreateDirectory(output_directory);}
    if(!ok)
    {
        msg_error("difx_interface", "Could not locate or create output directory: "<< output_directory << eom);
        std::exit(1);
    }

    MHO_StationCodeMap stcode_map; //TODO add option to enable/disable legacy code map (disabled by default)
    stcode_map.InitializeStationCodes(station_codes_file);

    MHO_DiFXInterface difxInterface;
    difxInterface.SetInputDirectory(input_dir);
    difxInterface.SetOutputDirectory(output_directory);
    difxInterface.SetStationCodes(&stcode_map);

    difxInterface.Initialize();
    difxInterface.ProcessScans();

    return 0;
}
