#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <getopt.h>
#include <iomanip>


#define EXTRA_DEBUG

//global messaging util
#include "MHO_Message.hh"

//snapshot utility lib
#include "MHO_Snapshot.hh"

//needed to read hops files and extract objects from scan dir
#include "MHO_ScanDataStore.hh"

//control
#include "MHO_ControlFileParser.hh"
#include "MHO_ControlConditionEvaluator.hh"

//operators
#include "MHO_ElementTypeCaster.hh"
#include "MHO_NormFX.hh"
#include "MHO_SelectRepack.hh"
#include "MHO_UniformGridPointsCalculator.hh"
#include "MHO_FringeRotation.hh"
#include "MHO_Reducer.hh"

#include "MHO_AbsoluteValue.hh"
#include "MHO_FunctorBroadcaster.hh"
#include "MHO_ExtremaSearch.hh"
#include "MHO_ManualChannelPhaseCorrection.hh"
#include "MHO_DelayRate.hh"
#include "MHO_MBDelaySearch.hh"
#include "MHO_InterpolateFringePeak.hh"

//initialization
#include "MHO_OperatorBuilderManager.hh"
#include "MHO_ParameterConfigurator.hh"
#include "MHO_ParameterManager.hh"

#include "MHO_ComputePlotData.hh"


//pybind11 stuff to interface with python
#ifdef USE_PYBIND11

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;
namespace nl = nlohmann;
using namespace pybind11::literals;

#endif

using namespace hops;


void configure_data_library(MHO_ContainerStore* store)
{
    //retrieve the (first) visibility and weight objects
    //(currently assuming there is only one object per type)
    visibility_store_type* vis_store_data = nullptr;
    weight_store_type* wt_store_data = nullptr;

    vis_store_data = store->GetObject<visibility_store_type>(0);
    wt_store_data = store->GetObject<weight_store_type>(0);

    if(vis_store_data == nullptr)
    {
        msg_fatal("main", "failed to read visibility data from the .cor file." <<eom);
        std::exit(1);
    }

    if(wt_store_data == nullptr)
    {
        msg_fatal("main", "failed to read weight data from the .cor file." <<eom);
        std::exit(1);
    }

    std::size_t n_vis = store->GetNObjects<visibility_store_type>();
    std::size_t n_wt = store->GetNObjects<weight_store_type>();

    if(n_vis != 1 || n_wt != 1)
    {
        msg_warn("main", "multiple visibility and/or weight types not yet supported" << eom);
    }

    std::string vis_shortname = store->GetShortName(vis_store_data->GetObjectUUID() );
    std::string wt_shortname = store->GetShortName(wt_store_data->GetObjectUUID() );

    visibility_type* vis_data = new visibility_type();
    weight_type* wt_data = new weight_type();

    MHO_ElementTypeCaster<visibility_store_type, visibility_type> up_caster;
    up_caster.SetArgs(vis_store_data, vis_data);
    up_caster.Initialize();
    up_caster.Execute();

    MHO_ElementTypeCaster< weight_store_type, weight_type> wt_up_caster;
    wt_up_caster.SetArgs(wt_store_data, wt_data);
    wt_up_caster.Initialize();
    wt_up_caster.Execute();

    //remove the original objects
    store->DeleteObject(vis_store_data);
    store->DeleteObject(wt_store_data);

    #pragma message("TODO - if we plan to rely on short-names to identify objects, we need to validate them here")
    //TODO make sure that the visibility object is called 'vis' and weights are called 'weights', etc.
    //TODO also validate the station data

    //now shove the double precision data into the container store with the same shortname
    store->AddObject(vis_data);
    store->AddObject(wt_data);
    store->SetShortName(vis_data->GetObjectUUID(), vis_shortname);
    store->SetShortName(wt_data->GetObjectUUID(), wt_shortname);
}


void build_and_exec_operators(MHO_OperatorBuilderManager& build_manager, MHO_OperatorToolbox* opToolbox, const char* category)
{
    std::string cat(category);
    build_manager.BuildOperatorCategory(cat);
    std::cout<<"toolbox has: "<<opToolbox->GetNOperators()<<" operators."<<std::endl;
    auto ops = opToolbox->GetOperatorsByCategory(cat);
    for(auto opIt= ops.begin(); opIt != ops.end(); opIt++)
    {
        std::cout<<"init and exec of: "<<(*opIt)->GetName()<<std::endl;
        (*opIt)->Initialize();
        (*opIt)->Execute();
    }    
}

int main(int argc, char** argv)
{

    std::string usage = "SimpleFringeSearchPlot -d <directory> -c <control file> -b <baseline> -p <pol. product>";

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

    //get scan name and source name
    mho_json::json_pointer sched_pointer("/$SCHED");
    auto sched = vexInfo.at(sched_pointer);
    if(sched.size() != 1)
    {
        msg_error("main", "root file " <<  scanStore.GetRootFileBasename() <<" contains missing or ambiguous $SCHED information." << eom );
        std::exit(1);
    }

    std::string scnName = sched.begin().key();
    std::string src_loc = "/$SCHED/" + scnName + "/source/0/source";
    mho_json::json_pointer src_jptr(src_loc);
    std::string srcName = vexInfo.at(src_jptr).get<std::string>();

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
    ceval.SetPassInformation(baseline, srcName, "?", scnName);//baseline, source, fgroup, scan
    control_statements = ceval.GetApplicableStatements(control_contents);
    std::cout<< control_statements.dump(2) <<std::endl;

    ////////////////////////////////////////////////////////////////////////////
    //LOAD DATA AND ASSEMBLE THE DATA STORE
    ////////////////////////////////////////////////////////////////////////////
    MHO_ParameterStore* paramStore = new MHO_ParameterStore();
    MHO_ContainerStore* conStore = new MHO_ContainerStore();
    MHO_OperatorToolbox* opToolbox = new MHO_OperatorToolbox();
    //load baseline data
    scanStore.LoadBaseline(baseline, conStore);

    configure_data_library(conStore);//momentarily needed for float -> double cast

    std::string ref_station_mk4id = std::string(1,baseline[0]);
    std::string rem_station_mk4id = std::string(1,baseline[1]);
    scanStore.LoadStation(ref_station_mk4id, conStore);
    scanStore.LoadStation(rem_station_mk4id, conStore);


    visibility_type* vis_data = conStore->GetObject<visibility_type>(std::string("vis"));
    weight_type* wt_data = conStore->GetObject<weight_type>(std::string("weight"));
    if( vis_data == nullptr || wt_data == nullptr )
    {
        msg_fatal("main", "could not find visibility or weight objects with names (vis, weight)." << eom);
        std::exit(1);
    }

    //DEBUG
    conStore->DumpShortNamesToIds();

    ////////////////////////////////////////////////////////////////////////////
    //PARAMETER SETTING
    ////////////////////////////////////////////////////////////////////////////
    MHO_ParameterManager paramManager(paramStore, control_format);
    //set defaults
    paramStore->Set(std::string("selected_polprod"), polprod);

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

    // //compute the sum of the weights
    weight_type temp_weights;
    temp_weights.Copy(*wt_data);
    MHO_Reducer<weight_type, MHO_CompoundSum> wt_reducer;
    wt_reducer.SetArgs(&temp_weights);
    for(std::size_t i=0; i<weight_type::rank::value; i++)
    {
        wt_reducer.ReduceAxis(i);
    }
    wt_reducer.Initialize();
    wt_reducer.Execute();

    double total_ap_frac = temp_weights[0];
    std::cout<<"reduced weights = "<<temp_weights[0]<<std::endl;

    wt_data->Insert("total_summed_weights", total_ap_frac);

    take_snapshot_here("test", "reduced_weights", __FILE__, __LINE__,  &temp_weights);


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

    //todo ought to make this a more uniform/cleaner interface (probably using the common label map store)
    double sbdelay = fringeInterp.GetSBDelay();
    double mbdelay = fringeInterp.GetMBDelay();
    double drate = fringeInterp.GetDelayRate();
    double frate = fringeInterp.GetFringeRate();
    double famp = fringeInterp.GetFringeAmplitude();

    ////////////////////////////////////////////////////////////////////////////
    //PLOTTING/DEBUG
    ////////////////////////////////////////////////////////////////////////////
    //TODO FIXME Organize all the plot data generation better

    std::string mbd_anchor;
    bool is_mbd_anchor_set = paramStore->Get(std::string("mbd_anchor"), mbd_anchor);

    MHO_ComputePlotData mk_plotdata;
    if(optimize_closure_flag){mk_plotdata.EnableOptimizeClosure();} //this does have an effect on overall fringe phase
    if(is_mbd_anchor_set){mk_plotdata.SetMBDAnchor(mbd_anchor);} //effect not yet implemented
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

    mho_json plot_dict = mk_plotdata.DumpInfoToJSON();

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
