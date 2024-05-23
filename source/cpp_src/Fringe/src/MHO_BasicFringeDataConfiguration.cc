#include "MHO_BasicFringeDataConfiguration.hh"
#include "MHO_DirectoryInterface.hh"

//snapshot utility lib
#include "MHO_Snapshot.hh"

//configure_data_library
#include "MHO_ElementTypeCaster.hh"

#include "MHO_VexInfoExtractor.hh"

//parse_command_line
#include "MHO_Tokenizer.hh"

namespace hops
{


void
MHO_BasicFringeDataConfiguration::parse_baseline_freqgrp(std::string baseline_freqgrp, std::string& baseline, std::string& freqgrp)
{
    MHO_Tokenizer tokenizer;

    if( baseline_freqgrp.find(':') == std::string::npos )
    {
        baseline = baseline_freqgrp;

        if(baseline.size() != 2)
        {
            msg_fatal("fringe", "baseline must be passed as 2-char code."<< eom);
            std::exit(1);
        }
    }
    else
    {
        //split on ':' into baseline and frequency group
        std::vector< std::string> tokens;
        std::string delim = ":";
        tokenizer.SetDelimiter(delim);
        tokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
        tokenizer.SetIncludeEmptyTokensFalse();
        tokenizer.SetString(&baseline_freqgrp);
        tokenizer.GetTokens(&tokens);
        if(tokens.size() != 2)
        {
            msg_fatal("fringe", "could not parse '-b' command line argument: "<<baseline_freqgrp<< eom);
            std::exit(1);
        }
        baseline = tokens[0];
        freqgrp = tokens[1];
    }
}

std::string
MHO_BasicFringeDataConfiguration::parse_set_string(const std::vector< std::string >& arglist, int& set_arg_index)
{
    set_arg_index = -1;
    int nargs = arglist.size();
    for(int i=0; i<nargs; i++)
    {
        if(arglist[i] == "set")
        {
            set_arg_index = i;
            break;
        }
    }

    //if we've found a 'set' command, assume everything after this is control
    //file syntax and concatenate everything together with spaces
    std::string set_string = "";
    int start_idx = set_arg_index+1;
    if(set_arg_index != -1 && start_idx < nargs-1 )
    {
        for(int i=start_idx; i<nargs; i++)
        {
            set_string += arglist[i];
            set_string += " ";
        }
    }

    return set_string;
}

std::string
MHO_BasicFringeDataConfiguration::sanitize_directory(std::string dir)
{
    //do not resolve the full path
    //just strip the root file if it exists in the dir name
    std::string path = dir;
    std::size_t dir_end = dir.find_last_of("/");
    if(dir_end != std::string::npos)
    {
        path = dir.substr(0,dir_end) + "/";
    }
    return path;

    // bool ok;
    // std::string fullpath = MHO_DirectoryInterface::GetDirectoryFullPath(dir);
    // ok = MHO_DirectoryInterface::DoesDirectoryExist(fullpath);
    // if(ok){return fullpath;}
    //
    // //check if we have actually been passed a root-file instead (and need to return the prefix)
    // std::string basename = MHO_DirectoryInterface::GetBasename(fullpath);
    // std::string prefix = MHO_DirectoryInterface::GetPrefix(fullpath);
    // std::size_t dot = basename.find('.');
    // if(dot != std::string::npos)
    // {
    //     std::string root_code = basename.substr(dot+1);
    //     if( root_code.size() == 6 )
    //     {
    //         //check if is a directory
    //         bool ok = MHO_DirectoryInterface::DoesDirectoryExist(fullpath);
    //         if(!ok)
    //         {
    //             //we were actually passed a root file, so return the prefix
    //             fullpath = prefix;
    //         }
    //     }
    // }
    //
    // return fullpath;

}

std::string
MHO_BasicFringeDataConfiguration::find_associated_root_file(std::string dir)
{
    std::string path = sanitize_directory(dir);
    MHO_DirectoryInterface dirInter;

    dirInter.SetCurrentDirectory(path);
    dirInter.ReadCurrentDirectory();
    std::vector< std::string > flist;
    std::string ext(".root.json");
    dirInter.GetFilesMatchingExtention(flist, ext);

    if(flist.size() != 1)
    {
        if(flist.size() == 0)
        {
            msg_fatal("fringe", "no root file found in: "<< dir << eom );
            std::exit(1);
        }
        if(flist.size() > 1 )
        {
            msg_warn("fringe", "multiple root files found in: "<< dir <<" using the first one found: "<< flist[0] << eom );
        }
    }

    std::string root_file = flist[0];
    root_file = MHO_DirectoryInterface::GetDirectoryFullPath(root_file);
    return root_file;

}



//sanity check of parameters after command line parsing
int
MHO_BasicFringeDataConfiguration::sanity_check(MHO_ParameterStore* paramStore)
{
    //command line parameters
    std::string baseline = paramStore->GetAs<std::string>("/cmdline/baseline");
    std::string freqgrp = paramStore->GetAs<std::string>("/cmdline/frequency_group");
    std::string control_file = paramStore->GetAs<std::string>("/cmdline/control_file");
    std::string directory = paramStore->GetAs<std::string>("/cmdline/directory");
    //bool estimate_time = false; //'-e' estimate run time
    int first_plot_chan = paramStore->GetAs<int>("/cmdline/first_plot_channel");
    int message_level = paramStore->GetAs<int>("/cmdline/message_level");
    int nplot_chans = paramStore->GetAs<int>("/cmdline/nplot_channels");
    bool show_plot = paramStore->GetAs<bool>("/cmdline/show_plot");
    //std::string refringe_alist_file = ""; // '-r' alist file for refringing - not yet enabled
    int ap_per_seg = paramStore->GetAs<int>("/cmdline/ap_per_seg");
    bool test_mode = paramStore->GetAs<bool>("/cmdline/test_mode");
    //bool update_mode = false; //'-u' not yet enabled
    std::string polprod = paramStore->GetAs<std::string>("/cmdline/polprod");
    //std::string reftime = "";
    //bool xpower_output = false;
    //std::string output_file = paramStore->GetAs<std::string>("/cmdline/output_file");

    #pragma message("TODO FIXME - fill out the sanity_check function for command line arguments")
    if( directory == "" || baseline == "" || polprod == "" || control_file == "")
    {
        return 1;
    }

    return 0;
}




void MHO_BasicFringeDataConfiguration::populate_initial_parameters(MHO_ParameterStore* paramStore, MHO_ScanDataStore* scanStore)
{
    // //initialize by setting "is_finished" to false, and 'skipped' to false
    // //these parameters must always be present
    // paramStore->Set("/status/is_finished", false);
    // paramStore->Set("/status/skipped", false);

    //these should all be present and ok at this point
    std::string directory = paramStore->GetAs<std::string>("/cmdline/directory");
    std::string control_file = paramStore->GetAs<std::string>("/cmdline/control_file");
    std::string baseline = paramStore->GetAs<std::string>("/cmdline/baseline");
    std::string polprod = paramStore->GetAs<std::string>("/cmdline/polprod");
    std::string fgroup = paramStore->GetAs<std::string>("/cmdline/frequency_group");

    ////////////////////////////////////////////////////////////////////////////
    //INITIALIZE PARAMETERS
    ////////////////////////////////////////////////////////////////////////////

    //set up the file section of the parameter store to record the directory, root file, and control file
    paramStore->Set("/files/control_file", control_file);
    paramStore->Set("/files/directory", directory);
    //paramStore->Set("/files/output_file", paramStore->GetAs<std::string>("/cmdline/output_file"));

    //put the baseline and pol product selection into the parameter store
    paramStore->Set("/config/polprod", polprod);
    paramStore->Set("/config/baseline", baseline);
    paramStore->Set("/config/fgroup", fgroup);

    //parse the polprod string in order to determine which pol-products are needed (if more than one)
    std::vector< std::string > pp_vec = determine_required_pol_products(polprod);
    paramStore->Set("/config/polprod_set", pp_vec);

    ////////////////////////////////////////////////////////////////////////////
    //INITIALIZE SCAN DIRECTORY
    ////////////////////////////////////////////////////////////////////////////

    //initialize the scan store from this directory
    scanStore->SetDirectory(directory);
    scanStore->Initialize();
    if( !scanStore->IsValid() )
    {
        msg_fatal("fringe", "cannot initialize a valid scan store from this directory: " << directory << eom);
        std::exit(1);
    }

    if( !scanStore->IsBaselinePresent(baseline) )
    {
        msg_fatal("fringe", "cannot find the specified baseline: " << baseline << " in " << directory << eom);
        std::exit(1);
    }

    //set the root file name
    paramStore->Set("/files/root_file", scanStore->GetRootFileBasename() );

    msg_debug("fringe", "loading root file: "<< scanStore->GetRootFileBasename() << eom);

     // //load root file and extract useful vex info into parameter store
    auto vexInfo = scanStore->GetRootFileData();
    MHO_VexInfoExtractor::extract_vex_info(vexInfo, paramStore);

}

//more helper functions
void
MHO_BasicFringeDataConfiguration::configure_visibility_data(MHO_ContainerStore* store)
{
    //first check if there are visibility_type and weight_type with double precision present
    std::size_t n_vis = store->GetNObjects<visibility_type>();
    std::size_t n_wt = store->GetNObjects<weight_type>();

    if(n_vis == 1 && n_wt == 1)
    {
        msg_debug("initialization", "double precision visibility and weight types found, these will be preferred and used over single precision types." << eom);
        return;
    }

    //evidently there are no double precision objects, so we look for the single-precision 'storage types'
    n_vis = store->GetNObjects<visibility_store_type>();
    n_wt = store->GetNObjects<weight_store_type>();

    //retrieve the (first) visibility and weight objects
    //(currently assuming there is only one object per type)
    visibility_store_type* vis_store_data = nullptr;
    weight_store_type* wt_store_data = nullptr;

    vis_store_data = store->GetObject<visibility_store_type>(0);
    wt_store_data = store->GetObject<weight_store_type>(0);

    if(vis_store_data == nullptr)
    {
        msg_fatal("initialization", "failed to read visibility data from the .cor file." <<eom);
        std::exit(1);
    }

    if(wt_store_data == nullptr)
    {
        msg_fatal("initialization", "failed to read weight data from the .cor file." <<eom);
        std::exit(1);
    }

    if(n_vis != 1 || n_wt != 1)
    {
        msg_warn("initialization", "multiple visibility and/or weight types per-baseline not yet supported, will use first located." << eom);
    }

    auto vis_store_uuid = vis_store_data->GetObjectUUID();
    auto wt_store_uuid = wt_store_data->GetObjectUUID();

    std::string vis_shortname = store->GetShortName(vis_store_uuid);
    std::string wt_shortname = store->GetShortName(wt_store_uuid);

    visibility_type* vis_data = new visibility_type();
    weight_type* wt_data = new weight_type();

    //assign the storage UUID's to their up-casted counter-parts
    //we do this so we can associate them to the file objects (w.r.t to program output, error messages, etc.)
    vis_data->SetObjectUUID(vis_store_uuid);
    wt_data->SetObjectUUID(wt_store_uuid);

    MHO_ElementTypeCaster<visibility_store_type, visibility_type> up_caster;
    up_caster.SetArgs(vis_store_data, vis_data);
    up_caster.Initialize();
    up_caster.Execute();

    MHO_ElementTypeCaster< weight_store_type, weight_type> wt_up_caster;
    wt_up_caster.SetArgs(wt_store_data, wt_data);
    wt_up_caster.Initialize();
    wt_up_caster.Execute();

    //remove the original objects to save space
    store->DeleteObject(vis_store_data);
    store->DeleteObject(wt_store_data);

    //warn on non-standard shortnames
    if(vis_shortname != "vis"){msg_warn("initialization", "visibilities do not use canonical short name 'vis', but are called: "<< vis_shortname << eom);}
    if(wt_shortname != "weight"){msg_warn("initialization", "weights do not use canonical short name 'weight', but are called: "<< wt_shortname << eom);}

    //now shove the double precision data into the container store with the same shortname
    store->AddObject(vis_data);
    store->AddObject(wt_data);
    store->SetShortName(vis_data->GetObjectUUID(), vis_shortname);
    store->SetShortName(wt_data->GetObjectUUID(), wt_shortname);
}

void
MHO_BasicFringeDataConfiguration::configure_station_data(MHO_ScanDataStore* scanStore, MHO_ContainerStore* containerStore,
                                                         std::string ref_station_mk4id, std::string rem_station_mk4id)
{
    //load station data and assign them the names 'ref_sta' or 'rem_sta'
    scanStore->LoadStation(ref_station_mk4id, containerStore);
    containerStore->RenameObject("sta", "ref_sta");
    MHO_UUID pcal_uuid;
    pcal_uuid = containerStore->GetObjectUUID("pcal");
    if( !(pcal_uuid.is_empty()) )
    {
        containerStore->RenameObject("pcal", "ref_pcal");
    }

    scanStore->LoadStation(rem_station_mk4id, containerStore);
    containerStore->RenameObject("sta", "rem_sta");
    pcal_uuid = containerStore->GetObjectUUID("pcal");
    if( !(pcal_uuid.is_empty()) )
    {
        containerStore->RenameObject("pcal", "rem_pcal");
    }
}

void
MHO_BasicFringeDataConfiguration::init_and_exec_operators(MHO_OperatorBuilderManager* build_manager, MHO_OperatorToolbox* opToolbox, const char* category)
{
    std::string cat(category);
    if(build_manager == nullptr || opToolbox == nullptr)
    {
        msg_error("fringe", "cannot initialize or execute operators if builder or toolbox is missing" << eom );
        return;
    }

    msg_debug("fringe", "initializing and executing operators in "<<cat<<" category."<<eom);

    build_manager->BuildOperatorCategory(cat);
    auto ops = opToolbox->GetOperatorsByCategory(cat);
    for(auto opIt= ops.begin(); opIt != ops.end(); opIt++)
    {
        msg_debug("fringe", "initializing and executing operator: "<< (*opIt)->GetName() <<", with priority: "<< (*opIt)->Priority() << "." << eom);
        (*opIt)->Initialize();
        (*opIt)->Execute();
    }
}

std::vector< std::string >
MHO_BasicFringeDataConfiguration::determine_required_pol_products(std::string polprod)
{
    MHO_Tokenizer tokenizer;
    std::set<std::string> pp_set;
    std::vector<std::string> pp_vec;
    //first we parse the polprod string to see what individual pol-products we need
    if( polprod.find("+") != std::string::npos)
    {
        //we have a pol-product summation like (RR+LL) or XX+YY, or RX+RY
        //so split on all '+' symbols (currently we only support '+' not '-')
        tokenizer.SetDelimiter("+");
        tokenizer.SetString(&polprod);
        //fTokenizer.SetUseMulticharacterDelimiterFalse();
        tokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
        tokenizer.SetIncludeEmptyTokensFalse();
        tokenizer.GetTokens(&pp_vec);
    }
    else if(polprod == "I") //special pseudo-Stokes-I mode (linear pol only)
    {
        pp_vec.push_back("XX");
        pp_vec.push_back("YY");
        pp_vec.push_back("XY");
        pp_vec.push_back("YX");
    }
    else
    {
        pp_vec.push_back(polprod); //polprod is just a single value
    }

    //push the values into a set, so we don't have any duplicates
    pp_set.insert( pp_vec.begin(), pp_vec.end() );

    //push the set values into the vector for return
    pp_vec.clear();
    pp_vec.insert(pp_vec.begin(), pp_set.begin(), pp_set.end() );

    std::stringstream ss;
    for(std::size_t i=0; i<pp_vec.size(); i++)
    {
        ss << pp_vec[i];
        if(i != pp_vec.size() - 1){ss <<", "; }
    }
    msg_debug("fringe", "required pol-products are: {" << ss.str() << "}." << eom );

    return pp_vec;
}


mho_json
MHO_BasicFringeDataConfiguration::ConvertProfileEvents(std::vector< MHO_ProfileEvent >& events)
{
    mho_json event_list;
    for(std::size_t i=0; i<events.size(); i++)
    {
        mho_json obj;
        obj["event_id"] = i;
        obj["flag"] = events[i].fFlag;
        obj["line"] = events[i].fLineNumber;
        obj["thread_id"] = events[i].fThreadID;
        obj["filename"] = std::string( events[i].fFilename );
        obj["funcname"] = std::string( events[i].fFuncname );
        obj["time"] = events[i].fTime;
        event_list.push_back(obj);
    }
    return event_list;
}


}//end namespace
