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


// //get list of all the files (and directories) in directory
// std::vector< std::string > allFiles;
// std::vector< std::string > allDirs;
// dirInterface.GetFileList(allFiles);
// dirInterface.GetSubDirectoryList(allDirs);
//
// //sort files, locate root, corel and station files
// std::vector< std::string > corelFiles;
// std::vector< std::string > stationFiles;
// std::string root_file;
// dirInterface.GetRootFile(allFiles, root_file);
// std::cout<<"root file = "<<root_file<<std::endl;
//
// //convert root file ovex data to JSON
// MHO_MK4VexInterface vexInter;
// vexInter.OpenVexFile(root_file);
// struct vex* root = vexInter.GetVex();
// json ovex;
// bool ovex_ok = vexInter.ExportVexFileToJSON(ovex);


//read and fill-in the vex data as json and vex struct objects
bool GetVex(MHO_DirectoryInterface& dirInterface,
            MHO_MK4VexInterface& vexInterface,
            json& json_vex,
            struct vex*& root)
{
    //get list of all the files (and directories) in directory
    std::vector< std::string > allFiles;
    dirInterface.GetFileList(allFiles);
    std::string root_file;
    dirInterface.GetRootFile(allFiles, root_file);

    //convert root file ovex data to JSON, and export root/ovex ptr
    vexInterface.OpenVexFile(root_file);
    bool ovex_ok = vexInterface.ExportVexFileToJSON(json_vex);
    root = vexInterface.GetVex();
    return ovex_ok;
}


// read a corel file and fill in old style and new style data containers
bool GetCorel(MHO_DirectoryInterface& dirInterface,
              MHO_MK4CorelInterface& corelInterface,
              const std::string& baseline,
              struct mk4_corel*& cdata,
              ch_baseline_data_type*& ch_bl_data,
              ch_baseline_weight_type*& ch_bl_wdata
)
{
    bool corel_ok = false;
    // ch_baseline_data_type* ch_bl_data = nullptr;
    // ch_baseline_weight_type* ch_bl_wdata = nullptr;
    std::string root_file;
    std::vector< std::string > allFiles, corelFiles;
    dirInterface.GetFileList(allFiles);
    dirInterface.GetRootFile(allFiles, root_file);
    dirInterface.GetCorelFiles(allFiles, corelFiles);

    for(auto it = corelFiles.begin(); it != corelFiles.end(); it++)
    {
        std::string st_pair, root_code;
        std::string input_basename = dirInterface.GetBasename(*it);
        dirInterface.SplitCorelFileBasename(input_basename, st_pair, root_code);
        if(st_pair == baseline)
        {
            std::cout<<"found corel file: "<< *it <<std::endl;
            corelInterface.SetCorelFile(*it);
            corelInterface.SetVexFile(root_file);
            corelInterface.ExtractCorelFile();
            baseline_data_type* bl_data = corelInterface.GetExtractedVisibilities();
            baseline_weight_type* bl_wdata = corelInterface.GetExtractedWeights();

            MHO_VisibilityChannelizer channelizer;
            channelizer.SetInput(bl_data);
            ch_bl_data = new ch_baseline_data_type();
            channelizer.SetOutput(ch_bl_data);
            bool init = channelizer.Initialize();
            bool exe = false;
            if(init)
            {
                std::cout<<"initialization done"<<std::endl;
                exe = channelizer.ExecuteOperation();
                if(exe){std::cout<<"vis channelizer done"<<std::endl;}
            }

            MHO_WeightChannelizer wchannelizer;
            wchannelizer.SetInput(bl_wdata);
            ch_bl_wdata = new ch_baseline_weight_type();
            wchannelizer.SetOutput(ch_bl_wdata);
            bool winit = wchannelizer.Initialize();
            bool wexe = false;
            if(winit)
            {
                std::cout<<"initialization done"<<std::endl;
                wexe = wchannelizer.ExecuteOperation();
                if(wexe){std::cout<<"weight channelizer done"<<std::endl;}
            }

            //get the raw mk4 type data
            cdata = corelInterface.GetCorelData();

            if(exe && wexe){corel_ok = true;}
            break;
        }
    }
    return corel_ok;
}


// bool GetStationData(MHO_DirectoryInterface& dirInterface,
//                     MHO_MK4CorelInterface& corelInterface,
//                     const std::string& baseline,
//                     struct mk4_sda*& cdata,
//                     ch_baseline_data_type*& ch_bl_data,
//                     ch_baseline_weight_type*& ch_bl_wdata
//
//

// ////////////////////////////////////////////////////////////////////////////////
//
// //convert a corel file
// void ConvertCorel(const std::string root_file,
//                   const std::string& input_file,
//                   ch_baseline_data_type*& ch_bl_data,
//                   ch_baseline_weight_type*& ch_bl_wdata
// )
// {

// }
//
//

// //convert a station data  file
// void ConvertStation(const std::string root_file, const std::string& input_file, station_coord_data_type*& st_data)
// {
//     MHO_MK4StationInterface mk4inter;
//
//     std::cout<<"input_file = "<<input_file<<std::endl;
//     mk4inter.SetStationFile(input_file);
//     mk4inter.SetVexFile(root_file);
//     st_data = mk4inter.ExtractStationFile();
// }
//
//
//
//
//

























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
    dirInterface.SetCurrentDirectory(input_dir);
    dirInterface.ReadCurrentDirectory();

    //get the root (ovex) file information
    MHO_MK4VexInterface vexInterface;
    json json_vex;
    struct vex* root = nullptr;
    bool ovex_ok = GetVex(dirInterface, vexInterface, json_vex, root);

    MHO_MK4CorelInterface corelInterface;
    struct mk4_corel* cdata = nullptr;
    ch_baseline_data_type* ch_bl_data = nullptr;
    ch_baseline_weight_type* ch_bl_wdata = nullptr;
    bool corel_ok = GetCorel(dirInterface, corelInterface, baseline, cdata, ch_bl_data, ch_bl_wdata);
    std::cout<<"data ptrs = "<<cdata<<", "<<ch_bl_data<<", "<<ch_bl_wdata<<std::endl;

    MHO_MK4StationInterface stationInterface;
    struct mk4_sdata sdata[2];



/*

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
