#include "MHO_FringeData.hh"
#include "MHO_UUIDGenerator.hh"

#include "MHO_BinaryFileInterface.hh"
#include "MHO_LockFileHandler.hh"

namespace hops 
{

//write data objects to output file...perhaps we may want to move this elsewhere?
int
MHO_FringeData::WriteOutput()
{
    std::string directory = fParameterStore.GetAs<std::string>("/files/directory");
    directory = MHO_DirectoryInterface::GetDirectoryFullPath(directory);

    // MHO_UUIDGenerator gen;
    // std::string temp_id = gen.GenerateUUIDAsString();

    //grab the name info
    std::string baseline;
    std::string root_code;
    std::string frequency_group;
    std::string polprod;

    bool ok1 = fParameterStore.Get("/config/baseline", baseline);
    bool ok2 = fParameterStore.Get("/config/root_code", root_code);
    bool ok3 = fParameterStore.Get("/config/frequency_group", frequency_group);
    bool ok4 = fParameterStore.Get("/config/polprod", polprod);

    if(!ok1){baseline = "??";}
    if(!ok2){root_code = "XXXXXX";}
    if(!ok3){frequency_group = "X";}
    if(!ok4){polprod = "?";}

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


    // for locking
    int lock_retval = LOCK_PROCESS_NO_PRIORITY;
    char lockfile_name[MAX_LOCKNAME_LEN] = {'\0'};
    int the_seq_no; //the fringe file sequence number
    
    //wait until we are the next process allowed to write an output file
    MHO_LockFileHandler::GetInstance().DisableLegacyMode();
    lock_retval = MHO_LockFileHandler::GetInstance().WaitForWriteLock(directory, the_seq_no);

    if(lock_retval == LOCK_STATUS_OK && the_seq_no > 0)
    {

        std::stringstream ss;
        // ss << directory << "/" << baseline << "." << frequency_group << "." << temp_id << "." << root_code << ".frng";
        ss << directory << "/" << baseline << "." << frequency_group << "." << polprod << "." << root_code << "." << the_seq_no << ".frng";
        std::string output_file = ss.str();

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
            msg_error("file", "error opening fringe output file for write: " << output_file << eom);
        }

        inter.Close();
    }

    usleep(5);
    MHO_LockFileHandler::GetInstance().RemoveWriteLock();

    return 0;
}


}//end namespace