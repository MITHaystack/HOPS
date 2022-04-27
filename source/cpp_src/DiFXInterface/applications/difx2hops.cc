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

#include "MHO_StationCodeReader.hh"



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

    //TODO...we need to pre-pass this directory to get a list of all stations we will encounter 
    //so we can allow for a default (generic) station code mapping (Aa -> a), etc.
    //when the 2-char to 1-char map is not specified. 
    //Alternatively, we could just fall back to using 2-char codes everywhere,
    //but that may break downstream things that expect 1-char station codes.

    MHO_StationCodeReader stcode_reader;
    if(station_codes_file != "")
    {
        //read and populate the station codes
        stcode_reader.ReadStationCodes(station_codes_file);
    }
    else
    {
        msg_fatal("difx_interface", "must specify the complete 2-char to 1-char station code map, unspecified station mapping not yet supported." << eom);
        std::exit(1);
    }

    MHO_DiFXInterface dinterface;
    dinterface.SetInputDirectory(input_dir);
    dinterface.SetOutputDirectory(output_dir);
    dinterface.SetStationCodes( stcode_reader.GetStationCodeMap() );

    dinterface.Initialize();
    dinterface.ProcessScans();

    return 0;
}
