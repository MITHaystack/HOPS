#ifndef MHO_BasicFringeInfo_HH__
#define MHO_BasicFringeInfo_HH__

#include <cmath>
#include <complex>
#include <iomanip>
#include <sstream>
#include <string>

#include "MHO_Clock.hh"
#include "MHO_Message.hh"

namespace hops
{

/*!
 *@file MHO_BasicFringeInfo.hh
 *@class MHO_BasicFringeInfo
 *@author J. Barrettj - barrettj@mit.edu
 *@date Wed Sep 20 14:38:33 2023 -0400
 *@brief collection of very simple static helper functions
 * used when computing fringe information/parameters
 */

/**
 * @brief Class MHO_BasicFringeInfo
 */
class MHO_BasicFringeInfo
{

    public:
        MHO_BasicFringeInfo(){};
        virtual ~MHO_BasicFringeInfo(){};

    public:
        //helper functions
        /**
         * @brief Pads an integer with leading zeros up to a specified number of places.
         * 
         * @param n_places Number of places to pad the integer with leading zeros.
         * @param value The integer value to be padded.
         * @return A string representation of the padded integer.
         * @note This is a static function.
         */
        static std::string leftpadzeros_integer(unsigned int n_places, int value);
        /**
         * @brief Formats legacy date/time as HHMMSS.xx for SWIN output.
         * 
         * @param ldate Input legacy_hops_date to format
         * @return Formatted datetime string in SWIN format
         * @note This is a static function.
         */
        static std::string make_legacy_datetime_format(legacy_hops_date ldate);
        /**
         * @brief Converts legacy_hops_date to SWIN format: YYYY:DDD:HHMMSS.
         * 
         * @param ldate Input date in legacy_hops_date format
         * @return Date string in SWIN format
         * @note This is a static function.
         */
        static std::string make_legacy_datetime_format_v2(legacy_hops_date ldate);
        /**
         * @brief Calculates Signal to Noise Ratio (SNR) using given parameters.
         * 
         * @param effective_npol Effective number of polarizations
         * @param ap_period Average period between arrivals
         * @param samp_period Sampling period
         * @param total_ap_frac Fraction of total arrival periods
         * @param amp Amplitude correlation coefficient
         * @param bw_corr_factor Bandwidth correction factor
         * @return Calculated SNR value
         * @note This is a static function.
         */
        static double calculate_snr(double effective_npol, double ap_period, double samp_period, double total_ap_frac,
                                    double amp, double bw_corr_factor);
        /**
         * @brief Calculates Multi-Band Delay error without ionospheric effects.
         * 
         * @param freq_spread Frequency spread in Hz
         * @param snr Signal-to-Noise Ratio (SNR)
         * @return Multi-Band Delay error without ionospheric effects
         * @note This is a static function.
         */
        static double calculate_mbd_no_ion_error(double freq_spread, double snr);
        /**
         * @brief Calculates Single Band Delay (SBD) error using separation, SNR and average SBD.
         * 
         * @param sbd_sep Single Band Delay separation
         * @param snr Signal to Noise Ratio
         * @param sbavg Average Single Band Delay
         * @return Calculated SBD error
         * @note This is a static function.
         */
        static double calculate_sbd_error(double sbd_sep, double snr, double sbavg);
        /**
         * @brief Calculates and returns the double rate error using SNR, reference frequency, total nap, and ap delta.
         * 
         * @param snr Signal to Noise Ratio
         * @param ref_freq Reference frequency in GHz
         * @param total_nap Total number of acquisition periods
         * @param ap_delta Acquisition period delta
         * @return Double rate error calculated using the provided parameters
         * @note This is a static function.
         */
        static double calculate_drate_error_v1(double snr, double ref_freq, double total_nap, double ap_delta);
        /**
         * @brief Calculates and returns the double rate error using SNR, reference frequency, and integration time.
         * 
         * @param snr Signal to Noise Ratio (SNR)
         * @param ref_freq Reference frequency in GHz
         * @param integration_time Integration time
         * @return Double rate error calculated as sqrt(12.0) / (2.0 * M_PI * snr * ref_freq * integration_time)
         * @note This is a static function.
         */
        static double calculate_drate_error_v2(double snr, double ref_freq, double integration_time);
        /**
         * @brief Calculates Probability of False Detection (PFD) given Signal-to-Noise Ratio (SNR) and points searched.
         * 
         * @param snr Signal-to-Noise Ratio
         * @param pts_searched Number of points searched
         * @return Probability of False Detection
         * @note This is a static function.
         */
        static double calculate_pfd(double snr, double pts_searched);
        /**
         * @brief Calculates phase error given signal bandwidth average and signal-to-noise ratio.
         * 
         * @param sbavg Average signal bandwidth
         * @param snr Signal to Noise Ratio
         * @return Phase error in degrees
         * @note This is a static function.
         */
        static double calculate_phase_error(double sbavg, double snr);
        /**
         * @brief Calculates phase delay error given sbavg, snr and reference frequency.
         * 
         * @param sbavg Average signal bandwidth
         * @param snr Signal to Noise Ratio
         * @param ref_freq Reference frequency
         * @return Phase delay error as a double value
         * @note This is a static function.
         */
        static double calculate_phase_delay_error(double sbavg, double snr, double ref_freq);

        /**
         * @brief Calculates theoretical RMS phase for given number of segments and SNR.
         * 
         * @param nseg Effective number of segments actually included in fit for switched mode.
         * @param snr Signal to Noise Ratio.
         * @return Theoretical RMS phase value in degrees.
         * @note This is a static function.
         */
        static double calculate_theory_timerms_phase(double nseg, double snr);
        /**
         * @brief Calculates theoretical time-averaged amplitude using nseg and SNR.
         * 
         * @param nseg Number of segments
         * @param snr Signal-to-Noise Ratio
         * @return Theoretical time-averaged amplitude as a double
         * @note This is a static function.
         */
        static double calculate_theory_timerms_amp(double nseg, double snr);
        /**
         * @brief Calculates theoretical frequency RMS phase using number of channels and signal-to-noise ratio.
         * 
         * @param nchan Number of channels
         * @param snr Signal-to-Noise Ratio
         * @return Theoretical frequency RMS phase as a double value
         * @note This is a static function.
         */
        static double calculate_theory_freqrms_phase(double nchan, double snr);
        /**
         * @brief Calculates theoretical frequency RMS amplitude using given number of channels and signal-to-noise ratio.
         * 
         * @param nchan Number of channels
         * @param snr Signal to Noise Ratio
         * @return Theoretical frequency RMS amplitude as a double value
         * @note This is a static function.
         */
        static double calculate_theory_freqrms_amp(double nchan, double snr);

        /**
         * @brief Corrects phase for MBD anchor SBD using reference frequency and frequency spacing.
         * 
         * @param ref_freq Reference frequency in GHz
         * @param freq0 Initial frequency in GHz
         * @param frequency_spacing Frequency spacing in Hz
         * @param delta_mbd Multi-Band Delay factor
         * @param totphase_deg Total phase degree (output)
         * @param resphase_deg Residual phase degree (output)
         * @note This is a static function.
         */
        static void correct_phases_mbd_anchor_sbd(double ref_freq, double freq0, double frequency_spacing, double delta_mbd,
                                                  double& totphase_deg, double& resphase_deg);

        //only used by MHO_IonosphericFringeFitter...for computing the ionosphere dTEC covariance
        /**
         * @brief Computes ionospheric delay time error covariance for fringe fitting.
         * 
         * @param nfreq Number of frequency channels.
         * @param famp Amplitude factor for ionospheric delay calculation.
         * @param snr Signal-to-noise ratio for ionospheric delay calculation.
         * @param ref_freq Reference frequency in GHz.
         * @param chan_freqs Vector of channel frequencies in GHz.
         * @param chan_phasors Vector of complex phasor values.
         * @param ion_sigmas Output vector of ionospheric delay standard deviations.
         * @note This is a static function.
         */
        static void ion_covariance(int nfreq, double famp, double snr, double ref_freq, const std::vector< double >& chan_freqs,
                                   const std::vector< std::complex< double > >& chan_phasors,
                                   std::vector< double >& ion_sigmas);
};

} // namespace hops

#endif /*! end of include guard: MHO_BasicFringeInfo_HH__ */
