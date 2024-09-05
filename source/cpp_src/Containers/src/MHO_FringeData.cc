#include "MHO_FringeData.hh"
#include "MHO_UUIDGenerator.hh"

#include "MHO_BinaryFileInterface.hh"
#include "MHO_LockFileHandler.hh"

#include <cstdio>

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
    std::string polprod;
    std::string ref_code;
    std::string rem_code;

    bool ok1 = fParameterStore.Get("/config/baseline", baseline);
    bool ok2 = fParameterStore.Get("/config/root_code", root_code);
    bool ok3 = fParameterStore.Get("/config/frequency_group", frequency_group);
    bool ok4 = fParameterStore.Get("/config/polprod", polprod);
    bool ok5 = fParameterStore.Get("/config/reference_station", ref_code);
    bool ok6 = fParameterStore.Get("/config/remote_station", rem_code);

    if(!ok1){baseline = "??";}
    if(!ok2){root_code = "XXXXXX";}
    if(!ok3){frequency_group = "X";}
    if(!ok4){polprod = "?";}
    if(!ok5){ref_code = "??";}
    if(!ok6){rem_code = "??";}

    //write out the data to disk (don't bother waiting for a directory write lock)
    //because we are going to write it to a unique temporary name
    //once it is complete, we will then get a write lock in order to rename it
    //to the properly sequenced ID
    //ideally this should reduce the amount of time each independent process waits to access the directory
    //since they can write in parallel and only need to queue up to rename their files

    std::string temp_name = ConstructFrngFileName(directory, baseline, ref_code, rem_code, frequency_group, polprod, root_code, temp_id);
    int write_ok = WriteDataObjects(temp_name);

    // for locking
    int lock_retval = LOCK_PROCESS_NO_PRIORITY;
    int the_seq_no; //the fringe file sequence number

    //wait until we are the next process allowed to write an output file
    MHO_LockFileHandler::GetInstance().DisableLegacyMode();
    lock_retval = MHO_LockFileHandler::GetInstance().WaitForWriteLock(directory, the_seq_no);

    if(lock_retval == LOCK_STATUS_OK && the_seq_no > 0)
    {
        std::stringstream ss2;
        ss2 << the_seq_no;
        std::string seq_code = ss2.str();
        std::string output_file = ConstructFrngFileName(directory, baseline, ref_code, rem_code, frequency_group, polprod, root_code, seq_code);

        //rename the temp file to the proper output name
        if(write_ok == 0)
        {
            std::rename(temp_name.c_str(), output_file.c_str());
            //kludge to get fourfit to feed the generated fringe file name
            //(but nothing else) as a return value to a
            //calling script (requires passing option "-m 4"); see
            //e.g. chops/source/python_src/hopstest_module/hopstestb/hopstestb.py
            //around line 74 in the FourFitThread class.
            auto msglev = MHO_Message::GetInstance().GetMessageLevel();
            if(msglev == eSpecial){fprintf(stderr,"fourfit: %s \n",output_file.c_str());}
        }
    }
    MHO_LockFileHandler::GetInstance().RemoveWriteLock();

    return write_ok;
}


int MHO_FringeData::WriteDataObjects(std::string filename)
{
    //now we attach the parameter store and plot data as object tags
    MHO_ObjectTags tags;
    tags.SetTagValue("plot_data", fPlotData);

    mho_json params;
    fParameterStore.DumpData(params);
    tags.SetTagValue("parameters", params);
    //TODO what other information should be tagged/included?

    //only enable this type of output iff the -X option has been passed
    bool xpower_output = false;
    xpower_output = fParameterStore.GetAs<bool>("/cmdline/xpower_output");

    visibility_type* vis_data = nullptr;
    weight_type* wt_data = nullptr;

    if(xpower_output)
    {
        vis_data = fContainerStore.GetObject<visibility_type>(std::string("vis"));
        if( vis_data == nullptr)
        {
            msg_fatal("fringe", "could not find visibility object to write output." << eom);
            std::exit(1);
        }

        wt_data = fContainerStore.GetObject<weight_type>(std::string("weight"));
        if( wt_data == nullptr)
        {
            msg_fatal("fringe", "could not find weights object to write output." << eom);
            std::exit(1);
        }
    }

    //Add the time/frequency averaged visibilities with the fringe solution applied (e.g. AP x CH) (e.g. type212 equivalent)
    phasor_type* phasor_data = fContainerStore.GetObject<phasor_type>(std::string("phasors"));
    if( phasor_data == nullptr)
    {
        msg_fatal("fringe", "could not find time/frequency averaged phasor object to write output." << eom);
        std::exit(1);
    }

    // visibility_store_type* vis_store_data = new visibility_store_type();
    // weight_store_type* wt_store_data = new weight_store_type();
    //
    // MHO_ElementTypeCaster<visibility_type, visibility_store_type> down_caster;
    // down_caster.SetArgs(vis_data, vis_store_data);
    // down_caster.Initialize();
    // down_caster.Execute();
    //
    // MHO_ElementTypeCaster< weight_store_type, weight_type> wt_down_caster;
    // wt_down_caster.SetArgs(wt_data, wt_store_data);
    // wt_down_caster.Initialize();
    // wt_down_caster.Execute();

    // //add these temporary objects to the container store so they get properly deleted later
    // fContainerStore->AddObject(vis_store_data);
    // fContainerStore->AddObject(wt_store_data);

    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(filename);
    if(status)
    {
        if(vis_data != nullptr){ tags.AddObjectUUID(vis_data->GetObjectUUID()); }
        if(wt_data != nullptr){ tags.AddObjectUUID(wt_data->GetObjectUUID()); }
        tags.AddObjectUUID(phasor_data->GetObjectUUID());
        inter.Write(tags, "tags");
        if(vis_data != nullptr){ inter.Write(*vis_data, "vis"); }
        if(wt_data != nullptr){ inter.Write(*wt_data, "weight"); }
        inter.Write(*phasor_data, "phasors");
        inter.Close();
    }
    else
    {
        msg_error("file", "error opening fringe output file for write: " << filename << eom);
        inter.Close();
        return 1;
    }

    return 0;
}


std::string MHO_FringeData::ConstructFrngFileName(const std::string directory,
                                                  const std::string& baseline,
                                                  const std::string& ref_station,
                                                  const std::string& rem_station,
                                                  const std::string& frequency_group,
                                                  const std::string& polprod,
                                                  const std::string& root_code,
                                                  const std::string& temp_id)
{
    std::stringstream ss;
    ss << directory << "/" << baseline << ".";
    ss << ref_station << "-" << rem_station << ".";
    ss << frequency_group << "." << polprod << ".";
    ss << root_code << "." << temp_id << ".frng";
    return ss.str();
}


}//end namespace
