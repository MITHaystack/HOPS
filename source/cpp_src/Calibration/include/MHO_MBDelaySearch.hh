#ifndef MHO_MBDelaySearch_HH__
#define MHO_MBDelaySearch_HH__

#include <cmath>
#include <complex>

#include "MHO_AxisPack.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_CyclicRotator.hh"
#include "MHO_DelayRate.hh"
#include "MHO_ExtremaSearch.hh"
#include "MHO_InspectingOperator.hh"
#include "MHO_TableContainer.hh"
#include "MHO_UniformGridPointsCalculator.hh"

#ifdef HOPS_USE_FFTW3
    #include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#else
    #include "MHO_MultidimensionalFastFourierTransform.hh"
#endif

namespace hops
{

/*!
 *@file MHO_MBDelaySearch.hh
 *@class MHO_MBDelaySearch
 *@author J. Barrett - barrettj@mit.edu
 *@date Tue Apr 11 16:50:11 2023 -0400
 *@brief implements the coarse MBD/SBD/DR search, see search.c
 */

using mbd_axis_pack = MHO_AxisPack< time_axis_type >;
using mbd_type = MHO_TableContainer< visibility_element_type, mbd_axis_pack >;
using mbd_amp_type = MHO_TableContainer< double, mbd_axis_pack >;

/**
 * @brief Class MHO_MBDelaySearch
 */
class MHO_MBDelaySearch: public MHO_InspectingOperator< visibility_type >
{
    public:
        MHO_MBDelaySearch();
        virtual ~MHO_MBDelaySearch();

        /**
         * @brief Setter for weights
         *
         * @param wt_data Pointer to weight_type data
         */
        void SetWeights(weight_type* wt_data) { fWeights = wt_data; }

        /**
         * @brief Setter for reference frequency
         *
         * @param ref_freq New reference frequency value in MHz
         */
        void SetReferenceFrequency(double ref_freq) { fRefFreq = ref_freq; }

        //configure the search windows (using floating point limits)
        //default is the full range
        /**
         * @brief Setter for sbd window
         *
         * @param low Lower limit of the SBD window
         * @param high Upper limit of the SBD window
         */
        void SetSBDWindow(double low, double high);
        /**
         * @brief Setter for mbd window
         *
         * @param low Lower bound of MBD window
         * @param high Upper bound of MBD window
         */
        void SetMBDWindow(double low, double high);
        /**
         * @brief Setter for dr window
         *
         * @param low Lower bound of the delay rate window.
         * @param high Upper bound of the delay rate window.
         */
        void SetDRWindow(double low, double high);

        //retrieve the window limits (that were actually used)
        /**
         * @brief Getter for sbd window
         *
         * @param low Reference to store the lower limit of the SBD window
         * @param high Reference to store the upper limit of the SBD window
         */
        void GetSBDWindow(double& low, double& high) const;
        /**
         * @brief Getter for mbd window
         *
         * @param low Reference to store the lower bound of the MBD window.
         * @param high Reference to store the higher bound of the MBD window.
         */
        void GetMBDWindow(double& low, double& high) const;
        /**
         * @brief Getter for dr window
         *
         * @param low Reference to store the lower bound of the delay-rate window.
         * @param high Reference to store the higher bound of the delay-rate window.
         */
        void GetDRWindow(double& low, double& high) const;

        /**
         * @brief Getter for mbdmax bin
         *
         * @return Current location of the bin containing the multi-band delay maximum as an integer
         */
        int GetMBDMaxBin() const { return fMBDMaxBin; }

        /**
         * @brief Getter for sbdmax bin
         *
         * @return Current location of the bin containing the single band delay maximum as an integer
         */
        int GetSBDMaxBin() const { return fSBDMaxBin; }

        /**
         * @brief Getter for drmax bin
         *
         * @return Current location of the bin containing the delay rate maximum as an integer
         */
        int GetDRMaxBin() const { return fDRMaxBin; }

        /**
         * @brief Getter for coarse mbd
         *
         * @return The current coarse MBD value as a double.
         */
        double GetCoarseMBD() const { return fCoarseMBD; }

        /**
         * @brief Getter for coarse sbd
         *
         * @return The current coarse sbd value as a double.
         */
        double GetCoarseSBD() const { return fCoarseSBD; }

        /**
         * @brief Getter for coarse dr
         *
         * @return Current coarse delay rate as a double
         */
        double GetCoarseDR() const { return fCoarseDR; }

        /**
         * @brief Getter for sbd bin separation
         *
         * @return Current SBDBin separation as a double.
         */
        double GetSBDBinSeparation() const { return fSBDBinSep; }

        /**
         * @brief Getter for N mbd bins
         *
         * @return Number of grid points (MBD)
         */
        int GetNMBDBins() const { return fNGridPoints; };

        /**
         * @brief Getter for n sbd bins
         *
         * @return N SBD bins value as int
         */
        int GetNSBDBins() const { return fNSBD; };

        /**
         * @brief Getter for ndrbins
         *
         * @return The number of delay rate (DR) bins as an integer.
         */
        int GetNDRBins() const { return fNDR; };

        int GetNDRSPBins() const { return fDelayRateCalc.GetDelayRateSearchSpaceSize(); }

        double GetNPointsSearched() const;

        double GetSearchMaximumAmplitude() const { return fMax; }

        double GetFrequencySpacing() const { return fGridSpace; }

        double GetAverageFrequency() const { return fAverageFreq; }

        time_axis_type* GetMBDAxis() { return &fMBDAxis; };

        delay_rate_axis_type* GetDRAxis() { return &fDRAxis; };

    protected:
        using XArgType = visibility_type;

        virtual bool InitializeImpl(const XArgType* in) override;
        virtual bool ExecuteImpl(const XArgType* in) override;

        // std::vector< double > DetermineFrequencyPoints(const XArgType* in);

    protected:
        void SetWindow(double* win, double low, double high);
        void GetWindow(const MHO_Axis< double >& axis, bool win_set, const double* win, double bin_width, double& low,
                       double& high) const;

        //workspace
        bool fInitialized;
        std::vector< double > fChannelFreqs;

        visibility_type fSBDDrWorkspace;
        visibility_type sbd_dr_data;
        weight_type fWeightsWorkspace;
        weight_type* fWeights;

        mbd_type fMBDWorkspace;
        mbd_amp_type fMBDAmpWorkspace;

        //dims and parameters
        double fGridStart;
        double fGridSpace;
        double fAverageFreq;
        std::size_t fNGridPoints;
        std::size_t fNSBD;
        std::size_t fNDR;
        std::size_t fNDRSP;
        std::map< std::size_t, std::size_t > fMBDBinMap;
        double fRefFreq;
        int fSBDStart;
        int fSBDStop;

        //the bin width in each dimension
        double fSBDBinSep;
        double fDRBinSep;
        double fMBDBinSep;

        //the window limits
        double fSBDWin[2];
        double fMBDWin[2];
        double fDRWin[2];

        //indicates if window limits have been set
        bool fSBDWinSet;
        bool fMBDWinSet;
        bool fDRWinSet;

        //location and value of the maximum
        double fMax;
        int fMBDMaxBin;
        int fSBDMaxBin;
        int fDRMaxBin;

        double fCoarseMBD;
        double fCoarseSBD;
        double fCoarseDR;

        //the number of points searched
        double fNPointsSearched;

        MHO_Axis< double > fSBDAxis;
        MHO_Axis< double > fMBDAxis;
        MHO_Axis< double > fDRAxis;

        MHO_UniformGridPointsCalculator fGridCalc;
        MHO_DelayRate fDelayRateCalc; //delay rate calculator

        //associated info for channel index re-mapping (needed for combining double sideband channel pairs)
        std::map< std::size_t, std::size_t > fChannelIndexToFreqPointIndex;

#ifdef HOPS_USE_FFTW3
        using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransformFFTW< mbd_type >;
#else
        using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransform< mbd_type >;
#endif

        FFT_ENGINE_TYPE fFFTEngine;

        MHO_CyclicRotator< mbd_type > fCyclicRotator;

        MHO_ExtremaSearch< mbd_amp_type > fMaxSearch;
};

} // namespace hops

#endif /*! end of include guard: MHO_MBDelaySearch_HH__ */
