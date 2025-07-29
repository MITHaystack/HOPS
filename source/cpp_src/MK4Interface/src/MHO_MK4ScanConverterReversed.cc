#include "MHO_MK4ScanConverterReversed.hh"

#include "MHO_Message.hh"
#include "MHO_DirectoryInterface.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ScanDataStore.hh"
#include "MHO_VexGenerator.hh"

#include "MHO_MK4CorelInterfaceReversed.hh"
#include "MHO_MK4StationInterfaceReversed.hh"


#ifndef HOPS3_USE_CXX
extern "C"
{
#endif
    #include "mk4_data.h"
    #include "mk4_dfio.h"
    #include "mk4_records.h"
#ifndef HOPS3_USE_CXX
}
#endif

namespace hops
{


MHO_MK4ScanConverterReversed::MHO_MK4ScanConverterReversed(){};

MHO_MK4ScanConverterReversed::~MHO_MK4ScanConverterReversed(){};



int MHO_MK4ScanConverterReversed::DetermineDirectoryType(const std::string& in_dir)
{
    //directory interface
    MHO_DirectoryInterface dirInterface;
    std::string input_dir = dirInterface.GetDirectoryFullPath(in_dir);

    //get list of all the files (and directories) in the input directory
    std::vector< std::string > allFiles;
    std::vector< std::string > allDirs;

    dirInterface.SetCurrentDirectory(input_dir);
    dirInterface.ReadCurrentDirectory();
    dirInterface.GetFileList(allFiles);
    dirInterface.GetSubDirectoryList(allDirs);

    //sort files, locate root, corel and station files
    std::vector< std::string > corelFiles;
    std::vector< std::string > stationFiles;
    std::vector< std::string > rootFiles;
    dirInterface.GetFilesMatchingExtention(rootFiles, ".root.json");

    //definitely a scan directory (a root file and no subdirs)
    if(rootFiles.size() >= 1 && allDirs.size() == 0)
    {
        return HOPS4_SCANDIR;
    }

    //likely an experiment directory (no root and 1 or more subdir)
    if(rootFiles.size() == 0 && allDirs.size() >= 1)
    {
        //TODO check that the directory name is 4 digit number
        return HOPS4_EXPDIR;
    }

    //probably a scan directory
    //for some reason it has a sub-dir, but we found a root file
    if(rootFiles.size() >= 1)
    {
        return HOPS4_SCANDIR;
    }

    //don't know what we have here, but probably cannot process it
    return HOPS4_UNKNOWNDIR;
}


void MHO_MK4ScanConverterReversed::ProcessScan(const std::string& in_dir, const std::string& out_dir)
{
    fOutputVexFile = "";

    fOutputDir = fDirInterface.GetDirectoryFullPath(out_dir);
    fInputDir = fDirInterface.GetDirectoryFullPath(in_dir);

    fStore.SetDirectory(fInputDir);
    fStore.Initialize();
    if(!fStore.IsValid())
    {
        msg_error("mk4interface", "the directory: "<< fInputDir <<", does not contain a valid HOPS4 scan" << eom);
        return;
    }

    msg_status("mk4interface_reversed",
               "processing scan from HOPS4 input directory: " << fInputDir << 
               " to Mark4 output directory: " << fOutputDir << eom);

    //create output directory if it doesn't exist
    if(!fDirInterface.DoesDirectoryExist(fOutputDir))
    {
        fDirInterface.CreateDirectory(fOutputDir);
    }

    //process the scan data
    ProcessVex();
    ProcessCorel();
    ProcessStation();
}


void 
MHO_MK4ScanConverterReversed::ProcessVex()
{
    //first convert the json ovex back to traditional ovex 
    MHO_VexGenerator vex_generator;
    fRootJSON = fStore.GetRootFileData();
    
    std::string root_json_basename = fStore.GetRootFileBasename();
    root_json_basename = fDirInterface.StripExtensionFromBasename(root_json_basename); //strips .json
    std::string ovex_basename = fDirInterface.StripExtensionFromBasename(root_json_basename); //strips .root
    std::string output_vex_file = fOutputDir + "/" + ovex_basename;
    fOutputVexFile = output_vex_file;
    vex_generator.SetFilename(output_vex_file);
    vex_generator.GenerateVex(fRootJSON);
}

void 
MHO_MK4ScanConverterReversed::ProcessCorel()
{
    std::vector< std::string > baselines =  fStore.GetBaselinesPresent();
    //loop over baselines and convert .cor files 
    for(auto it = baselines.begin(); it != baselines.end(); it++)
    {
        std::string bl = *it;
        MHO_ContainerStore cStore;
        fStore.LoadBaseline(bl, &cStore);
        std::size_t nvis = cStore.GetNObjects< visibility_store_type >();
        std::size_t nwt = cStore.GetNObjects< weight_store_type >();

        visibility_store_type* vis_store_data = nullptr; 
        if(nvis >= 1){vis_store_data =  cStore.GetObject< visibility_store_type >(0); }
        else
        {
            msg_error("mk4interface", "error retrieving visibility data for baseline: "<<bl<< eom); 
            continue;
        }

        weight_store_type* wt_store_data = nullptr; 
        if(nwt >= 1){wt_store_data = cStore.GetObject< weight_store_type >(0);}
        else
        {
            msg_error("mk4interface", "error retrieving weight data for baseline: "<<bl<< eom);
            continue;
        }

        MHO_MK4CorelInterfaceReversed converter;
        converter.SetOutputDirectory(fOutputDir);
        converter.SetRootFileName(fOutputVexFile); //have to set the ovex file name in the type_100
        converter.SetVisibilityData(vis_store_data);
        converter.SetWeightData(wt_store_data);
        converter.GenerateCorelStructure();
        converter.WriteCorelFile();
        converter.FreeAllocated();
    }
}

void 
MHO_MK4ScanConverterReversed::ProcessStation()
{
    std::vector< std::string > stations = fStore.GetStationsPresent();
    //loop over stations and convert .sta files 
    for(auto it = stations.begin(); it != stations.end(); it++)
    {
        std::string st = *it;
        MHO_ContainerStore cStore;
        fStore.LoadStation(st, &cStore);

        station_coord_type* sta_data = nullptr;
        std::size_t ncoord = cStore.GetNObjects< station_coord_type >();
        if(ncoord >= 1){ sta_data = cStore.GetObject< station_coord_type >(0);}
        else
        {
            msg_error("mk4interface", "error retrieving station data for : "<<st<< eom); 
            continue;
        }
        
        //pcal may or may not be present
        multitone_pcal_type* pcal_data = nullptr;
        std::size_t npcal = cStore.GetNObjects< multitone_pcal_type >();
        if(npcal >= 1){ pcal_data = cStore.GetObject< multitone_pcal_type >(0); }

        MHO_MK4StationInterfaceReversed converter;
        converter.SetVexData(fRootJSON);
        converter.SetOutputDirectory(fOutputDir);
        converter.SetStationCoordData(sta_data);
        converter.SetPCalData(pcal_data);
        converter.GenerateStationStructure();
        converter.WriteStationFile();
        converter.FreeAllocated();
    }

}


} // namespace hops
