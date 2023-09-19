#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define EXTRA_DEBUG

#include "ffit.hh"

//control
#include "MHO_ControlFileParser.hh"
#include "MHO_ControlConditionEvaluator.hh"

//needed to read hops files and extract objects from scan dir
#include "MHO_ScanDataStore.hh"

//initialization
#include "MHO_OperatorBuilderManager.hh"
#include "MHO_ParameterConfigurator.hh"
#include "MHO_ParameterManager.hh"


#ifdef USE_PYBIND11
#include "MHO_PythonOperatorBuilder.hh"
#endif

int main(int argc, char** argv)
{
    //TODO allow messaging keys to be set via command line arguments
    MHO_Message::GetInstance().AcceptAllKeys();

    MHO_Snapshot::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().SetExecutableName(std::string("ffit"));

    //provide necessary objects for operation
    MHO_ParameterStore paramStore; //stores various parameters using string keys
    MHO_ScanDataStore scanStore; //provides access to data associated with this scan
    MHO_ContainerStore conStore; //stores data containers for in-use data
    MHO_OperatorToolbox opToolbox; //stores the data operator objects

    int parse_status = parse_command_line(argc, argv, &paramStore);
    if(parse_status != 0){msg_fatal("main", "could not parse command line options." << eom); std::exit(1);}

    std::string directory = paramStore.GetAs<std::string>("/cmdline/directory");
    std::string control_file = paramStore.GetAs<std::string>("/cmdline/control_file");
    std::string baseline = paramStore.GetAs<std::string>("/cmdline/baseline");
    std::string polprod = paramStore.GetAs<std::string>("/cmdline/polprod");

    ////////////////////////////////////////////////////////////////////////////
    //INITIALIZE SCAN DIRECTORY
    ////////////////////////////////////////////////////////////////////////////

    //initialize the scan store from this directory
    scanStore.SetDirectory(directory);
    scanStore.Initialize();
    if( !scanStore.IsValid() )
    {
        msg_fatal("main", "cannot initialize a valid scan store from this directory: " << directory << eom);
        std::exit(1);
    }
    //pass the directory and root file info the the parameter store
    paramStore.Set("directory", directory);
    paramStore.Set("root_file", scanStore.GetRootFileBasename() );

    //load root file and extract useful vex info
    mho_json vexInfo = scanStore.GetRootFileData();
    extract_vex_info(vexInfo, &paramStore);

    ////////////////////////////////////////////////////////////////////////////
    //CONTROL CONSTRUCTION
    ////////////////////////////////////////////////////////////////////////////
    MHO_ControlFileParser cparser;
    MHO_ControlConditionEvaluator ceval;
    cparser.SetControlFile(control_file);
    mho_json control_format = MHO_ControlDefinitions::GetControlFormat();
    auto control_contents = cparser.ParseControl();
    mho_json control_statements;

    //TODO -- where should frequency group information get stashed/retrieved?
    std::string srcName = paramStore.GetAs<std::string>("/vex/scan/source/name");
    std::string scnName = paramStore.GetAs<std::string>("/vex/scan/name");
    ceval.SetPassInformation(baseline, srcName, "?", scnName);//baseline, source, fgroup, scan
    control_statements = ceval.GetApplicableStatements(control_contents);

    ////////////////////////////////////////////////////////////////////////////
    //LOAD DATA AND ASSEMBLE THE DATA STORE
    ////////////////////////////////////////////////////////////////////////////

    //load baseline data
    scanStore.LoadBaseline(baseline, &conStore);
    configure_data_library(&conStore);//momentarily needed for float -> double cast
    //load and rename station data according to reference/remote
    std::string ref_station_mk4id = std::string(1,baseline[0]);
    std::string rem_station_mk4id = std::string(1,baseline[1]);
    scanStore.LoadStation(ref_station_mk4id, &conStore);
    conStore.RenameObject("sta", "ref_sta");
    scanStore.LoadStation(rem_station_mk4id, &conStore);
    conStore.RenameObject("sta", "rem_sta");

    station_coord_type* ref_data = conStore.GetObject<station_coord_type>(std::string("ref_sta"));
    station_coord_type* rem_data = conStore.GetObject<station_coord_type>(std::string("rem_sta"));

    visibility_type* vis_data = conStore.GetObject<visibility_type>(std::string("vis"));
    weight_type* wt_data = conStore.GetObject<weight_type>(std::string("weight"));
    if( vis_data == nullptr || wt_data == nullptr )
    {
        msg_fatal("main", "could not find visibility or weight objects with names (vis, weight)." << eom);
        std::exit(1);
    }

    //DEBUG
    //conStore.DumpShortNamesToIds();
    #pragma message("TODO FIXME -- formalize the manner in which we identify data container objects via UUID")
    //temporarily put the object uuid's in the parameter store so we can look it up on the python side
    std::string vis_uuid = vis_data->GetObjectUUID().as_string();
    std::string wt_uuid = wt_data->GetObjectUUID().as_string();
    std::string ref_uuid = ref_data->GetObjectUUID().as_string();
    std::string rem_uuid = rem_data->GetObjectUUID().as_string();

    paramStore.Set("/uuid/visibilities", vis_uuid);
    paramStore.Set("/uuid/weights", wt_uuid);
    paramStore.Set("/uuid/ref_station", ref_uuid);
    paramStore.Set("/uuid/rem_station", rem_uuid);

    


    ////////////////////////////////////////////////////////////////////////////
    //PARAMETER SETTING
    ////////////////////////////////////////////////////////////////////////////
    set_default_parameters(&conStore, &paramStore); //set some default parameters (polprod, ref_freq)
    MHO_ParameterManager paramManager(&paramStore, control_format);

    paramManager.SetControlStatements(&control_statements);
    paramManager.ConfigureAll();
    // paramStore.Dump();

    //test grab the reference freq
    double ref_freq = paramStore.GetAs<double>(std::string("ref_freq"));

    ////////////////////////////////////////////////////////////////////////////
    //OPERATOR CONSTRUCTION
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

    MHO_OperatorBuilderManager build_manager(&opToolbox, &conStore, &paramStore, control_format);

    #ifdef USE_PYBIND11
    py::scoped_interpreter guard{}; // start the interpreter and keep it alive, need this or we segfault
    #pragma message("TODO FIXME -- formalize the means by which plugin dependent operator builders are added")
    build_manager.AddBuilderType<MHO_PythonOperatorBuilder>("python_labelling", "python_labelling"); 
    #endif


    build_manager.SetControlStatements(&control_statements);

    build_manager.BuildOperatorCategory("default");
    init_and_exec_operators(build_manager, &opToolbox, "labelling");
    init_and_exec_operators(build_manager, &opToolbox, "selection");

    //safety check
    if(vis_data->GetSize() == 0){msg_fatal("main", "no data left after cuts." << eom); std::exit(1);}

    init_and_exec_operators(build_manager, &opToolbox, "flagging");
    init_and_exec_operators(build_manager, &opToolbox, "calibration");

    //take a snapshot
    take_snapshot_here("test", "visib", __FILE__, __LINE__, vis_data);
    take_snapshot_here("test", "weights", __FILE__, __LINE__,  wt_data);

    //calulate useful quantities to stash in the parameter store
    precalculate_quantities(&conStore, &paramStore);

    //execute the basic fringe search algorithm
    basic_fringe_search(&conStore, &paramStore);

    //calculate the fringe properties
    calculate_fringe_info(&conStore, &paramStore, vexInfo);

    ////////////////////////////////////////////////////////////////////////////
    //PLOTTING/DEBUG
    ////////////////////////////////////////////////////////////////////////////
    //TODO may want to reorg the way this is done
    mho_json plot_dict = construct_plot_data(&conStore, &paramStore, vexInfo);
    fill_plot_data(&paramStore, plot_dict);

    //open and dump to file
    std::string output_file = paramStore.GetAs<std::string>("/cmdline/output_file");
    std::ofstream fdumpFile(output_file.c_str(), std::ofstream::out);
    fdumpFile << plot_dict;
    fdumpFile.close();

    #ifdef USE_PYBIND11
    //py::scoped_interpreter guard{}; // start the interpreter and keep it alive, need this or we segfault
    msg_debug("main", "python plot generation enabled." << eom );
    //test stuff

    py::dict plot_obj = plot_dict;

    //load our interface module
    auto vis_module = py::module::import("hops_visualization");
    auto plot_lib = vis_module.attr("fourfit_plot");
    //call a python function on the interface class instance
    plot_lib.attr("make_fourfit_plot")(plot_obj, "fplot.png");



    #endif //USE_PYBIND11

    return 0;
}
