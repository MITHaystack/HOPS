#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <getopt.h>
#include <iomanip>

#define EXTRA_DEBUG

#include "ffit.hh"

int main(int argc, char** argv)
{

    std::string usage = "ffit -d <directory> -c <control file> -b <baseline> -p <pol. product>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    MHO_Snapshot::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().SetExecutableName(std::string("SimpleFringeSearchPlot"));

    std::string directory = "";
    std::string control_file = "";
    std::string baseline = "";
    std::string polprod = "";
    std::string output_file = "fdump.json"; //for testing
    bool ok;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"directory", required_argument, 0, 'd'},
                                          {"control", required_argument, 0, 'c'},
                                          {"baseline", required_argument, 0, 'b'},
                                          {"polarization-product", required_argument, 0, 'p'},
                                          {"output", required_argument, 0, 'o'}};

    static const char* optString = "hd:c:b:p:o:";

    while(true)
    {
        char optId = getopt_long(argc, argv, optString, longOptions, NULL);
        if (optId == -1)
            break;
        switch(optId)
        {
            case ('h'):  // help
                std::cout << usage << std::endl;
                return 0;
            case ('d'):
                directory = std::string(optarg);
                break;
            case ('c'):
                control_file = std::string(optarg);
                break;
            case ('b'):
                baseline = std::string(optarg);
                break;
            case ('p'):
                polprod = std::string(optarg);
                break;
            case ('o'):
                output_file = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    if( directory == "" || baseline == "" || polprod == "" || control_file == "")
    {
        std::cout << usage << std::endl;
        return 1;
    }

    if(baseline.size() != 2){msg_fatal("main", "baseline must be passed as 2-char code."<<eom); std::exit(1);}

    ////////////////////////////////////////////////////////////////////////////
    //INITIAL SCAN DIRECTORY
    ////////////////////////////////////////////////////////////////////////////

    //initialize the scan store from this directory
    MHO_ScanDataStore scanStore;
    scanStore.SetDirectory(directory);
    scanStore.Initialize();
    if( !scanStore.IsValid() )
    {
        msg_fatal("main", "cannot initialize a valid scan store from this directory: " << directory << eom);
        std::exit(1);
    }

    //load root file and container store for this baseline
    mho_json vexInfo = scanStore.GetRootFileData();

    //create the parameter store
    MHO_ParameterStore* paramStore = new MHO_ParameterStore();
    extract_vex_info(vexInfo, paramStore);

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
    std::string srcName = paramStore->GetAs<std::string>("/vex/scan/source/name");
    std::string scnName = paramStore->GetAs<std::string>("/vex/scan/name");
    ceval.SetPassInformation(baseline, srcName, "?", scnName);//baseline, source, fgroup, scan
    control_statements = ceval.GetApplicableStatements(control_contents);
    std::cout<< control_statements.dump(2) <<std::endl;

    ////////////////////////////////////////////////////////////////////////////
    //LOAD DATA AND ASSEMBLE THE DATA STORE
    ////////////////////////////////////////////////////////////////////////////

    MHO_ContainerStore* conStore = new MHO_ContainerStore();
    MHO_OperatorToolbox* opToolbox = new MHO_OperatorToolbox();
    

    
    std::cout<<"dumping parameter store"<<std::endl;
    paramStore->Dump();
    
    
    //load baseline data
    scanStore.LoadBaseline(baseline, conStore);

    configure_data_library(conStore);//momentarily needed for float -> double cast

    std::string ref_station_mk4id = std::string(1,baseline[0]);
    std::string rem_station_mk4id = std::string(1,baseline[1]);
    scanStore.LoadStation(ref_station_mk4id, conStore);
    conStore->RenameObject("sta", "ref_sta");
    scanStore.LoadStation(rem_station_mk4id, conStore);
    conStore->RenameObject("sta", "rem_sta");

    visibility_type* vis_data = conStore->GetObject<visibility_type>(std::string("vis"));
    weight_type* wt_data = conStore->GetObject<weight_type>(std::string("weight"));
    if( vis_data == nullptr || wt_data == nullptr )
    {
        msg_fatal("main", "could not find visibility or weight objects with names (vis, weight)." << eom);
        std::exit(1);
    }

    //DEBUG
    //conStore->DumpShortNamesToIds();

    ////////////////////////////////////////////////////////////////////////////
    //PARAMETER SETTING
    ////////////////////////////////////////////////////////////////////////////
    MHO_ParameterManager paramManager(paramStore, control_format);
    //set defaults
    paramStore->Set(std::string("selected_polprod"), polprod);
    // paramStore->Set(std::string("fourfit_reftime_vex_string"), frt_vex_string);

    paramManager.SetControlStatements(&control_statements);
    paramManager.ConfigureAll();
    paramStore->Dump();

    //test grab the reference freq
    double ref_freq = paramStore->GetAs<double>(std::string("ref_freq"));

    ////////////////////////////////////////////////////////////////////////////
    //OPERATOR CONSTRUCTION
    ////////////////////////////////////////////////////////////////////////////
    //add the data selection operator
    //TODO FIXME -- this is a horrible hack to get this operator into the initialization stream
    #pragma message("fix this horrible hack")
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


    MHO_OperatorBuilderManager build_manager(opToolbox, conStore, paramStore, control_format);
    build_manager.SetControlStatements(&control_statements);

    build_manager.BuildOperatorCategory("default");
    std::cout<<"toolbox has: "<<opToolbox->GetNOperators()<<" operators."<<std::endl;

    build_and_exec_operators(build_manager, opToolbox, "labelling");
    build_and_exec_operators(build_manager, opToolbox, "selection");
    
    //safety check
    std::size_t bl_dim[visibility_type::rank::value];
    vis_data->GetDimensions(bl_dim);
    for(std::size_t i=0; i < visibility_type::rank::value; i++)
    {
        if(bl_dim[i] == 0){msg_fatal("main", "no data left after cuts." << eom); std::exit(1);}
    }
    
    build_and_exec_operators(build_manager, opToolbox, "flagging");
    build_and_exec_operators(build_manager, opToolbox, "calibration");

    //take a snapshot
    take_snapshot_here("test", "visib", __FILE__, __LINE__, vis_data);
    take_snapshot_here("test", "weights", __FILE__, __LINE__,  wt_data);

    //calulate useful quantities to stash in the parameter store
    precalculate_quantities(conStore, paramStore);

    //output for the delay
    visibility_type* sbd_data = vis_data->CloneEmpty();
    bl_dim[FREQ_AXIS] *= 4; //normfx implementation demands this
    sbd_data->Resize(bl_dim);

    ////////////////////////////////////////////////////////////////////////////
    //COARSE SBD, DR, MBD SEARCH ALGO
    ////////////////////////////////////////////////////////////////////////////

    //run norm-fx via the wrapper class (x-form to SBD space)
    MHO_NormFX nfxOp;
    nfxOp.SetArgs(vis_data, wt_data, sbd_data);
    ok = nfxOp.Initialize();
    check_step_fatal(ok, "main", "normfx initialization." << eom );

    ok = nfxOp.Execute();
    check_step_fatal(ok, "main", "normfx execution." << eom );

    //take snapshot of sbd data after normfx
    take_snapshot_here("test", "sbd", __FILE__, __LINE__, sbd_data);

    //run the transformation to delay rate space (this also involves a zero padded FFT)
    MHO_DelayRate drOp;
    visibility_type* sbd_dr_data = sbd_data->CloneEmpty();
    drOp.SetReferenceFrequency(ref_freq);
    drOp.SetArgs(sbd_data, wt_data, sbd_dr_data);
    ok = drOp.Initialize();
    check_step_fatal(ok, "main", "dr initialization." << eom );
    ok = drOp.Execute();
    check_step_fatal(ok, "main", "dr execution." << eom );

    take_snapshot_here("test", "sbd_dr", __FILE__, __LINE__, sbd_dr_data);

    //coarse SBD/MBD/DR search (locates max bin)
    MHO_MBDelaySearch mbdSearch;
    mbdSearch.SetArgs(sbd_dr_data);
    ok = mbdSearch.Initialize();
    check_step_fatal(ok, "main", "mbd initialization." << eom );
    ok = mbdSearch.Execute();
    check_step_fatal(ok, "main", "mbd execution." << eom );

    int c_mbdmax = mbdSearch.GetMBDMaxBin();
    int c_sbdmax = mbdSearch.GetSBDMaxBin();
    int c_drmax = mbdSearch.GetDRMaxBin();

    paramStore->Set("/fringe/max_mbd_bin", c_mbdmax);
    paramStore->Set("/fringe/max_sbd_bin", c_sbdmax);
    paramStore->Set("/fringe/max_dr_bin", c_drmax);

    std::cout<<"SBD/MBD/DR max bins = "<<c_sbdmax<<", "<<c_mbdmax<<", "<<c_drmax<<std::endl;

    ////////////////////////////////////////////////////////////////////////////
    //FINE INTERPOLATION STEP (search over 5x5x5 grid around peak)
    ////////////////////////////////////////////////////////////////////////////
    MHO_InterpolateFringePeak fringeInterp;
        
    bool optimize_closure_flag = false;
    bool is_oc_set = paramStore->Get(std::string("optimize_closure"), optimize_closure_flag );
    std::cout<<"optimize closure??? "<<is_oc_set<<", "<<optimize_closure_flag<<std::endl;
    //NOTE, this has no effect on fringe phase when using 'simul' algo (which is the only one implemented currently)
    if(optimize_closure_flag){fringeInterp.EnableOptimizeClosure();}
    
    fringeInterp.SetReferenceFrequency(ref_freq);
    fringeInterp.SetMaxBins(c_sbdmax, c_mbdmax, c_drmax);

    fringeInterp.SetSBDArray(sbd_data);
    fringeInterp.SetWeights(wt_data);

    #pragma message("TODO FIXME -- we shouldn't be referencing internal members of the MHO_MBDelaySearch class workspace")
    //Figure out how best to present this axis data to the fine-interp function.
    fringeInterp.SetMBDAxis( mbdSearch.GetMBDAxis() );
    fringeInterp.SetDRAxis( mbdSearch.GetDRAxis() ); 

    fringeInterp.Initialize();
    fringeInterp.Execute();

    double sbdelay = fringeInterp.GetSBDelay();
    double mbdelay = fringeInterp.GetMBDelay();
    double drate = fringeInterp.GetDelayRate();
    double frate = fringeInterp.GetFringeRate();
    double famp = fringeInterp.GetFringeAmplitude();

    paramStore->Set("/fringe/sbdelay", sbdelay);
    paramStore->Set("/fringe/mbdelay", mbdelay);
    paramStore->Set("/fringe/drate", drate);
    paramStore->Set("/fringe/frate", frate);
    paramStore->Set("/fringe/famp", famp);

    station_coord_type* ref_data = conStore->GetObject<station_coord_type>(std::string("ref_sta"));
    station_coord_type* rem_data = conStore->GetObject<station_coord_type>(std::string("rem_sta"));
    MHO_DelayModel delay_model;
    std::string frt_vex_string = paramStore->GetAs<std::string>("/vex/scan/fourfit_reftime");
    delay_model.SetFourfitReferenceTimeVexString(frt_vex_string);
    delay_model.SetReferenceStationData(ref_data);
    delay_model.SetRemoteStationData(rem_data);
    delay_model.ComputeModel();

    double ap_delay = delay_model.GetDelay();
    double ap_rate = delay_model.GetRate();
    double ap_accel = delay_model.GetAcceleration();

    paramStore->Set("/model/ap_delay", ap_delay);
    paramStore->Set("/model/ap_rate", ap_rate);
    paramStore->Set("/model/ap_accel", ap_accel);
    
    paramStore->Dump();

    ////////////////////////////////////////////////////////////////////////////
    //PLOTTING/DEBUG
    ////////////////////////////////////////////////////////////////////////////
    //TODO FIXME Organize all the plot data generation better

    std::string mbd_anchor;
    bool is_mbd_anchor_set = paramStore->Get(std::string("mbd_anchor"), mbd_anchor);

    MHO_ComputePlotData mk_plotdata;
    if(optimize_closure_flag){mk_plotdata.EnableOptimizeClosure();} //this does have an effect on overall fringe phase
    if(is_mbd_anchor_set){mk_plotdata.SetMBDAnchor(mbd_anchor);} //effect not yet implemented

    double total_ap_frac = paramStore->GetAs<double>("/fringe/total_summed_weights");
    mk_plotdata.SetSummedWeights(total_ap_frac);
    mk_plotdata.SetReferenceFrequency(ref_freq);
    mk_plotdata.SetMBDelay(mbdelay);
    mk_plotdata.SetDelayRate(drate);
    mk_plotdata.SetFringeRate(frate);
    mk_plotdata.SetSBDelay(sbdelay);
    mk_plotdata.SetSBDArray(sbd_data);
    mk_plotdata.SetSBDelayBin(c_sbdmax);
    mk_plotdata.SetAmplitude(famp);
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

    delete paramStore;
    delete conStore;
    delete opToolbox;

    return 0;
}
