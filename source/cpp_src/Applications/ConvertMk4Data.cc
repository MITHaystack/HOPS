#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <utility>
#include <map>
#include <getopt.h>


//needed for listing/navigating files/directories on *nix
#include <dirent.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fstream>

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"
#include "MHO_MK4VexInterface.hh"
#include "MHO_MK4CorelInterface.hh"
#include "MHO_MK4StationInterface.hh"

#include "MHO_Reducer.hh"
#include "MHO_FunctorBroadcaster.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"

#include "MHO_Visibilities.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_VisibilityChannelizer.hh"
#include "MHO_WeightChannelizer.hh"
#include "MHO_StationCoordinates.hh"

#include "MHO_BinaryFileStreamer.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_ClassIdentityMap.hh"

#include "MHO_DirectoryInterface.hh"

using namespace hops;


//convert a corel file
void ConvertCorel(const std::string root_file, const std::string& input_file, const std::string& output_file)
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
    ch_baseline_data_type* ch_bl_data = new ch_baseline_data_type();
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
    ch_baseline_weight_type* ch_bl_wdata = new ch_baseline_weight_type();
    wchannelizer.SetOutput(ch_bl_wdata);
    bool winit = wchannelizer.Initialize();
    if(winit)
    {
        std::cout<<"initialization done"<<std::endl;
        bool wexe = wchannelizer.ExecuteOperation();
        if(wexe){std::cout<<"weight channelizer done"<<std::endl;}
    }

    //std::string index_file = output_file + ".index";
    //bool status = inter.OpenToWrite(output_file, index_file);
    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(output_file);
    if(status)
    {
        uint32_t label = 0xFFFFFFFF;
        inter.Write(*ch_bl_data, "vis", label);
        inter.Write(*ch_bl_wdata, "weight", label);
        inter.Close();
    }
    else
    {
        std::cout<<"error opening file"<<std::endl;
    }

    inter.Close();

    delete bl_data;
    delete ch_bl_data;
}



//convert a station data  file
void ConvertStation(const std::string root_file, const std::string& input_file, const std::string& output_file)
{
    MHO_MK4StationInterface mk4inter;

    std::cout<<"input_file = "<<input_file<<std::endl;
    mk4inter.SetStationFile(input_file);
    mk4inter.SetVexFile(root_file);
    station_coord_data_type* st_data = mk4inter.ExtractStationFile();

    MHO_BinaryFileInterface inter;
    //std::string index_file = output_file + ".index";
    //bool status = inter.OpenToWrite(output_file, index_file);

    bool status = inter.OpenToWrite(output_file);
    if(status)
    {
        uint32_t label = 0xFFFFFFFF;
        inter.Write(*st_data, "sta", label);
        inter.Close();
    }
    else
    {
        std::cout<<"error opening file"<<std::endl;
    }

    inter.Close();
    delete st_data;
}




int main(int argc, char** argv)
{
    std::string usage = "ConvertMk4Data -i <input_directory> -o <output_directory>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string input_dir;
    std::string output_dir, odir;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"input_directory", required_argument, 0, 'i'},
                                          {"output_directory", required_argument, 0, 'o'}};

    static const char* optString = "hi:o:";

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
            case ('o'):
                odir = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    //directory interface
    MHO_DirectoryInterface dirInterface;
    output_dir = dirInterface.GetDirectoryFullPath(odir);
    if( !dirInterface.DoesDirectoryExist(output_dir) )
    {
        dirInterface.CreateDirectory(output_dir);
    }

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

    MHO_MK4VexInterface vexInter;
    vexInter.OpenVexFile(root_file);
    json ovex;
    bool ovex_ok = vexInter.ExportVexFileToJSON(ovex);
    if(ovex_ok)
    {
        std::cout<<ovex.dump(2)<<std::endl;
        //write out to a json file
        std::string output_file = output_dir + "/" + dirInterface.GetBasename(root_file) + ".json";

        //open file for binary writing
        std::fstream jfile;
        jfile.open(output_file.c_str(), std::fstream::out);
        if( !jfile.is_open() || !jfile.good() )
        {
            msg_error("file", "Failed to open for writing, file: " << output_file << eom);
        }
        else
        {
            jfile << std::setw(4) << ovex;
            jfile.close();
        }
    }

    // // write prettified JSON to another file
    // std::ofstream o("pretty.json");
    // o << std::setw(4) << j << std::endl;

    dirInterface.GetCorelFiles(allFiles, corelFiles);
    for(auto it = corelFiles.begin(); it != corelFiles.end(); it++)
    {
        std::cout<<"corel file: "<< *it <<std::endl;
        std::string st_pair, root_code;
        std::string input_basename = dirInterface.GetBasename(*it);
        dirInterface.SplitCorelFileBasename(input_basename, st_pair, root_code);
        std::string output_file = output_dir + "/" + st_pair + "." + root_code + ".cor";
        ConvertCorel(root_file, *it, output_file);
    }

    dirInterface.GetStationFiles(allFiles, stationFiles);
    for(auto it = stationFiles.begin(); it != stationFiles.end(); it++)
    {
        std::cout<<"station file: "<< *it <<std::endl;
        std::string st, root_code;
        std::string input_basename = dirInterface.GetBasename(*it);
        dirInterface.SplitStationFileBasename(input_basename, st, root_code);
        std::string output_file = output_dir + "/" + st + "." + root_code + ".sta";
        ConvertStation(root_file, *it, output_file);
    }

    return 0;
}
