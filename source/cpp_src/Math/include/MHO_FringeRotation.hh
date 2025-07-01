#ifndef MHO_FringeRotation_HH__
#define MHO_FringeRotation_HH__

#include <cmath>
#include <complex>

namespace hops
{

/*!
 *@file MHO_FringeRotation.hh
 *@class MHO_FringeRotation
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Feb 17 13:35:37 2023 -0500
 *@brief class to implement functionality originally found in vrot.c
 */

/**
 * @brief Class MHO_FringeRotation
 */
class MHO_FringeRotation
{
    public:
        MHO_FringeRotation();
        virtual ~MHO_FringeRotation(){};

        //virtual function (can be pointed to different implementations)
        /**
         * @brief Calculates fringe rotation using delay rate and frequency difference.
         * 
         * @param time_delta Time interval between measurements
         * @param freq Frequency of the signal
         * @param ref_freq Reference frequency for comparison
         * @param dr Delay rate, change of delay with time
         * @param mbd Modulation bandwidth
         * @return Complex value representing fringe rotation
         * @note This is a virtual function.
         */
        virtual std::complex< double > vrot(double time_delta, double freq, double ref_freq, double dr, double mbd) const;

        /**
         * @brief Setter for sbdseparation
         * 
         * @param sbd_sep New separation value in time offset between two signals
         */
        void SetSBDSeparation(double sbd_sep) { fSBDSep = sbd_sep; }; //value of the delta between points in SBD space

        /**
         * @brief Setter for sbdmax
         * 
         * @param sbd_max Maximum delay time offset between two signals.
         */
        void SetSBDMax(double sbd_max) { fSBDMax = sbd_max; } //sbd delay at maximum

        /**
         * @brief Setter for sbdmax bin
         * 
         * @param sbd_max_bin Maximum bin value for Single Band Delay
         */
        void SetSBDMaxBin(int sbd_max_bin) { fSBDMaxBin = sbd_max_bin; }

        /**
         * @brief Setter for nsbdbins
         * 
         * @param n_sbd_bins Number of Single Band Delay bins
         */
        void SetNSBDBins(int n_sbd_bins) { fNSBDBins = n_sbd_bins; }

        //pass the sideband information if optimize_closure of single-sideband correction is needed
        /**
         * @brief Setter for sideband
         * 
         * @param sb Sideband type: 1 for USB, 0 for DSB, -1 for LSB
         */
        void SetSideband(int sb) { fSideband = sb; } //1 is USB, 0 if DSB, -1 if LSB

        /**
         * @brief Setter for optimize closure true
         */
        void SetOptimizeClosureTrue() { fOptimizeClosure = true; }

        /**
         * @brief Setter for optimize closure false
         */
        void SetOptimizeClosureFalse() { fOptimizeClosure = false; }

    private:
        static const std::complex< double > fImagUnit;

        /**
         * @brief Calculates and returns a phasor for fringe rotation given time delta, frequencies, delay rate, and modulation bandwidth deviation.
         * 
         * @param time_delta Time interval between two consecutive samples
         * @param freq Frequency of the signal
         * @param ref_freq Reference frequency used in calculations
         * @param dr Delay rate (fringe parameter concerning change of delay with time)
         * @param mbd Modulation bandwidth deviation
         * @return Phasor representing fringe rotation as a complex exponential
         */
        std::complex< double > vrot_v1(double time_delta, double freq, double ref_freq, double dr, double mbd) const;

        /**
         * @brief Calculates sideband correction for fringe rotation due to delay and frequency difference.
         * 
         * @param mbd Multi-Band Delay, input delay parameter
         * @return Sideband correction value as a double
         */
        double calc_sideband_correction(double mbd) const;

        int fSideband;
        int fNSBDBins;
        int fSBDMaxBin;
        double fSBDMax;
        double fSBDSep;
        bool fOptimizeClosure;
};

} // namespace hops

#endif /*! end of include guard: MHO_FringeRotation_HH__ */
