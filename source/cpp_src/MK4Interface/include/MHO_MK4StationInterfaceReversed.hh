#ifndef MHO_MK4StationInterfaceReversed_HH__
#define MHO_MK4StationInterfaceReversed_HH__

#include <cstdlib>
#include <cstring>

#include <set>
#include <string>
#include <vector>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"

#ifndef HOPS3_USE_CXX
extern "C"
{
#endif

    //forward declare this struct
    struct mk4_sdata;

#ifndef HOPS3_USE_CXX
}
#endif

namespace hops
{

/*!
 *@file MHO_MK4StationInterfaceReversed.hh
 *@class MHO_MK4StationInterfaceReversed
 *@author J. Barrett - barrettj@mit.edu
 *@brief MHO_MK4StationInterfaceReversed - Converts HOPS4 station containers back to mk4_sdata format.
 * This class performs the reverse conversion of MHO_MK4StationInterface, taking MHO_TableContainer objects
 * (station_coord_type and multitone_pcal_type) and generating the corresponding mk4_sdata C structs.
 */

class MHO_MK4StationInterfaceReversed
{
    public:
        MHO_MK4StationInterfaceReversed();
        virtual ~MHO_MK4StationInterfaceReversed();

        //we don't strictly need the vex file data unless we are doing a direct conversion from difx
        //since in order to fill out the type_309s, we need to know the station channel/freq set-up
        void SetVexData(const mho_json& vex_data) { fVexData = vex_data; };

        void SetOutputDirectory(const std::string& output_dir);

        void SetStationCoordData(station_coord_type* coord_data) { fStationCoordData = coord_data; }

        void SetPCalData(multitone_pcal_type* pcal_data) { fPCalData = pcal_data; }

        void SetOutputFile(const std::string& output_file) { fOutputFile = output_file; }

        void GenerateStationStructure();

        int WriteStationFile();

        struct mk4_sdata* GetStationStructure() { return fGeneratedStation; }

        void FreeAllocated();

    private:
        void InitializeStationStructure();
        void GenerateType000();
        std::string ConstructType000FileName();

        void GenerateType300();
        void GenerateType301Records();
        void GenerateType303Records();
        void GenerateType309Records();
        void setstr(const std::string& str, char* char_array, std::size_t max_size);

        double ComputeType309Rot(double ap_offset, std::string start_time, double start_time_mjd);
        double ComputeType309RotFallback(double ap_offset);

        /**
         * @brief Convert multitone PCal phasor back to uint32_t counts
         *
         * @param phasor Complex phasor value
         * @param acc_period Accumulation period in seconds
         * @param sample_period Sample period in seconds
         * @param real_count Output real component count
         * @param imag_count Output imaginary component count
         */
        void ConvertPhasorToCounts(const std::complex< double >& phasor, double acc_period, double sample_period,
                                   uint32_t& real_count, uint32_t& imag_count);

        /**
         * @brief Extract channel information from PCal frequency axis
         */
        void ExtractPCalChannelInfo();
        void ExtractPCalChannelInfoFromVex();
        double DeterminePCalToneSpacing();

        double FactorConvertToMHz(std::string units);
        void DetermineChannelToneIndexes(double lower_freq, double upper_freq, std::size_t& start_idx, std::size_t& ntones);

        std::string GetStationMode();

        station_coord_type* fStationCoordData;
        multitone_pcal_type* fPCalData;
        struct mk4_sdata* fGeneratedStation;
        std::string fOutputDir;
        std::string fOutputFile;

        //copy of vex data
        mho_json fVexData;
        std::string fStationCode;
        std::string fMode;
        std::string fFreqSetupName;

        // Container dimensions
        std::size_t fNCoord;
        std::size_t fNIntervals;
        std::size_t fNCoeffs;
        std::size_t fNPols;
        std::size_t fNAPs;
        std::size_t fNTones;

        // PCal channel information
        struct PCalChannelInfo
        {
                std::string channel_name;
                std::string polarization;
                std::string net_sideband;
                int tone_start;
                int ntones;
                int accumulator_start_index;
                double sky_freq;
                double bandwidth;
                double sample_period;
        };

        //the ordering operator for channel info, sort by frequency and pol
        class chan_predicate
        {
            public:
                chan_predicate(){};
                virtual ~chan_predicate(){};

                virtual bool operator()(const PCalChannelInfo& a, const PCalChannelInfo& b)
                {
                    if(a.sky_freq == b.sky_freq)
                    {
                        return a.polarization < b.polarization;
                    }
                    return a.sky_freq < b.sky_freq;
                }
        };

        std::vector< PCalChannelInfo > fPCalChannelList;

        std::vector< void* > fAllocated;
};

} // namespace hops

#endif /*! end of include guard: MHO_MK4StationInterfaceReversed */
