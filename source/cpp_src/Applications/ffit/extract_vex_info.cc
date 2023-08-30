#include "ffit.hh"

void extract_clock_model(const mho_json& vexInfo, MHO_ParameterStore* paramStore)
{
    //pulls out the appropriate segment of the clock model 
    //for the reference and remote stations and stashes in the parameter store
    
    
}


//extract useful information about this scan from the vex info
void extract_vex_info(const mho_json& vexInfo, MHO_ParameterStore* paramStore)
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
    std::string rem_id = paramStore->GetAs<std::string>("/ref_station/site_id");

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
        std::cout<<"STATION CODE = "<<code<<std::endl;
        if(code == ref_id)
        {
            std::string site_ref = station->at("$SITE")[0]["keyword"].get<std::string>();
            std::string clock_ref = station->at("$CLOCK")[0]["keyword"].get<std::string>();
            std::string antenna_ref = station->at("$ANTENNA")[0]["keyword"].get<std::string>();
            paramStore->Set("/ref_station/site_ref", site_ref);
            paramStore->Set("/ref_station/clock_ref", clock_ref);
            paramStore->Set("/ref_station/antenna_ref", antenna_ref);
        }
        
        if(code == rem_id)
        {
            std::string site_ref = station->at("$SITE")[0]["keyword"].get<std::string>();
            std::string clock_ref = station->at("$CLOCK")[0]["keyword"].get<std::string>();
            std::string antenna_ref = station->at("$ANTENNA")[0]["keyword"].get<std::string>();
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
    
    //loop over the 'clock_early' entries
    std::size_t n_clocks = ref_clock["clock_early"].size();
    std::size_t selected_interval = 0;
    for(std::size_t i = 0; i<n_clocks; i++)
    {
        auto clk = ref_clock["clock_early"][i];
        
        if(clk.contains("clock_early_offset"))
        {
            
        }

        if(clk.contains("clock_rate"))
        {
            
        }
        
        if(clk.contains("origin_epoch"))
        {
            
        }
        
        if(clk.contains("start_validity_epoch"))
        {
            
        }
    }
    
    
    
    // mho_json::json_pointer clock_pointer("/$CLOCK");
    // auto clocks = vexInfo.at(clock_pointer);
    // if(clocks.size() < 2)
    // {
    //     msg_error("main", "root file contains missing or $CLOCK information." << eom );
    //     std::exit(1);
    // }
    // 
    // 
    // 
    
    
    
}
