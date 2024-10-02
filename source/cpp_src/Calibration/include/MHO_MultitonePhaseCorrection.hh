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

namespace hops
{

/*!
 *@file MHO_MultitonePhaseCorrection.hh
 *@class MHO_MultitonePhaseCorrection
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Jan 27 10:36:00 2022 -0500
 *@brief
 */

class MHO_MultitonePhaseCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_MultitonePhaseCorrection();
        virtual ~MHO_MultitonePhaseCorrection();

        void SetStation(std::string station) { fStationCode = station; }; //2-char station code

        void SetStationMk4ID(std::string station_id) { fMk4ID = station_id; } //1-char mk4id

        void SetPCPeriod(std::size_t pc_period) { fPCPeriod = pc_period; }

        //channel label -> pc_phases
        void SetMultitonePCData(multitone_pcal_type* pcal) { fPCData = pcal; };

        //pass in the data weights (to be applied to the pcal phasors as well?)
        void SetWeights(weight_type* w) { fWeights = w; }

    protected:
        virtual bool InitializeInPlace(visibility_type* /*!in*/) override { return true; };

        virtual bool InitializeOutOfPlace(const visibility_type* /*!in*/, visibility_type* /*!out*/) override { return true; };

        virtual bool ExecuteInPlace(visibility_type* in) override;
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:
        using pcal_axis_pack = MHO_AxisPack< frequency_axis_type >;
        using pcal_type = MHO_TableContainer< std::complex< double >, pcal_axis_pack >;

#ifdef HOPS_USE_FFTW3
        using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransformFFTW< pcal_type >;
#else
        using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransform< pcal_type >;
#endif

        FFT_ENGINE_TYPE fFFTEngine;

        bool IsApplicable(const visibility_type* in);

        bool PolMatch(std::size_t station_idx, std::string& pc_pol, std::string& polprod);
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
