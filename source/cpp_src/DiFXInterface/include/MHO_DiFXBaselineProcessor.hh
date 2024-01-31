#ifndef MHO_DiFXBaselineProcessor_HH__
#define MHO_DiFXBaselineProcessor_HH__

/*
*@file: MHO_DiFXBaselineProcessor.hh
*@class: MHO_DiFXBaselineProcessor
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief: accumulates visbility records from a single baseline, sorts and re-packs them into a visbility container
*/

#include <vector>
#include <string>
#include <set>
#include <map>

#include "MHO_DiFXVisibilityRecord.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ObjectTags.hh"
#include "MHO_StationCodeMap.hh"
#include "MHO_JSONHeaderWrapper.hh"




namespace hops
{

class MHO_DiFXBaselineProcessor
{
    public:
        MHO_DiFXBaselineProcessor();
        virtual ~MHO_DiFXBaselineProcessor();

        int GetBaselineID() const {return fBaselineID;};
        void SetDiFXInputData(const mho_json* input){fInput = input;}
        void SetRootCode(std::string rcode){fRootCode = rcode;}
        void SetCorrelationDate(std::string corrdate){fCorrDate = corrdate;}

        //apply mk4 style visibility normalization
        void SetRescaleFalse(){fRescale = false;}
        void SetRescaleTrue(){fRescale = true;}

        void AddRecord(MHO_DiFXVisibilityRecord* record);

        void SetStationCodes(MHO_StationCodeMap* code_map);
        void ConstructVisibilityFileObjects();
        void WriteVisibilityObjects(std::string output_dir);

        bool IsAutoCorr() const {if(fRefStation == fRemStation){return true;} return false;};
        std::string GetRefStationMk4Id() const {return fRefStationMk4Id;}
        std::string GetRemStationMk4Id() const {return fRemStationMk4Id;}
        std::string GetBaselineShortName() const {return fBaselineShortName;}

        visibility_store_type* GetVisibilities() {return fV;}

        void Clear();

    private:

        void Organize();
        void DeleteDiFXVisRecords();

        std::string fRootCode;
        std::string fCorrDate;
        int fBaselineID;
        std::string fRefStation;
        std::string fRemStation;
        std::string fRefStationMk4Id;
        std::string fRemStationMk4Id;
        std::string fBaselineName;
        std::string fBaselineShortName;
        int fRefStationBits;
        int fRemStationBits;
        double fAPLength;

        //the station 2-char to 1-char code map (user specified)
        MHO_StationCodeMap* fStationCodeMap;

        const mho_json* fInput;
        std::vector< MHO_DiFXVisibilityRecord* > fRecords;
        //for a single baseline, maps pol-pair, then freqindex to visiblity records
        //needed to recorganized the visibilities into tables
        std::map< std::string, std::map<int, std::vector<MHO_DiFXVisibilityRecord* > > > fVisibilities;

        std::set<std::string> fPolPairSet;
        std::set<int> fFreqIndexSet;
        std::set<int> fAPSet;
        std::set<int> fSpecPointSet;
        std::size_t fNPolPairs;
        std::size_t fNChannels;
        std::size_t fNAPs;
        std::size_t fNSpectralPoints;
        bool fCanChannelize;

        bool fRescale;
        std::map<int, double> fNBitsToFactor;
        double fScaleFactor;

        //list of channel frequencies for this baseline, sorted in ascending order (freq)
        std::vector< std::pair<int, mho_json> > fBaselineFreqs;

        //the baseline data in hops data containers
        weight_store_type* fW;
        visibility_store_type* fV;
        MHO_ObjectTags fTags;

        //comparison predicate for time-sorting visibility record data
        struct VisRecordTimeLess
        {
            bool operator()(const MHO_DiFXVisibilityRecord* a, const MHO_DiFXVisibilityRecord* b) const
            {
                if(a->mjd == b->mjd){return a->seconds < b->seconds;}
                else{return a->mjd < b->mjd;}
            }
        };
        VisRecordTimeLess fTimePredicate;

        //comparison predicate for sorting index-frequency record pairs
        struct FreqIndexPairLess
        {
            bool operator()(const std::pair<int, mho_json>& a, const std::pair<int, mho_json>& b) const
            {
                double a_freq = a.second["freq"];
                double b_freq = b.second["freq"];
                return a_freq < b_freq;
            }
        };
        FreqIndexPairLess fFreqPredicate;




};

}//end of namespace

#endif /* end of include guard: MHO_DiFXBaselineProcessor */
