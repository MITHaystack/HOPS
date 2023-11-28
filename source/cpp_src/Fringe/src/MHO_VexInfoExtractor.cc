#include "MHO_VexInfoExtractor.hh"

namespace hops 
{

void 
MHO_VexInfoExtractor::extract_clock_early(const mho_json& clk, 
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
MHO_VexInfoExtractor::extract_clock_model(const mho_json& vexInfo, MHO_ParameterStore* paramStore)
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
        msg_error("fringe", "root file missing $CLOCK information for reference station." << eom );
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
        msg_error("fringe", "root file missing $CLOCK information for remote station."<< eom );
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
MHO_VexInfoExtractor::extract_sampler_bits(const mho_json& vexInfo, MHO_ParameterStore* paramStore)
{
    // std::string ref_id = paramStore->GetAs<std::string>("/ref_station/site_id");
    // std::string rem_id = paramStore->GetAs<std::string>("/rem_station/site_id");
    // 
    // ["$MODE"]["XXX"]["$TRACKS"][0]["qualifiers"]
    // .$MODE["1mmlcp"].$TRACKS[0].qualifiers
}

void 
MHO_VexInfoExtractor::extract_vex_info(const mho_json& vexInfo, MHO_ParameterStore* paramStore)
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
        msg_error("fringe", "root file contains missing or ambiguous $SITE information." << eom );
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
        msg_error("fringe", "root file contains missing or $STATION information." << eom );
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
        msg_error("fringe", "root file contains missing or ambiguous $SCHED information." << eom );
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
    
    //extract the number of bits per sample for each station 
    paramStore->Set("/ref_station/mk4id", ref_station_mk4id);
    paramStore->Set("/rem_station/mk4id", rem_station_mk4id);
    
    //extract the number of bits/sample for each station 
    extract_sampler_bits(vexInfo, paramStore);

}


}//end namespace
