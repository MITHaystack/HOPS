#include "ffit.hh"

#include <sstream>
#include <iomanip>
#include <cmath>

#include "MHO_Clock.hh"
#include "MHO_FringeRotation.hh"

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

std::string calculate_qf()
{
    //dummy 
    return std::string("?");
}

// double calculate_residual_phase()
// {
//     return 0.0;
// }


double
calculate_residual_phase(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
{
    double total_summed_weights = paramStore->GetAs<double>("/fringe/total_summed_weights");
    double ref_freq = paramStore->GetAs<double>("ref_freq");
    double mbd = paramStore->GetAs<double>("/fringe/mbdelay");
    double drate = paramStore->GetAs<double>("/fringe/drate");
    double sbd = paramStore->GetAs<double>("/fringe/sbdelay");
    double sbd_max_bin = paramStore->GetAs<double>("/fringe/max_sbd_bin");
    double frt_offset = paramStore->GetAs<double>("frt_offset");
    double ap_delta =  paramStore->GetAs<double>("ap_period");

    auto weights = conStore->GetObject<weight_type>(std::string("weight"));
    auto sbd_arr = conStore->GetObject<visibility_type>(std::string("sbd"));
    
    std::size_t POLPROD = 0;
    std::size_t nchan = sbd_arr->GetDimension(CHANNEL_AXIS);
    std::size_t nap = sbd_arr->GetDimension(TIME_AXIS);

    //now we are going to loop over all of the channels/AP
    //and perform the weighted sum of the data at the max-SBD bin
    //with the fitted delay-rate rotation (but mbd=0) applied
    auto chan_ax = &( std::get<CHANNEL_AXIS>(*sbd_arr) );
    auto ap_ax = &(std::get<TIME_AXIS>(*sbd_arr));
    auto sbd_ax = &( std::get<FREQ_AXIS>(*sbd_arr) );
    // double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);
    
    MHO_FringeRotation frot;
    frot.SetSBDSeparation(sbd_delta);
    frot.SetSBDMaxBin(sbd_max_bin);
    frot.SetNSBDBins(sbd_ax->GetSize()/4);  //this is nlags, FACTOR OF 4 is because sbd space is padded by a factor of 4
    frot.SetSBDMax( sbd );

    std::complex<double> sum_all = 0.0;
    for(std::size_t ch=0; ch < nchan; ch++)
    {
        double freq = (*chan_ax)(ch);//sky freq of this channel
        MHO_IntervalLabel ilabel(ch,ch);
        std::string net_sideband = "?";
        std::string sidebandlabelkey = "net_sideband";
        auto other_labels = chan_ax->GetIntervalsWhichIntersect(&ilabel);
        for(auto olit = other_labels.begin(); olit != other_labels.end(); olit++)
        {
            if( (*olit)->HasKey(sidebandlabelkey) )
            {
                (*olit)->Retrieve(sidebandlabelkey, net_sideband);
                break;
            }
        }

        frot.SetSideband(0); //DSB
        if(net_sideband == "U")
        {
            frot.SetSideband(1);
        }

        if(net_sideband == "L")
        {
            frot.SetSideband(-1);
        }

        for(std::size_t ap=0; ap < nap; ap++)
        {
            double tdelta = (ap_ax->at(ap) + ap_delta/2.0) - frt_offset; //need time difference from the f.r.t?
            std::complex<double> vis = (*sbd_arr)(POLPROD, ch, ap, sbd_max_bin); //pick out data at SBD max bin
            std::complex<double> vr = frot.vrot(tdelta, freq, ref_freq, drate, mbd);
            std::complex<double> z = vis*vr;
            //apply weight and sum
            double w = (*weights)(POLPROD, ch, ap, 0);
            std::complex<double> wght_phsr = z*w;
            if(net_sideband == "U")
            {
                sum_all += -1.0*wght_phsr;
            }
            else 
            {
                sum_all += wght_phsr;
            }
        }
    }

    std::cout<<"sbd sep = "<<sbd_delta<<" sbd max = "<<sbd<<std::endl;
    std::cout<<"sum all = "<<sum_all<<std::endl;

    double coh_avg_phase = std::arg(sum_all);

    return coh_avg_phase; //not quite the value which is displayed in the fringe plot (see fill type 208)
}


void calculate_fringe_info(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore, const mho_json& vexInfo)
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
    double adelay = paramStore->GetAs<double>("/model/adelay");
    double arate = paramStore->GetAs<double>("/model/arate");
    double aaccel = paramStore->GetAs<double>("/model/aaccel");
    

    

    //TODO FIXME -- -acount for units (convert to usec)
    //NEED to acout for units everywhere!
    // adelay *= 1.0e6;
    // arate *= 1.0e6;
    // aaccel *= 1.0e6;

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

    //figure out the legacy date/time stamps for start, stop, and FRT
    paramStore->Set("/fringe/year_doy", year_doy);
    paramStore->Set("/fringe/legacy_start_timestamp", make_legacy_datetime_format(start_ldate) );
    paramStore->Set("/fringe/legacy_stop_timestamp", make_legacy_datetime_format(stop_ldate) );
    paramStore->Set("/fringe/legacy_frt_timestamp", make_legacy_datetime_format(frt_ldate) );

    //calculate SNR
    double eff_npol = 1.0; //TODO FIXME
    double snr = calculate_snr(eff_npol, ap_delta, samp_period, total_summed_weights, famp);
    paramStore->Set("/fringe/snr", snr);

    //calculate integration time
    int nchan = paramStore->GetAs<int>("nchannels");
    double integration_time =  (total_summed_weights*ap_delta)/(double)nchan;
    paramStore->Set("/fringe/integration_time", integration_time);

    //calculate quality code 
    std::string quality_code = calculate_qf();
    paramStore->Set("/fringe/quality_code", quality_code);
    
    //total number of points searched 
    std::size_t nmbd = paramStore->GetAs<std::size_t>("/fringe/n_mbd_points");
    std::size_t nsbd = paramStore->GetAs<std::size_t>("/fringe/n_sbd_points");
    std::size_t ndr = paramStore->GetAs<std::size_t>("/fringe/n_dr_points");
    double total_npts_searched = (double)nmbd * (double)nsbd *(double)ndr;

    
    double resid_phase = calculate_residual_phase(conStore, paramStore);
    // //plot_dict["ResPhase"] = std::fmod(coh_avg_phase * (180.0/M_PI), 360.0);
    paramStore->Set("/fringe/resid_phase", resid_phase);

    double resid_ph_delay = resid_phase / (2.0 * M_PI * ref_freq);
    paramStore->Set("/fringe/resid_ph_delay", resid_ph_delay);
    
    double ph_delay = adelay + resid_ph_delay;
    std::cout<<"ph_delay = adelay + resid_ph_delay = "<<ph_delay<<" = "<<adelay<<" + "<<resid_ph_delay<<std::endl;

    paramStore->Set("/fringe/phase_delay", ph_delay);
    
    
    //calculate the a priori phase and total phase
    double aphase = std::fmod( ref_freq*adelay*360.0, 360.0); //from fill_208.c, no conversion from radians??
    double tot_phase = std::fmod( aphase + resid_phase*(180.0/M_PI), 360.0 );

    std::cout<<"APHASE = "<<aphase<<std::endl;
    std::cout<<"RESID PHASE = "<<resid_phase<<std::endl;
    std::cout<<"TOTPHASE = "<<tot_phase<<std::endl;

    paramStore->Set("/fringe/aphase", aphase);
    paramStore->Set("/fringe/tot_phase", tot_phase);

    //calculate the probability of false detection, THIS IS BROKEN
    #pragma message("TODO FIXME - PFD calculation needs the MBD/SBD/DR windows defined")
    double pfd = calculate_pfd(snr, total_npts_searched);
    pfd = 0.0;
    paramStore->Set("/fringe/prob_false_detect", pfd);
    std::cout<<"SNR, NPTS, PFD = "<<snr<<", "<<total_npts_searched<<", "<<pfd<<std::endl;

    //TODO FIXME -- -acount for units (these are printed on fringe plot in usec)
    double tot_mbd = adelay + mbdelay;
    double tot_sbd = adelay + sbdelay;

    std::cout<<"mbdelay = "<<mbdelay<<std::endl;
    std::cout<<"adelay = "<<adelay<<std::endl;
    std::cout<<"tot_mbd = "<<tot_mbd<<std::endl;
    std::cout<<"tot_sbd = "<<tot_sbd<<std::endl;

    double ambig = paramStore->GetAs<double>("/fringe/ambiguity");
    std::string mbd_anchor = paramStore->GetAs<std::string>("mbd_anchor");
    // anchor total mbd to sbd if desired
    double delta_mbd = 0.0;
    if(mbd_anchor == "sbd")
    {
        std::cout<<"MBDANCHOR IS SBD!"<<std::endl;
        delta_mbd = ambig * std::floor( (tot_sbd - tot_mbd) / ambig + 0.5);
    }

    tot_mbd += delta_mbd;
    std::cout<<"tot_mbd = "<<tot_mbd<<" and delta mbd = "<<delta_mbd<<std::endl;

    double tot_drate = arate + drate;

    paramStore->Set("/fringe/total_sbdelay", tot_sbd);
    paramStore->Set("/fringe/total_mbdelay", tot_mbd);
    paramStore->Set("/fringe/total_drate", tot_drate);

    //now calculate the errors (ionosphere fitting not yet included)
    //double mbd_error = 1.0 / (2.0 * M_PI * status->freq_spread * snr);
    // t208->mbd_error = (status->nion == 0) ?
    //     (float)(1.0 / (2.0 * M_PI * status->freq_spread * status->snr)) :
    //     1e-3 * status->ion_sigmas[0];
    //     msg ("mbd sigma w/ no ionosphere %f with ion %f ps", 1, 
    //         (double)(1e6 / (2.0 * M_PI * status->freq_spread * status->snr)), 1e3 * status->ion_sigmas[0]);
    //                                     /* get proper weighting for sbd error estimate */

    //             //                                     /* ref. stn. time-tagged observables are
    //             //                                      * approximated by combining retarded a prioris
    //             //                                      * with non-retarded residuals */
    //             // t208->tot_mbd_ref  = adelay_ref * 1e6 + status->mbd_max_global;
    //             // t208->tot_sbd_ref  = adelay_ref * 1e6 + status->sbd_max;
    //             //                                     // anchor ref mbd as above
    //             // if (param->mbd_anchor == SBD)
    //             //     t208->tot_mbd_ref += ambig 
    //             //                        * floor ((t208->tot_sbd_ref - t208->tot_mbd_ref) / ambig + 0.5);
    //             // t208->tot_rate_ref = arate_ref * 1e6 + status->corr_dr_max;
    // 
    // 
    // 
    // 
    //             // 
    //             // t208->resid_mbd = status->mbd_max_global;
    //             // t208->resid_sbd = status->sbd_max;
    //             // t208->resid_rate = status->corr_dr_max;
    //             // t208->mbd_error = (status->nion == 0) ?
    //             //     (float)(1.0 / (2.0 * M_PI * status->freq_spread * status->snr)) :
    //             //     1e-3 * status->ion_sigmas[0];
    //             //     msg ("mbd sigma w/ no ionosphere %f with ion %f ps", 1, 
    //             //         (double)(1e6 / (2.0 * M_PI * status->freq_spread * status->snr)), 1e3 * status->ion_sigmas[0]);
    //             //                                     /* get proper weighting for sbd error estimate */
    //             // status->sbavg = 0.0;
    //             // for (fr = 0; fr < pass->nfreq; fr++)
    //             //     for (ap = pass->ap_off; ap < pass->ap_off + pass->num_ap; ap++) 
    //             //         status->sbavg += pass->pass_data[fr].data[ap].sband;
    //             // status->sbavg /= status->total_ap;
    //             // t208->sbd_error = (float)(sqrt (12.0) * status->sbd_sep * 4.0
    //             //             / (2.0 * M_PI * status->snr * (2.0 - fabs (status->sbavg) )));
    //             // temp = status->total_ap * param->acc_period / pass->channels;
    //             // t208->rate_error = (float)(sqrt(12.0) 
    //             //                     / ( 2.0 * M_PI * status->snr * param->ref_freq * temp));
    //             // 
    //             // t208->ambiguity = 1.0 / status->freq_space;
    //             // t208->amplitude = status->delres_max/10000.;
    //             // t208->inc_seg_ampl = status->inc_avg_amp;
    //             // t208->inc_chan_ampl = status->inc_avg_amp_freq;
    //             // t208->snr = status->snr;
    //             // t208->prob_false = status->prob_false;
    //             // status->apphase = fmod (param->ref_freq * t208->adelay * 360.0, 360.0);
    //             // t208->totphase = fmod (status->apphase + status->coh_avg_phase
    //             //                     * (180.0/M_PI) , 360.0);
    //             //                                     /* Ref stn frame apriori delay usec */
    //             // adelay_ref *= 1.0e6;
    //             //                                     /* ref_stn_delay in sec, rate in usec/sec */
    //             // adelay_ref -= ref_stn_delay * t208->resid_rate;
    //             // apphase_ref = fmod (param->ref_freq * adelay_ref * 360.0, 360.0);
    //             // t208->totphase_ref = fmod (apphase_ref + status->coh_avg_phase
    //             //                     * (180.0/M_PI) , 360.0);
    //             // t208->resphase = fmod (status->coh_avg_phase * (180.0/M_PI), 360.0);
    //             //                                 // adjust phases for mbd ambiguity
    //             // if (param->mbd_anchor == SBD)
    //             //     {
    //             //     delta_f = fmod (param->ref_freq - pass->pass_data[0].frequency, status->freq_space);
    //             //     msg ("delta_mbd %g delta_f %g", 1, delta_mbd, delta_f);
    //             //     t208->totphase += 360.0 * delta_mbd * delta_f;
    //             //     t208->totphase = fmod(t208->totphase, 360.0);
    //             //     t208->resphase += 360.0 * delta_mbd * delta_f;
    //             //     t208->resphase = fmod(t208->resphase, 360.0);
    //             //     }
    //             // 
    //             // msg ("residual phase %f", 1, t208->resphase);
    //             // 
    //             // t208->tec_error = (status->nion) ? status->ion_sigmas[2] : 0.0;
    //             // 
    //             // 
    //             // 
    //             // 
    // 
    // 
    // // dp->param->mbd_anchor == MODEL ? "Model(usec)" : "SBD(usec)  ",
    // plot_dict["GroupDelay"] = paramStore->GetAs<double>("/fringe/total_mbdelay");         // dp->fringe->t208->tot_mbd);
    // plot_dict["SbandDelay(usec)"] = paramStore->GetAs<double>("/fringe/total_sbdelay");   //dp->fringe->t208->tot_sbd);
    // 
    // plot_dict["PhaseDelay(usec)"] = 0;
    //     //dp->fringe->t208->adelay + dp->status->resid_ph_delay);
    // plot_dict["TotalPhase(deg)"] = 0;
    //     //dp->fringe->t208->totphase);
    // 
    //
    double ref_clock_off = paramStore->GetAs<double>("/ref_station/clock_offset_at_frt");
    double rem_clock_off = paramStore->GetAs<double>("/rem_station/clock_offset_at_frt");
    double ref_rate = paramStore->GetAs<double>("/ref_station/clock_rate");
    double rem_rate = paramStore->GetAs<double>("/rem_station/clock_rate");
    
    double clock_offset = rem_clock_off - ref_clock_off;
    double clock_rate =  rem_rate - ref_rate;
    
    paramStore->Set("/fringe/relative_clock_offset", clock_offset); //usec
    paramStore->Set("/fringe/relative_clock_rate", clock_rate*1e6); //usec/s
    
    // plot_dict["AprioriClock(usec)"] = clock_offset;
    //     //dp->fringe->t202->rem_clock - dp->fringe->t202->ref_clock);
    // plot_dict["AprioriClockrate(us/s)"] = clock_rate;;
    //     //(dp->fringe->t202->rem_clockrate - dp->fringe->t202->ref_clockrate));
    // 
    
    
    // 
    // plot_dict["AprioriDelay(usec)"] = paramStore->GetAs<double>("/model/adelay");         //dp->fringe->t208->adelay);
    // plot_dict["AprioriRate(us/s)"] = paramStore->GetAs<double>("/model/arate");         //dp->fringe->t208->arate);
    // plot_dict["AprioriAccel(us/s/s)"] = paramStore->GetAs<double>("/model/aaccel");         //dp->fringe->t208->aaccel);
    // plot_dict["ResidMbdelay(usec)"] = paramStore->GetAs<double>("/fringe/mbdelay");         //dp->fringe->t208->resid_mbd);
    // plot_dict["ResidSbdelay(usec)"] = paramStore->GetAs<double>("/fringe/sbdelay");         //dp->fringe->t208->resid_sbd);
    // 
    // plot_dict["ResidPhdelay(usec)"] = 0;
    //     //dp->status->resid_ph_delay);
    // 
    // plot_dict["ResidRate(us/s)"] = paramStore->GetAs<double>("/fringe/drate");
    //     //dp->fringe->t208->resid_rate);
    // 
    // plot_dict["ResidPhase(deg)"] = 0;
    //     //dp->fringe->t208->resphase);
    // 
    // 
    // 
    // 
    // plot_dict["ResidMbdelayError(usec)"] = 0;
    //     //dp->fringe->t208->mbd_error);
    // plot_dict["ResidSbdelayError(usec)"] = 0;
    //     //dp->fringe->t208->sbd_error);
    // plot_dict["ResidPhdelayError(usec)"] = 0;
    //     //dp->status->ph_delay_err);
    // plot_dict["ResidRateError(us/s)"] = 0;
    //     //dp->fringe->t208->rate_error);
    // plot_dict["ResidPhaseError(deg)"] = 0;
    //     //dp->status->phase_err);
}
