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
#include "MHO_ContainerDefinitions.hh"
#include "MHO_MK4StationInterface.hh"


using namespace hops;


int main(int argc, char** argv)
{
    std::string usage = "TestMK4StationImport -r <root_filename> -f <station_filename>";

    std::string root_filename;
    std::string station_filename;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"root (vex) file", required_argument, 0, 'r'},
                                          {"station file", required_argument, 0, 's'}};

    static const char* optString = "hr:s:";

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
            case ('s'):
                station_filename = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    MHO_MK4StationInterface mk4inter;

    mk4inter.SetStationFile(station_filename);
    mk4inter.SetVexFile(root_filename);
    station_coord_type* st_data = mk4inter.ExtractStationFile();

    std::size_t dim[STATION_NDIM];
    st_data->GetDimensions(dim);

    std::cout<<"dimensions = "<<dim[0]<<", "<<dim[1]<<", "<<dim[2]<<", "<<dim[3]<<std::endl;

    auto* coord_axis = &(std::get<COORD_AXIS>(*st_data));
    auto* int_axis = &(std::get<INTERVAL_AXIS>(*st_data));
    auto* coeff_axis = &(std::get<COEFF_AXIS>(*st_data));
    size_t coord_axis_size = coord_axis->GetSize();
    size_t int_axis_size = int_axis->GetSize();
    size_t coeff_axis_size = coeff_axis->GetSize();

    //lets print out the axis labels
    std::cout<<"coord axis labels = "<<std::endl;
    for(std::size_t i=0; i<coord_axis_size; i++)
    {
        std::cout<<coord_axis->at(i)<<", ";
    }
    std::cout<<std::endl;

    //lets print out the axis labels
    std::cout<<"interval axis labels = "<<std::endl;
    for(std::size_t i=0; i<int_axis_size; i++)
    {
        std::cout<<int_axis->at(i)<<", ";
    }
    std::cout<<std::endl;

    //lets print out the axis labels
    std::cout<<"coeff axis labels = "<<std::endl;
    for(std::size_t i=0; i<coeff_axis_size; i++)
    {
        std::cout<<coeff_axis->at(i)<<", ";
    }
    std::cout<<std::endl;



    return 0;
}
