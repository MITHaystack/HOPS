#ifndef MHO_InterpolateSpectralLinePeak_HH__
#define MHO_InterpolateSpectralLinePeak_HH__

#include <cmath>
#include <complex>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_Operator.hh"

namespace hops
{

/*!
 *@file MHO_InterpolateSpectralLinePeak.hh
 *@class MHO_InterpolateSpectralLinePeak
 *@author J. Barrett - barrettj@mit.edu
 *@date
 *@brief Fine interpolation around the coarse fringe peak for spectral-line data.
 *
 * Operates on the (channel x delay-rate x freq) amplitude surface produced by
 * MHO_SpectralLineFringeSearch.  Applies independent parabolic interpolation in
 * the delay-rate and intra-channel frequency dimensions to locate the sub-bin
 * amplitude maximum, then extracts the fringe phase and computes the phase delay.
 *
 * Outputs:
 *   - delay rate (sec/sec)
 *   - fringe rate (Hz) = delay_rate x ref_freq
 *   - fringe amplitude (normalised by total_summed_weights)
 *   - fringe phase (radians) at the peak
 *   - phase delay (seconds) = fringe_phase / (2pi x nu_peak_Hz)
 *   - peak sky frequency (MHz) - the CHANNEL_AXIS value for the peak channel
 */

/**
 * @brief Class MHO_InterpolateSpectralLinePeak
 */
class MHO_InterpolateSpectralLinePeak: public MHO_Operator
{
    public:
        MHO_InterpolateSpectralLinePeak();
        virtual ~MHO_InterpolateSpectralLinePeak(){};

        /**
         * @brief Set the delay-rate / frequency workspace produced by MHO_SpectralLineFringeSearch.
         *        Layout: [1][channel][DR_bin][freq_bin].
         */
        void SetSpecDRData(const visibility_type* spec_dr_data) { fSpecDRData = spec_dr_data; }

        /**
         * @brief Set the weight array (needed for the total_summed_weights tag).
         */
        void SetWeights(const weight_type* weights) { fWeights = weights; }

        /**
         * @brief Set the coarse peak bin indices obtained from MHO_SpectralLineFringeSearch.
         */
        void SetMaxBins(int peak_chan, int peak_dr_bin, int peak_freq_bin);

        /**
         * @brief Copy the delay-rate axis from the search operator.
         */
        void SetDRAxis(const delay_rate_axis_type* dr_ax) { fDRAxis.Copy(*dr_ax); }

        /**
         * @brief Reference frequency in MHz (used to compute fringe rate from delay rate).
         */
        void SetReferenceFrequency(double ref_freq_mhz) { fRefFreqMHz = ref_freq_mhz; }

        virtual bool Initialize() override;
        virtual bool Execute() override;

        /** @brief Sky frequency of the peak channel (MHz), refined by intra-channel interpolation. */
        double GetPeakSkyFrequencyMHz() const { return fPeakSkyFreqMHz; }

        /** @brief Fine-interpolated delay rate (sec/sec). */
        double GetDelayRate() const { return fDelayRate; }

        /** @brief Fine-interpolated fringe rate (Hz) = delay_rate x ref_freq. */
        double GetFringeRate() const { return fFringeRate; }

        /** @brief Fringe amplitude normalised by total_summed_weights. */
        double GetFringeAmplitude() const { return fFringeAmp; }

        /** @brief Fringe phase at the peak (radians). */
        double GetFringePhase() const { return fFringePhase; }

        /**
         * @brief Phase delay at the spectral-line frequency (seconds).
         *        tau_phase = fringe_phase / (2pi x nu_peak_Hz).
         *        NOTE: this is the phase delay, NOT the group delay (which is
         *        undefined for a spectrally narrow source).
         */
        double GetPhaseDelay() const { return fPhaseDelay; }

    private:
        const visibility_type* fSpecDRData;
        const weight_type* fWeights;

        delay_rate_axis_type fDRAxis;
        double fRefFreqMHz;
        double fTotalSummedWeights;

        int fPeakChan;
        int fPeakDRBin;
        int fPeakFreqBin;

        double fPeakSkyFreqMHz;
        double fDelayRate;
        double fFringeRate;
        double fFringeAmp;
        double fFringePhase;
        double fPhaseDelay;

        /**
         * @brief 3-point parabolic sub-bin offset.
         *        Given amplitudes at (k-1, k, k+1), returns the fractional offset [-0.5, 0.5]
         *        of the interpolated maximum from bin k.
         *        Returns 0 if the denominator is zero or the curvature is wrong sign.
         */
        double parabolic_offset(double fm1, double f0, double fp1) const;
};

} // namespace hops

#endif /*! end of include guard: MHO_InterpolateSpectralLinePeak_HH__ */
