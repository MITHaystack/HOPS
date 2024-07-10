#include "MHO_BasicFringeInfo.hh"
#include "MHO_MathUtilities.hh"

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

std::string
MHO_BasicFringeInfo::make_legacy_datetime_format_v2(legacy_hops_date ldate)
{
    //formats the time as:
    //YYYY:DDD:HHMMSS
    int iyear = (int) ldate.year;
    int iday = (int) ldate.day;
    int isec = (int) ldate.second;
    std::string dt;
    dt = leftpadzeros_integer(4, iyear) + ":" +
         leftpadzeros_integer(3, iday) + ":" + 
         leftpadzeros_integer(2, ldate.hour) + leftpadzeros_integer(2, ldate.minute) + leftpadzeros_integer(2, isec);
    return dt;
}


double
MHO_BasicFringeInfo::calculate_snr(double effective_npol, double ap_period, double samp_period, double total_ap_frac, double amp, double bw_corr_factor)
{
    //Poor imitation of SNR -- needs to be revisited and formalized
    //some hardcoded values used right now
    #pragma message("TODO FIXME -- need to accommodate stations with non-2bit sampling")
    double amp_corr_factor = 1.0;
    double fact1 = 1.0; //more than 16 lags
    double fact2 = 0.881; //2bit x 2bit
    double fact3 = 0.970; //difx
    double whitneys = 1e4; //unit conversion to 'Whitneys'
    double inv_sigma = fact1 * fact2 * fact3 * std::sqrt(ap_period/samp_period);
    double snr = bw_corr_factor * amp * inv_sigma *  sqrt(total_ap_frac * effective_npol)/(whitneys * amp_corr_factor);
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
    // for (fr = 0; fr < nfreq; fr++)
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

    //NOTE: because of the very large power (pts_searched), very small changes to SNR
    //within a certain range can lead to large changes in the PFD
    pfd = 1.0 - (std::pow(1.0 - std::exp(-1.0*snr * snr / 2.0), pts_searched));
    if(pfd < 0.01)
    {
        //approximate when exp term is small
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

double
MHO_BasicFringeInfo::calculate_theory_timerms_phase(double nseg, double snr)
{
    /* Theoretical RMS values */
    /* true_nseg is meant to be effective */
    /* number of segments actually included */
    /* in the fit for switched mode */
    //true_nseg = status.nseg * totap / (pass->num_ap * nfreq);
    double true_nseg = nseg;
    double th_timerms_phase = std::sqrt(true_nseg) * 180. / (M_PI * snr);
    return th_timerms_phase;
}

double
MHO_BasicFringeInfo::calculate_theory_timerms_amp(double nseg, double snr)
{
    double th_timerms_phase = calculate_theory_timerms_phase(nseg, snr);
    double th_timerms_amp = th_timerms_phase * M_PI * 100. / 180.;
    return th_timerms_amp;
}

double
MHO_BasicFringeInfo::calculate_theory_freqrms_phase(double nchan, double snr)
{
    double th_freqrms_phase = std::sqrt(nchan) * 180. / (M_PI * snr);
    return th_freqrms_phase;
}

double
MHO_BasicFringeInfo::calculate_theory_freqrms_amp(double nchan, double snr)
{
    double th_freqrms_phase = calculate_theory_freqrms_phase(nchan, snr);
    double th_freqrms_amp = th_freqrms_phase * M_PI * 100. / 180.;
    return th_freqrms_amp;
}

void
MHO_BasicFringeInfo::correct_phases_mbd_anchor_sbd(double ref_freq, double freq0, double frequency_spacing, double delta_mbd, double& totphase_deg, double& resphase_deg)
{
    //see fill_208.c
    //fixes up the phase when mbd_anchor = 'sbd' is chosen (instead of 'model')
    double delta_f = std::fmod(ref_freq - freq0, frequency_spacing);
    totphase_deg += 360.0 * delta_mbd * delta_f;
    totphase_deg = std::fmod(totphase_deg, 360.0);
    resphase_deg += 360.0 * delta_mbd * delta_f;
    resphase_deg = std::fmod(resphase_deg, 360.0);
}

void
MHO_BasicFringeInfo::ion_covariance(int nfreq, double famp, double snr, double ref_freq, 
                                    const std::vector<double>& chan_freqs,
                                    const std::vector< std::complex<double> >& chan_phasors,
                                    std::vector< double >& ion_sigmas)
{
    int i, j, fr;
    double sigma_fr,
           fk,
           f0,
           w,
           A[3][3],                     // normal equation matrix
           C[3][3];                     // covariance matrix

    const double b = -1.3445;           // for correct TEC units
    ion_sigmas.resize(3,0.0);

    // pre-clear the normal matrix
    for (i=0; i<3; i++)
    {
        for (j=0; j<3; j++)
        {
            A[i][j] = 0.0;
        }
    }
    
    for (fr = 0; fr < nfreq; fr++)
    {
        // increment normal equations
        sigma_fr = std::sqrt ((double)nfreq) * famp / ( 2.0 * M_PI * snr * std::abs(chan_phasors[fr]) );
        // coefficient matrix weight
        w = 1.0 / (sigma_fr * sigma_fr);
        // convenience variables to match rjc memo
        fk = 1e-3 * chan_freqs[fr]; //channel frequency;
        f0 = 1e-3 * ref_freq;     // (GHz)

        A[0][0] += w * (fk - f0) * (fk - f0);
        A[0][1] += w * (fk - f0);
        A[0][2] += w * b * (fk - f0) / fk;
        A[1][1] += w;
        A[1][2] += w * b / fk;
        A[2][2] += w * (b / fk) * (b / fk);
    }
    A[1][0] = A[0][1];                  // fill in rest of symmetric normal matrix
    A[2][0] = A[0][2];
    A[2][1] = A[1][2];
    
    int ecode = MHO_MathUtilities::minvert3(A,C);
    if(ecode)
    {
        msg_error("fringe", "unable to compute ionosphere errors due to singular matrix" << eom);
    }

    // std devs. are sqrt of diag of covariance matrix
    for(i=0; i<3; i++){ ion_sigmas[i] = std::sqrt(C[i][i]); }
        
    // normalize covariance to get correlation matrix
    for(i=0; i<3; i++)
    {
        for(j=0; j<3; j++)
        {
            C[i][j] /= (ion_sigmas[i] * ion_sigmas[j]);
        }
    }
    
    msg_debug("fringe", "ionospheric sigmas: delay "<< 1e3*ion_sigmas[0] <<" (ps), phase "<< 360.0* ion_sigmas[1]<<" (deg), dTEC " << ion_sigmas[2] << eom);
    msg_debug("fringe", "ionosphere correlation matrix: " << eol);
    msg_debug("fringe", C[0][0] <<", "<<  C[0][1] <<", "<<  C[0][2] << eol);
    msg_debug("fringe", C[1][0] <<", "<<  C[1][1] <<", "<<  C[1][2] << eol); 
    msg_debug("fringe", C[2][0] <<", "<<  C[2][1] <<", "<<  C[2][2] << eom);
}



}//end namespace
