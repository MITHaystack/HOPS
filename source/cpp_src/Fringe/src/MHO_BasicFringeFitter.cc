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


MHO_BasicFringeFitter::MHO_BasicFringeFitter():MHO_FringeFitter()
{
    fIsFinished = false;
};

MHO_BasicFringeFitter::~MHO_BasicFringeFitter(){};

void MHO_BasicFringeFitter::Configure()
{
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
        msg_fatal("main", "cannot initialize a valid scan store from this directory: " << directory << eom);
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
        msg_fatal("main", "could not find visibility or weight objects with names (vis, weight)." << eom);
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
    mho_json control_format = MHO_ControlDefinitions::GetControlFormat();
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
    MHO_ParameterManager paramManager(&fParameterStore, control_format);

    paramManager.SetControlStatements(&control_statements);
    paramManager.ConfigureAll();
    // fParameterStore.Dump();
    
    ////////////////////////////////////////////////////////////////////////////
    //CONFIGURE THE OPERATOR BUILD MANAGER
    ////////////////////////////////////////////////////////////////////////////
    //add the data selection operator
    //TODO FIXME -- this is a horrible hack to get this operator into the initialization stream
    #pragma message("fix this horrible hack -- where we modify the control format itself to inject a default operator")
    mho_json data_select_format =
    {
        {"name", "coarse_selection"},
        {"statement_type", "operator"},
        {"operator_category" , "selection"},
        {"type" , "empty"},
        {"priority", 1.01}
    };
    control_format["coarse_selection"] = data_select_format;
    (*(control_statements.begin()))["statements"].push_back(data_select_format);

    fOperatorBuildManager = new MHO_OperatorBuilderManager(&fOperatorToolbox, &fContainerStore, &fParameterStore, control_format);

    // #ifdef USE_PYBIND11
    // py::scoped_interpreter guard{}; // start the interpreter and keep it alive, need this or we segfault
    // #pragma message("TODO FIXME -- formalize the means by which plugin dependent operator builders are added")
    // fOperatorBuildManager->AddBuilderType<MHO_PythonOperatorBuilder>("python_labelling", "python_labelling"); 
    // fOperatorBuildManager->AddBuilderType<MHO_PythonOperatorBuilder>("python_flagging", "python_flagging"); 
    // fOperatorBuildManager->AddBuilderType<MHO_PythonOperatorBuilder>("python_calibration", "python_calibration"); 
    // #endif


    fOperatorBuildManager->SetControlStatements(&control_statements);


    //take a snapshot
    take_snapshot_here("test", "visib", __FILE__, __LINE__, vis_data);
    take_snapshot_here("test", "weights", __FILE__, __LINE__,  wt_data);
    
    
    ////////////////////////////////////////////////////////////////////////////
    //OPERATOR CONSTRUCTION
    ////////////////////////////////////////////////////////////////////////////
    
    fOperatorBuildManager->BuildOperatorCategory("default");
    MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "labelling");
    MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "selection");

    // //safety check TODO FIXME!
    // if(vis_data->GetSize() == 0){msg_fatal("main", "no data left after cuts." << eom); std::exit(1);}

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
    
    fIsFinished = true;
}

void MHO_BasicFringeFitter::PostRun()
{
    
}


bool MHO_BasicFringeFitter::IsFinished()
{
    return fIsFinished;
}


void MHO_BasicFringeFitter::Finalize()
{
    ////////////////////////////////////////////////////////////////////////////
    //PLOTTING/DEBUG
    ////////////////////////////////////////////////////////////////////////////
    //TODO may want to reorg the way this is done
    mho_json plot_dict = MHO_FringePlotInfo::construct_plot_data(&fContainerStore, &fParameterStore, fVexInfo);
    MHO_FringePlotInfo::fill_plot_data(&fParameterStore, plot_dict);

    //open and dump to file
    std::string output_file = fParameterStore.GetAs<std::string>("/cmdline/output_file");
    std::ofstream fdumpFile(output_file.c_str(), std::ofstream::out);
    fdumpFile << plot_dict;
    fdumpFile.close();
    // 
    // #ifdef USE_PYBIND11
    // //py::scoped_interpreter guard{}; // start the interpreter and keep it alive, need this or we segfault
    // msg_debug("main", "python plot generation enabled." << eom );
    // //test stuff
    // 
    // py::dict plot_obj = plot_dict;
    // 
    // //load our interface module
    // auto vis_module = py::module::import("hops_visualization");
    // auto plot_lib = vis_module.attr("fourfit_plot");
    // //call a python function on the interface class instance
    // plot_lib.attr("make_fourfit_plot")(plot_obj, "fplot.png");
}



}//end namespace
