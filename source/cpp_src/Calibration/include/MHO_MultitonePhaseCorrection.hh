#ifndef MHO_MultitonePhaseCorrection_HH__
#define MHO_MultitonePhaseCorrection_HH__

/*
*File: MHO_MultitonePhaseCorrection.hh
*Class: MHO_MultitonePhaseCorrection
*Author:
*Email:
*Date:
*Description:
*/

#include <cmath>
#include <complex>
#include <vector>
#include <map>
#include <cctype>

#include "MHO_Message.hh"
#include "MHO_Constants.hh"

#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_UnaryOperator.hh"



namespace hops
{


class MHO_MultitonePhaseCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:

        MHO_MultitonePhaseCorrection();
        virtual ~MHO_MultitonePhaseCorrection();

        void SetStation(std::string station){fStationCode = station;}; //2-char station code
        void SetStationMk4ID(std::string station_id){fMk4ID = station_id;} //1-char mk4id

         //channel label -> pc_phases
         void SetMultitonePCData(multitone_pcal_type* pcal);

    protected:

        virtual bool InitializeInPlace(visibility_type* in) override;
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        virtual bool ExecuteInPlace(visibility_type* in) override;
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:

        std::size_t DetermineStationIndex(const visibility_type* in);
        bool PolMatch(std::size_t station_idx, std::string& pc_pol, std::string& polprod);
        void DetermineChannelFrequencyLimits(double sky_freq, double bandwidth, std::string net_sideband, double& lower_freq, double& upper_freq);
        void DetermineChannelToneIndexes(double lower_freq, double upper_freq, std::size_t& lower_idx, std::size_t& upper_idx);

        //fit a mean pcal offset and delay from this set of tones
        //for a particular pol and ap index (uses pcalibrate.c algo)
        void FitPCData(std::size_t pol_idx, std::size_t ap_idx, std::size_t tone_start_idx, std::size_t ntones);

        //constants
        std::complex<double> fImagUnit;
        double fDegToRad;
        double fNanoSecToSecond;
        double fMHzToHz;
        double fPi;

        //selection
        std::string fStationCode;
        std::string fMk4ID;

        //the multi-tone pcal data 
        multitone_pcal_type* fPCData;

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

        //minor helper function to make sure all strings are compared as upper-case only
        void make_upper(std::string& s){ for(char& c : s){c = toupper(c); };
    }


};


}


#endif /* end of include guard: MHO_MultitonePhaseCorrection */
