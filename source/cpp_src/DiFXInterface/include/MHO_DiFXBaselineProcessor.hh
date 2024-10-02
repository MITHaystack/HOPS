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

class MHO_DiFXBaselineProcessor
{
    public:
        MHO_DiFXBaselineProcessor();
        virtual ~MHO_DiFXBaselineProcessor();

        //needed for processing!
        void SetDiFXInputData(const mho_json* input) { fInput = input; }

        int GetBaselineID() const { return fBaselineID; };

        void SetRootCode(std::string rcode) { fRootCode = rcode; }

        void SetCorrelationDate(std::string corrdate) { fCorrDate = corrdate; }

        //apply mk4 style visibility normalization
        void SetRescaleFalse() { fRescale = false; }

        void SetRescaleTrue() { fRescale = true; }

        void AddRecord(MHO_DiFXVisibilityRecord* record);

        void SetStationCodes(MHO_StationCodeMap* code_map);

        void SetDiFXCodes2VexCodes(const std::map< std::string, std::string >& d2v_map) { fDiFX2VexStationCodes = d2v_map; };

        void SetDiFXCodes2VexNames(const std::map< std::string, std::string >& d2v_map) { fDiFX2VexStationNames = d2v_map; };

        void ConstructVisibilityFileObjects();
        void WriteVisibilityObjects(std::string output_dir);

        bool IsAutoCorr() const
        {
            if(fRefStation == fRemStation)
            {
                return true;
            }
            return false;
        };

        std::string GetRefStationMk4Id() const { return fRefStationMk4Id; }

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

    private:
        void Organize();
        void DeleteDiFXVisRecords();

        std::string ConstructCorFileName(const std::string& output_dir, const std::string& root_code,
                                         const std::string& baseline, const std::string& baseline_shortname);

        std::string DetermineFreqGroup(const double& freq);

        std::string fRootCode;
        std::string fCorrDate;
        int fBaselineID;
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

        std::set< std::string > fPolPairSet;
        std::set< int > fFreqIndexSet;
        std::set< int > fAPSet;
        std::set< int > fSpecPointSet;
        std::set< double > fBandwidthSet;
        std::size_t fNPolPairs;
        std::size_t fNChannels;
        std::size_t fNAPs;
        std::size_t fNSpectralPoints;
        bool fCanChannelize;
        bool fHaveBaselineData;

        bool fRescale;
        std::map< int, double > fNBitsToFactor;
        double fScaleFactor;

        //list of channel frequencies for this baseline, sorted in ascending order (freq)
        std::vector< std::pair< int, mho_json > > fBaselineFreqs;
        std::set< std::string > fFreqBandLabelSet;

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

        std::string ConstructMK4ChannelID(std::string fgroup, int index, std::string sideband, char pol);
};

} // namespace hops

#endif /*! end of include guard: MHO_DiFXBaselineProcessor */
