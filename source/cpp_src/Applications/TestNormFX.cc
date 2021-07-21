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
////////////////////////////////////////////////////////////////////////////////


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
////////////////////////////////////////////////////////////////////////////////
//note: we normally wouldn't bother passing around two station interfaces, but
//we need to keep a pointer to the raw mk4 data, so for now keeping the interface
//classes around is the simplest way to do this with the least work

bool GetStationData(MHO_DirectoryInterface& dirInterface,
                    MHO_MK4StationInterface& refInterface,
                    MHO_MK4StationInterface& remInterface,
                    const std::string& baseline,
                    struct mk4_sdata*& ref_sdata,
                    struct mk4_sdata*& rem_sdata,
                    station_coord_data_type*& ref_stdata,
                    station_coord_data_type*& rem_stdata)
{
    std::string ref_st, rem_st, root_file;
    ref_st = baseline.at(0);
    rem_st = baseline.at(1);
    std::vector< std::string > allFiles, stationFiles;
    dirInterface.GetFileList(allFiles);
    dirInterface.GetStationFiles(allFiles, stationFiles);
    dirInterface.GetRootFile(allFiles, root_file);
    bool ref_ok = false;
    bool rem_ok = false;
    for(auto it = stationFiles.begin(); it != stationFiles.end(); it++)
    {
        std::cout<<"station file: "<< *it <<std::endl;
        std::string st, root_code;
        std::string input_basename = dirInterface.GetBasename(*it);
        dirInterface.SplitStationFileBasename(input_basename, st, root_code);

        if(st == ref_st)
        {
            refInterface.SetStationFile(*it);
            refInterface.SetVexFile(root_file);
            ref_stdata = refInterface.ExtractStationFile();
            ref_sdata = refInterface.GetStationData();
            ref_ok = true;
        }

        if(st == rem_st)
        {
            remInterface.SetStationFile(*it);
            remInterface.SetVexFile(root_file);
            rem_stdata = remInterface.ExtractStationFile();
            rem_sdata = remInterface.GetStationData();
            rem_ok = true;
        }
        if(ref_ok && rem_ok){break;}
    }

    return (ref_ok && rem_ok);

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

    //split the baseline into reference/remote station IDs

    if(baseline.size() != 2){msg_fatal("main", "Baseline: "<<baseline<<" is not of length 2."<<eom);}


    //directory interface, load up the directory information
    MHO_DirectoryInterface dirInterface;
    dirInterface.SetCurrentDirectory(input_dir);
    dirInterface.ReadCurrentDirectory();

    //get the root (ovex) file information
    MHO_MK4VexInterface vexInterface;
    json json_vex;
    struct vex* root = nullptr;
    bool ovex_ok = GetVex(dirInterface, vexInterface, json_vex, root);

    //the corel file information for this baseline
    MHO_MK4CorelInterface corelInterface;
    struct mk4_corel* cdata = nullptr;
    ch_baseline_data_type* ch_bl_data = nullptr;
    ch_baseline_weight_type* ch_bl_wdata = nullptr;
    bool corel_ok = GetCorel(dirInterface, corelInterface, baseline, cdata, ch_bl_data, ch_bl_wdata);
    std::cout<<"data ptrs = "<<cdata<<", "<<ch_bl_data<<", "<<ch_bl_wdata<<std::endl;

    //get the station data information for the ref/rem stations of this baseline
    MHO_MK4StationInterface refInterface, remInterface;
    struct mk4_sdata* ref_sdata = nullptr;
    struct mk4_sdata* rem_sdata = nullptr;
    station_coord_data_type* ref_stdata = nullptr;
    station_coord_data_type* rem_stdata = nullptr;
    bool sta_ok = GetStationData(dirInterface, refInterface, remInterface, baseline, ref_sdata, rem_sdata, ref_stdata, rem_stdata);





    // //Now make the pass/param structs
    // struct type_pass pass;
    // struct type_param param;
    // struct type_status status;
    // ConstructPassStruct(&pass);
    // ConstructParamStruct(&param);
    // ConstructStatusStruct(&status);
    //

    return 0;
}
