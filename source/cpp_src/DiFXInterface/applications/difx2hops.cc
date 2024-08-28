#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <set>
#include <utility>
#include <map>
#include <getopt.h>

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"
#include "MHO_StationCodeMap.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_VisibilityChannelizer.hh"
#include "MHO_WeightChannelizer.hh"

#include "MHO_BinaryFileStreamer.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_ClassIdentityMap.hh"

#include "MHO_DirectoryInterface.hh"
#include "MHO_DiFXInterface.hh"

using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "difx2hops -e <exp. number> -i <input_directory> -c <station_codes_file> -o <output_directory>";

    //MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);
    MHO_Message::GetInstance().AddKey("difx_interface");

    std::string input_dir = "./";
    std::string output_dir = "./";
    bool output_dir_specified = false;
    std::string station_codes_file = "";
    int exper_num = 1234;
    bool normalize = true;
    bool preserve = false;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"input_directory", required_argument, 0, 'i'},
                                          {"station_codes", required_argument, 0, 'c'},
                                          {"experiment_number", required_argument, 0, 'e'},
                                          {"output_directory", required_argument, 0, 'o'},
                                          {"raw", no_argument, 0, 'r'}, //turns on 'raw' mode, no normalization done
                                          {"preserve", no_argument, 0, 'p'} //uses original difx scan names to name the scans (otherwise uses DOY-HHMM)
                                        };

    static const char* optString = "hi:c:e:o:rp";

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
            case ('e'):
                exper_num = std::atoi(optarg);
                break;
            case ('o'):
                output_dir = std::string(optarg);
                output_dir_specified = true;
                break;
            case ('r'):
                normalize = false;
                break;
            case ('p'):
                preserve = true;
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    //if not output directory was specified assume we are going to dump the
    //converted data into <input_directory>/exper_num
    if(!output_dir_specified)
    {
        std::stringstream ss;
        ss << exper_num;
        output_dir = input_dir + "/" + ss.str();
    }

    //if the output directory doesn't exist, then create it
    MHO_DirectoryInterface dirInterface;
    std::string output_directory = dirInterface.GetDirectoryFullPath(output_dir);
    bool ok = dirInterface.DoesDirectoryExist(output_directory);
    if(!ok){ ok = dirInterface.CreateDirectory(output_directory);}
    if(!ok)
    {
        msg_fatal("difx_interface", "Could not locate or create output directory: "<< output_directory << eom);
        std::exit(1);
    }

    //TODO add option to enable/disable legacy code map (disabled by default)
    MHO_StationCodeMap stcode_map;
    stcode_map.InitializeStationCodes(station_codes_file);

    MHO_DiFXInterface difxInterface;
    difxInterface.SetInputDirectory(input_dir);
    difxInterface.SetOutputDirectory(output_directory);
    difxInterface.SetStationCodes(&stcode_map);
    difxInterface.SetExperimentNumber(exper_num);
    difxInterface.SetNormalizeFalse();
    if(normalize){difxInterface.SetNormalizeTrue();}
    if(preserve){difxInterface.SetPreserveDiFXScanNamesTrue();}

    difxInterface.Initialize();
    difxInterface.ProcessScans();

    return 0;
}
