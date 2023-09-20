#include "MHO_BasicFringeInfo.hh"

namespace hops 
{

std::string 
MHO_BasicFringeInfo::leftpadzeros_integer(unsigned int n_places, int value)
{
    std::stringstream ss;
    ss << std::setw(n_places);
    ss << std::setfill('0');
    ss << value;
    return ss.str();
}

std::string 
MHO_BasicFringeInfo::make_legacy_datetime_format(legacy_hops_date ldate)
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


double 
MHO_BasicFringeInfo::calculate_snr(double effective_npol, double ap_period, double samp_period, double total_ap_frac, double amp)
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

double 
MHO_BasicFringeInfo::calculate_mbd_no_ion_error(double freq_spread, double snr)
{
    double mbd_err = (1.0 / (2.0 * M_PI * freq_spread * snr) );
    return mbd_err;
}


double 
MHO_BasicFringeInfo::calculate_sbd_error(double sbd_sep, double snr, double sbavg)
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


double 
MHO_BasicFringeInfo::calculate_drate_error_v1(double snr, double ref_freq, double total_nap, double ap_delta)
{
    //originally: temp = status->total_ap * param->acc_period / pass->channels;
    //but we don't need the number of channels due to the difference in the way
    //we count 'APs' vs original c-code.
    double temp = total_nap*ap_delta;
    double drate_error = std::sqrt(12.0) / ( 2.0 * M_PI * snr * ref_freq * temp) ;
    return drate_error;
}


double 
MHO_BasicFringeInfo::calculate_drate_error_v2(double snr, double ref_freq, double integration_time)
{
    double temp = integration_time;
    double drate_error = std::sqrt(12.0) / ( 2.0 * M_PI * snr * ref_freq * temp) ;
    return drate_error;
}

double 
MHO_BasicFringeInfo::calculate_pfd(double snr, double pts_searched)
{
    double a = 1.0 - std::exp(-1.0*(snr*snr)/ 2.0);
    double pfd =  1.0 - std::pow(a, pts_searched);
    if(pfd < 0.01)
    {
        pfd = pts_searched * std::exp(-1.0*(snr*snr)/ 2.0);
    }
    return pfd;
}

double 
MHO_BasicFringeInfo::calculate_phase_error(double sbavg, double snr)
{
    //no ionosphere
    double sband_err = std::sqrt (1.0 + 3.0 * (sbavg * sbavg)); //why?
    double phase_err = 180.0 * sband_err / (M_PI * snr);
    return phase_err;
}

double 
MHO_BasicFringeInfo::calculate_phase_delay_error(double sbavg, double snr, double ref_freq)
{
    //no ionosphere
    double sband_err = std::sqrt (1.0 + 3.0 * (sbavg * sbavg));
    double ph_delay_err = sband_err / (2.0 * M_PI * snr * ref_freq);
    return ph_delay_err;
}


std::string 
MHO_BasicFringeInfo::calculate_qf()
{
    //dummy
    #pragma message("TODO FIXME, implement quality code calculation.")
    return std::string("?");
}

void correct_phases_mbd_anchor_sbd(double ref_freq, double freq0, double frequency_spacing, double delta_mbd, double& totphase_deg, double& resphase_deg)
{
    //see fill_208.c
    //fixes up the phase when mbd_anchor = 'sbd' is chosen (instead of 'model')
    double delta_f = std::fmod(ref_freq - freq0, frequency_spacing);
    totphase_deg += 360.0 * delta_mbd * delta_f;
    totphase_deg = std::fmod(totphase_deg, 360.0);
    resphase_deg += 360.0 * delta_mbd * delta_f;
    resphase_deg = std::fmod(resphase_deg, 360.0);
}


}//end namespace
