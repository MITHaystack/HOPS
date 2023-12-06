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
#include "MHO_MathUtilities.hh"

#include "MHO_MultitonePhaseCorrection.hh"
#include "MHO_BasicFringeDataConfiguration.hh"


#ifdef HOPS_USE_FFTW3
#include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#else
#include "MHO_MultidimensionalFastFourierTransform.hh"
#endif

using namespace hops;





















int main(int argc, char** argv)
{
    
    //TestPCalExtraction -d ./ -s E -b GE -P X
    
    //TODO allow messaging keys to be set via command line arguments
    MHO_Message::GetInstance().AcceptAllKeys();

    //TODO make this conform/support most of the command line options of fourfit
    std::string usage = "TestPCalExtraction -d <directory> -s <station> -P <polarizaton>";

    std::string directory = "";
    std::string station = "";
    std::string pol = "";
    std::string baseline = "";
    std::string output_file = "pcal.json"; //for testing
    int message_level = -1;
    bool ok;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"directory", required_argument, 0, 'd'},
                                          {"baseline", required_argument, 0, 'b'},
                                          {"station", required_argument, 0, 's'},
                                          {"polarization", required_argument, 0, 'P'},
                                          {"message-level", required_argument, 0, 'm'},
                                          {"output", required_argument, 0, 'o'}};

    static const char* optString = "hd:b:s:P:o:m:";

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
            case ('b'):
                baseline = std::string(optarg);
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
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    if( directory == "" || baseline == "" || station == "" || pol == "" )
    {
        msg_fatal("main", "usage: "<< usage << eom);
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
    
    //load baseline data
    scanStore.LoadBaseline(baseline, &containerStore);
    MHO_BasicFringeDataConfiguration::configure_data_library(&containerStore);

    //load station data
    scanStore.LoadStation(station, &containerStore);

    visibility_type* vis_data = containerStore.GetObject<visibility_type>(std::string("vis"));
    station_coord_type* sdata = containerStore.GetObject<station_coord_type>(std::string("sta"));
    multitone_pcal_type* pcal_data = containerStore.GetObject<multitone_pcal_type>(std::string("pcal"));

    std::cout<<sdata<<std::endl;
    std::cout<<pcal_data<<std::endl;

    if(vis_data == nullptr || sdata == nullptr || pcal_data == nullptr)
    {
        msg_fatal("main", "failed to load baseline, station, or pcal data" << eom );
        std::exit(1);
    }

    MHO_MultitonePhaseCorrection pcal_corr;
    
    pcal_corr.SetStationMk4ID("E");
    pcal_corr.SetMultitonePCData(pcal_data);
    pcal_corr.SetArgs(vis_data);
    pcal_corr.Initialize();
    pcal_corr.Execute();

    return 0;
}
