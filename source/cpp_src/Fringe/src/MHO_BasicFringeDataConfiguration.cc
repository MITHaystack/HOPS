#include "MHO_BasicFringeDataConfiguration.hh"

//snapshot utility lib
#include "MHO_Snapshot.hh"

//configure_data_library
#include "MHO_ElementTypeCaster.hh"

//parse_command_line
#include <getopt.h>


namespace hops 
{


int 
MHO_BasicFringeDataConfiguration::parse_command_line(int argc, char** argv, MHO_ParameterStore* paramStore)
{
    //TODO make this conform/support most of the command line options of fourfit
    std::string usage = "ffit -d <directory> -c <control file> -b <baseline> -P <pol. product>";

    std::string directory = "";
    std::string control_file = "";
    std::string baseline = "";
    std::string polprod = "";
    std::string output_file = "fdump.json"; //for testing
    int message_level = -1;
    int ap_per_seg = 0;
    bool ok;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"directory", required_argument, 0, 'd'},
                                          {"control", required_argument, 0, 'c'},
                                          {"baseline", required_argument, 0, 'b'},
                                          {"polarization-product", required_argument, 0, 'P'},
                                          {"message-level", required_argument, 0, 'm'},
                                          {"ap-per-seg", required_argument, 0, 's'},
                                          {"output", required_argument, 0, 'o'}};

    static const char* optString = "hd:c:b:P:o:m:s:";

    while(true)
    {
        char optId = getopt_long(argc, argv, optString, longOptions, NULL);
        if (optId == -1)
            break;
        switch(optId)
        {
            case ('h'):  // help
                std::cout << usage << std::endl;
                std::exit(0);
            case ('d'):
                directory = std::string(optarg);
                break;
            case ('c'):
                control_file = std::string(optarg);
                break;
            case ('b'):
                baseline = std::string(optarg);
                break;
            case ('P'):
                polprod = std::string(optarg);
                break;
            case ('o'):
                output_file = std::string(optarg);
                break;
            case ('m'):
                message_level = std::atoi(optarg);
                break;
            case ('s'):
                ap_per_seg = std::atoi(optarg);
                if(ap_per_seg < 0){ap_per_seg = 0; msg_warn("fringe", "invalid ap_per_seg, ignoring." << eom);}
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    if( directory == "" || baseline == "" || polprod == "" || control_file == "")
    {
        msg_fatal("fringe", "usage: "<< usage << eom);
        return 1;
    }

    //set the message level according to the fourfit style
    //where 3 is least verbose, and '-1' is most verbose
    switch (message_level)
    {
        case -2:
            //NOTE: debug messages must be compiled-in
            #ifndef HOPS_ENABLE_DEBUG_MSG
            MHO_Message::GetInstance().SetMessageLevel(eInfo);
            msg_warn("fringe", "debug messages are toggled via compiler flag, re-compile with ENABLE_DEBUG_MSG=ON to enable." << eom);
            #else
            MHO_Message::GetInstance().SetMessageLevel(eDebug);
            #endif
        break;
        case -1:
            MHO_Message::GetInstance().SetMessageLevel(eInfo);
        break;
        case 0:
            MHO_Message::GetInstance().SetMessageLevel(eStatus);
        break;
        case 1:
            MHO_Message::GetInstance().SetMessageLevel(eWarning);
        break;
        case 2:
            MHO_Message::GetInstance().SetMessageLevel(eError);
        break;
        case 3:
            MHO_Message::GetInstance().SetMessageLevel(eFatal);
        break;
        case 4:
            MHO_Message::GetInstance().SetMessageLevel(eSilent);
        break;
        default:
            //for now default is most verbose, eventually will change this to silent
            MHO_Message::GetInstance().SetMessageLevel(eDebug);
    }

    if(baseline.size() != 2)
    {
        msg_fatal("fringe", "baseline must be passed as 2-char code."<< eom);
        return 1;
    }

    //store the raw arguments in the parameter store
    std::vector<std::string> arglist;
    for(int i=0; i<argc; i++)
    {
        arglist.push_back( std::string(argv[i]) );
    }
    paramStore->Set("/cmdline/args", arglist);

    //pass the extracted info back in the parameter store
    paramStore->Set("/cmdline/directory", directory);
    paramStore->Set("/cmdline/baseline", baseline);
    paramStore->Set("/cmdline/polprod", polprod);
    paramStore->Set("/cmdline/control_file",control_file);
    paramStore->Set("/cmdline/ap_per_seg",ap_per_seg);
    paramStore->Set("/cmdline/output_file",output_file);

    return 0;

}

//more helper functions
void 
MHO_BasicFringeDataConfiguration::configure_visibility_data(MHO_ContainerStore* store)
{
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

    std::size_t n_vis = store->GetNObjects<visibility_store_type>();
    std::size_t n_wt = store->GetNObjects<weight_store_type>();

    if(n_vis != 1 || n_wt != 1)
    {
        msg_warn("initialization", "multiple visibility and/or weight types per-baseline not yet supported" << eom);
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
        std::cout<<"Loading ref pcal object with uuid: "<< pcal_uuid.as_string()<<std::endl;
        containerStore->RenameObject("pcal", "ref_pcal");
    }
    
    scanStore->LoadStation(rem_station_mk4id, containerStore);
    containerStore->RenameObject("sta", "rem_sta");
    pcal_uuid = containerStore->GetObjectUUID("pcal");
    if( !(pcal_uuid.is_empty()) )
    {
        std::cout<<"Loading rem pcal object with uuid: "<< pcal_uuid.as_string()<<std::endl;
        containerStore->RenameObject("pcal", "rem_pcal");
    }
    //load pcal data if it is present

}


void 
MHO_BasicFringeDataConfiguration::init_and_exec_operators(MHO_OperatorBuilderManager* build_manager, MHO_OperatorToolbox* opToolbox, const char* category)
{
    if(build_manager == nullptr || opToolbox == nullptr)
    {
        msg_error("fringe", "cannot initialize or execute operators if builder or toolbox is missing" << eom );
        return;
    }

    std::string cat(category);
    build_manager->BuildOperatorCategory(cat);
    auto ops = opToolbox->GetOperatorsByCategory(cat);
    for(auto opIt= ops.begin(); opIt != ops.end(); opIt++)
    {
        msg_debug("fringe", "initializing and executing operator: "<< (*opIt)->GetName() << eom);
        (*opIt)->Initialize();
        (*opIt)->Execute();
    }
}


}//end namespace
