#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "MHO_Message.hh"
#include "MHO_Snapshot.hh"

//parse_command_line
#include <getopt.h>


//data/config passing classes
#include "MHO_ParameterStore.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_JSONHeaderWrapper.hh"

//needed to read hops files and extract objects from scan dir
#include "MHO_ScanDataStore.hh"

//initialization
#include "MHO_OperatorBuilderManager.hh"
#include "MHO_ParameterConfigurator.hh"
#include "MHO_ParameterManager.hh"

#include "MHO_VexInfoExtractor.hh"

using namespace hops;

int main(int argc, char** argv)
{
    //TODO allow messaging keys to be set via command line arguments
    MHO_Message::GetInstance().AcceptAllKeys();

    //TODO make this conform/support most of the command line options of fourfit
    std::string usage = "TestPCalExtraction -d <directory> -s <station> -P <polarizaton>";

    std::string directory = "";
    std::string station = "";
    std::string pol = "";
    std::string output_file = "pcal.json"; //for testing
    double lower_freq = -100.0;
    double upper_freq = -100.0;
    int message_level = -1;
    bool ok;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"directory", required_argument, 0, 'd'},
                                          {"station", required_argument, 0, 's'},
                                          {"polarization", required_argument, 0, 'P'},
                                          {"message-level", required_argument, 0, 'm'},
                                          {"lower-frequency", required_argument, 0, 'l'},
                                          {"upper-frequency", required_argument, 0, 'u'},
                                          {"output", required_argument, 0, 'o'}};

    static const char* optString = "hd:s:P:o:m:l:u:";

    while(true)
    {
        char optId = getopt_long(argc, argv, optString, longOptions, NULL);
        if (optId == -1)
            break;
        switch(optId)
        {
            case ('h'):  // help
                std::cout << usage << std::endl;
                std::exit(0);
            case ('d'):
                directory = std::string(optarg);
                break;
            case ('s'):
                station = std::string(optarg);
                break;
            case ('P'):
                pol = std::string(optarg);
                break;
            case ('o'):
                output_file = std::string(optarg);
                break;
            case ('m'):
                message_level = std::atoi(optarg);
                break;
            case ('l'):
                lower_freq = std::atof(optarg);
                break;
            case ('u'):
                upper_freq = std::atof(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    if( directory == "" || station == "" || pol == "" )
    {
        msg_fatal("main", "usage: "<< usage << eom);
        return 1;
    }

    std::cout<<"freq bounds = "<<lower_freq<<", "<<upper_freq<<std::endl;

    if(lower_freq < 0 || upper_freq < 0)
    {
        msg_fatal("main", "lower/upper channel frequency limits must be non-zero and specificied in MHz." << eom );
        return 1;
    }


    //set the message level according to the fourfit style
    //where 3 is least verbose, and '-1' is most verbose
    switch (message_level)
    {
        case -2:
            //NOTE: debug messages must be compiled-in
            #ifndef HOPS_ENABLE_DEBUG_MSG
            MHO_Message::GetInstance().SetMessageLevel(eInfo);
            msg_warn("fringe", "debug messages are toggled via compiler flag, re-compile with ENABLE_DEBUG_MSG=ON to enable." << eom);
            #else
            MHO_Message::GetInstance().SetMessageLevel(eDebug);
            #endif
        break;
        case -1:
            MHO_Message::GetInstance().SetMessageLevel(eInfo);
        break;
        case 0:
            MHO_Message::GetInstance().SetMessageLevel(eStatus);
        break;
        case 1:
            MHO_Message::GetInstance().SetMessageLevel(eWarning);
        break;
        case 2:
            MHO_Message::GetInstance().SetMessageLevel(eError);
        break;
        case 3:
            MHO_Message::GetInstance().SetMessageLevel(eFatal);
        break;
        case 4:
            MHO_Message::GetInstance().SetMessageLevel(eSilent);
        break;
        default:
            //for now default is most verbose, eventually will change this to silent
            MHO_Message::GetInstance().SetMessageLevel(eDebug);
    }

    if(station.size() != 1)
    {
        msg_fatal("main", "station must be passed as 1-char mk4-id code."<< eom);
        return 1;
    }

    //data objects
    MHO_ParameterStore paramStore; //stores various parameters using string keys
    MHO_ScanDataStore scanStore; //provides access to data associated with this scan
    MHO_ContainerStore containerStore; //stores data containers for in-use data


    //initialize the scan store from this directory
    scanStore.SetDirectory(directory);
    scanStore.Initialize();
    if( !scanStore.IsValid() )
    {
        msg_fatal("fringe", "cannot initialize a valid scan store from this directory: " << directory << eom);
        std::exit(1);
    }

    //set the root file name
    paramStore.Set("/files/root_file", scanStore.GetRootFileBasename() );

    //load root file and extract useful vex info
    auto vexInfo = scanStore.GetRootFileData();
    //MHO_VexInfoExtractor::extract_vex_info(vexInfo, &paramStore);

    //load station data
    scanStore.LoadStation(station, &containerStore);

    station_coord_type* sdata = containerStore.GetObject<station_coord_type>(std::string("sta"));
    multitone_pcal_type* pcal_data = containerStore.GetObject<multitone_pcal_type>(std::string("pcal"));

    std::cout<<sdata<<std::endl;
    std::cout<<pcal_data<<std::endl;

    if(sdata == nullptr || pcal_data == nullptr)
    {
        msg_fatal("main", "failed to load station or pcal data" << eom );
        std::exit(1);
    }


    std::size_t rank = pcal_data->GetRank();
    auto dims = pcal_data->GetDimensionArray();

    for(std::size_t i=0; i<rank; i++)
    {
        std::cout<<"pcal dim @"<<i<<" = "<<dims[i]<<std::endl;
    }

    auto tone_freq_ax = std::get<MTPCAL_FREQ_AXIS>(*pcal_data);
    double start_tone_frequency = 0;
    std::size_t start_tone_index = 0;
    std::size_t ntones = 0;
    for(std::size_t j=0; j<tone_freq_ax.GetSize(); j++)
    {
        if( tone_freq_ax(j) < upper_freq && lower_freq <= tone_freq_ax(j) )
        {
            if(ntones == 0)
            {
                start_tone_frequency = tone_freq_ax(j) ;
                start_tone_index = j;
            }
            ntones++;
            std::cout<<"tone: "<<j<<" = "<<tone_freq_ax(j)<<std::endl;
        }
    }
    std::cout<<"start tone = "<<start_tone_frequency<<", start tone index = "<<start_tone_index<<", ntones = "<<ntones<<std::endl;

    //should we channelize the pca-data? yes...but for now just do an FFT on one chunk to test





    return 0;
}
