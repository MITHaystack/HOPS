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
 *@brief class to implement the fringe rotation functionality originally found in vrot.c
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
         * @brief Calculates fringe rotation for given delay, delay-rate, time and frequency difference.
         * 
         * @param time_delta Time difference from fourfit reference time (s)
         * @param freq Channel frequency (MHz)
         * @param ref_freq Reference frequency (MHz)
         * @param dr Delay rate ()
         * @param mbd Multi-Band Delay (us)
         * @return Complex value representing fringe rotation
         * @note This is a virtual function.
         */
        virtual std::complex< double > vrot(double time_delta, double freq, double ref_freq, double dr, double mbd) const;

        /**
         * @brief Setter for SBD bin separation
         * 
         * @param sbd_sep value of the (time) delta between two adjacent points in SBD space
         */
        void SetSBDSeparation(double sbd_sep) { fSBDSep = sbd_sep; }; 

        /**
         * @brief Setter for sbd max
         * 
         * @param sbd_max the single-band delay value at the fringe maximum
         */
        void SetSBDMax(double sbd_max) { fSBDMax = sbd_max; }

        /**
         * @brief Setter for sbd max bin
         * 
         * @param sbd_max_bin the bin value for the fringe maximum (Single Band Delay axis)
         */
        void SetSBDMaxBin(int sbd_max_bin) { fSBDMaxBin = sbd_max_bin; }

        /**
         * @brief Setter for N sbd bins
         * 
         * @param n_sbd_bins set the number of Single Band Delay bins
         */
        void SetNSBDBins(int n_sbd_bins) { fNSBDBins = n_sbd_bins; }

        /**
         * @brief Setter for sideband - passes the sideband information if optimize_closure requires single-sideband correction         * 
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
         * @brief Calculates and returns a phasor for fringe rotation given time delta, frequencies, delay rate, and Multi-Band Delay.
         * 
         * @param time_delta Time interval between current AP and fourfit reference time
         * @param freq channel frequency
         * @param ref_freq Fringe reference frequency
         * @param dr Delay rate
         * @param mbd Multi-Band Delay
         * @return Phasor representing fringe rotation as a complex exponential
         */
        std::complex< double > vrot_v1(double time_delta, double freq, double ref_freq, double dr, double mbd) const;

        /**
         * @brief Calculates sideband correction for fringe rotation (if optimize_closure is true)
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
