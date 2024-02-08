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

#include "MHO_ContainerDefinitions.hh"

#include "MHO_VisibilityChannelizer.hh"
#include "MHO_WeightChannelizer.hh"


#include "MHO_BinaryFileStreamer.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_ClassIdentityMap.hh"

#include "MHO_DirectoryInterface.hh"

using namespace hops;


//convert a corel file
void ConvertCorel(const std::string root_file, const std::string& input_file, const std::string& output_file)
{
    MHO_MK4CorelInterface mk4inter;

    msg_info("file", "Converting corel input file: " << input_file << eom);
    mk4inter.SetCorelFile(input_file);
    mk4inter.SetVexFile(root_file);
    mk4inter.ExtractCorelFile();
    uch_visibility_store_type* bl_data = mk4inter.GetExtractedVisibilities();
    uch_weight_store_type* bl_wdata = mk4inter.GetExtractedWeights();

    MHO_VisibilityChannelizer channelizer;
    visibility_store_type* ch_bl_data = new visibility_store_type();
    channelizer.SetArgs(bl_data, ch_bl_data);
    bool init = channelizer.Initialize();
    if(init)
    {
        bool exe = channelizer.Execute();
        if(!exe){msg_error("main", "failed to channelize visibility data." << eom);}
    }
    ch_bl_data->CopyTags(*bl_data);

    MHO_WeightChannelizer wchannelizer;
    weight_store_type* ch_bl_wdata = new weight_store_type();
    wchannelizer.SetArgs(bl_wdata, ch_bl_wdata);
    bool winit = wchannelizer.Initialize();
    if(winit)
    {
        bool wexe = wchannelizer.Execute();
        if(!wexe){msg_error("main", "failed to channelize weight data." << eom);}
    }
    ch_bl_wdata->CopyTags(*bl_wdata);

    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(output_file);
    if(status)
    {
        uint32_t label = 0xFFFFFFFF; //someday make this mean something
        inter.Write(*ch_bl_data, "vis", label);
        inter.Write(*ch_bl_wdata, "weight", label);
        //
        // // //TODO AFTER DEBUG RETURN TO ch_* data
        // inter.Write(*bl_data, "uch_vis", label);
        // inter.Write(*bl_wdata, "uch_weight", label);
        // inter.Close();
    }
    else
    {
        msg_error("file", "Error opening corel output file: " << output_file << eom);
    }

    inter.Close();

    delete bl_data;
    delete ch_bl_data;
}



//convert a station data  file
void ConvertStation(const std::string root_file, const std::string& input_file, const std::string& output_file)
{
    MHO_MK4StationInterface mk4inter;

    msg_info("file", "Converting station input file: " << input_file << eom);
    mk4inter.SetStationFile(input_file);
    // mk4inter.SetVexFile(root_file);
    station_coord_type* st_data = mk4inter.ExtractStationFile();

    std::size_t n_pcal_obj = mk4inter.GetNPCalObjects();

    MHO_BinaryFileInterface inter;
    //std::string index_file = output_file + ".index";
    //bool status = inter.OpenToWrite(output_file, index_file);

    bool status = inter.OpenToWrite(output_file);
    if(status)
    {
        uint32_t label = 0xFFFFFFFF;
        inter.Write(*st_data, "sta", label); //write out station data
        //write out pcal objects
        for(std::size_t i=0; i<n_pcal_obj; i++)
        {
            inter.Write( *(mk4inter.GetPCalObject(i)), "pcal", label);
        }
        inter.Close();
    }
    else
    {
        msg_error("file", "Error opening station output file: " << output_file << eom);
    }

    inter.Close();
    delete st_data;
}




int main(int argc, char** argv)
{
    std::string usage = "mark42hops -i <input_directory> -o <output_directory>";

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
    input_dir = dirInterface.GetDirectoryFullPath(input_dir);

    msg_info("main", "input directory: " << input_dir << eom);
    msg_info("main", "output directory: " << output_dir << eom);

    if( !dirInterface.DoesDirectoryExist(output_dir) )
    {
        dirInterface.CreateDirectory(output_dir);
    }

    //get list of all the files (and directories) in directory
    std::vector< std::string > allFiles;
    std::vector< std::string > allDirs;

    dirInterface.SetCurrentDirectory(input_dir);
    dirInterface.ReadCurrentDirectory();
    dirInterface.GetFileList(allFiles);
    dirInterface.GetSubDirectoryList(allDirs);

    //sort files, locate root, corel and station files
    std::vector< std::string > corelFiles;
    std::vector< std::string > stationFiles;
    std::string root_file;
    dirInterface.GetRootFile(allFiles, root_file);

    MHO_MK4VexInterface vexInter;
    vexInter.OpenVexFile(root_file);
    mho_json ovex;
    bool ovex_ok = vexInter.ExportVexFileToJSON(ovex);
    if(ovex_ok)
    {
        //std::cout<<ovex.dump(2)<<std::endl; //dump the json to terminal

        //write out to a json file
        std::string output_file = output_dir + "/" + dirInterface.GetBasename(root_file) + ".root.json";

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

    dirInterface.GetCorelFiles(allFiles, corelFiles);
    for(auto it = corelFiles.begin(); it != corelFiles.end(); it++)
    {
        std::string st_pair, root_code;
        std::string input_basename = dirInterface.GetBasename(*it);
        dirInterface.SplitCorelFileBasename(input_basename, st_pair, root_code);
        std::string output_file = output_dir + "/" + st_pair + "." + root_code + ".cor";
        ConvertCorel(root_file, *it, output_file);
    }

    dirInterface.GetStationFiles(allFiles, stationFiles);
    for(auto it = stationFiles.begin(); it != stationFiles.end(); it++)
    {
        std::string st, root_code;
        std::string input_basename = dirInterface.GetBasename(*it);
        dirInterface.SplitStationFileBasename(input_basename, st, root_code);
        std::string output_file = output_dir + "/" + st + "." + root_code + ".sta";
        ConvertStation(root_file, *it, output_file);
    }

    return 0;
}
