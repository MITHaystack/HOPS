#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define EXTRA_DEBUG

#include "ffit.hh"

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
    std::string output_file = "fdump.json"; //for testing
    bool ok;

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

    //std::cout<<control_contents.dump(4)<<std::endl;

    //TODO -- where should frequency group information get stashed/retrieved?
    std::string srcName = paramStore.GetAs<std::string>("/vex/scan/source/name");
    std::string scnName = paramStore.GetAs<std::string>("/vex/scan/name");
    ceval.SetPassInformation(baseline, srcName, "?", scnName);//baseline, source, fgroup, scan
    control_statements = ceval.GetApplicableStatements(control_contents);
    std::cout << control_statements.dump(2) <<std::endl;

    ////////////////////////////////////////////////////////////////////////////
    //LOAD DATA AND ASSEMBLE THE DATA STORE
    ////////////////////////////////////////////////////////////////////////////

    std::cout<<"dumping parameter store"<<std::endl;
    paramStore.Dump();    
    
    //load baseline data
    scanStore.LoadBaseline(baseline, &conStore);
    configure_data_library(&conStore);//momentarily needed for float -> double cast

    std::string ref_station_mk4id = std::string(1,baseline[0]);
    std::string rem_station_mk4id = std::string(1,baseline[1]);
    scanStore.LoadStation(ref_station_mk4id, &conStore);
    conStore.RenameObject("sta", "ref_sta");
    scanStore.LoadStation(rem_station_mk4id, &conStore);
    conStore.RenameObject("sta", "rem_sta");

    visibility_type* vis_data = conStore.GetObject<visibility_type>(std::string("vis"));
    weight_type* wt_data = conStore.GetObject<weight_type>(std::string("weight"));
    if( vis_data == nullptr || wt_data == nullptr )
    {
        msg_fatal("main", "could not find visibility or weight objects with names (vis, weight)." << eom);
        std::exit(1);
    }

    //DEBUG
    //conStore.DumpShortNamesToIds();

    ////////////////////////////////////////////////////////////////////////////
    //PARAMETER SETTING
    ////////////////////////////////////////////////////////////////////////////
    MHO_ParameterManager paramManager(&paramStore, control_format);
    //set defaults
    paramStore.Set(std::string("selected_polprod"), polprod);

    paramManager.SetControlStatements(&control_statements);
    paramManager.ConfigureAll();
    paramStore.Dump();

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
    build_manager.SetControlStatements(&control_statements);

    build_manager.BuildOperatorCategory("default");
    build_and_exec_operators(build_manager, &opToolbox, "labelling");
    build_and_exec_operators(build_manager, &opToolbox, "selection");
    
    //safety check
    if(vis_data->GetSize() == 0){msg_fatal("main", "no data left after cuts." << eom); std::exit(1);}
    
    build_and_exec_operators(build_manager, &opToolbox, "flagging");
    build_and_exec_operators(build_manager, &opToolbox, "calibration");

    //take a snapshot
    take_snapshot_here("test", "visib", __FILE__, __LINE__, vis_data);
    take_snapshot_here("test", "weights", __FILE__, __LINE__,  wt_data);

    //calulate useful quantities to stash in the parameter store
    precalculate_quantities(&conStore, &paramStore);
    
    //execute the basic fringe search algorithm
    basic_fringe_search(&conStore, &paramStore);

    station_coord_type* ref_data = conStore.GetObject<station_coord_type>(std::string("ref_sta"));
    station_coord_type* rem_data = conStore.GetObject<station_coord_type>(std::string("rem_sta"));
    MHO_DelayModel delay_model;
    std::string frt_vex_string = paramStore.GetAs<std::string>("/vex/scan/fourfit_reftime");
    delay_model.SetFourfitReferenceTimeVexString(frt_vex_string);
    delay_model.SetReferenceStationData(ref_data);
    delay_model.SetRemoteStationData(rem_data);
    delay_model.ComputeModel();

    double ap_delay = delay_model.GetDelay();
    double ap_rate = delay_model.GetRate();
    double ap_accel = delay_model.GetAcceleration();

    paramStore.Set("/model/ap_delay", ap_delay);
    paramStore.Set("/model/ap_rate", ap_rate);
    paramStore.Set("/model/ap_accel", ap_accel);
    
    paramStore.Dump();

    ////////////////////////////////////////////////////////////////////////////
    //PLOTTING/DEBUG
    ////////////////////////////////////////////////////////////////////////////
    //TODO FIXME Organize all the plot data generation better

    int c_mbdmax = paramStore.GetAs<int>("/fringe/max_mbd_bin");
    int c_sbdmax = paramStore.GetAs<int>("/fringe/max_sbd_bin");
    int c_drmax = paramStore.GetAs<int>("/fringe/max_dr_bin");
    double sbdelay = paramStore.GetAs<double>("/fringe/sbdelay");
    double mbdelay = paramStore.GetAs<double>("/fringe/mbdelay");
    double drate = paramStore.GetAs<double>("/fringe/drate");
    double frate = paramStore.GetAs<double>("/fringe/frate");
    double famp = paramStore.GetAs<double>("/fringe/famp");

    std::string mbd_anchor;
    bool is_mbd_anchor_set = paramStore.Get(std::string("mbd_anchor"), mbd_anchor);

    MHO_ComputePlotData mk_plotdata;
    bool optimize_closure_flag = false;
    bool is_oc_set = paramStore.Get(std::string("optimize_closure"), optimize_closure_flag );
    if(optimize_closure_flag){mk_plotdata.EnableOptimizeClosure();} //this does have an effect on overall fringe phase
    if(is_mbd_anchor_set){mk_plotdata.SetMBDAnchor(mbd_anchor);} //effect not yet implemented


    visibility_type* sbd_data = conStore.GetObject<visibility_type>(std::string("sbd"));
    //visibility_type* sbd_dr_data = conStore.GetObject<weight_type>(std::string("sbd_dr"));

    double total_ap_frac = paramStore.GetAs<double>("/fringe/total_summed_weights");
    mk_plotdata.SetSummedWeights(total_ap_frac);
    mk_plotdata.SetReferenceFrequency(ref_freq);
    mk_plotdata.SetMBDelay(mbdelay);
    mk_plotdata.SetDelayRate(drate);
    mk_plotdata.SetFringeRate(frate);
    mk_plotdata.SetSBDelay(sbdelay);
    mk_plotdata.SetSBDelayBin(c_sbdmax);
    mk_plotdata.SetAmplitude(famp);

    mk_plotdata.SetSBDArray(sbd_data);
    mk_plotdata.SetWeights(wt_data);
    mk_plotdata.SetVisibilities(vis_data);

    mk_plotdata.SetVexInfo(vexInfo);

    mho_json plot_dict;
    mk_plotdata.DumpInfoToJSON(plot_dict);

    mho_json sched_section = vexInfo["$SCHED"];
    std::string scan_name = sched_section.begin().key();
    auto sched_info = sched_section.begin().value();
    plot_dict["RootScanBaseline"] = scanStore.GetRootFileBasename() + ", " + scan_name + ", " + baseline;
    plot_dict["CorrVers"] = "HOPS4/DiFX fourfit  rev 0";
    plot_dict["PolStr"] = polprod;

    //open and dump to file
    std::string fdump = output_file;
    std::ofstream fdumpFile(fdump.c_str(), std::ofstream::out);
    fdumpFile << plot_dict;
    fdumpFile.close();

    #ifdef USE_PYBIND11
    
    std::cout<<"python plotting"<<std::endl;
    //test stuff
    py::scoped_interpreter guard{}; // start the interpreter and keep it alive, need this or we segfault
    py::dict plot_obj = plot_dict;

    //load our interface module
    auto ff_test = py::module::import("ff_plot_test");
    //call a python functioin on the interface class instance
    ff_test.attr("fourfit_plot")(plot_obj, "fplot.png");

    #endif //USE_PYBIND11

    return 0;
}
