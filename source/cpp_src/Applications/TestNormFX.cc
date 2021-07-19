#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>

#include "MHO_Message.hh"
#include "MHO_MK4VexInterface.hh"
#include "MHO_MK4CorelInterface.hh"
#include "MHO_MK4StationInterface.hh"

#include "MHO_Visibilities.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_VisibilityChannelizer.hh"
#include "MHO_WeightChannelizer.hh"
#include "MHO_StationCoordinates.hh"

#include "MHO_DirectoryInterface.hh"

#include "MHO_NormFX.hh"

using namespace hops;


//convert a corel file
void ConvertCorel(const std::string root_file,
                  const std::string& input_file,
                  ch_baseline_data_type*& ch_bl_data,
                  ch_baseline_weight_type*& ch_bl_wdata
)
{
    MHO_MK4CorelInterface mk4inter;

    std::cout<<"input_file = "<<input_file<<std::endl;
    mk4inter.SetCorelFile(input_file);
    mk4inter.SetVexFile(root_file);
    mk4inter.ExtractCorelFile();
    baseline_data_type* bl_data = mk4inter.GetExtractedVisibilities();
    baseline_weight_type* bl_wdata = mk4inter.GetExtractedWeights();

    MHO_VisibilityChannelizer channelizer;
    channelizer.SetInput(bl_data);
    ch_bl_data = new ch_baseline_data_type();
    channelizer.SetOutput(ch_bl_data);
    bool init = channelizer.Initialize();
    if(init)
    {
        std::cout<<"initialization done"<<std::endl;
        bool exe = channelizer.ExecuteOperation();
        if(exe){std::cout<<"vis channelizer done"<<std::endl;}
    }

    MHO_WeightChannelizer wchannelizer;
    wchannelizer.SetInput(bl_wdata);
    ch_bl_wdata = new ch_baseline_weight_type();
    wchannelizer.SetOutput(ch_bl_wdata);
    bool winit = wchannelizer.Initialize();
    if(winit)
    {
        std::cout<<"initialization done"<<std::endl;
        bool wexe = wchannelizer.ExecuteOperation();
        if(wexe){std::cout<<"weight channelizer done"<<std::endl;}
    }

    delete bl_data;
    delete bl_wdata;

}



//convert a station data  file
void ConvertStation(const std::string root_file, const std::string& input_file, station_coord_data_type*& st_data)
{
    MHO_MK4StationInterface mk4inter;

    std::cout<<"input_file = "<<input_file<<std::endl;
    mk4inter.SetStationFile(input_file);
    mk4inter.SetVexFile(root_file);
    st_data = mk4inter.ExtractStationFile();
}



//in original code PARAM and STATUS structs are global extern variables
// void ConstructPassStruct();
// void ConstructParamStruct();
// void ConstructStatusStruct();


int main(int argc, char** argv)
{
    std::string usage = "ConvertMk4Data -i <input_directory> -b <baseline>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string input_dir;
    std::string baseline;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"input_directory", required_argument, 0, 'i'},
                                          {"baseline", required_argument, 0, 'b'}};

    static const char* optString = "hi:b:";

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
            case ('b'):
                baseline = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    //directory interface
    MHO_DirectoryInterface dirInterface;

    //get list of all the files (and directories) in directory
    std::vector< std::string > allFiles;
    std::vector< std::string > allDirs;

    dirInterface.GetFileList(allFiles);
    dirInterface.GetSubDirectoryList(allDirs);

    //debug
    for(auto it=allFiles.begin(); it != allFiles.end(); it++)
    {
        std::cout<<"file: "<<*it<<std::endl;
    }

    //sort files, locate root, corel and station files
    std::vector< std::string > corelFiles;
    std::vector< std::string > stationFiles;
    std::string root_file;
    dirInterface.GetRootFile(allFiles, root_file);
    std::cout<<"root file = "<<root_file<<std::endl;

    //convert root file ovex data to JSON
    MHO_MK4VexInterface vexInter;
    vexInter.OpenVexFile(root_file);
    json ovex;
    bool ovex_ok = vexInter.ExportVexFileToJSON(ovex);

    bool corel_ok = false;
    ch_baseline_data_type* ch_bl_data = nullptr;
    ch_baseline_weight_type* ch_bl_wdata = nullptr;

    dirInterface.GetCorelFiles(allFiles, corelFiles);
    for(auto it = corelFiles.begin(); it != corelFiles.end(); it++)
    {
        std::cout<<"corel file: "<< *it <<std::endl;
        std::string st_pair, root_code;
        std::string input_basename = dirInterface.GetBasename(*it);
        dirInterface.SplitCorelFileBasename(input_basename, st_pair, root_code);
        if(st_pair == baseline)
        {
            ConvertCorel(root_file, *it, ch_bl_data, ch_bl_wdata);
            if(ch_bl_data != nullptr && ch_bl_wdata != nullptr){corel_ok = true;}
            break;
        }
    }

    // dirInterface.GetStationFiles(allFiles, stationFiles);
    // for(auto it = stationFiles.begin(); it != stationFiles.end(); it++)
    // {
    //     std::cout<<"station file: "<< *it <<std::endl;
    //     std::string st, root_code;
    //     std::string input_basename = dirInterface.GetBasename(*it);
    //     dirInterface.SplitStationFileBasename(input_basename, st, root_code);
    //     std::string output_file = output_dir + "/" + st + "." + root_code + ".sta";
    //     ConvertStation(root_file, *it, output_file);
    // }


    MHO_NormFX normFXCalc;





    return 0;
}
