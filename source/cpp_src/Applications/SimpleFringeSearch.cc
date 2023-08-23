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
#include "MHO_DelayModel.hh"

#include "MHO_Clock.hh"


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




void fill_output_info(const MHO_ParameterStore* paramStore, const mho_json& vexInfo, mho_json& plot_dict)
{
    //vex section info and quantities
    mho_json exper_section = vexInfo["$EXPER"];
    auto exper_info = exper_section.begin().value();

    mho_json src_section = vexInfo["$SOURCE"];
    auto src_info = src_section.begin().value();

    mho_json freq_section = vexInfo["$FREQ"];
    auto freq_info = freq_section.begin().value();
    double sample_rate = freq_info["sample_rate"]["value"];
    double samp_period = 1.0/(sample_rate*1e6);


    //configuration parameters 
    double ref_freq = paramStore->GetAs<double>("ref_freq");
    double ap_delta = paramStore->GetAs<double>("ap_period");

    //fringe quantities
    double sbdelay = paramStore->GetAs<double>("/fringe/sbdelay");
    double mbdelay = paramStore->GetAs<double>("/fringe/mbdelay");
    double drate = paramStore->GetAs<double>("/fringe/drate");
    double frate = paramStore->GetAs<double>("/fringe/frate");
    double famp = paramStore->GetAs<double>("/fringe/famp");

    //a priori (correlator) model quantities 
    double ap_delay = paramStore->GetAs<double>("/model/ap_delay");
    double ap_rate = paramStore->GetAs<double>("/model/ap_rate");
    double ap_accel = paramStore->GetAs<double>("/model/ap_accel");

    // 
    // //plot_dict["ResPhase"] = std::fmod(coh_avg_phase * (180.0/M_PI), 360.0);
    plot_dict["PFD"] = "-";
    plot_dict["ResidSbd(us)"] = sbdelay;
    plot_dict["ResidMbd(us)"] = mbdelay;
    plot_dict["FringeRate(Hz)"]  = frate;
    plot_dict["IonTEC(TEC)"] = "-";
    plot_dict["RefFreq(MHz)"] = ref_freq;
    plot_dict["AP(sec)"] = ap_delta;
    plot_dict["ExperName"] = exper_info["exper_name"];
    plot_dict["ExperNum"] = "-";
    
    std::string frt_vex_string = paramStore->GetAs<std::string>("fourfit_reftime_vex_string");
    auto frt = hops_clock::from_vex_format(frt_vex_string);
    legacy_hops_date frt_ldate = hops_clock::to_legacy_hops_date(frt);
    std::stringstream ss;
    ss << frt_ldate.year;
    ss << ":";
    ss << std::setw(3) << std::setfill('0') << frt_ldate.day;
    std::string year_doy = ss.str();
    // 
    // std::cout<<"hops time-point converted to legacy hops-date-struct: "<<std::endl;
    // std::cout<<"year = "<<ldate.year<<std::endl;
    // std::cout<<"date = "<<ldate.day<<std::endl;
    // std::cout<<"hour = "<<ldate.hour<<std::endl;
    // std::cout<<"mins = "<<ldate.minute<<std::endl;
    // std::cout<<"secs = "<< std::setprecision(9) <<ldate.second<<std::endl;
    // 
    // 
    // t200->frt.year = t200->scantime.year;
    // t200->frt.second = fmod ((double)param->reftime,  60.0);
    // int_reftime = param->reftime;       /* In seconds */
    // int_reftime /= 60;                  /* Now in minutes */
    // t200->frt.minute = int_reftime % 60;
    // int_reftime /= 60;                  /* Now in hours */
    // t200->frt.hour = int_reftime % 24;
    // t200->frt.day = int_reftime / 24 + 1; /* days start with 001 */
    // 
    // 
    // 
    plot_dict["YearDOY"] = year_doy;
    plot_dict["Start"] = "-";
    plot_dict["Stop"] = "-";
    plot_dict["FRT"] = "-";
    plot_dict["CorrTime"] = "-";
    plot_dict["FFTime"] = "-";
    plot_dict["BuildTime"] = "-";
    
    plot_dict["RA"] = src_info["ra"];
    plot_dict["Dec"] = src_info["dec"];


    plot_dict["GroupDelay"] = 0;
        // dp->param->mbd_anchor == MODEL ? "Model(usec)" : "SBD(usec)  ",
        // dp->fringe->t208->tot_mbd);
    plot_dict["SbandDelay(usec)"] = 0;
        //dp->fringe->t208->tot_sbd);
    plot_dict["PhaseDelay(usec)"] = 0;
        //dp->fringe->t208->adelay + dp->status->resid_ph_delay);
    plot_dict["TotalPhase(deg)"] = 0;
        //dp->fringe->t208->totphase);
    plot_dict["AprioriDelay(usec)"] = 0;
        //dp->fringe->t208->adelay);
    plot_dict["AprioriClock(usec)"] = 0;
        //dp->fringe->t202->rem_clock - dp->fringe->t202->ref_clock);
    plot_dict["AprioriClockrate(us/s)"] = 0;
        //(dp->fringe->t202->rem_clockrate - dp->fringe->t202->ref_clockrate));
    plot_dict["AprioriRate(us/s)"] = 0;
        //dp->fringe->t208->arate);
    plot_dict["AprioriAccel(us/s/s)"] = 0;
        //dp->fringe->t208->aaccel);
    plot_dict["ResidMbdelay(usec)"] = 0;
        //dp->fringe->t208->resid_mbd);
    plot_dict["ResidSbdelay(usec)"] = 0;
        //dp->fringe->t208->resid_sbd);
    plot_dict["ResidPhdelay(usec)"] = 0;
        //dp->status->resid_ph_delay);
    plot_dict["ResidRate(us/s)"] = 0;
        //dp->fringe->t208->resid_rate);
    plot_dict["ResidPhase(deg)"] = 0;
        //dp->fringe->t208->resphase);
    plot_dict["ResidMbdelayError(usec)"] = 0;
        //dp->fringe->t208->mbd_error);
    plot_dict["ResidSbdelayError(usec)"] = 0;
        //dp->fringe->t208->sbd_error);
    plot_dict["ResidPhdelayError(usec)"] = 0;
        //dp->status->ph_delay_err);
    plot_dict["ResidRateError(us/s)"] = 0;
        //dp->fringe->t208->rate_error);
    plot_dict["ResidPhaseError(deg)"] = 0;
        //dp->status->phase_err);
}

//extract useful information about this scan from the vex info
void extract_vex_info(const mho_json& vexInfo, MHO_ParameterStore* paramStore)
{
    //get scan name and source name
    mho_json::json_pointer sched_pointer("/$SCHED");
    auto sched = vexInfo.at(sched_pointer);
    if(sched.size() != 1)
    {
        msg_error("main", "root file contains missing or ambiguous $SCHED information." << eom );
        std::exit(1);
    }

    //get the source information
    std::string scnName = sched.begin().key();
    std::string src_loc = "/$SCHED/" + scnName + "/source/0/source";
    mho_json::json_pointer src_jptr(src_loc);
    std::string srcName = vexInfo.at(src_jptr).get<std::string>();
    paramStore->Set("/vex/scan/name",scnName);
    paramStore->Set("/vex/scan/source/name",srcName);

    mho_json src_section = vexInfo["$SOURCE"];
    auto src_info = src_section.begin().value();
    std::string ra = src_info["ra"];
    std::string dec = src_info["dec"];
    paramStore->Set("/vex/scan/source/ra", ra);
    paramStore->Set("/vex/scan/source/dec", dec);

    //get the fourfit reference time 
    std::string frt_loc = "/$SCHED/" + scnName + "/fourfit_reftime";
    mho_json::json_pointer frt_jptr(frt_loc);
    std::string frt_string = vexInfo.at(frt_jptr).get<std::string>();
    std::cout<<"FOURFIT REFERENCE TIME = "<<frt_string<<std::endl;
    paramStore->Set("/vex/scan/fourfit_reftime", frt_string);
    
    std::string start_loc = "/$SCHED/" + scnName + "/start";
    mho_json::json_pointer start_jptr(start_loc);
    std::string start_string = vexInfo.at(start_jptr).get<std::string>();
    std::cout<<"START TIME = "<<start_string<<std::endl;
    paramStore->Set("/vex/scan/start", start_string);
    
    //get experiment info
    mho_json exper_section = vexInfo["$EXPER"];
    auto exper_info = exper_section.begin().value();
    std::string exper_name = exper_info["exper_name"];
    paramStore->Set("/vex/experiment_name", exper_name);

    //NOTE: this implicitly assumes that all channels have the same bandwidth!
    mho_json freq_section = vexInfo["$FREQ"];
    auto freq_info = freq_section.begin().value();
    double sample_rate = freq_info["sample_rate"]["value"];
    std::string sample_rate_units = freq_info["sample_rate"]["units"];
    double factor = 1.0;
    #pragma message("TODO: handle units other than MHz")
    if(sample_rate_units == "MHz")
    {
        factor = 1e6;
    }
    sample_rate *= sample_rate*factor;
    
    paramStore->Set("/vex/scan/sample_rate", sample_rate);
    paramStore->Set("/vex/scan/sample_period", 1.0/sample_rate);
    
    //TODO FIXME, we also need extract the clock model, as well as station information 
    
    
        
    
}



//calculate useful quantities used later throughout the program
void precalculate_quantities(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
{
    visibility_type* vis_data = conStore->GetObject<visibility_type>(std::string("vis"));
    weight_type* wt_data = conStore->GetObject<weight_type>(std::string("weight"));
    if( vis_data == nullptr || wt_data == nullptr )
    {
        msg_fatal("main", "could not find visibility or weight objects with names (vis, weight)." << eom);
        std::exit(1);
    }

    auto ap_ax = &(std::get<TIME_AXIS>(*vis_data));
    if(ap_ax->GetSize() <= 1)
    {
        msg_fatal("main", "could not determine AP period for data." << eom);
        std::exit(1);
    }

    double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    paramStore->Set("ap_period", ap_delta);


    // //TODO FIXME -- should this be the fourfit refrence time? Also...should this be calculated elsewhere?
    // double midpoint_time = ( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    // std::cout<<"time midpoint = "<<midpoint_time<<std::endl;


    //compute the sum of the data weights
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
    paramStore->Set("/fringe/total_summed_weights", total_ap_frac);
    wt_data->Insert("total_summed_weights", total_ap_frac);
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
