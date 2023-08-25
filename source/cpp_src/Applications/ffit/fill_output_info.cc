#include "ffit.hh"

#include <sstream>
#include <iomanip>
#include <cmath>

#include "MHO_Clock.hh"

std::string 
leftpadzeros_integer(unsigned int n_places, int value)
{
    std::stringstream ss;
    ss << std::setw(n_places);
    ss << std::setfill('0');
    ss << value;
    return ss.str();
}

std::string
make_legacy_datetime_format(legacy_hops_date ldate)
{
    //formats the time as HHMMSS.xx with no separators (except the '.' for the fractional second)
    int isec = (int) ldate.second;
    float fsec = ldate.second - isec;
    std::string dt;
    dt = leftpadzeros_integer(2, ldate.hour) + leftpadzeros_integer(2, ldate.minute) + leftpadzeros_integer(2, isec);
    int fsec_dummy = (int) (100*fsec);
    dt += "." + leftpadzeros_integer(2, fsec_dummy);
    return dt;
}

double calculate_snr(double effective_npol, double ap_period, double samp_period, double total_ap_frac, double amp)
{
    //Poor imitation of SNR -- needs corrections
    //some hardcoded values used right now
    double amp_corr_factor = 1.0;
    double fact1 = 1.0; //more than 16 lags
    double fact2 = 0.881; //2bit x 2bit
    double fact3 = 0.970; //difx
    double whitneys = 1e4; //unit conversion to 'Whitneys'
    double inv_sigma = fact1 * fact2 * fact3 * std::sqrt(ap_period/samp_period);
    double snr = amp * inv_sigma *  sqrt(total_ap_frac * effective_npol)/(whitneys * amp_corr_factor);
    return snr;
}

double calculate_pfd(double snr, double pts_searched)
{
    double a = 1.0 - std::exp(-1.0*(snr*snr)/ 2.0);
    double pfd =  1.0 - std::pow(a, pts_searched);
    if(pfd < 0.01)
    {
        pfd = pts_searched * std::exp(-1.0*(snr*snr)/ 2.0);
    }
    return pfd;
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
    //TODO FIXME (what if channels have multi-bandwiths?, units?)
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
    double ap_delay = paramStore->GetAs<double>("/model/ap_delay");
    double ap_rate = paramStore->GetAs<double>("/model/ap_rate");
    double ap_accel = paramStore->GetAs<double>("/model/ap_accel");

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
    std::string year_doy = leftpadzeros_integer(4, frt_ldate.year) +":" + leftpadzeros_integer(3, frt_ldate.day);

    
    plot_dict["RA"] = src_info["ra"];
    plot_dict["Dec"] = src_info["dec"];

    plot_dict["Quality"] = "-";
    double eff_npol = 1.0; //TODO FIXME
    double snr = calculate_snr(eff_npol, ap_delta, samp_period, total_summed_weights, famp);
    plot_dict["SNR"] = snr;

    int nchan = paramStore->GetAs<int>("nchannels");
    plot_dict["IntgTime"] = (total_summed_weights*ap_delta)/(double)nchan;
        
    plot_dict["Amp"] = famp;
        
    
    //total number of points searched 
    std::size_t nmbd = paramStore->GetAs<std::size_t>("/fringe/n_mbd_points");
    std::size_t nsbd = paramStore->GetAs<std::size_t>("/fringe/n_sbd_points");
    std::size_t ndr = paramStore->GetAs<std::size_t>("/fringe/n_dr_points");
    double total_npts_searched = (double)nmbd * (double)nsbd *(double)ndr;

    // //plot_dict["ResPhase"] = std::fmod(coh_avg_phase * (180.0/M_PI), 360.0);


    #pragma message("TODO FIXME - PFD calculation needs the MBD/SBD/DR windows defined")
    double pfd = calculate_pfd(snr, total_npts_searched);
    std::cout<<"SNR, NPTS, PFD = "<<snr<<", "<<total_npts_searched<<", "<<pfd<<std::endl;
    plot_dict["PFD"] = 0.0; 


    plot_dict["ResidSbd(us)"] = sbdelay;
    plot_dict["ResidMbd(us)"] = mbdelay;
    plot_dict["FringeRate(Hz)"]  = frate;
    plot_dict["IonTEC(TEC)"] = "-";
    plot_dict["RefFreq(MHz)"] = ref_freq;
    plot_dict["AP(sec)"] = ap_delta;
    plot_dict["ExperName"] = exper_info["exper_name"];
    plot_dict["ExperNum"] = "-";
    plot_dict["YearDOY"] = year_doy;
    plot_dict["Start"] = make_legacy_datetime_format(start_ldate);
    plot_dict["Stop"] = make_legacy_datetime_format(stop_ldate);;
    plot_dict["FRT"] = make_legacy_datetime_format(frt_ldate);
    plot_dict["CorrTime"] = "-";
    plot_dict["FFTime"] = "-";
    plot_dict["BuildTime"] = "-";

    plot_dict["GroupDelay"] = 0;
        // dp->param->mbd_anchor == MODEL ? "Model(usec)" : "SBD(usec)  ",
        // dp->fringe->t208->tot_mbd);
    plot_dict["SbandDelay(usec)"] = 0;
        //dp->fringe->t208->tot_sbd);
    plot_dict["PhaseDelay(usec)"] = 0;
        //dp->fringe->t208->adelay + dp->status->resid_ph_delay);
    plot_dict["TotalPhase(deg)"] = 0;
        //dp->fringe->t208->totphase);
    plot_dict["AprioriDelay(usec)"] = paramStore->GetAs<double>("/model/adelay");
        //dp->fringe->t208->adelay);
    plot_dict["AprioriClock(usec)"] = 0;
        //dp->fringe->t202->rem_clock - dp->fringe->t202->ref_clock);
    plot_dict["AprioriClockrate(us/s)"] = 0;
        //(dp->fringe->t202->rem_clockrate - dp->fringe->t202->ref_clockrate));
    plot_dict["AprioriRate(us/s)"] = paramStore->GetAs<double>("/model/arate");
        //dp->fringe->t208->arate);
    plot_dict["AprioriAccel(us/s/s)"] = paramStore->GetAs<double>("/model/aaccel");
        //dp->fringe->t208->aaccel);
    plot_dict["ResidMbdelay(usec)"] = paramStore->GetAs<double>("/fringe/mbdelay");
        //dp->fringe->t208->resid_mbd);
    plot_dict["ResidSbdelay(usec)"] = paramStore->GetAs<double>("/fringe/sbdelay");
        //dp->fringe->t208->resid_sbd);
    plot_dict["ResidPhdelay(usec)"] = 0;
        //dp->status->resid_ph_delay);
    plot_dict["ResidRate(us/s)"] = paramStore->GetAs<double>("/fringe/drate");
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
