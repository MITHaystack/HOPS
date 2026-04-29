#include "MHO_FringeDataInitializer.hh"

//configure_data_library
#include "MHO_ElementTypeCaster.hh"
#include "MHO_VexInfoExtractor.hh"

#include "MHO_FringeDataDiscovery.hh"

namespace hops
{

bool MHO_FringeDataInitializer::initialize_scan_data(MHO_ParameterStore* paramStore, MHO_ScanDataStore* scanStore)
{
    //this should all be present and ok at this point
    std::string input_directory = paramStore->GetAs< std::string >("/pass/input_directory");

    ////////////////////////////////////////////////////////////////////////////
    //INITIALIZE SCAN DIRECTORY
    ////////////////////////////////////////////////////////////////////////////

    //initialize the scan store from this directory
    scanStore->SetDirectory(input_directory);
    scanStore->Initialize();
    if(!scanStore->IsValid())
    {
        msg_error("fringe", "cannot initialize a valid scan store from this directory: " << input_directory << eom);
        return false;
    }

    std::string bl = paramStore->GetAs< std::string >("/pass/baseline");
    std::string pp = paramStore->GetAs< std::string >("/pass/polprod");
    std::string fg = paramStore->GetAs< std::string >("/pass/frequency_group");
    msg_info("fringe", "fringing data for baseline: " << bl << ", pol-product: " << pp << ", freq-group: " << fg
                                                      << ", in directory: " << input_directory << eom);

    return true;
}

void MHO_FringeDataInitializer::populate_initial_parameters(MHO_ParameterStore* paramStore, MHO_ScanDataStore* scanStore)
{
    //initialize by setting "is_finished" to false, and 'skipped' to false
    //these parameters must always be present
    paramStore->Set("/status/is_finished", false);
    paramStore->Set("/status/skipped", false);

    //these should all be present and ok at this point
    std::string input_directory = paramStore->GetAs< std::string >("/pass/input_directory");
    std::string control_file = paramStore->GetAs< std::string >("/cmdline/control_file");
    std::string baseline = paramStore->GetAs< std::string >("/pass/baseline");
    std::string polprod = paramStore->GetAs< std::string >("/pass/polprod");
    std::string fgroup = paramStore->GetAs< std::string >("/pass/frequency_group");

    //we will need the scan name to construct the output_directory,
    //if it is different from the input directory
    std::string scan = paramStore->GetAs< std::string >("/pass/scan");

    ////////////////////////////////////////////////////////////////////////////
    //INITIALIZE PARAMETERS
    ////////////////////////////////////////////////////////////////////////////

    //set up the file section of the parameter store to record the directory, root file, and control file
    paramStore->Set("/files/control_file", control_file);
    paramStore->Set("/files/input_directory", input_directory);
    std::string output_directory = paramStore->GetAs< std::string >("/cmdline/output_directory");
    paramStore->Set("/files/output_directory", output_directory);

    if(output_directory != input_directory)
    {
        //input and output directory are different
        //so we need to construct the top-level output directory if it doesn't exist
        if(!MHO_DirectoryInterface::DoesDirectoryExist(output_directory))
        {
            MHO_DirectoryInterface::CreateDirectory(output_directory);
        }

        //now check if the output directory incorporates the scan name,
        //if not, (it is being treated as an experiment directory)
        //we will need to construct a more specific output directory
        std::string trailing_directory = MHO_DirectoryInterface::GetTrailingDirectory(output_directory);
        if(trailing_directory != scan)
        {
            //now we have to form the scan-specific output_directory with the scan name prefix
            std::string pass_output_directory = output_directory + "/" + scan + "/";
            paramStore->Set("/files/output_directory", pass_output_directory);
        }
    }

    //set the software version info
    paramStore->Set("/config/software_version", std::string(HOPS_VERSION) + "-" + std::string(HOPS_GIT_REV));

    //put the baseline and pol product selection into the parameter store
    paramStore->Set("/config/polprod", polprod);
    paramStore->Set("/config/baseline", baseline);
    paramStore->Set("/config/fgroup", fgroup);

    //parse the polprod string in order to determine which pol-products are needed (if more than one)
    std::vector< std::string > pp_vec = MHO_FringeDataDiscovery::determine_required_pol_products(polprod);
    paramStore->Set("/config/polprod_set", pp_vec);

    ////////////////////////////////////////////////////////////////////////////
    //INITIALIZE SCAN DIRECTORY
    ////////////////////////////////////////////////////////////////////////////

    //initialize the scan store from this directory
    scanStore->SetDirectory(input_directory);
    scanStore->Initialize();
    if(!scanStore->IsValid())
    {
        msg_fatal("fringe", "cannot initialize a valid scan store from this directory: " << input_directory << eom);
        paramStore->Set("/status/skipped", true);
    }

    if(!scanStore->IsBaselinePresent(baseline))
    {
        msg_fatal("fringe", "cannot find the specified baseline: " << baseline << " in " << input_directory << eom);
        paramStore->Set("/status/skipped", true);
    }

    //set the root file name
    paramStore->Set("/files/root_file", scanStore->GetRootFileBasename());

    msg_debug("fringe", "loading root file: " << scanStore->GetRootFileBasename() << eom);

    //load root file and extract useful vex info into parameter store
    auto vexInfo = scanStore->GetRootFileData();
    MHO_VexInfoExtractor::extract_vex_info(vexInfo, paramStore);

    //make sure we construct our global map of station identities (mk4id <-> 2 char code <-> station name)
    MHO_VexInfoExtractor::extract_station_identities(vexInfo);
}

//more helper functions
void MHO_FringeDataInitializer::configure_visibility_data(MHO_ContainerStore* store)
{
    //first check if there are visibility_type and weight_type with double precision present
    std::size_t n_vis = store->GetNObjects< visibility_type >();
    std::size_t n_wt = store->GetNObjects< weight_type >();

    if(n_vis == 1 && n_wt == 1)
    {
        msg_debug(
            "initialization",
            "double precision visibility and weight types found, these will be preferred and used over single precision types."
                << eom);
        return;
    }

    //evidently there are no double precision objects, so we look for the single-precision 'storage types'
    n_vis = store->GetNObjects< visibility_store_type >();
    n_wt = store->GetNObjects< weight_store_type >();

    //retrieve the (first) visibility and weight objects
    //(currently assuming there is only one object per type)
    visibility_store_type* vis_store_data = nullptr;
    weight_store_type* wt_store_data = nullptr;

    vis_store_data = store->GetObject< visibility_store_type >(0);
    wt_store_data = store->GetObject< weight_store_type >(0);

    if(vis_store_data == nullptr)
    {
        msg_error("initialization", "failed to read visibility data from the .cor file." << eom);
        return;
    }

    if(wt_store_data == nullptr)
    {
        msg_error("initialization", "failed to read weight data from the .cor file." << eom);
        return;
    }

    if(n_vis != 1 || n_wt != 1)
    {
        msg_warn("initialization",
                 "multiple visibility and/or weight types per-baseline not yet supported, will use first one located." << eom);
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

    MHO_ElementTypeCaster< visibility_store_type, visibility_type > up_caster;
    up_caster.SetArgs(vis_store_data, vis_data);
    up_caster.Initialize();
    up_caster.Execute();

    MHO_ElementTypeCaster< weight_store_type, weight_type > wt_up_caster;
    wt_up_caster.SetArgs(wt_store_data, wt_data);
    wt_up_caster.Initialize();
    wt_up_caster.Execute();

    //remove the original objects to save space
    store->DeleteObject(vis_store_data);
    store->DeleteObject(wt_store_data);

    //warn on non-standard shortnames
    if(vis_shortname != "vis")
    {
        msg_warn("initialization",
                 "visibilities do not use canonical short name 'vis', but are called: " << vis_shortname << eom);
    }
    if(wt_shortname != "weight")
    {
        msg_warn("initialization", "weights do not use canonical short name 'weight', but are called: " << wt_shortname << eom);
    }

    //now shove the double precision data into the container store with the same shortname
    store->AddObject(vis_data);
    store->AddObject(wt_data);
    store->SetShortName(vis_data->GetObjectUUID(), vis_shortname);
    store->SetShortName(wt_data->GetObjectUUID(), wt_shortname);
}

void MHO_FringeDataInitializer::configure_station_data(MHO_ScanDataStore* scanStore, MHO_ContainerStore* containerStore,
                                                       std::string ref_station_mk4id, std::string rem_station_mk4id)
{
    //load station data and assign them the names 'ref_sta' or 'rem_sta'
    scanStore->LoadStation(ref_station_mk4id, containerStore);
    containerStore->RenameObject("sta", "ref_sta");
    MHO_UUID pcal_uuid;
    pcal_uuid = containerStore->GetObjectUUID("pcal");
    if(!(pcal_uuid.is_empty()))
    {
        containerStore->RenameObject("pcal", "ref_pcal");
    }

    scanStore->LoadStation(rem_station_mk4id, containerStore);
    containerStore->RenameObject("sta", "rem_sta");
    pcal_uuid = containerStore->GetObjectUUID("pcal");
    if(!(pcal_uuid.is_empty()))
    {
        containerStore->RenameObject("pcal", "rem_pcal");
    }
}

void MHO_FringeDataInitializer::init_and_exec_operators(MHO_OperatorBuilderManager* build_manager,
                                                        MHO_OperatorToolbox* opToolbox, const char* category)
{
    std::string cat(category);
    if(build_manager == nullptr || opToolbox == nullptr)
    {
        msg_error("fringe", "cannot initialize or execute operators if builder or toolbox is missing" << eom);
        return;
    }

    msg_debug("fringe", "initializing and executing operators in " << cat << " category." << eom);

    //build_manager->BuildOperatorCategory(cat);
    auto ops = opToolbox->GetOperatorsByCategory(cat);
    for(auto opIt = ops.begin(); opIt != ops.end(); opIt++)
    {
        msg_debug("fringe", "initializing and executing operator: " << (*opIt)->GetName() << ", with priority: "
                                                                    << (*opIt)->Priority() << "." << eom);
        (*opIt)->Initialize();
        (*opIt)->Execute();
    }
}

} // namespace hops
