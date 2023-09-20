#include "MHO_BasicFringeUtilities.hh"

//snapshot utility lib
#include "MHO_Snapshot.hh"

//configure_data_library
#include "MHO_ElementTypeCaster.hh"

//calculate_clock_model
#include "MHO_DelayModel.hh"

//precalculate_quantities
#include "MHO_Reducer.hh"
#include "MHO_DelayModel.hh"
#include "MHO_Clock.hh"
#include "MHO_UniformGridPointsCalculator.hh"

//parse_command_line
#include <getopt.h>

//basic_fringe_search
#include "MHO_NormFX.hh"
#include "MHO_DelayRate.hh"
#include "MHO_MBDelaySearch.hh"
#include "MHO_InterpolateFringePeak.hh"
#include "MHO_UniformGridPointsCalculator.hh"



//construct_plot_data
#include "MHO_ComputePlotData.hh"

namespace hops 
{
    

int 
MHO_BasicFringeUtilities::parse_command_line(int argc, char** argv, MHO_ParameterStore* paramStore)
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
                if(ap_per_seg < 0){ap_per_seg = 0; msg_warn("main", "invalid ap_per_seg, ignoring." << eom);}
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    if( directory == "" || baseline == "" || polprod == "" || control_file == "")
    {
        msg_fatal("main", "usage: "<< usage << eom);
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
            msg_warn("main", "debug messages are toggled via compiler flag, re-compile with ENABLE_DEBUG_MSG=ON to enable." << eom);
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
        msg_fatal("main", "baseline must be passed as 2-char code."<< eom);
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
MHO_BasicFringeUtilities::configure_data_library(MHO_ContainerStore* store)
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
    if(wt_shortname != "weights"){msg_warn("initialization", "weights do not use canonical short name 'weights', but are called: "<< wt_shortname << eom);}

    //now shove the double precision data into the container store with the same shortname
    store->AddObject(vis_data);
    store->AddObject(wt_data);
    store->SetShortName(vis_data->GetObjectUUID(), vis_shortname);
    store->SetShortName(wt_data->GetObjectUUID(), wt_shortname);
}



void 
MHO_BasicFringeUtilities::extract_clock_early(const mho_json& clk, 
                         double& clock_early, 
                         std::string& clock_early_units, 
                         double& clock_rate, 
                         std::string& clock_rate_units,
                         std::string& origin,
                         std::string& validity)
{
    clock_early = 0.;
    clock_early_units = "";
    clock_rate = 0.;
    clock_rate_units = "";
    origin = "";
    validity = "";
    //TODO deal with units!
    if(clk.contains("clock_early_offset"))
    {
        clock_early = clk["clock_early_offset"]["value"].get<double>();
        if(clk["clock_early_offset"].contains("units"))
        {
            clock_early_units = clk["clock_early_offset"]["units"].get<std::string>();
        }
    }

    if(clk.contains("clock_rate")) //this is an optional vex field
    {
        clock_rate = clk["clock_rate"]["value"].get<double>();
        if(clk["clock_rate"].contains("units"))
        {
            clock_rate_units =  clk["clock_rate"]["units"].get<std::string>();
        }
    }
    
    if(clk.contains("origin_epoch")) //this is an optional vex field
    {
        origin = clk["origin_epoch"].get<std::string>();
    }
    
    if(clk.contains("start_validity_epoch")) //this is an optional vex field
    {
        validity = clk["start_validity_epoch"].get<std::string>();
    }
}

void 
MHO_BasicFringeUtilities::extract_clock_model(const mho_json& vexInfo, MHO_ParameterStore* paramStore)
{
    std::size_t n_clocks = 0;
    double clock_early_offset = 0.;
    std::string offset_units = "";
    double clock_rate = 0.;
    std::string rate_units = "";
    std::string origin_epoch = "";
    std::string start_validity_epoch = "";
    

    //pulls out the appropriate segment of the clock model 
    //for the reference and remote stations and stashes in the parameter store
    //TODO FIXME, we also need extract the clock model, as well as station information
    
    //get the clock sections for the reference and remote stations
    std::string ref_clock_key = "/$CLOCK/" + paramStore->GetAs<std::string>("/ref_station/clock_ref");

    mho_json::json_pointer ref_clock_pointer(ref_clock_key);
    auto ref_clock = vexInfo.at(ref_clock_pointer);
    if( !ref_clock.contains("clock_early"))
    {
        msg_error("main", "root file missing $CLOCK information for reference station." << eom );
        std::exit(1);
    }

    #pragma message("TODO FIXME - select the approprate clock interval closest to the FRT.")
    //std::string frt_string = paramStore->GetAs<std::string>("/vex/scan/fourfit_reftime");

    n_clocks = ref_clock["clock_early"].size();
    if(n_clocks == 1)
    {
        auto clk = ref_clock["clock_early"][0];
        extract_clock_early(clk, clock_early_offset, offset_units, clock_rate, rate_units, origin_epoch, start_validity_epoch);
        paramStore->Set("/ref_station/clock_early_offset", clock_early_offset);
        paramStore->Set("/ref_station/clock_rate", clock_rate);
        if(offset_units != ""){paramStore->Set("/ref_station/clock_early_offset_units", offset_units);}
        if(rate_units != ""){paramStore->Set("/ref_station/clock_rate_units", rate_units);}
        if(origin_epoch != ""){paramStore->Set("/ref_station/clock_origin", origin_epoch);}
        if(start_validity_epoch != ""){paramStore->Set("/ref_station/clock_validity", start_validity_epoch);}
    }
    
    std::string rem_clock_key = "/$CLOCK/" + paramStore->GetAs<std::string>("/rem_station/clock_ref");
    mho_json::json_pointer rem_clock_pointer(rem_clock_key);
    auto rem_clock = vexInfo.at(rem_clock_pointer);
    if( !rem_clock.contains("clock_early"))
    {
        msg_error("main", "root file missing $CLOCK information for remote station."<< eom );
        std::exit(1);
    }
    
    n_clocks = rem_clock["clock_early"].size();
    if(n_clocks == 1)
    {
        auto clk = rem_clock["clock_early"][0];
        extract_clock_early(clk, clock_early_offset, offset_units, clock_rate, rate_units, origin_epoch, start_validity_epoch);
        paramStore->Set("/rem_station/clock_early_offset", clock_early_offset);
        paramStore->Set("/rem_station/clock_rate", clock_rate);
        if(offset_units != ""){paramStore->Set("/rem_station/clock_early_offset_units", offset_units);}
        if(rate_units != ""){paramStore->Set("/rem_station/clock_rate_units", rate_units);}
        if(origin_epoch != ""){paramStore->Set("/rem_station/clock_origin", origin_epoch);}
        if(start_validity_epoch != ""){paramStore->Set("/rem_station/clock_validity", start_validity_epoch);}
    }

}



void 
MHO_BasicFringeUtilities::extract_vex_info(const mho_json& vexInfo, MHO_ParameterStore* paramStore)
{
    std::string baseline = paramStore->GetAs<std::string>("/cmdline/baseline");
    std::string ref_station_mk4id = std::string(1,baseline[0]);
    std::string rem_station_mk4id = std::string(1,baseline[1]);
    
    paramStore->Set("/ref_station/mk4id", ref_station_mk4id);
    paramStore->Set("/rem_station/mk4id", rem_station_mk4id);
    

    //now get the rest of the station information from the $SITE section 
    //NOTE: to do this properly we should use the reference name specified under 
    //the $STATION section, but to do that we would need to know the station 2-char id,
    //but at this point all we have is the single-char mk4_id
    mho_json::json_pointer site_pointer("/$SITE");
    auto sites = vexInfo.at(site_pointer);
    if(sites.size() < 1)
    {
        msg_error("main", "root file contains missing or ambiguous $SITE information." << eom );
        std::exit(1);
    }
    
    //loop looking for sites which match the mk4 site ids
    for(auto site = sites.begin(); site != sites.end(); site++)
    {
        if(site->contains("mk4_site_ID"))
        {
            std::string mk4id = site->at("mk4_site_ID").get<std::string>();
            std::string site_id = site->at("site_ID").get<std::string>();
            std::string site_name = site->at("site_name").get<std::string>();
            std::string site_type = site->at("site_type").get<std::string>();
            
            if(mk4id == ref_station_mk4id)
            {
                paramStore->Set("/ref_station/site_id", site_id);
                paramStore->Set("/ref_station/site_name", site_name);
                paramStore->Set("/ref_station/site_type", site_type);
                //TODO maybe extract site position/velocity too? Do we need this?
                //position...yes, if we want to be able to calculate parallactic angle at any time from az,el spline
            }
            
            if(mk4id == rem_station_mk4id)
            {
                paramStore->Set("/rem_station/site_id", site_id);
                paramStore->Set("/rem_station/site_name", site_name);
                paramStore->Set("/rem_station/site_type", site_type);
            }
        }
    }

    //need to loop over the $STATION section to grab the reference names
    //for the other sections (namely $ANTENNA and $CLOCK)
    std::string ref_id = paramStore->GetAs<std::string>("/ref_station/site_id");
    std::string rem_id = paramStore->GetAs<std::string>("/rem_station/site_id");

    mho_json::json_pointer station_pointer("/$STATION");
    auto stations = vexInfo.at(station_pointer);
    if(stations.size() < 2)
    {
        msg_error("main", "root file contains missing or $STATION information." << eom );
        std::exit(1);
    }
    
    //loop over stations, their identifier needs to match the 2-char code
    for(auto station = stations.begin(); station != stations.end(); station++)
    {
        std::string code = station.key();
        //default reference keywords are just the 2-char station code
        std::string site_ref = code;
        std::string clock_ref = code;
        std::string antenna_ref = code;
        if(code == ref_id)
        {
            if(station->contains("$SITE")){site_ref = station->at("$SITE")[0]["keyword"].get<std::string>();}
            if(station->contains("$CLOCK")){clock_ref = station->at("$CLOCK")[0]["keyword"].get<std::string>();}
            if(station->contains("$ANTENNA")){antenna_ref = station->at("$ANTENNA")[0]["keyword"].get<std::string>();}
            paramStore->Set("/ref_station/site_ref", site_ref);
            paramStore->Set("/ref_station/clock_ref", clock_ref);
            paramStore->Set("/ref_station/antenna_ref", antenna_ref);
        }
        if(code == rem_id)
        {
            if(station->contains("$SITE")){site_ref = station->at("$SITE")[0]["keyword"].get<std::string>();}
            if(station->contains("$CLOCK")){clock_ref = station->at("$CLOCK")[0]["keyword"].get<std::string>();}
            if(station->contains("$ANTENNA")){antenna_ref = station->at("$ANTENNA")[0]["keyword"].get<std::string>();}
            paramStore->Set("/rem_station/site_ref", site_ref);
            paramStore->Set("/rem_station/clock_ref", clock_ref);
            paramStore->Set("/rem_station/antenna_ref", antenna_ref);
        }
    }

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
    paramStore->Set("/vex/scan/fourfit_reftime", frt_string);
    
    std::string start_loc = "/$SCHED/" + scnName + "/start";
    mho_json::json_pointer start_jptr(start_loc);
    std::string start_string = vexInfo.at(start_jptr).get<std::string>();
    paramStore->Set("/vex/scan/start", start_string);
    
    //get experiment info
    mho_json exper_section = vexInfo["$EXPER"];
    auto exper_info = exper_section.begin().value();
    std::string exper_name = exper_info["exper_name"];
    paramStore->Set("/vex/experiment_name", exper_name); //required

    if(exper_info.contains("exper_num"))
    {
        std::stringstream ss;
        ss << exper_info["exper_num"];
        paramStore->Set("/vex/experiment_number", ss.str());
    }
    else
    {
        paramStore->Set("/vex/experiment_number", "----");
    }

    //NOTE: this implicitly assumes that all channels and all stations have the same bandwidth!
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
    
    //extract the clock model, as well as station information
    extract_clock_model(vexInfo, paramStore);

}




void 
MHO_BasicFringeUtilities::calculate_fringe_info(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore, const mho_json& vexInfo)
{
    //vex section info and quantities
    mho_json exper_section = vexInfo["$EXPER"];
    auto exper_info = exper_section.begin().value();

    mho_json src_section = vexInfo["$SOURCE"];
    auto src_info = src_section.begin().value();

    mho_json freq_section = vexInfo["$FREQ"];
    auto freq_info = freq_section.begin().value();
    double sample_rate = freq_info["sample_rate"]["value"];
    //TODO FIXME (what if channels have multiple-bandwidths?, units?)
    double samp_period = 1.0/(sample_rate*1e6);

    //configuration parameters
    double ref_freq = paramStore->GetAs<double>("ref_freq");
    double ap_delta = paramStore->GetAs<double>("ap_period");

    //fringe quantities
    double total_summed_weights = paramStore->GetAs<double>("/fringe/total_summed_weights");
    double sbdelay = paramStore->GetAs<double>("/fringe/sbdelay");
    double mbdelay = paramStore->GetAs<double>("/fringe/mbdelay");
    double drate = paramStore->GetAs<double>("/fringe/drate");
    double frate = paramStore->GetAs<double>("/fringe/frate");
    double famp = paramStore->GetAs<double>("/fringe/famp");

    //a priori (correlator) model quantities
    double adelay = paramStore->GetAs<double>("/model/adelay");
    double arate = paramStore->GetAs<double>("/model/arate");
    double aaccel = paramStore->GetAs<double>("/model/aaccel");

    std::string frt_vex_string = paramStore->GetAs<std::string>("/vex/scan/fourfit_reftime");
    auto frt = hops_clock::from_vex_format(frt_vex_string);
    legacy_hops_date frt_ldate = hops_clock::to_legacy_hops_date(frt);

    std::string start_vex_string = paramStore->GetAs<std::string>("/vex/scan/start");
    auto start_time = hops_clock::from_vex_format(start_vex_string);
    double start_offset = paramStore->GetAs<double>("start_offset");
    int64_t start_offset_as_nanosec = (int64_t)start_offset*SEC_TO_NANOSEC; //KLUDGE;

    start_time = start_time + hops_clock::duration(start_offset_as_nanosec);
    legacy_hops_date start_ldate = hops_clock::to_legacy_hops_date(start_time);

    double stop_offset = paramStore->GetAs<double>("stop_offset");
    int64_t stop_offset_as_nanosec = (int64_t)stop_offset*SEC_TO_NANOSEC; //KLUDGE;

    auto stop_time = hops_clock::from_vex_format(start_vex_string);
    stop_time = stop_time + hops_clock::duration(stop_offset_as_nanosec);
    legacy_hops_date stop_ldate = hops_clock::to_legacy_hops_date(stop_time);
    std::string year_doy = 
        MHO_BasicFringeInfo::leftpadzeros_integer(4, frt_ldate.year) + ":" + 
        MHO_BasicFringeInfo::leftpadzeros_integer(3, frt_ldate.day);

    //figure out the legacy date/time stamps for start, stop, and FRT
    paramStore->Set("/fringe/year_doy", year_doy);
    paramStore->Set("/fringe/legacy_start_timestamp", MHO_BasicFringeInfo::make_legacy_datetime_format(start_ldate) );
    paramStore->Set("/fringe/legacy_stop_timestamp", MHO_BasicFringeInfo::make_legacy_datetime_format(stop_ldate) );
    paramStore->Set("/fringe/legacy_frt_timestamp", MHO_BasicFringeInfo::make_legacy_datetime_format(frt_ldate) );

    //calculate SNR
    #pragma message("TODO FIXME -- properly calcualte the effective number of pol-products.")
    double eff_npol = 1.0;
    double snr = MHO_BasicFringeInfo::calculate_snr(eff_npol, ap_delta, samp_period, total_summed_weights, famp);
    paramStore->Set("/fringe/snr", snr);

    //calculate integration time
    int nchan = paramStore->GetAs<int>("nchannels");
    double integration_time =  (total_summed_weights*ap_delta)/(double)nchan;
    paramStore->Set("/fringe/integration_time", integration_time);

    //calculate quality code
    std::string quality_code = MHO_BasicFringeInfo::calculate_qf();
    paramStore->Set("/fringe/quality_code", quality_code);

    //total number of points searched
    std::size_t nmbd = paramStore->GetAs<std::size_t>("/fringe/n_mbd_points");
    std::size_t nsbd = paramStore->GetAs<std::size_t>("/fringe/n_sbd_points");
    std::size_t ndr = paramStore->GetAs<std::size_t>("/fringe/n_dr_points");
    double total_npts_searched = (double)nmbd * (double)nsbd *(double)ndr;

    //residual phase in radians and degrees
    double resid_phase_rad = calculate_residual_phase(conStore, paramStore);
    double resid_phase_deg = std::fmod(resid_phase_rad*(180.0/M_PI), 360.0);

    //calculate the a priori phase and total phase
    double aphase = std::fmod( ref_freq*adelay*360.0, 360.0); //from fill_208.c, no conversion from radians??
    double tot_phase_deg = std::fmod( aphase + resid_phase_rad*(180.0/M_PI), 360.0 );
    paramStore->Set("/fringe/aphase", aphase);

    //calculate the probability of false detection, THIS IS BROKEN
    #pragma message("TODO FIXME - PFD calculation needs the MBD/SBD/DR windows defined")
    double pfd = MHO_BasicFringeInfo::calculate_pfd(snr, total_npts_searched);
    pfd = 0.0;
    paramStore->Set("/fringe/prob_false_detect", pfd);

    //TODO FIXME -- -acount for units (these are printed on fringe plot in usec)
    double tot_mbd = adelay + mbdelay;
    double tot_sbd = adelay + sbdelay;

    double ambig = paramStore->GetAs<double>("/fringe/ambiguity");
    double freq_spacing = paramStore->GetAs<double>("/fringe/frequency_spacing");
    std::string mbd_anchor = paramStore->GetAs<std::string>("mbd_anchor");

    // anchor total mbd to sbd if desired by control
    double delta_mbd = 0.0;
    double freq0 = paramStore->GetAs<double>("/fringe/start_frequency");
    if(mbd_anchor == "sbd")
    {
        delta_mbd = ambig * std::floor( (tot_sbd - tot_mbd) / ambig + 0.5);
        tot_mbd += delta_mbd;
        //tweaks tot_phase_deg and resid_phase_deg
        MHO_BasicFringeInfo::correct_phases_mbd_anchor_sbd(ref_freq, freq0, freq_spacing, delta_mbd, tot_phase_deg, resid_phase_deg);
    }
    double resid_ph_delay = resid_phase_rad / (2.0 * M_PI * ref_freq);
    double ph_delay = adelay + resid_ph_delay;

    paramStore->Set("/fringe/resid_phase", resid_phase_deg);
    paramStore->Set("/fringe/resid_ph_delay", resid_ph_delay);
    paramStore->Set("/fringe/phase_delay", ph_delay);
    paramStore->Set("/fringe/tot_phase", tot_phase_deg);

    double tot_drate = arate + drate;
    paramStore->Set("/fringe/total_sbdelay", tot_sbd);
    paramStore->Set("/fringe/total_mbdelay", tot_mbd);
    paramStore->Set("/fringe/total_drate", tot_drate);

    double sbd_sep = paramStore->GetAs<double>("/fringe/sbd_separation");
    double freq_spread = paramStore->GetAs<double>("/fringe/frequency_spread");
    double mbd_error = MHO_BasicFringeInfo::calculate_mbd_no_ion_error(freq_spread, snr);

    #pragma message("TODO FIXME, calculate SBAVG properly")
    double sbavg = 1.0;

    double sbd_error = MHO_BasicFringeInfo::calculate_sbd_error(sbd_sep, snr, sbavg);

    int total_naps = paramStore->GetAs<int>("total_naps");
    double drate_error = MHO_BasicFringeInfo::calculate_drate_error_v1(snr, ref_freq, total_naps, ap_delta);
    //may want to consider using this version in the future
    //double drate_error = calculate_drate_error_v2(snr, ref_freq, integration_time);

    paramStore->Set("/fringe/mbd_error", mbd_error);
    paramStore->Set("/fringe/sbd_error", sbd_error);
    paramStore->Set("/fringe/drate_error", drate_error);

    double ph_err = MHO_BasicFringeInfo::calculate_phase_error(sbavg, snr);
    double phdelay_err = MHO_BasicFringeInfo::calculate_phase_delay_error(sbavg, snr, ref_freq);

    paramStore->Set("/fringe/phase_error", ph_err);
    paramStore->Set("/fringe/phase_delay_error", phdelay_err);

    double ref_clock_off = paramStore->GetAs<double>("/ref_station/clock_offset_at_frt");
    double rem_clock_off = paramStore->GetAs<double>("/rem_station/clock_offset_at_frt");
    double ref_rate = paramStore->GetAs<double>("/ref_station/clock_rate");
    double rem_rate = paramStore->GetAs<double>("/rem_station/clock_rate");

    double clock_offset = rem_clock_off - ref_clock_off;
    double clock_rate =  rem_rate - ref_rate;

    paramStore->Set("/fringe/relative_clock_offset", clock_offset); //usec
    paramStore->Set("/fringe/relative_clock_rate", clock_rate*1e6); //usec/s

}





void 
MHO_BasicFringeUtilities::fill_plot_data(MHO_ParameterStore* paramStore, mho_json& plot_dict)
{
    plot_dict["Quality"] = paramStore->GetAs<std::string>("/fringe/quality_code");
    plot_dict["SNR"] = paramStore->GetAs<double>("/fringe/snr");
    plot_dict["IntgTime"] = paramStore->GetAs<double>("/fringe/integration_time");
    plot_dict["Amp"] = paramStore->GetAs<double>("/fringe/famp");
    plot_dict["ResPhase"] = paramStore->GetAs<double>("/fringe/resid_phase");
    plot_dict["PFD"] = paramStore->GetAs<double>("/fringe/prob_false_detect");

    plot_dict["ResidSbd(us)"] = paramStore->GetAs<double>("/fringe/sbdelay");
    plot_dict["ResidMbd(us)"] = paramStore->GetAs<double>("/fringe/mbdelay");
    plot_dict["FringeRate(Hz)"]  = paramStore->GetAs<double>("/fringe/frate");
    plot_dict["IonTEC(TEC)"] = "-";
    plot_dict["RefFreq(MHz)"] = paramStore->GetAs<double>("ref_freq");
    plot_dict["AP(sec)"] = paramStore->GetAs<double>("ap_period");
    plot_dict["ExperName"] = paramStore->GetAs<std::string>("/vex/experiment_name");
    plot_dict["ExperNum"] = paramStore->GetAs<std::string>("/vex/experiment_number");

    //export the legacy date/time stamps for start, stop, and FRT
    plot_dict["YearDOY"] = paramStore->GetAs<std::string>("/fringe/year_doy");
    plot_dict["Start"] = paramStore->GetAs<std::string>("/fringe/legacy_start_timestamp");
    plot_dict["Stop"] = paramStore->GetAs<std::string>("/fringe/legacy_stop_timestamp");
    plot_dict["FRT"] = paramStore->GetAs<std::string>("/fringe/legacy_frt_timestamp");
    plot_dict["CorrTime"] = "-";
    plot_dict["FFTime"] = "-";
    plot_dict["BuildTime"] = "-";

    plot_dict["RA"] = paramStore->GetAs<std::string>("/vex/scan/source/ra");
    plot_dict["Dec"] = paramStore->GetAs<std::string>("/vex/scan/source/dec");

    //in order to follow the PDD interface, the name of the following output
    //parameter changes depending on whether or not we are using mbd_anchor = sbd

    std::string mbd_anchor = paramStore->GetAs<std::string>("mbd_anchor");
    if(mbd_anchor == "sbd")
    {
        plot_dict["GroupDelaySBD(usec)"] = paramStore->GetAs<double>("/fringe/total_mbdelay");
    }
    else
    {
        plot_dict["GroupDelayModel(usec)"] = paramStore->GetAs<double>("/fringe/total_mbdelay");
    }
    plot_dict["SbandDelay(usec)"] = paramStore->GetAs<double>("/fringe/total_sbdelay");
    plot_dict["DelayRate(ps/s)"] = paramStore->GetAs<double>("/fringe/total_drate");
    plot_dict["PhaseDelay(usec)"] = paramStore->GetAs<double>("/fringe/phase_delay");
    plot_dict["TotalPhase(deg)"] = paramStore->GetAs<double>("/fringe/tot_phase");

    plot_dict["AprioriClock(usec)"] = paramStore->GetAs<double>("/fringe/relative_clock_offset");
    plot_dict["AprioriClockrate(us/s)"] = paramStore->GetAs<double>("/fringe/relative_clock_rate");
    plot_dict["AprioriDelay(usec)"] = paramStore->GetAs<double>("/model/adelay");
    plot_dict["AprioriRate(us/s)"] = paramStore->GetAs<double>("/model/arate");
    plot_dict["AprioriAccel(us/s/s)"] = paramStore->GetAs<double>("/model/aaccel");

    plot_dict["ResidMbdelay(usec)"] = paramStore->GetAs<double>("/fringe/mbdelay");
    plot_dict["ResidSbdelay(usec)"] = paramStore->GetAs<double>("/fringe/sbdelay");
    plot_dict["ResidPhdelay(usec)"] = paramStore->GetAs<double>("/fringe/resid_ph_delay");
    plot_dict["ResidRate(us/s)"] = paramStore->GetAs<double>("/fringe/drate");
    plot_dict["ResidPhase(deg)"] = paramStore->GetAs<double>("/fringe/resid_phase"); //degrees

    plot_dict["ResidMbdelayError(usec)"] = paramStore->GetAs<double>("/fringe/mbd_error");
    plot_dict["ResidSbdelayError(usec)"] = paramStore->GetAs<double>("/fringe/sbd_error");
    plot_dict["ResidPhdelayError(usec)"] = paramStore->GetAs<double>("/fringe/phase_delay_error");
    plot_dict["ResidRateError(us/s)"] = paramStore->GetAs<double>("/fringe/drate_error");
    plot_dict["ResidPhaseError(deg)"] =  paramStore->GetAs<double>("/fringe/phase_error");
}


void 
MHO_BasicFringeUtilities::calculate_freq_space(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
{
    visibility_type* vis_data = conStore->GetObject<visibility_type>(std::string("vis"));

    //calculate the frequency grid for MBD search
    MHO_UniformGridPointsCalculator fGridCalc;
    fGridCalc.SetPoints( std::get<CHANNEL_AXIS>(*vis_data).GetData(), std::get<CHANNEL_AXIS>(*vis_data).GetSize() );
    fGridCalc.Calculate();

    double gridStart = fGridCalc.GetGridStart();
    double gridSpace = fGridCalc.GetGridSpacing();
    double ambig = 1.0/gridSpace;
    double nGridPoints = fGridCalc.GetNGridPoints();
    double averageFreq = fGridCalc.GetGridAverage();
    double freqSpread = fGridCalc.GetSpread();

    //correct the frequency spread if we only have 1 channel
    //the number of channels present after cuts
    int nchan = paramStore->GetAs<int>("nchannels");
    double channel_bandwidth = paramStore->GetAs<double>("channel_bandwidth");

    if(nchan == 1)
    {
        freqSpread = channel_bandwidth/std::sqrt(12.0); //uniform distribution
    }

    paramStore->Set("/fringe/start_frequency", gridStart);
    paramStore->Set("/fringe/frequency_spacing", gridSpace);
    paramStore->Set("/fringe/ambiguity", ambig);
    paramStore->Set("/fringe/n_frequency_points", nGridPoints);
    paramStore->Set("/fringe/average_frequency", averageFreq);
    paramStore->Set("/fringe/frequency_spread", freqSpread);

}

void 
MHO_BasicFringeUtilities::calculate_clock_model(MHO_ParameterStore* paramStore)
{
    std::string frt_string = paramStore->GetAs<std::string>("/vex/scan/fourfit_reftime");
    auto frt_tp = hops_clock::from_vex_format(frt_string);


    double ref_offset = paramStore->GetAs<double>("/ref_station/clock_early_offset");
    double ref_rate = paramStore->GetAs<double>("/ref_station/clock_rate");
    std::string ref_validity_epoch = paramStore->GetAs<std::string>("/ref_station/clock_validity");
    std::string ref_origin_epoch = paramStore->GetAs<std::string>("/ref_station/clock_origin");

    auto ref_origin_tp = frt_tp;
    if(ref_origin_epoch != ""){ref_origin_tp = hops_clock::from_vex_format(ref_origin_epoch);}

    double rem_offset = paramStore->GetAs<double>("/rem_station/clock_early_offset");
    double rem_rate = paramStore->GetAs<double>("/rem_station/clock_rate");
    std::string rem_validity_epoch = paramStore->GetAs<std::string>("/rem_station/clock_validity");
    std::string rem_origin_epoch = paramStore->GetAs<std::string>("/rem_station/clock_origin");

    auto rem_origin_tp = frt_tp;
    if(rem_origin_epoch != ""){rem_origin_tp = hops_clock::from_vex_format(rem_origin_epoch);}


    //TODO ALSO ENSURE THAT THE VALIDITY EPOCH IS BEFORE THE FRT
    double refdiff = 0.0;
    if(ref_rate != 0.0)
    {
        auto ref_tdiff_duration = frt_tp - ref_origin_tp;
        refdiff = std::chrono::duration<double>(ref_tdiff_duration).count();
    }
    if(refdiff > 3.0e5)
    {
        msg_info("main", "reference station clockrate epoch: " <<
        hops_clock::to_iso8601_format(ref_origin_tp) <<
        ", is highly discrepant from FRT: " << hops_clock::to_iso8601_format(frt_tp) << eom );
    }

    double remdiff = 0.0;
    if(rem_rate != 0.0)
    {
        auto rem_tdiff_duration = frt_tp - rem_origin_tp;
        remdiff = std::chrono::duration<double>(rem_tdiff_duration).count();
    }

    if(remdiff > 3.0e5)
    {
        msg_info("main", "remote station clockrate epoch: " <<
        hops_clock::to_iso8601_format(rem_origin_tp) <<
        ", is highly discrepant from FRT: " << hops_clock::to_iso8601_format(frt_tp) << eom );
    }

    double ref_clock = ref_offset + (refdiff*ref_rate)*1e6; //in usec!
    double rem_clock = rem_offset + (remdiff*rem_rate)*1e6; //in usec!

    paramStore->Set("/ref_station/clock_offset_at_frt", ref_clock);
    paramStore->Set("/rem_station/clock_offset_at_frt", rem_clock);
}





void 
MHO_BasicFringeUtilities::precalculate_quantities(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
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

    //append info about the total number of APs
    int naps = ap_ax->GetSize();
    paramStore->Set("total_naps", naps);


    //grab the channel bandwidth (assume to be the same for all channels)
    auto chan_ax = &(std::get<CHANNEL_AXIS>(*vis_data));
    auto ch0_labels = chan_ax->GetIntervalsWhichIntersect((std::size_t)0);
    double bandwidth = 0;
    for(auto it = ch0_labels.begin(); it != ch0_labels.end(); it++)
    {
        if( (*it)->HasKey("bandwidth") )
        {
            (*it)->Retrieve("bandwidth", bandwidth);
            break;
        }
    }
    paramStore->Set("channel_bandwidth", bandwidth);

    //offset to the start of the data
    double start_offset = ap_ax->at(0);
    paramStore->Set("start_offset", start_offset);

    //offset to the start of the data
    double stop_offset = ap_ax->at( ap_ax->GetSize()-1 ) + ap_delta;
    paramStore->Set("stop_offset", stop_offset);

    //the number of channels present after cuts
    int nchan = chan_ax->GetSize();
    paramStore->Set("nchannels", nchan);

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
    paramStore->Set("/fringe/total_summed_weights", total_ap_frac);
    wt_data->Insert("total_summed_weights", total_ap_frac);

    //compute the a priori delay model
    station_coord_type* ref_data = conStore->GetObject<station_coord_type>(std::string("ref_sta"));
    station_coord_type* rem_data = conStore->GetObject<station_coord_type>(std::string("rem_sta"));
    MHO_DelayModel delay_model;
    std::string frt_vex_string = paramStore->GetAs<std::string>("/vex/scan/fourfit_reftime");
    delay_model.SetFourfitReferenceTimeVexString(frt_vex_string);
    delay_model.SetReferenceStationData(ref_data);
    delay_model.SetRemoteStationData(rem_data);
    delay_model.ComputeModel();

    //calculate the offset to the refence time (within the scan)
    auto frt = hops_clock::from_vex_format(frt_vex_string);
    std::string start_vex_string = paramStore->GetAs<std::string>("/vex/scan/start");
    auto start_time = hops_clock::from_vex_format(start_vex_string);
    auto offset_to_frt_duration = frt - start_time;
    double frt_offset = std::chrono::duration<double>(offset_to_frt_duration).count();
    paramStore->Set("frt_offset", frt_offset);

    double adelay = delay_model.GetDelay();
    double arate = delay_model.GetRate();
    double aaccel = delay_model.GetAcceleration();

    //store and  convert to microsec
    //TODO FIXME - document units of all the various parameters/quantities
    paramStore->Set("/model/adelay", 1e6*adelay);
    paramStore->Set("/model/arate", 1e6*arate);
    paramStore->Set("/model/aaccel", 1e6*aaccel);

    //figure out the clock information at the FRT
    calculate_clock_model(paramStore);

    //determines properties of the frequency grid used in the MBD search
    calculate_freq_space(conStore, paramStore);

}


// void 
// MHO_BasicFringeUtilities::init_and_exec_operators(MHO_OperatorBuilderManager& build_manager, MHO_OperatorToolbox* opToolbox, const char* category)
// {
//     std::string cat(category);
//     build_manager.BuildOperatorCategory(cat);
//     auto ops = opToolbox->GetOperatorsByCategory(cat);
//     for(auto opIt= ops.begin(); opIt != ops.end(); opIt++)
//     {
//         msg_debug("main", "initializing and executing operator: "<< (*opIt)->GetName() << eom);
//         (*opIt)->Initialize();
//         (*opIt)->Execute();
//     }
// }




void 
MHO_BasicFringeUtilities::basic_fringe_search(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
{
    bool ok;
    visibility_type* vis_data = conStore->GetObject<visibility_type>(std::string("vis"));
    weight_type* wt_data = conStore->GetObject<weight_type>(std::string("weight"));
    if( vis_data == nullptr || wt_data == nullptr )
    {
        msg_fatal("main", "could not find visibility or weight objects with names (vis, weight)." << eom);
        std::exit(1);
    }

    //temporarily organize the main fringe search in this function
    //TODO consolidate the coarse search in a single class, so it can be mixed-and-matched
    //with different interpolation schemes

    //output for the delay
    std::size_t bl_dim[visibility_type::rank::value];
    vis_data->GetDimensions(bl_dim);
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
    double ref_freq = paramStore->GetAs<double>("ref_freq");
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
    double freq_spacing = mbdSearch.GetFrequencySpacing();
    double ave_freq = mbdSearch.GetAverageFrequency();


    paramStore->Set("/fringe/max_mbd_bin", c_mbdmax);
    paramStore->Set("/fringe/max_sbd_bin", c_sbdmax);
    paramStore->Set("/fringe/max_dr_bin", c_drmax);
    // paramStore->Set("/fringe/ambiguity", 1.0/freq_spacing);
    // paramStore->Set("/fringe/average_frequency", ave_freq);

    std::size_t n_mbd_pts = sbd_dr_data->GetDimension(CHANNEL_AXIS);
    std::size_t n_dr_pts = sbd_dr_data->GetDimension(TIME_AXIS);
    std::size_t n_sbd_pts = sbd_dr_data->GetDimension(FREQ_AXIS);
    paramStore->Set("/fringe/n_mbd_points", n_mbd_pts);
    paramStore->Set("/fringe/n_sbd_points", n_sbd_pts);
    paramStore->Set("/fringe/n_dr_points", n_dr_pts);

    ////////////////////////////////////////////////////////////////////////////
    //FINE INTERPOLATION STEP (search over 5x5x5 grid around peak)
    ////////////////////////////////////////////////////////////////////////////
    MHO_InterpolateFringePeak fringeInterp;

    bool optimize_closure_flag = false;
    bool is_oc_set = paramStore->Get(std::string("optimize_closure"), optimize_closure_flag );
    //NOTE, this has no effect on fringe-phase when using 'simul' algo
    //This is also true in the legacy code simul implementation, and which is
    //currently is the only one implemented
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

    //add the sbd_data, and sbd_dr_data to the container store
    conStore->AddObject(sbd_data);
    conStore->AddObject(sbd_dr_data);
    conStore->SetShortName(sbd_data->GetObjectUUID(), "sbd");
    conStore->SetShortName(sbd_dr_data->GetObjectUUID(), "sbd_dr");
}


mho_json 
MHO_BasicFringeUtilities::construct_plot_data(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore, mho_json& vexInfo)
{
    ////////////////////////////////////////////////////////////////////////////
    //PLOTTING/DEBUG
    ////////////////////////////////////////////////////////////////////////////
    visibility_type* vis_data = conStore->GetObject<visibility_type>(std::string("vis"));
    weight_type* wt_data = conStore->GetObject<weight_type>(std::string("weight"));
    visibility_type* sbd_data = conStore->GetObject<visibility_type>(std::string("sbd"));
    //visibility_type* sbd_dr_data = conStore->GetObject<weight_type>(std::string("sbd_dr"));
    
    //test grab the reference freq
    double ref_freq = paramStore->GetAs<double>(std::string("ref_freq"));
    
    std::string directory = paramStore->GetAs<std::string>("/cmdline/directory");
    std::string control_file = paramStore->GetAs<std::string>("/cmdline/control_file");
    std::string baseline = paramStore->GetAs<std::string>("/cmdline/baseline");
    std::string polprod = paramStore->GetAs<std::string>("/cmdline/polprod");
    std::string root_file = paramStore->GetAs<std::string>("root_file");

    std::string mbd_anchor;
    bool is_mbd_anchor_set = paramStore->Get(std::string("mbd_anchor"), mbd_anchor);

    MHO_ComputePlotData mk_plotdata;
    mk_plotdata.SetParameterStore(paramStore);
    mk_plotdata.SetContainerStore(conStore);
    mk_plotdata.SetVexInfo(vexInfo);
    bool optimize_closure_flag = false;
    bool is_oc_set = paramStore->Get(std::string("optimize_closure"), optimize_closure_flag );
    if(optimize_closure_flag){mk_plotdata.EnableOptimizeClosure();} //this does have an effect on overall fringe phase
    if(is_mbd_anchor_set){mk_plotdata.SetMBDAnchor(mbd_anchor);} //effect not yet implemented
    mk_plotdata.Initialize();

    mho_json plot_dict;
    mk_plotdata.DumpInfoToJSON(plot_dict);

    mho_json sched_section = vexInfo["$SCHED"];
    std::string scan_name = sched_section.begin().key();
    auto sched_info = sched_section.begin().value();
    plot_dict["RootScanBaseline"] = root_file + ", " + scan_name + ", " + baseline;
    plot_dict["CorrVers"] = "HOPS4/DiFX fourfit  rev 0";
    
    auto ref_name = paramStore->GetAs<std::string>("/ref_station/site_name");
    auto rem_name = paramStore->GetAs<std::string>("/rem_station/site_name");
    std::string freq_group = "fgroup ?";

    plot_dict["PolStr"] = ref_name + " - " + rem_name +", " + freq_group + ", " + "pol " + polprod;

    //DEBUG! open and dump to file
    std::string output_file = "fdump.json"; //for testing
    std::string fdump = output_file;
    std::ofstream fdumpFile(fdump.c_str(), std::ofstream::out);
    fdumpFile << plot_dict;
    fdumpFile.close();

    return plot_dict;
}

void 
MHO_BasicFringeUtilities::set_default_parameters(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
{
    //set the selected pol-product
    std::string polprod = paramStore->GetAs<std::string>("/cmdline/polprod");
    paramStore->Set("selected_polprod", polprod);

    //grab the visibility data, so we can determine the default reference frequency
    visibility_type* vis_data = conStore->GetObject<visibility_type>(std::string("vis"));

    //the first frequency in the array serves as the reference frequency if this value remains unset in the control file
    double first_freq = std::get<CHANNEL_AXIS>(*vis_data)(0);
    paramStore->Set("ref_freq", first_freq);

    //default mbd_anchor is model (instead of sbd)
    std::string mbd_anchor = "model";
    paramStore->Set("mbd_anchor", mbd_anchor);
};



}//end namespace
