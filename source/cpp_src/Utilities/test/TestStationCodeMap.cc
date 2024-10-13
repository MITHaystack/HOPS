#include <getopt.h>
#include <string>
#include <vector>

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

#include "MHO_StationCodeMap.hh"

using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "TestStationCodeMap -c <station_codes_file>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string station_codes_file = "";

    static struct option longOptions[] = {
        {"help",                no_argument,       0, 'h'},
        {"station_codes",       required_argument, 0, 'c'},
        {"enable_legacy_codes", no_argument,       0, 'e'}
    };

    static const char* optString = "hc:e";

    bool enable_legacy_station_codes = false;
    while(true)
    {
        char optId = getopt_long(argc, argv, optString, longOptions, NULL);
        if(optId == -1)
            break;
        switch(optId)
        {
            case('h'): // help
                std::cout << usage << std::endl;
                return 0;
            case('c'):
                station_codes_file = std::string(optarg);
                break;
            case('e'):
                enable_legacy_station_codes = true;
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    MHO_StationCodeMap stcode_map(enable_legacy_station_codes);
    if(station_codes_file != "")
    {
        //read and populate the station codes
        stcode_map.InitializeStationCodes(station_codes_file);
    }

    std::vector< std::string > all_mk4_ids = stcode_map.GetAllMk4Ids();

    for(auto it = all_mk4_ids.begin(); it != all_mk4_ids.end(); it++)
    {
        std::string mk4id = *it;
        std::string code = stcode_map.GetStationCodeFromMk4Id(mk4id);
        std::cout << mk4id << " <-> " << code << std::endl;
    }

    return 0;
}