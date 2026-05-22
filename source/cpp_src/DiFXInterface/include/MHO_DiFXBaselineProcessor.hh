#ifndef MHO_DiFXBaselineProcessor_HH__
#define MHO_DiFXBaselineProcessor_HH__

#include <map>
#include <set>
#include <string>
#include <vector>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_DiFXVisibilityRecord.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_ObjectTags.hh"
#include "MHO_SkyFreqGrid.hh"
#include "MHO_StationCodeMap.hh"

namespace hops
{

/*!
 *@file  MHO_DiFXBaselineProcessor.hh
 *@class  MHO_DiFXBaselineProcessor
 *@author  J. Barrett - barrettj@mit.edu
 *@date Tue Mar 1 16:44:37 2022 -0500
 *@brief  accumulates visbility records from a single baseline, sorts and re-packs them into a visbility container
 */

/**
 * @brief Class MHO_DiFXBaselineProcessor
 */
class MHO_DiFXBaselineProcessor
{
    public:
        MHO_DiFXBaselineProcessor();
        virtual ~MHO_DiFXBaselineProcessor();

        /**
         * @brief Setter for difx .input data (needed for processing visibilities)
         *
         * @param input Input mho_json object containing DiFX data
         */
        void SetDiFXInputData(const mho_json* input) { fInput = input; }

        /**
         * @brief Setter for scan index
         *
         * @param idx Index value of type std::size_t
         */
        void SetScanIndex(std::size_t idx) { fIndex = idx; }

        /**
         * @brief Getter for (difx) baseline id
         *
         * @return The current baseline ID as an integer.
         */
        int GetBaselineID() const { return fBaselineID; };

        /**
         * @brief Setter for (hops) root code
         *
         * @param rcode New root code as string
         */
        void SetRootCode(std::string rcode) { fRootCode = rcode; }

        /**
         * @brief Setter for correlation date
         *
         * @param corrdate New correlation date as string
         */
        void SetCorrelationDate(std::string corrdate) { fCorrDate = corrdate; }

        /**
         * @brief Setter for rescale false - Do NOT apply mk4 style visibility normalization
         */
        void SetRescaleFalse() { fRescale = false; }

        /**
         * @brief Setter for rescale true - apply mk4 style visibility normalization
         */
        void SetRescaleTrue() { fRescale = true; }

        /**
         * @brief Setter for attach difx .input true (attaches json object containing difx .input info to visibilities)
         */
        void SetAttachDiFXInputTrue() { fAttachDiFXInput = true; }

        /**
         * @brief Setter for attach difx input false (do not attach difx .input info to visibilities)
         */
        void SetAttachDiFXInputFalse() { fAttachDiFXInput = false; }

        void SetExportAsMark4True() { fExportAsMark4 = true; }

        void SetExportAsMark4False() { fExportAsMark4 = false; }

        /**
         * @brief Adds a visibility record (chunk of difx data) to the processor if it matches baseline and selection criteria.
         *
         * @param record Input MHO_DiFXVisibilityRecord to be added
         */
        void AddRecord(MHO_DiFXVisibilityRecord* record);

        /**
         * @brief Setter for station codes (2 characater -> 1 character)
         *
         * @param code_map Pointer to an MHO_StationCodeMap object containing station codes.
         */
        void SetStationCodes(MHO_StationCodeMap* code_map);

        /**
         * @brief Setter for difx station codes to vex codes (2 char -> 2 char), but difx uses all caps
         *
         * @param d2v_map Input map of DiFX to Vex station codes
         */
        void SetDiFXCodes2VexCodes(const std::map< std::string, std::string >& d2v_map) { fDiFX2VexStationCodes = d2v_map; };

        /**
         * @brief Setter for difx codes to vex names (difx 2 char code to canonical station names (8 char))
         *
         * @param d2v_map Input map of DiFX codes to Vex names
         */
        void SetDiFXCodes2VexNames(const std::map< std::string, std::string >& d2v_map) { fDiFX2VexStationNames = d2v_map; };

        /**
         * @brief Constructs visibility file objects by organizing data and setting tags for visibilities and weights.
         */
        void ConstructVisibilityFileObjects();
        /**
         * @brief Writes visibility objects in HOPS4 format to an output directory.
         *
         * @param output_dir Output directory path for writing files.
         */
        void WriteVisibilityObjects(std::string output_dir);

        /**
         * @brief Checks if reference station is equal to remote station.
         *
         * @return True if stations are equal, false otherwise.
         */
        bool IsAutoCorr() const
        {
            if(fRefStation == fRemStation)
            {
                return true;
            }
            return false;
        };

        /**
         * @brief Getter for reference station mk4id
         *
         * @return Reference station Mk4 ID as a std::string.
         */
        std::string GetRefStationMk4Id() const { return fRefStationMk4Id; }

        /**
         * @brief Getter for remote station mk4id
         *
         * @return The Mk4 ID of the remote station as a string.
         */
        std::string GetRemStationMk4Id() const { return fRemStationMk4Id; }

        std::string GetBaselineShortName() const { return fBaselineShortName; }

        visibility_store_type* GetVisibilities() { return fV; }

        void Clear();

        void SetFrequencyBands(std::vector< std::tuple< std::string, double, double > > fbands) { fFreqBands = fbands; }

        void SetFreqGroups(std::vector< std::string > fgroups) { fOnlyFreqGroups = fgroups; }

        void SetOnlyBandwidth(double bw)
        {
            fOnlyBandwidth = bw;
            fSelectByBandwidth = true;
        }

        void SetZoomFreqIndices(const std::set< int >& zoom_indices) { fZoomFreqIndices = zoom_indices; }

        /**
         * @brief Provides a precomputed global sky-frequency grid (MHz, sorted ascending,
         * deduplicated over freq) used to number channel ids for mark4 t101 chan_id strings. When set,
         * the chidx embedded in ref_chan_id / rem_chan_id (and the mk4_channel_id label)
         * is the position of the channel's sky_freq in this grid, instead of its dense
         * position within this baseline's fBaselineFreqs. This keeps the mark4 chan_ids
         * aligned with MHO_DiFXChannelNameConstructor's chan_def.channel_name across
         * heterogeneous-bandwidth station combinations.
         *
         * @param grid_MHz Sorted, deduped sky frequencies in MHz
         * @param tol Match tolerance in MHz (default 0.001)
         */
        void SetGlobalSkyFreqGrid(const std::vector< double >& grid_MHz, double tol = MHO_SkyFreqGrid::DEFAULT_TOL_MHZ)
        {
            fGlobalGrid = MHO_SkyFreqGrid(grid_MHz, tol);
            fHasGlobalGrid = true;
        }

        /**
         * @brief Read-only access to the ordered (ascending sky_freq) list of channels this
         * baseline will export. Populated by Organize(). Each element is (difx_freq_idx, fInput["freq"][...]).
         */
        const std::vector< std::pair< int, mho_json > >& GetBaselineFreqs() const { return fBaselineFreqs; }

        /**
         * @brief Build the per-baseline derived state (fFreqIndexSet -> fBaselineFreqs, AP counts,
         * scan times, source name, etc). Idempotent. Exposed publicly so MHO_DiFXScanProcessor can
         * collect each baseline's sky_freq list before computing the scan-wide global grid that
         * has to be injected back into both this processor and the channel-name constructor.
         */
        void Organize();

    private:
        void DeleteDiFXVisRecords();

        std::string ConstructCorFileName(const std::string& output_dir, const std::string& root_code,
                                         const std::string& baseline, const std::string& baseline_shortname);

        std::string DetermineFreqGroup(double freq);

        std::string fRootCode;
        std::string fCorrDate;
        int fBaselineID;
        std::size_t fIndex;
        std::string fRefStation;
        std::string fRemStation;
        std::string fRefStationName;
        std::string fRemStationName;
        std::string fRefStationMk4Id;
        std::string fRemStationMk4Id;
        std::string fBaselineName;
        std::string fBaselineShortName;
        std::string fBaselineDelim;
        int fRefStationBits;
        int fRemStationBits;
        double fAPLength;

        //the station 2-char to 1-char code map (user specified)
        MHO_StationCodeMap* fStationCodeMap;

        //the station 2-char to 2-char code map (to deal with DiFX capitalization issue)
        std::map< std::string, std::string > fDiFX2VexStationCodes;
        //the difx station 2-char to vex station name map
        std::map< std::string, std::string > fDiFX2VexStationNames;

        const mho_json* fInput;
        std::vector< MHO_DiFXVisibilityRecord* > fRecords;
        //for a single baseline, maps pol-pair, then freqindex to visiblity records
        //needed to recorganized the visibilities into tables
        std::map< std::string, std::map< int, std::vector< MHO_DiFXVisibilityRecord* > > > fVisibilities;

        std::set< int > fZoomFreqIndices; //when non-empty, discard native-band records keeping root/vis consistent

        std::set< std::string > fPolPairSet;
        std::set< int > fFreqIndexSet;
        std::set< int > fAPSet;
        std::set< int > fSpecPointSet;
        std::set< double > fBandwidthSet;
        std::size_t fNPolPairs;
        std::size_t fNChannels;
        std::size_t fNAPs;
        std::size_t fNSpectralPoints;
        double fStartMJD; //scan start time in MJD
        std::string fStartTime;
        std::string fStopTime;
        std::string fSourceName;
        bool fCanChannelize;
        bool fHaveBaselineData;

        bool fRescale;
        std::map< int, double > fNBitsToFactor;
        double fScaleFactor;

        bool fAttachDiFXInput;
        bool fExportAsMark4;

        //list of channel frequencies for this baseline, sorted in ascending order (freq)
        std::vector< std::pair< int, mho_json > > fBaselineFreqs;
        std::set< std::string > fFreqBandLabelSet;

        //scan-wide ordered sky-freq grid used to number mark4 chan_ids consistently across
        //all baselines and stations. set by MHO_DiFXScanProcessor once visibility records
        //have been read and each baseline organized (so the union of sky_freqs is known).
        MHO_SkyFreqGrid fGlobalGrid;
        bool fHasGlobalGrid;

        //the baseline data in hops data containers
        weight_store_type* fW;
        visibility_store_type* fV;
        MHO_ObjectTags fTags;

        //selection information
        std::vector< std::tuple< std::string, double, double > > fFreqBands; //frequency band/group labels and ranges
        std::vector< std::string > fOnlyFreqGroups;                          //limit output to matching frequency groups
        bool fSelectByBandwidth;
        double fOnlyBandwidth; //limit output to only channels of this bandwidth

        //comparison predicate for time-sorting visibility record data
        struct VisRecordTimeLess
        {
                bool operator()(const MHO_DiFXVisibilityRecord* a, const MHO_DiFXVisibilityRecord* b) const
                {
                    if(a->mjd == b->mjd)
                    {
                        return a->seconds < b->seconds;
                    }
                    else
                    {
                        return a->mjd < b->mjd;
                    }
                }
        };

        VisRecordTimeLess fTimePredicate;

        //comparison predicate for sorting index-frequency record pairs (use center-frequency)
        struct FreqIndexPairLess
        {
                bool operator()(const std::pair< int, mho_json >& a, const std::pair< int, mho_json >& b) const
                {
                    double a_freq = a.second["freq"];
                    double b_freq = b.second["freq"];

                    double a_bw = a.second["bw"];
                    double b_bw = b.second["bw"];

                    std::string a_sb = a.second["sideband"];
                    std::string b_sb = b.second["sideband"];
                    double a_sign = 0;
                    double b_sign = 0;
                    if(a_sb == "U")
                    {
                        a_sign = 1.0;
                    }
                    if(a_sb == "L")
                    {
                        a_sign = -1.0;
                    }
                    if(b_sb == "U")
                    {
                        b_sign = 1.0;
                    }
                    if(b_sb == "L")
                    {
                        b_sign = -1.0;
                    }

                    //figure out the channel center frequencies and compare using that
                    double a_center = a_freq + a_sign * (a_bw / 2.0);
                    double b_center = b_freq + b_sign * (b_bw / 2.0);

                    return a_center < b_center;
                }
        };

        FreqIndexPairLess fFreqPredicate;

};

} // namespace hops

#endif /*! end of include guard: MHO_DiFXBaselineProcessor */
