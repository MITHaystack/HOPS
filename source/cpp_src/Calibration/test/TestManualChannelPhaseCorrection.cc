#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>

#include "MHO_Message.hh"
#include "MHO_DirectoryInterface.hh"
#include "MHO_BinaryFileInterface.hh"

#include "MHO_Visibilities.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_VisibilityChannelizer.hh"
#include "MHO_WeightChannelizer.hh"
#include "MHO_StationCoordinates.hh"
#include "MHO_FFTWTypes.hh"

#include "MHO_ManualChannelPhaseCorrection.hh"

using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "TestManualChannelPhaseCorrection -d <input_directory> -b <baseline>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string directory;
    std::string baseline;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"directory", required_argument, 0, 'd'},
                                          {"baseline", required_argument, 0, 'b'}};

    static const char* optString = "hd:b:";

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
            case ('d'):
                directory = std::string(optarg);
                break;
            case ('b'):
                baseline = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    //read the directory file list
    std::vector< std::string > allFiles;
    std::vector< std::string > corFiles;
    std::vector< std::string > staFiles;
    std::vector< std::string > jsonFiles;
    MHO_DirectoryInterface dirInterface;
    dirInterface.SetCurrentDirectory(directory);
    dirInterface.ReadCurrentDirectory();

    std::cout<<"Using directory = "<<dirInterface.GetCurrentDirectory()<<std::endl;

    dirInterface.GetFileList(allFiles);
    dirInterface.GetFilesMatchingExtention(corFiles, "cor");
    dirInterface.GetFilesMatchingExtention(staFiles, "sta");
    dirInterface.GetFilesMatchingExtention(jsonFiles, "json");

    // for(auto it = corFiles.begin(); it != corFiles.end(); it++)
    // {
    //     std::cout<<"cor: "<< *it <<std::endl;
    // }
    // 
    // for(auto it = staFiles.begin(); it != staFiles.end(); it++)
    // {
    //     std::cout<<"sta: "<< *it <<std::endl;
    // }
    // for(auto it = jsonFiles.begin(); it != jsonFiles.end(); it++)
    // {
    //     std::cout<<"json: "<< *it <<std::endl;
    // }

    //check that there is only one json file
    std::string root_file = "";
    if(jsonFiles.size() != 1)
    {
        msg_fatal("main", "There are: "<<jsonFiles.size()<<" root files." << eom);
        std::exit(1);
    }
    else
    {
        root_file = jsonFiles[0];
    }

    //locate the corel file that contains the baseline of interest
    std::string corel_file = "";
    bool found_baseline = false;
    for(auto it = corFiles.begin(); it != corFiles.end(); it++)
    {
        std::size_t index = it->find(baseline);
        if(index != std::string::npos)
        {
            corel_file = *it;
            found_baseline = true;
        }
    }

    if(!found_baseline)
    {
        msg_fatal("main", "Could not find a file for baseline: "<< baseline << eom);
        std::exit(1);
    }

    std::cout<<"Will use root file: "<<root_file<<std::endl;
    std::cout<<"Will use corel file: "<<corel_file<<std::endl;

    //now open and read the (channelized) baseline visibility data
    ch_baseline_data_type* bl_data = new ch_baseline_data_type();
    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToRead(corel_file);
    if(status)
    {
        MHO_FileKey key;
        inter.Read(*bl_data, key);
        std::cout<<"Total size of baseline data = "<<bl_data->GetSerializedSize()<<std::endl;
    }
    else
    {
        std::cout<<" error opening file to read"<<std::endl;
        inter.Close();
        std::exit(1);
    }
    inter.Close();

    std::size_t bl_dim[CH_VIS_NDIM];
    bl_data->GetDimensions(bl_dim);
    //print the dimensions of this array of visibilities
    for(std::size_t i=0;i<CH_VIS_NDIM; i++)
    {
        std::cout<<"Data dimension: "<<i<<" has size: "<<bl_dim[i]<<std::endl;
    }

    //now construct the manual phase cal operator and apply it to bl_data    
    MHO_ManualChannelPhaseCorrection phase_corrector;
    phase_corrector.SetArgs(bl_data);

    //set the phase corrections here

    phase_corrector.Initialize();
    phase_corrector.Execute();

    //verify the output

    //clean up 
    delete bl_data;

    return 0;

}


