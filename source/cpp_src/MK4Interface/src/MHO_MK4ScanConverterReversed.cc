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


void MHO_MK4ScanConverterReversed::ProcessScan(const std::string& in_dir, const std::string& out_dir)
{
    MHO_DirectoryInterface dirInterface;
    std::string output_dir = dirInterface.GetDirectoryFullPath(out_dir);
    std::string input_dir = dirInterface.GetDirectoryFullPath(in_dir);

    fStore.SetDirectory(input_dir);
    fStore.Initialize();
    if(!fStore.IsValid()){msg_error("mk4interface", "the directory: "<<input_dir<<", does not contain a valid HOPS4 scan" << eom);}

    std::vector< std::string > baselines =  fStore.GetBaselinesPresent();
    std::vector< std::string > stations = fStore.GetStationsPresent();

    msg_status("mk4interface_reversed",
               "processing scan from HOPS4 input directory: " << input_dir << 
               " to Mark4 output directory: " << output_dir << eom);

    //create output directory if it doesn't exist
    if(!dirInterface.DoesDirectoryExist(output_dir))
    {
        dirInterface.CreateDirectory(output_dir);
    }

    //first convert the json ovex back to traditional ovex 
    MHO_VexGenerator vex_generator;
    mho_json root_json = fStore.GetRootFileData();
    std::string root_json_basename = fStore.GetRootFileBasename();
    root_json_basename = dirInterface.StripExtensionFromBasename(root_json_basename); //strips .json
    std::string ovex_basename = dirInterface.StripExtensionFromBasename(root_json_basename); //strips .root
    std::string output_vex_file = output_dir + "/" + ovex_basename;
    vex_generator.SetFilename(output_vex_file);
    vex_generator.GenerateVex(root_json);

    //loop over baselines and convert .cor files 
    for(auto it = baselines.begin(); it != baselines.end(); it++)
    {
        std::string bl = *it;
        MHO_ContainerStore cStore;
        fStore.LoadBaseline(bl, &cStore);
        std::size_t nvis = cStore.GetNObjects< visibility_store_type >();
        std::size_t nwt =cStore.GetNObjects< weight_store_type >();

        visibility_store_type* vis_store_data = nullptr; 
        if(nvis >= 1){vis_store_data =  cStore.GetObject< visibility_store_type >(0); }
        else{msg_error("mk4interface", "error retrieving visibility data for baseline: "<<bl<< eom);}

        weight_store_type* wt_store_data = nullptr; 
        if(nwt >= 1){wt_store_data = cStore.GetObject< weight_store_type >(0);}
        else{msg_error("mk4interface", "error retrieving weight data for baseline: "<<bl<< eom);}

        MHO_MK4CorelInterfaceReversed converter;
        converter.SetOutputDirectory(output_dir);
        converter.SetRootFileName(output_vex_file);
        converter.SetVisibilityData(vis_store_data);
        converter.SetWeightData(wt_store_data);
        converter.GenerateCorelStructure();
        converter.WriteCorelFile();
        converter.FreeAllocated();
    }

    //loop over stations and convert .sta files 
    for(auto it = stations.begin(); it != stations.end(); it++)
    {
        std::string st = *it;
        MHO_ContainerStore cStore;
        fStore.LoadStation(st, &cStore);

        station_coord_type* sta_data = nullptr;
        std::size_t ncoord = cStore.GetNObjects< station_coord_type >();
        if(ncoord >= 1){ sta_data = cStore.GetObject< station_coord_type >(0);}
        else{msg_error("mk4interface", "error retrieving station data for : "<<st<< eom);}
        
        //pcal may or may not be present
        multitone_pcal_type* pcal_data = nullptr;
        std::size_t npcal = cStore.GetNObjects< multitone_pcal_type >();
        if(npcal >= 1){ pcal_data = cStore.GetObject< multitone_pcal_type >(0); }

        MHO_MK4StationInterfaceReversed converter;
        converter.SetOutputDirectory(output_dir);
        converter.SetStationCoordData(sta_data);
        converter.SetPCalData(pcal_data);
        converter.GenerateStationStructure();
        converter.WriteStationFile();
        converter.FreeAllocated();
    }
}

} // namespace hops