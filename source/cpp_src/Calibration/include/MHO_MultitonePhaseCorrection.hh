#ifndef MHO_MultitonePhaseCorrection_HH__
#define MHO_MultitonePhaseCorrection_HH__

#include <cctype>
#include <cmath>
#include <complex>
#include <map>
#include <vector>

#include "MHO_Constants.hh"
#include "MHO_Message.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_TableContainer.hh"
#include "MHO_UnaryOperator.hh"

#ifdef HOPS_USE_FFTW3
    #include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#else
    #include "MHO_MultidimensionalFastFourierTransform.hh"
#endif

#include "MHO_PhaseCalibrationTrim.hh"

namespace hops
{

/*!
 *@file MHO_MultitonePhaseCorrection.hh
 *@class MHO_MultitonePhaseCorrection
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Jan 27 10:36:00 2022 -0500
 *@brief
 */

/**
 * @brief Class MHO_MultitonePhaseCorrection
 */
class MHO_MultitonePhaseCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_MultitonePhaseCorrection();
        virtual ~MHO_MultitonePhaseCorrection();

        /**
         * @brief Setter for station
         * 
         * @param station Two-character station code
         */
        void SetStation(std::string station) { fStationCode = station; }; //2-char station code

        /**
         * @brief Setter for station mk4id
         * 
         * @param station_id New Mk4ID to be set
         */
        void SetStationMk4ID(std::string station_id) { fMk4ID = station_id; } //1-char mk4id

        /**
         * @brief Setter for pcperiod
         * 
         * @param pc_period New AP period in milliseconds
         */
        void SetPCPeriod(std::size_t pc_period) { fPCPeriod = pc_period; }

        /**
         * @brief Setter for multitone pcdata
         * 
         * @param pcal Input pointer to multitone_pcal_type data
         */
        void SetMultitonePCData(multitone_pcal_type* pcal);

        //pass in the data weights (to be applied to the pcal phasors as well?)
        /**
         * @brief Setter for weights
         * 
         * @param w Input pointer to weight_type array.
         */
        void SetWeights(weight_type* w) { fWeights = w; }

    protected:

        /**
         * @brief Initializes visibility_type object in-place and checks if fPCData is nullptr.
         * 
         * @param !in Pointer to visibility_type object to initialize
         * @return True if initialization successful, false otherwise
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(visibility_type* /*!in*/) override 
        {
            if(fPCData != nullptr){return true;}
            return false;
        };

        /**
         * @brief Initializes out-of-place visibility data if fPCData is nullptr.
         * 
         * @param !in Input pointer to const visibility_type
         * @param !out Output pointer to visibility_type
         * @return True if initialization succeeds, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const visibility_type* /*!in*/, visibility_type* /*!out*/) override 
        {
            if(fPCData != nullptr){return true;}
            return false;
        };

        /**
         * @brief Applies phase calibration to visibility data in-place.
         * 
         * @param in Input visibility_type pointer for processing.
         * @return True if execution was successful, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(visibility_type* in) override;
        /**
         * @brief Copies input visibility data and executes in-place correction.
         * 
         * @param in Input visibility data to be copied.
         * @param out (visibility_type*)
         * @return Result of ExecuteInPlace operation on the copied data.
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

        //temporal interpolation of tone phasors
        /**
         * @brief Temporally interpolates phase calibration (Pcal) tone phasors.
         * 
         * @param pcal_minus_visib_toffset Time offset to apply before interpolation
         */
        void InterpolatePCData(double pcal_minus_visib_toffset);

    private:

        using pcal_axis_pack = MHO_AxisPack< frequency_axis_type >;
        using pcal_type = MHO_TableContainer< std::complex< double >, pcal_axis_pack >;

#ifdef HOPS_USE_FFTW3
        using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransformFFTW< pcal_type >;
#else
        using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransform< pcal_type >;
#endif

        /**
         * @brief Initializes FFT engine with workspace and settings for forward DFT.
         */
        void InitializeFFTEngine();
        FFT_ENGINE_TYPE fFFTEngine;

        /**
         * @brief Checks if a visibility_type object is applicable for correction based on Mk4ID and station code.
         * 
         * @param in Input visibility_type object to check applicability
         * @return Boolean indicating whether the input object is applicable for correction
         */
        bool IsApplicable(const visibility_type* in);

        /**
         * @brief Checks if the first character of pc_pol matches the station_idx-th character of polprod after converting both to uppercase.
         * 
         * @param station_idx Index of the character in polprod to compare
         * @param pc_pol Input polarization code string
         * @param polprod Polarization product string
         * @return True if characters match, false otherwise
         */
        bool PolMatch(std::size_t station_idx, std::string& pc_pol, std::string& polprod);
        /**
         * @brief Determines start index and number of tones within frequency range.
         * 
         * @param lower_freq Lower frequency limit in Hz
         * @param upper_freq Upper frequency limit in Hz
         * @param lower_idx (std::size_t&)
         * @param upper_idx (std::size_t&)
         */
        void DetermineChannelToneIndexes(double lower_freq, double upper_freq, std::size_t& lower_idx, std::size_t& upper_idx);

        //applies the station's phase-cal data for the polarization 'pc_pol' to the appropriate pol-product
        void ApplyPCData(std::size_t pc_pol, std::size_t vis_pp, visibility_type* in);

        //fit a mean pcal offset and delay from this set of tones
        void FitPCData(std::size_t ntones, double chan_center_freq, double sampler_delay, double* phase_spline,
                       std::string net_sideband);

        //constants
        std::complex< double > fImagUnit;
        double fDegToRad;
        double fNanoSecToSecond;
        double fMHzToHz;
        double fPi;

        //selection
        std::string fStationCode;
        std::string fMk4ID;
        std::size_t fStationIndex; //0 is reference, 1 is remote, unknown = 2

        //the multi-tone pcal data
        std::size_t fPCPeriod;
        multitone_pcal_type* fPCData;

        //the data weights
        weight_type* fWeights;

        //workspace for delay fit
        std::size_t fWorkspaceSize;
        pcal_type fPCWorkspace;

        //keys for tag retrieval
        std::string fStationKey;
        std::string fRemStationKey;
        std::string fRefStationKey;
        std::string fRemStationMk4IDKey;
        std::string fRefStationMk4IDKey;
        std::string fChannelLabelKey;
        std::string fSidebandLabelKey;
        std::string fBandwidthKey;
        std::string fSkyFreqKey;
        std::string fLowerSideband;
        std::string fUpperSideband;

        //need for pc_tonemask
        std::string fPCToneMaskChannelsKey;
        std::string fPCToneMaskBitmasksKey;
        bool fHavePCToneMask;
        std::string fPCToneMaskChannels;
        std::vector< int > fPCToneMaskBitmasks;

        //needed if there have been time cuts to the visibilities, to trim to the appropriate range
        MHO_PhaseCalibrationTrim fPCalTrimmer;

        //controls if pc delays are applied (no -- if there are no sampler delays)
        bool fApplyPCDelay;

        //minor helper function to make sure all strings are compared as upper-case only
        void make_upper(std::string& s)
        {
            for(char& c : s)
            {
                c = toupper(c);
            };
        }
};

} // namespace hops

#endif /*! end of include guard: MHO_MultitonePhaseCorrection */
