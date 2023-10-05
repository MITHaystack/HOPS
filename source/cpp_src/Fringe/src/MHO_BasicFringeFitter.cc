#include "MHO_BasicFringeFitter.hh"

//snapshot utility lib
#include "MHO_Snapshot.hh"

//control
#include "MHO_ControlFileParser.hh"
#include "MHO_ControlConditionEvaluator.hh"

//initialization
#include "MHO_OperatorBuilderManager.hh"
#include "MHO_ParameterConfigurator.hh"
#include "MHO_ParameterManager.hh"

//fringe finding library helper functions
#include "MHO_BasicFringeDataConfiguration.hh"
#include "MHO_BasicFringeInfo.hh"
#include "MHO_InitialFringeInfo.hh"
#include "MHO_BasicFringeUtilities.hh"
#include "MHO_FringePlotInfo.hh"
#include "MHO_VexInfoExtractor.hh"

namespace hops
{


MHO_BasicFringeFitter::MHO_BasicFringeFitter():MHO_FringeFitter(){ count =0;};

MHO_BasicFringeFitter::~MHO_BasicFringeFitter(){};

void MHO_BasicFringeFitter::Configure()
{
    //set "is finished to false"
    fParameterStore.Set("/status/is_finished", false);


    std::string directory = fParameterStore.GetAs<std::string>("/cmdline/directory");
    std::string control_file = fParameterStore.GetAs<std::string>("/cmdline/control_file");
    std::string baseline = fParameterStore.GetAs<std::string>("/cmdline/baseline");
    std::string polprod = fParameterStore.GetAs<std::string>("/cmdline/polprod");

    //if any of these are empty, fail out for now
    if(directory == "" || control_file == "" || baseline == "" || polprod == "")
    {
        msg_fatal("fringe", "missing command line information (directory, control file, baseline, or pol-product)" << eom );
        std::exit(1);
    }

    ////////////////////////////////////////////////////////////////////////////
    //INITIALIZE SCAN DIRECTORY
    ////////////////////////////////////////////////////////////////////////////

    //initialize the scan store from this directory
    fScanStore.SetDirectory(directory);
    fScanStore.Initialize();
    if( !fScanStore.IsValid() )
    {
        msg_fatal("fringe", "cannot initialize a valid scan store from this directory: " << directory << eom);
        std::exit(1);
    }
    //pass the directory and root file info the the parameter store
    fParameterStore.Set("directory", directory);
    fParameterStore.Set("root_file", fScanStore.GetRootFileBasename() );

    //load root file and extract useful vex info
    fVexInfo = fScanStore.GetRootFileData();
    MHO_VexInfoExtractor::extract_vex_info(fVexInfo, &fParameterStore);

    ////////////////////////////////////////////////////////////////////////////
    //LOAD DATA AND ASSEMBLE THE DATA STORE
    ////////////////////////////////////////////////////////////////////////////

    //load baseline data
    fScanStore.LoadBaseline(baseline, &fContainerStore);
    MHO_BasicFringeDataConfiguration::configure_data_library(&fContainerStore);//momentarily needed for float -> double cast
    //load and rename station data according to reference/remote
    std::string ref_station_mk4id = std::string(1,baseline[0]);
    std::string rem_station_mk4id = std::string(1,baseline[1]);
    fScanStore.LoadStation(ref_station_mk4id, &fContainerStore);
    fContainerStore.RenameObject("sta", "ref_sta");
    fScanStore.LoadStation(rem_station_mk4id, &fContainerStore);
    fContainerStore.RenameObject("sta", "rem_sta");

    station_coord_type* ref_data = fContainerStore.GetObject<station_coord_type>(std::string("ref_sta"));
    station_coord_type* rem_data = fContainerStore.GetObject<station_coord_type>(std::string("rem_sta"));

    visibility_type* vis_data = fContainerStore.GetObject<visibility_type>(std::string("vis"));
    weight_type* wt_data = fContainerStore.GetObject<weight_type>(std::string("weight"));
    if( vis_data == nullptr || wt_data == nullptr )
    {
        msg_fatal("fringe", "could not find visibility or weight objects with names (vis, weight)." << eom);
        std::exit(1);
    }


    //DEBUG
    //fContainerStore.DumpShortNamesToIds();
    #pragma message("TODO FIXME -- formalize the manner in which we identify data container objects via UUID")
    //temporarily put the object uuid's in the parameter store so we can look it up on the python side
    std::string vis_uuid = vis_data->GetObjectUUID().as_string();
    std::string wt_uuid = wt_data->GetObjectUUID().as_string();
    std::string ref_uuid = ref_data->GetObjectUUID().as_string();
    std::string rem_uuid = rem_data->GetObjectUUID().as_string();

    fParameterStore.Set("/uuid/visibilities", vis_uuid);
    fParameterStore.Set("/uuid/weights", wt_uuid);
    fParameterStore.Set("/uuid/ref_station", ref_uuid);
    fParameterStore.Set("/uuid/rem_station", rem_uuid);

    //specify the control format
    fControlFormat = MHO_ControlDefinitions::GetControlFormat();

    //add the data selection operator
    //TODO FIXME -- this is a horrible hack to get this operator into the initialization stream
    #pragma message("fix this horrible hack -- where we modify the control format itself to inject a default operator")
    fDataSelectFormat =
    {
        {"name", "coarse_selection"},
        {"statement_type", "operator"},
        {"operator_category" , "selection"},
        {"type" , "empty"},
        {"priority", 1.01}
    };
    fControlFormat["coarse_selection"] = fDataSelectFormat;

    ////////////////////////////////////////////////////////////////////////////
    //CONFIGURE THE OPERATOR BUILD MANAGER
    ////////////////////////////////////////////////////////////////////////////
    fOperatorBuildManager = new MHO_OperatorBuilderManager(&fOperatorToolbox, &fContainerStore, &fParameterStore, fControlFormat);
}

void MHO_BasicFringeFitter::Initialize()
{

    ////////////////////////////////////////////////////////////////////////////
    //CONTROL CONSTRUCTION
    ////////////////////////////////////////////////////////////////////////////
    std::string control_file = fParameterStore.GetAs<std::string>("/cmdline/control_file");
    std::string baseline = fParameterStore.GetAs<std::string>("/cmdline/baseline");
    MHO_ControlFileParser cparser;
    MHO_ControlConditionEvaluator ceval;
    cparser.SetControlFile(control_file);
    auto control_contents = cparser.ParseControl();
    mho_json control_statements;

    //TODO -- where should frequency group information get stashed/retrieved?
    std::string srcName = fParameterStore.GetAs<std::string>("/vex/scan/source/name");
    std::string scnName = fParameterStore.GetAs<std::string>("/vex/scan/name");
    ceval.SetPassInformation(baseline, srcName, "?", scnName);//baseline, source, fgroup, scan
    control_statements = ceval.GetApplicableStatements(control_contents);

    ////////////////////////////////////////////////////////////////////////////
    //PARAMETER SETTING
    ////////////////////////////////////////////////////////////////////////////
    MHO_InitialFringeInfo::set_default_parameters(&fContainerStore, &fParameterStore); //set some default parameters (polprod, ref_freq)
    MHO_ParameterManager paramManager(&fParameterStore, fControlFormat);
    paramManager.SetControlStatements(&control_statements);
    paramManager.ConfigureAll();
    // fParameterStore.Dump();

    mho_json coarse_selection_hack =
    {
        {"name", "coarse_selection"},
        {"statement_type", "operator"},
        {"operator_category" , "selection"}
    };
     //part of the ugly default coarse selection hack, triggers the build of this operator at the 'selection' step
    (*(control_statements.begin()))["statements"].push_back(coarse_selection_hack);

    std::cout<<"fDataSelectFormat = "<<fDataSelectFormat.dump(2)<<std::endl;
    std::cout<<"control statements = "<<control_statements.dump(2)<<std::endl;

    fOperatorBuildManager->SetControlStatements(&control_statements);

    //take a snapshot if enabled
    visibility_type* vis_data = fContainerStore.GetObject<visibility_type>(std::string("vis"));
    weight_type* wt_data = fContainerStore.GetObject<weight_type>(std::string("weight"));
    take_snapshot_here("test", "visib", __FILE__, __LINE__, vis_data);
    take_snapshot_here("test", "weights", __FILE__, __LINE__,  wt_data);


    ////////////////////////////////////////////////////////////////////////////
    //OPERATOR CONSTRUCTION
    ////////////////////////////////////////////////////////////////////////////

    fOperatorBuildManager->BuildOperatorCategory("default");
    MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "labelling");
    MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "selection");

    //safety check
    if(vis_data->GetSize() == 0){msg_fatal("fringe", "no visibility data left after cuts." << eom); std::exit(1);}
    if(wt_data->GetSize() == 0){msg_fatal("fringe", "no weight data left after cuts." << eom); std::exit(1);}


    MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "flagging");
    MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "calibration");

    //calulate useful quantities to stash in the parameter store
    MHO_InitialFringeInfo::precalculate_quantities(&fContainerStore, &fParameterStore);
}

void MHO_BasicFringeFitter::PreRun()
{

}

void MHO_BasicFringeFitter::Run()
{
    //execute the basic fringe search algorithm
    MHO_BasicFringeUtilities::basic_fringe_search(&fContainerStore, &fParameterStore);
    //calculate the fringe properties
    MHO_BasicFringeUtilities::calculate_fringe_solution_info(&fContainerStore, &fParameterStore, fVexInfo);

    fParameterStore.Set("/status/is_finished", true);
    
    fParameterStore.Dump();
}

void MHO_BasicFringeFitter::PostRun()
{

}


bool MHO_BasicFringeFitter::IsFinished()
{
    bool is_finished = fParameterStore.GetAs<bool>("/status/is_finished");
    return is_finished;
}


void MHO_BasicFringeFitter::Finalize()
{
    ////////////////////////////////////////////////////////////////////////////
    //PLOTTING/DEBUG
    ////////////////////////////////////////////////////////////////////////////
    //TODO may want to reorg the way this is done
    fPlotData = MHO_FringePlotInfo::construct_plot_data(&fContainerStore, &fParameterStore, fVexInfo);
    MHO_FringePlotInfo::fill_plot_data(&fParameterStore, fPlotData);
}



}//end namespace
