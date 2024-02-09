#include "MHO_FringeData.hh"
#include "MHO_UUIDGenerator.hh"

#include "MHO_BinaryFileInterface.hh"

namespace hops 
{

//write data objects to output file...perhaps we may want to move this elsewhere?
int
MHO_FringeData::WriteOutput()
{
    std::string directory = fParameterStore.GetAs<std::string>("/files/directory");
    directory = MHO_DirectoryInterface::GetDirectoryFullPath(directory);

    MHO_UUIDGenerator gen;
    std::string temp_id = gen.GenerateUUIDAsString();

    //grab the name info
    std::string baseline;
    std::string root_code;
    std::string frequency_group;

    bool ok1 = fParameterStore.Get("/config/baseline", baseline);
    bool ok2 = fParameterStore.Get("/config/root_code", root_code);
    bool ok3 = fParameterStore.Get("/config/frequency_group", frequency_group);

    if(!ok1){baseline = "??";}
    if(!ok2){root_code = "XXXXXX";}
    if(!ok3){frequency_group = "X";}

    std::stringstream ss;
    ss << directory << "/" << baseline << "." << frequency_group << "." << temp_id << "." << root_code << ".frng";
    std::string output_file = ss.str();

    //TODO REPLACE THIS WITH:
    //(1) The full visibilities (with spectral information) with the fringe solution applied 
    //(2) The time/frequency averaged visibilities with the fringe solution applied (e.g. AP x CH)
    visibility_type* vis_data = fContainerStore.GetObject<visibility_type>(std::string("vis"));
    if( vis_data == nullptr)
    {
        msg_fatal("fringe", "could not find visibility object to write output." << eom);
        std::exit(1);
    }
    
    //now we attach the parameter store and plot data as object tags
    MHO_ObjectTags tags;
    tags.SetTagValue("plot_data", fPlotData);

    mho_json params;
    fParameterStore.DumpData(params);
    tags.SetTagValue("parameters", params);

    //TODO what other information should be tagged/included?

    // mho_json fVexInfo;
    mho_json fControlFormat;
    mho_json fControlStatements;

    //plot data storage
    mho_json fPlotData;

    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(output_file);
    if(status)
    {
        uint32_t label = 0xFFFFFFFF; //someday make this mean something
        tags.AddObjectUUID(vis_data->GetObjectUUID());
        inter.Write(tags, "tags", label);
        inter.Write(*vis_data, "vis", label);
        inter.Close();
    }
    else
    {
        msg_error("file", "error opening fringe output file: " << output_file << eom);
    }

    inter.Close();

    return 0;
}


}//end namespace