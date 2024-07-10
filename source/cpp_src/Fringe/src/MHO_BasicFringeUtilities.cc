#include "MHO_BasicFringeUtilities.hh"

//helper functions
#include "MHO_BasicFringeInfo.hh"

//snapshot utility lib
#include "MHO_Snapshot.hh"

//basic_fringe_search
#include "MHO_NormFX.hh"
#include "MHO_DelayRate.hh"
#include "MHO_MBDelaySearch.hh"

#include "MHO_InterpolateFringePeak.hh"
#include "MHO_UniformGridPointsCalculator.hh"

#include "MHO_LegacyDateConverter.hh"

//construct_plot_data
#include "MHO_ComputePlotData.hh"

namespace hops
{

void
MHO_BasicFringeUtilities::calculate_fringe_solution_info(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore, const mho_json& vexInfo)
{
    //vex section info
    // mho_json freq_section = vexInfo["$FREQ"];
    // auto freq_info = freq_section.begin().value();
    // double sample_rate = freq_info["sample_rate"]["value"];
    // //TODO FIXME (what if channels have multiple-bandwidths?, units?)
    // double samp_period = 1.0/(sample_rate*1e6);

    determine_sample_rate(conStore, paramStore);
    double sample_rate = paramStore->GetAs<double>("/vex/scan/sample_rate/value");
    double samp_period = paramStore->GetAs<double>("/vex/scan/sample_period/value");

    //configuration parameters
    double ref_freq = paramStore->GetAs<double>("/control/config/ref_freq");
    double ap_delta = paramStore->GetAs<double>("/config/ap_period");

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

    //needed for alist output 'scan duration'
    int64_t scan_duration = std::chrono::duration_cast<std::chrono::seconds>(stop_time - start_time).count();
    paramStore->Set("/fringe/scan_duration", scan_duration);

        //get the current time as the time stamp for the 'fourfit' processing time
    auto tnow = hops_clock::now();
    std::string tnow_vex = hops_clock::to_vex_format(tnow);

    //figure out the legacy date/time stamps for start, stop, and FRT
    paramStore->Set("/fringe/year_doy", year_doy);
    paramStore->Set("/fringe/legacy_start_timestamp", MHO_BasicFringeInfo::make_legacy_datetime_format(start_ldate) );
    paramStore->Set("/fringe/legacy_stop_timestamp", MHO_BasicFringeInfo::make_legacy_datetime_format(stop_ldate) );
    paramStore->Set("/fringe/legacy_frt_timestamp", MHO_BasicFringeInfo::make_legacy_datetime_format(frt_ldate) );
    paramStore->Set("/fringe/procdate", tnow_vex );

    legacy_hops_date proc_ldate  = MHO_LegacyDateConverter::ConvertFromVexFormat(tnow_vex);
    std::string procdate_string = MHO_BasicFringeInfo::make_legacy_datetime_format_v2(proc_ldate);
    paramStore->Set("/fringe/legacy_procdate_timestamp", procdate_string );

    std::string corrdate_vex = paramStore->GetAs<std::string>("/config/correlation_date");
    legacy_hops_date corr_ldate  = MHO_LegacyDateConverter::ConvertFromVexFormat(corrdate_vex);
    std::string corrdate_string = MHO_BasicFringeInfo::make_legacy_datetime_format_v2(corr_ldate);
    paramStore->Set("/fringe/legacy_corrdate_timestamp", corrdate_string );

    //TODO FIXME -- add the software build date as a legacy timestamp
    std::string buildtime = paramStore->GetAs<std::string>("/pass/build_time");
    legacy_hops_date build_ldate  = MHO_LegacyDateConverter::ConvertFromISO8601Format(buildtime);
    std::string build_string = MHO_BasicFringeInfo::make_legacy_datetime_format_v2(build_ldate);
    paramStore->Set("/fringe/legacy_build_timestamp", build_string );

    //calculate SNR
    std::vector< std::string > pp_vec = paramStore->GetAs< std::vector< std::string > >("/config/polprod_set");
    double eff_npols = 1.0;
    if(pp_vec.size() > 2 ){eff_npols = 2.0;}

    double bw_corr_factor = calculate_snr_correction_factor(conStore, paramStore); //correction if notches/passband applied
    double snr = MHO_BasicFringeInfo::calculate_snr(eff_npols, ap_delta, samp_period, total_summed_weights, famp, bw_corr_factor);
    paramStore->Set("/fringe/snr", snr);

    //calculate integration time
    double n_polprod;
    weight_type* wt_data = conStore->GetObject<weight_type>(std::string("weight"));
    if( wt_data == nullptr )
    {
        msg_fatal("fringe", "could not find visibility or weight objects with names (weight)." << eom);
        std::exit(1);
    }
    bool ok2 = wt_data->Retrieve("n_summed_polprod", n_polprod);
    if(!ok2){n_polprod = 1.0;}
    int nchan = paramStore->GetAs<int>("/config/nchannels");
    double integration_time =  (total_summed_weights*ap_delta)/( (n_polprod) * (double)nchan );
    paramStore->Set("/fringe/integration_time", integration_time);

    //residual phase in radians and degrees
    double resid_phase_rad = calculate_residual_phase(conStore, paramStore);
    paramStore->Set("/fringe/raw_resid_phase_rad", resid_phase_rad);
    double resid_phase_deg = std::fmod(resid_phase_rad*(180.0/M_PI), 360.0);

    //calculate the a priori phase and total phase
    double aphase = std::fmod( ref_freq*adelay*360.0, 360.0); //from fill_208.c
    double tot_phase_deg = std::fmod( aphase + resid_phase_rad*(180.0/M_PI), 360.0 );
    paramStore->Set("/fringe/aphase", aphase);

    //get total number of points searched
    double total_npts_searched = paramStore->GetAs<double>("/fringe/n_pts_searched");
    //calculate the probability of false detection
    double pfd = MHO_BasicFringeInfo::calculate_pfd(snr, total_npts_searched);
    paramStore->Set("/fringe/prob_false_detect", pfd);

    //TODO FIXME -- -acount for units (these are printed on fringe plot in usec)
    double tot_mbd = adelay + mbdelay;
    double tot_sbd = adelay + sbdelay;

    double ambig = paramStore->GetAs<double>("/fringe/ambiguity");
    double freq_spacing = paramStore->GetAs<double>("/fringe/frequency_spacing");
    std::string mbd_anchor = paramStore->GetAs<std::string>("/control/config/mbd_anchor");

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
    
    //stored for alist
    double total_sbresid = tot_sbd - tot_mbd;
    paramStore->Set("/fringe/total_sbresid", total_sbresid);

    //totals computed at the reference station (instead of geocenter)
    //section is only needed to populate type_208s when exporting to mk4 output
    //this is a candidate for deprecation
    double ref_adelay = paramStore->GetAs<double>("/model/ref_adelay");
    double ref_arate = paramStore->GetAs<double>("/model/ref_arate");
    double ref_station_delay = paramStore->GetAs<double>("/model/ref_station_delay");
    double tot_mbd_ref  = ref_adelay + mbdelay;
    double tot_sbd_ref = ref_adelay + sbdelay;
    if(mbd_anchor == "sbd")
    {
        delta_mbd = ambig * std::floor( (tot_sbd_ref - tot_mbd_ref) / ambig + 0.5);
        tot_mbd_ref += delta_mbd;
    }
    double tot_rate_ref = ref_arate + drate;

    paramStore->Set("/fringe/total_sbdelay_ref", tot_sbd_ref);
    paramStore->Set("/fringe/total_mbdelay_ref", tot_mbd_ref);
    paramStore->Set("/fringe/total_rate_ref", tot_rate_ref);

    //calculate the ref station a priori phase and total phase /////////////////
    /* ref_stn_delay in sec, rate in usec/sec */
    ref_adelay -= ref_station_delay * drate;
    double aphase_ref = std::fmod( ref_freq*ref_adelay*360.0, 360.0); //from fill_208.c
    double tot_phase_ref_deg = std::fmod( aphase_ref + resid_phase_rad*(180.0/M_PI), 360.0 );
    paramStore->Set("/fringe/tot_phase_ref", tot_phase_ref_deg); //not modified by mbd_anchor, see fill_208

    //end of section on ref station totals /////////////////////////////////////

    double sbd_sep = paramStore->GetAs<double>("/fringe/sbd_separation");
    double freq_spread = paramStore->GetAs<double>("/fringe/frequency_spread");
    double mbd_no_ion_error = MHO_BasicFringeInfo::calculate_mbd_no_ion_error(freq_spread, snr);
    double mbd_error = mbd_no_ion_error;
    
    //if we fit for ionosphere dTEC, calculate the covariance mx
    bool do_ion = false;
    paramStore->Get("/config/do_ion", do_ion);
    if(do_ion)
    {
        calculate_ion_covariance(conStore, paramStore); 
        std::vector< double > ion_sigmas;
        paramStore->Get("/fringe/ion_sigmas", ion_sigmas);
        mbd_error = 1e-3 * ion_sigmas[0]; //convert ns to us
        msg_debug("fringe", "mbd sigma w/ no ionosphere "<<mbd_no_ion_error<<" with ion " << mbd_error << eom);
        
        //set the dtec error 
        paramStore->Set("/fringe/dtec_error", ion_sigmas[2]);
    }

    #pragma message("TODO FIXME, calculate SBAVG properly")
    double sbavg = 1.0;

    double sbd_error = MHO_BasicFringeInfo::calculate_sbd_error(sbd_sep, snr, sbavg);

    int total_naps = paramStore->GetAs<int>("/config/total_naps");
    double drate_error = MHO_BasicFringeInfo::calculate_drate_error_v1(snr, ref_freq, total_naps, ap_delta);
    //may want to consider using this version in the future
    //double drate_error = calculate_drate_error_v2(snr, ref_freq, integration_time);

    paramStore->Set("/fringe/mbd_error", mbd_error);
    paramStore->Set("/fringe/mbd_no_ion_error", mbd_no_ion_error);
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
    
    //calculate the (U,V) coordinates
    //TODO FIXME...move all constants to MHO_Constants
    double speed_of_light_Mm = 299.792458; // in mega-meters
    double radians_to_arcsec = 4.848137e-6;
    double lambda = speed_of_light_Mm / ref_freq; // wavelength (m)

    double ref_u = paramStore->GetAs<double>("/ref_station/u");
    double ref_v = paramStore->GetAs<double>("/ref_station/v");
    double rem_u = paramStore->GetAs<double>("/rem_station/u");
    double rem_v = paramStore->GetAs<double>("/rem_station/v");

    double du = radians_to_arcsec * (rem_u - ref_u) / lambda;
    double dv = radians_to_arcsec * (rem_v - ref_v) / lambda;
    paramStore->Set("/fringe/du", du);
    paramStore->Set("/fringe/dv", dv);
    
    //needed by alist -- residual delay corrected by mbd_anchor=sbd
    double alist_resid_delay = mbdelay + ambig * std::floor( ((sbdelay - mbdelay)/ambig) + 0.5);

    paramStore->Set("/fringe/resid_delay", alist_resid_delay);
    

}






double
MHO_BasicFringeUtilities::calculate_residual_phase(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
{
    double total_summed_weights = paramStore->GetAs<double>("/fringe/total_summed_weights");
    double ref_freq = paramStore->GetAs<double>("/control/config/ref_freq");
    double mbd = paramStore->GetAs<double>("/fringe/mbdelay");
    double drate = paramStore->GetAs<double>("/fringe/drate");
    double sbd = paramStore->GetAs<double>("/fringe/sbdelay");
    double sbd_max_bin = paramStore->GetAs<double>("/fringe/max_sbd_bin");
    double frt_offset = paramStore->GetAs<double>("/config/frt_offset");
    double ap_delta =  paramStore->GetAs<double>("/config/ap_period");

    auto weights = conStore->GetObject<weight_type>(std::string("weight"));
    auto sbd_arr = conStore->GetObject<visibility_type>(std::string("sbd"));

    std::size_t POLPROD = 0;
    std::size_t nchan = sbd_arr->GetDimension(CHANNEL_AXIS);
    std::size_t nap = sbd_arr->GetDimension(TIME_AXIS);

    //now we are going to loop over all of the channels/AP
    //and perform the weighted sum of the data at the max-SBD bin
    //with the fitted delay-rate rotation applied
    auto sbd_ax = &( std::get<FREQ_AXIS>(*sbd_arr) );
    auto chan_ax = std::get<CHANNEL_AXIS>(*sbd_arr);
    auto ap_ax = &(std::get<TIME_AXIS>(*sbd_arr));

    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);
    paramStore->Set("/fringe/sbd_separation", sbd_delta);

    MHO_FringeRotation frot;
    frot.SetSBDSeparation(sbd_delta);
    frot.SetSBDMaxBin(sbd_max_bin);
    frot.SetNSBDBins(sbd_ax->GetSize()/4);  //this is nlags, FACTOR OF 4 is because sbd space is padded by a factor of 4
    frot.SetSBDMax( sbd );

    std::complex<double> sum_all = 0.0;
    std::string sidebandlabelkey = "net_sideband";
    for(std::size_t ch=0; ch < nchan; ch++)
    {
        double freq = chan_ax(ch);//sky freq of this channel
        std::string net_sideband = "?";
        bool key_present = chan_ax.RetrieveIndexLabelKeyValue(ch, sidebandlabelkey, net_sideband);
        if(!key_present){msg_error("fringe", "missing net_sideband label for channel "<< ch << "." << eom);}

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

    double coh_avg_phase = std::arg(sum_all);
    return coh_avg_phase; //radians
}


void
MHO_BasicFringeUtilities::determine_sample_rate(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
{
    //checks the actual sample rate in the data, against that which is reported in the VEX file
    //first get the value determined from the VEX file (may not be correct if zoom-bands have been used)
    double sample_rate = paramStore->GetAs<double>("/vex/scan/sample_rate/value");

    //grab visibilities and loop over channel axis
    visibility_type* vis_data = conStore->GetObject<visibility_type>(std::string("vis"));
    if( vis_data != nullptr )
    {
        auto chan_ax = &(std::get<CHANNEL_AXIS>(*vis_data));
        std::set<double> bw_set;
        for(std::size_t i=0; i<chan_ax->GetSize(); i++)
        {
            double bw;
            bool ok = chan_ax->RetrieveIndexLabelKeyValue(i, "bandwidth", bw);
            if(ok){bw_set.insert(bw);}
        }

        double bandwidth = *(bw_set.rbegin());//should only be one value, just use the last
        if(bw_set.size() != 1)
        {
            msg_warn("fringe", "multiple channel bandwidths detected when determining sample rate, SNR may not be accurate" << eom );
        }
        bandwidth *= 1e6; //channel labels are given in MHz
        double alt_sample_rate = 2.0*bandwidth; //assume Nyquist

        //sample rate from channel info differs from VEX...
        //so use the channel bandwidth label info to figure out sample_rate/period instead
        if( std::fabs(alt_sample_rate - sample_rate) > 1e-12 )
        {
            sample_rate = alt_sample_rate;
            paramStore->Set("/vex/scan/sample_rate/value", sample_rate);
            paramStore->Set("/vex/scan/sample_period/value", 1.0/sample_rate);
            paramStore->Set("/vex/scan/sample_rate/units", std::string("Hz"));
            paramStore->Set("/vex/scan/sample_period/units", std::string("s"));
        }
    }

}

double
MHO_BasicFringeUtilities::calculate_snr_correction_factor(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
{
    //grab visibilities and weights
    bool ok;
    visibility_type* vis_data = conStore->GetObject<visibility_type>(std::string("vis"));
    weight_type* wt_data = conStore->GetObject<weight_type>(std::string("weight"));
    if( vis_data == nullptr || wt_data == nullptr )
    {
        msg_fatal("fringe", "could not find visibility or weight objects with names (vis, weight)." << eom);
        std::exit(1);
    }

    //grab the channel axes
    auto ap_ax = &(std::get<TIME_AXIS>(*vis_data) );
    auto chan_ax = &(std::get<CHANNEL_AXIS>(*vis_data) );
    auto wchan_ax = &(std::get<CHANNEL_AXIS>(*wt_data) );

    std::size_t nchan = chan_ax->GetSize();
    std::size_t nap = ap_ax->GetSize();

    double net_bw = 0;
    double net_ap = 0;
    for(std::size_t ch=0; ch<nchan; ch++)
    {
        double ap_weight = nap;
        double frac;
        double factor;
        std::string ubf_key = "used_bandwidth_fraction";
        std::string rf_key = "rescaling_factor";
        bool frac_present = chan_ax->RetrieveIndexLabelKeyValue(ch, ubf_key, frac);
        bool factor_present = wchan_ax->RetrieveIndexLabelKeyValue(ch, rf_key, factor);
        if(!frac_present){frac = 1.0;}

        net_bw += ap_weight*frac;
        net_ap += ap_weight;
    }

    double bw_corr = 1.0;
    if(net_ap > 0){bw_corr = std::sqrt(net_bw/net_ap); }
    msg_debug("fringe", "bandwidth correction factor (passband/notches) is: "<< bw_corr << eom );
    
    //correct for number of summed pol-products
    double n_polprod;
    bool ok2 = wt_data->Retrieve("n_summed_polprod", n_polprod);
    if(!ok2){n_polprod = 1.0;}
    bw_corr *= 1.0/std::sqrt(n_polprod);

    return bw_corr;
}

void 
MHO_BasicFringeUtilities::calculate_ion_covariance(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
{
    double total_summed_weights = paramStore->GetAs<double>("/fringe/total_summed_weights");
    double ref_freq = paramStore->GetAs<double>("/control/config/ref_freq");
    double mbd = paramStore->GetAs<double>("/fringe/mbdelay");
    double drate = paramStore->GetAs<double>("/fringe/drate");
    double sbd = paramStore->GetAs<double>("/fringe/sbdelay");
    double sbd_max_bin = paramStore->GetAs<double>("/fringe/max_sbd_bin");
    double frt_offset = paramStore->GetAs<double>("/config/frt_offset");
    double ap_delta =  paramStore->GetAs<double>("/config/ap_period");

    auto weights = conStore->GetObject<weight_type>(std::string("weight"));
    auto sbd_arr = conStore->GetObject<visibility_type>(std::string("sbd"));

    std::size_t POLPROD = 0;
    std::size_t nchan = sbd_arr->GetDimension(CHANNEL_AXIS);
    std::size_t nap = sbd_arr->GetDimension(TIME_AXIS);

    //now we are going to loop over all of the channels/AP
    //and perform the weighted sum of the data at the max-SBD bin
    //with the fitted delay-rate rotation applied
    auto sbd_ax = &( std::get<FREQ_AXIS>(*sbd_arr) );
    auto chan_ax = std::get<CHANNEL_AXIS>(*sbd_arr);
    auto ap_ax = &(std::get<TIME_AXIS>(*sbd_arr));

    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);
    paramStore->Set("/fringe/sbd_separation", sbd_delta);

    MHO_FringeRotation frot;
    frot.SetSBDSeparation(sbd_delta);
    frot.SetSBDMaxBin(sbd_max_bin);
    frot.SetNSBDBins(sbd_ax->GetSize()/4);  //this is nlags, FACTOR OF 4 is because sbd space is padded by a factor of 4
    frot.SetSBDMax( sbd );

    std::complex<double> sum_all = 0.0;
    std::vector< std::complex<double> > chan_phasors; chan_phasors.resize(nchan, 0);
    std::vector< double > chan_freqs; chan_freqs.resize(nchan, 0);
    
    std::string sidebandlabelkey = "net_sideband";
    for(std::size_t ch=0; ch < nchan; ch++)
    {
        double freq = chan_ax(ch);//sky freq of this channel
        chan_freqs[ch] = freq;

        std::string net_sideband = "?";
        bool key_present = chan_ax.RetrieveIndexLabelKeyValue(ch, sidebandlabelkey, net_sideband);
        if(!key_present){msg_error("fringe", "missing net_sideband label for channel "<< ch << "." << eom);}

        frot.SetSideband(0); //DSB
        if(net_sideband == "U")
        {
            frot.SetSideband(1);
        }

        if(net_sideband == "L")
        {
            frot.SetSideband(-1);
        }

        std::complex<double> ch_sum = 0.0;
        double sumwt = 0.0;
        for(std::size_t ap=0; ap < nap; ap++)
        {
            double tdelta = (ap_ax->at(ap) + ap_delta/2.0) - frt_offset; //need time difference from the f.r.t?
            std::complex<double> vis = (*sbd_arr)(POLPROD, ch, ap, sbd_max_bin); //pick out data at SBD max bin
            std::complex<double> vr = frot.vrot(tdelta, freq, ref_freq, drate, mbd);
            std::complex<double> z = vis*vr;
            //apply weight and sum
            double w = (*weights)(POLPROD, ch, ap, 0);
            sumwt += w;
            std::complex<double> wght_phsr = z*w;
            if(net_sideband == "U")
            {
                sum_all += -1.0*wght_phsr;
                ch_sum += -1.0*wght_phsr;
            }
            else
            {
                sum_all += wght_phsr;
                ch_sum += wght_phsr;
            }
        }
        chan_phasors[ch] = ch_sum;
        
        //divide out summed AP weights
        double c = 0.0;
        if(sumwt > 0){c = 1.0/sumwt;} //TODO replace 1.0 with amp_corr_fact
        chan_phasors[ch] *= c;
    }

    double famp = paramStore->GetAs<double>("/fringe/famp");
    double snr = paramStore->GetAs<double>("/fringe/snr");
    
    //only used by MHO_IonosphericFringeFitter...for computing the ionosphere dTEC covariance
    std::vector< double > ion_sigmas;
    MHO_BasicFringeInfo::ion_covariance(nchan, famp, snr, ref_freq, chan_freqs, chan_phasors, ion_sigmas);
    paramStore->Set("/fringe/ion_sigmas", ion_sigmas);

    //scale the ion_sigmas to (ps, deg, and dTEC units)
    ion_sigmas[0] *= 1e3;
    ion_sigmas[1] *= 360.0;
    paramStore->Set("/fringe/scaled_ion_sigmas", ion_sigmas);

}


}//end namespace
