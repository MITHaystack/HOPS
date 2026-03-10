#ifndef MHO_SpectralLineFringeSearch_HH__
#define MHO_SpectralLineFringeSearch_HH__

#include <cmath>
#include <complex>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_CyclicRotator.hh"
#include "MHO_InspectingOperator.hh"

#ifdef HOPS_USE_FFTW3
    #include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#else
    #include "MHO_MultidimensionalFastFourierTransform.hh"
#endif

namespace hops
{

/*!
 *@file MHO_SpectralLineFringeSearch.hh
 *@class MHO_SpectralLineFringeSearch
 *@author J. Barrett - barrettj@mit.edu
 *@date
 *@brief Coarse fringe search for spectral line (narrow-band) VLBI data.
 *
 * Instead of the broadband (SBD, MBD, DR) search, this operator works in
 * (channel, intra-channel frequency bin, delay rate) space.  It applies a
 * 1-D FFT along the accumulation-period (time) axis of the weighted
 * visibilities to produce a coherent delay-rate spectrum for every spectral
 * bin, then finds the global amplitude peak.
 *
 * Assumes the polarization-product axis has size 1 (either a single pol
 * product or a pre-summed coherent combination).
 */

/**
 * @brief Class MHO_SpectralLineFringeSearch
 */
class MHO_SpectralLineFringeSearch: public MHO_InspectingOperator< visibility_type >
{
    public:
        MHO_SpectralLineFringeSearch();
        virtual ~MHO_SpectralLineFringeSearch();

        /**
         * @brief Set the weight array (per polprod x channel x AP, freq dim ignored).
         */
        void SetWeights(weight_type* wt_data) { fWeights = wt_data; }

        /**
         * @brief Set the reference frequency in MHz (used to convert fringe rate -> delay rate).
         */
        void SetReferenceFrequency(double ref_freq_mhz) { fRefFreq = ref_freq_mhz; }

        /**
         * @brief Restrict the delay-rate search to [low, high] (units: sec/sec).
         */
        void SetDRWindow(double low, double high);

        /**
         * @brief Retrieve the delay-rate window actually used (sec/sec).
         */
        void GetDRWindow(double& low, double& high) const;

        /**
         * @brief Restrict the frequency search to channels whose sky frequency falls
         *        in [low_mhz, high_mhz].  Both bounds are in MHz.
         */
        void SetFrequencyWindow(double low_mhz, double high_mhz);

        /**
         * @brief Retrieve the sky-frequency search window (MHz).
         */
        void GetFrequencyWindow(double& low_mhz, double& high_mhz) const;

        /** @brief Channel index of the coarse peak (-1 if not found). */
        int GetChanMaxBin() const { return fChanMaxBin; }

        /** @brief Delay-rate bin index of the coarse peak (-1 if not found). */
        int GetDRMaxBin() const { return fDRMaxBin; }

        /** @brief Intra-channel frequency bin index of the coarse peak (-1 if not found). */
        int GetFreqMaxBin() const { return fFreqMaxBin; }

        /** @brief Delay rate (sec/sec) at the coarse peak. */
        double GetCoarseDR() const { return fCoarseDR; }

        /**
         * @brief Sky frequency (MHz) at the coarse peak channel.
         *        This is the CHANNEL_AXIS value for the peak channel.
         */
        double GetCoarsePeakSkyFrequencyMHz() const { return fCoarsePeakSkyFreqMHz; }

        /** @brief Raw (un-normalised) amplitude at the search maximum. */
        double GetSearchMaximumAmplitude() const { return fMax; }

        /** @brief Number of delay-rate bins after zero-padding. */
        int GetNDRBins() const { return static_cast< int >(fNDR); }

        /** @brief Delay-rate bin spacing (sec/sec). */
        double GetDRBinSeparation() const { return fDRBinSep; }

        /**
         * @brief Pointer to the delay-rate axis (size = GetNDRBins()).
         *        Values are delay rates in sec/sec, centred on zero.
         */
        delay_rate_axis_type* GetDRAxis() { return &fDRAxis; }

        /**
         * @brief Pointer to the internal (channel x DR x freq) workspace.
         *        Valid after Execute(); used by MHO_InterpolateSpectralLinePeak.
         */
        visibility_type* GetSpecDRData() { return &fSpecDRWorkspace; }

    protected:
        using XArgType = visibility_type;

        virtual bool InitializeImpl(const XArgType* in) override;
        virtual bool ExecuteImpl(const XArgType* in) override;

    private:
        void SetWindow(double* win, double low, double high);

        weight_type* fWeights;
        double fRefFreq; // MHz

        bool fDRWinSet;
        bool fFreqWinSet;
        double fDRWin[2];   // sec/sec
        double fFreqWin[2]; // MHz

        bool fInitialized;

        std::size_t fNAP;       // original number of accumulation periods
        std::size_t fNPaddedAP; // zero-padded size (next power of 2 >= 2*N_AP)
        std::size_t fNChan;
        std::size_t fNFreq;
        std::size_t fNDR; // = fNPaddedAP

        double fDRBinSep; // sec/sec

        int fChanMaxBin;
        int fDRMaxBin;
        int fFreqMaxBin;

        double fCoarseDR;
        double fCoarsePeakSkyFreqMHz;
        double fMax;

        delay_rate_axis_type fDRAxis;

        // Internal workspace: [1][N_chan][N_padded_AP][N_freq]
        // Filled with weighted visibilities, then transformed in-place by the FFT engine.
        visibility_type fSpecDRWorkspace;

#ifdef HOPS_USE_FFTW3
        using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransformFFTW< visibility_type >;
#else
        using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransform< visibility_type >;
#endif

        FFT_ENGINE_TYPE fFFTEngine;
        MHO_CyclicRotator< visibility_type > fCyclicRotator;
};

} // namespace hops

#endif /*! end of include guard: MHO_SpectralLineFringeSearch_HH__ */
