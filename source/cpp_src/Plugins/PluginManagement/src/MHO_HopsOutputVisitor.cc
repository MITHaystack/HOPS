#include "MHO_HopsOutputVisitor.hh"

#include "MHO_FringeData.hh"
#include "MHO_UUIDGenerator.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_LockFileHandler.hh"

#include <cstdio>

namespace hops 
{


MHO_HopsOutputVisitor::MHO_HopsOutputVisitor(){}

MHO_HopsOutputVisitor::~MHO_HopsOutputVisitor(){}

void 
MHO_HopsOutputVisitor::Visit(MHO_FringeFitter* fitter)
{
    MHO_FringeData* fringeData = fitter->GetFringeData();
    if(!fringeData)
    {
        msg_error("fringe", "fringe data is null, aborting output write" << eom);
        return;
    }

    std::string directory = fringeData->GetParameterStore()->GetAs< std::string >("/files/directory");
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

    bool ok1 = fringeData->GetParameterStore()->Get("/config/baseline", baseline);
    bool ok2 = fringeData->GetParameterStore()->Get("/config/root_code", root_code);
    bool ok3 = fringeData->GetParameterStore()->Get("/config/frequency_group", frequency_group);
    bool ok4 = fringeData->GetParameterStore()->Get("/config/polprod", polprod);
    bool ok5 = fringeData->GetParameterStore()->Get("/config/reference_station", ref_code);
    bool ok6 = fringeData->GetParameterStore()->Get("/config/remote_station", rem_code);

    if(!ok1)
    {
        baseline = "??";
    }
    if(!ok2)
    {
        root_code = "XXXXXX";
    }
    if(!ok3)
    {
        frequency_group = "X";
    }
    if(!ok4)
    {
        polprod = "?";
    }
    if(!ok5)
    {
        ref_code = "??";
    }
    if(!ok6)
    {
        rem_code = "??";
    }

    //write out the data to disk (don't bother waiting for a directory write lock)
    //because we are going to write it to a unique temporary name
    //once it is complete, we will then get a write lock in order to rename it
    //to the properly sequenced ID
    //ideally this should reduce the amount of time each independent process waits to access the directory
    //since they can write in parallel and only need to queue up to rename their files

    std::string temp_name =
        fringeData->ConstructTempFileName(directory, baseline, ref_code, rem_code, frequency_group, polprod, root_code, temp_id);
    int write_ok = WriteDataObjects(fringeData, temp_name);

    // for locking
    int lock_retval = LOCK_PROCESS_NO_PRIORITY;
    int the_seq_no; //the fringe file sequence number

    //wait until we are the next process allowed to write an output file
    MHO_LockFileHandler::GetInstance().DisableLegacyMode();
    lock_retval = MHO_LockFileHandler::GetInstance().WaitForWriteLock(directory, the_seq_no);

    if(lock_retval == LOCK_STATUS_OK && the_seq_no > 0)
    {
        std::string output_file =
            fringeData->ConstructFrngFileName(directory, baseline, ref_code, rem_code, frequency_group, polprod, root_code, the_seq_no);

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
            if(msglev == eSpecial)
            {
                fprintf(stderr, "fourfit: %s \n", output_file.c_str());
            }
        }
    }
    MHO_LockFileHandler::GetInstance().RemoveWriteLock();

    //return write_ok;
}

int MHO_HopsOutputVisitor::WriteDataObjects(MHO_FringeData* fringeData, std::string filename)
{
    //now we attach the parameter store and plot data as object tags
    MHO_ObjectTags tags;
    tags.SetTagValue("plot_data", fringeData->GetPlotData());

    mho_json params;
    fringeData->GetParameterStore()->DumpData(params);
    tags.SetTagValue("parameters", params);

    //only enable this type of output iff the -X option has been passed
    //and it has a value of 0 or greater
    int xpower_output = -1;
    xpower_output = fringeData->GetParameterStore()->GetAs< int >("/cmdline/xpower_output");

    visibility_type* vis_data = nullptr;
    weight_type* wt_data = nullptr;

    if(0 <= xpower_output)
    {
        vis_data = fringeData->GetContainerStore()->GetObject< visibility_type >(std::string("vis"));
        if(vis_data == nullptr)
        {
            msg_error("fringe", "could not find visibility object to write output." << eom);
        }

        wt_data = fringeData->GetContainerStore()->GetObject< weight_type >(std::string("weight"));
        if(wt_data == nullptr)
        {
            msg_error("fringe", "could not find weights object to write output." << eom);
        }
    }

    //Add the time/frequency averaged visibilities with the fringe solution applied (e.g. AP x CH) (e.g. type212 equivalent)
    phasor_type* phasor_data = fringeData->GetContainerStore()->GetObject< phasor_type >(std::string("phasors"));
    if(phasor_data == nullptr)
    {
        msg_error("fringe", "could not find time/frequency averaged phasor object to write output." << eom);
    }


    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(filename);
    if(status)
    {
        if(vis_data != nullptr)
        {
            tags.AddObjectUUID(vis_data->GetObjectUUID());
        }
        if(wt_data != nullptr)
        {
            tags.AddObjectUUID(wt_data->GetObjectUUID());
        }
        if(phasor_data != nullptr)
        {
            tags.AddObjectUUID(phasor_data->GetObjectUUID());
        }

        inter.Write(tags, "tags");
        if(vis_data != nullptr)
        {
            inter.Write(*vis_data, "vis");
        }
        if(wt_data != nullptr)
        {
            inter.Write(*wt_data, "weight");
        }
        if(phasor_data != nullptr)
        {
            inter.Write(*phasor_data, "phasors");
        }
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

} // namespace hops
