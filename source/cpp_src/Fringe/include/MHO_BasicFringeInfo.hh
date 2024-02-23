#ifndef MHO_BasicFringeInfo_HH__
#define MHO_BasicFringeInfo_HH__

/*!
*@file MHO_BasicFringeInfo.hh
*@class MHO_BasicFringeInfo
*@author J. Barrettj - barrettj@mit.edu
*@date Tue Sep 19 04:11:24 PM EDT 2023
*@brief collection of very simple static helper functions
* used when computing fringe information/parameters
*/

#include <string>
#include <cmath>
#include <complex>
#include <sstream>
#include <iomanip>

#include "MHO_Message.hh"
#include "MHO_Clock.hh"

namespace hops
{

class MHO_BasicFringeInfo
{

    public:
        MHO_BasicFringeInfo(){};
        virtual ~MHO_BasicFringeInfo(){};

    public:

        //helper functions
        static std::string leftpadzeros_integer(unsigned int n_places, int value);
        static std::string make_legacy_datetime_format(legacy_hops_date ldate);
        static double calculate_snr(double effective_npol, double ap_period, double samp_period, double total_ap_frac, double amp);
        static double calculate_mbd_no_ion_error(double freq_spread, double snr);
        static double calculate_sbd_error(double sbd_sep, double snr, double sbavg);
        static double calculate_drate_error_v1(double snr, double ref_freq, double total_nap, double ap_delta);
        static double calculate_drate_error_v2(double snr, double ref_freq, double integration_time);
        static double calculate_pfd(double snr, double pts_searched);
        static double calculate_phase_error(double sbavg, double snr);
        static double calculate_phase_delay_error(double sbavg, double snr, double ref_freq);

        static double calculate_theory_timerms_phase(double nseg, double snr);
        static double calculate_theory_timerms_amp(double nseg, double snr);
        static double calculate_theory_freqrms_phase(double nchan, double snr);
        static double calculate_theory_freqrms_amp(double nchan, double snr);

        static void correct_phases_mbd_anchor_sbd(double ref_freq, double freq0, double frequency_spacing, double delta_mbd, double& totphase_deg, double& resphase_deg);

};

}//end namespace

#endif /*! end of include guard: MHO_BasicFringeInfo_HH__ */
