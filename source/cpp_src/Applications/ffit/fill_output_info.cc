#include "ffit.hh"

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
