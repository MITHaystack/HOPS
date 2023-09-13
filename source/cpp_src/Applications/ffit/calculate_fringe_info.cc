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
    #pragma message("TODO FIXME -- need to accommodate stations with non-2bit sampling")
    double amp_corr_factor = 1.0;
    double fact1 = 1.0; //more than 16 lags
    double fact2 = 0.881; //2bit x 2bit
    double fact3 = 0.970; //difx
    double whitneys = 1e4; //unit conversion to 'Whitneys'
    double inv_sigma = fact1 * fact2 * fact3 * std::sqrt(ap_period/samp_period);
    double snr = amp * inv_sigma *  sqrt(total_ap_frac * effective_npol)/(whitneys * amp_corr_factor);
    return snr;
}

double calculate_mbd_no_ion_error(double freq_spread, double snr)
{
    double mbd_err = (1.0 / (2.0 * M_PI * freq_spread * snr) );
    return mbd_err;
}

double calculate_sbd_error(double sbd_sep, double snr, double sbavg)
{
    /* get proper weighting for sbd error estimate */
    // status->sbavg = 0.0;
    // for (fr = 0; fr < pass->nfreq; fr++)
    // for (ap = pass->ap_off; ap < pass->ap_off + pass->num_ap; ap++)
    // status->sbavg += pass->pass_data[fr].data[ap].sband;
    // status->sbavg /= status->total_ap;

    double sbd_err = (std::sqrt(12.0) * sbd_sep * 4.0) / (2.0 * M_PI * snr * (2.0 - std::fabs(sbavg)) ) ;
    return sbd_err;
}

//this is the fourfit original version (unweighted integration time - may under estimate the error)
double calculate_drate_error_v1(double snr, double ref_freq, double total_nap, double ap_delta)
{
    //originally: temp = status->total_ap * param->acc_period / pass->channels;
    //but we don't need the number of channels due to the difference in the way
    //we count 'APs' vs original c-code.
    double temp = total_nap*ap_delta;
    double drate_error = std::sqrt(12.0) / ( 2.0 * M_PI * snr * ref_freq * temp) ;
    return drate_error;
}


//probably makes more sense to use the integration time to compute this uncertainty,
//but original fourfit version (_v1) just uses the total (unweighted) time
double calculate_drate_error_v2(double snr, double ref_freq, double integration_time)
{
    double temp = integration_time;
    double drate_error = std::sqrt(12.0) / ( 2.0 * M_PI * snr * ref_freq * temp) ;
    return drate_error;
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

double calculate_phase_error(double sbavg, double snr)
{
    //no ionosphere
    double sband_err = std::sqrt (1.0 + 3.0 * (sbavg * sbavg)); //why?
    double phase_err = 180.0 * sband_err / (M_PI * snr);
    return phase_err;
}

double calculate_phase_delay_error(double sbavg, double snr, double ref_freq)
{
    //no ionosphere
    double sband_err = std::sqrt (1.0 + 3.0 * (sbavg * sbavg));
    double ph_delay_err = sband_err / (2.0 * M_PI * snr * ref_freq);
    return ph_delay_err;
}

std::string calculate_qf()
{
    //dummy
    return std::string("?");
}


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
    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);

    paramStore->Set("/fringe/sbd_separation", sbd_delta);

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

    double coh_avg_phase = std::arg(sum_all);
    return coh_avg_phase; //radians
}

void correct_phases_mbd_anchor_sbd(double ref_freq, double freq0, double frequency_spacing, double delta_mbd, double& totphase_deg, double& resphase_deg)
{
    //see fill_208.c
    double delta_f = std::fmod(ref_freq - freq0, frequency_spacing);
    totphase_deg += 360.0 * delta_mbd * delta_f;
    totphase_deg = std::fmod(totphase_deg, 360.0);
    resphase_deg += 360.0 * delta_mbd * delta_f;
    resphase_deg = std::fmod(resphase_deg, 360.0);
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
    std::string year_doy = leftpadzeros_integer(4, frt_ldate.year) +":" + leftpadzeros_integer(3, frt_ldate.day);

    //figure out the legacy date/time stamps for start, stop, and FRT
    paramStore->Set("/fringe/year_doy", year_doy);
    paramStore->Set("/fringe/legacy_start_timestamp", make_legacy_datetime_format(start_ldate) );
    paramStore->Set("/fringe/legacy_stop_timestamp", make_legacy_datetime_format(stop_ldate) );
    paramStore->Set("/fringe/legacy_frt_timestamp", make_legacy_datetime_format(frt_ldate) );

    //calculate SNR
    #pragma message("TODO FIXME -- properly calcualte the effective number of pol-products.")
    double eff_npol = 1.0;
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

    //residual phase in radians and degrees
    double resid_phase_rad = calculate_residual_phase(conStore, paramStore);
    double resid_phase_deg = std::fmod(resid_phase_rad*(180.0/M_PI), 360.0);

    //calculate the a priori phase and total phase
    double aphase = std::fmod( ref_freq*adelay*360.0, 360.0); //from fill_208.c, no conversion from radians??
    double tot_phase_deg = std::fmod( aphase + resid_phase_rad*(180.0/M_PI), 360.0 );
    paramStore->Set("/fringe/aphase", aphase);

    //calculate the probability of false detection, THIS IS BROKEN
    #pragma message("TODO FIXME - PFD calculation needs the MBD/SBD/DR windows defined")
    double pfd = calculate_pfd(snr, total_npts_searched);
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
        correct_phases_mbd_anchor_sbd(ref_freq, freq0, freq_spacing, delta_mbd, tot_phase_deg, resid_phase_deg);
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
    double mbd_error = calculate_mbd_no_ion_error(freq_spread, snr);

    #pragma message("TODO FIXME, calculate SBAVG properly")
    double sbavg = 1.0;

    double sbd_error = calculate_sbd_error(sbd_sep, snr, sbavg);

    int total_naps = paramStore->GetAs<int>("total_naps");
    double drate_error = calculate_drate_error_v1(snr, ref_freq, total_naps, ap_delta);
    //may want to consider using this version in the future
    //double drate_error = calculate_drate_error_v2(snr, ref_freq, integration_time);

    paramStore->Set("/fringe/mbd_error", mbd_error);
    paramStore->Set("/fringe/sbd_error", sbd_error);
    paramStore->Set("/fringe/drate_error", drate_error);

    double ph_err = calculate_phase_error(sbavg, snr);
    double phdelay_err = calculate_phase_delay_error(sbavg, snr, ref_freq);

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
