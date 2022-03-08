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
#include "MHO_Visibilities.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_VisibilityChannelizer.hh"
#include "MHO_WeightChannelizer.hh"

#include "MHO_JSONHeaderWrapper.hh"

namespace hops 
{

class MHO_DiFXBaselineProcessor
{
    public:
        MHO_DiFXBaselineProcessor();
        virtual ~MHO_DiFXBaselineProcessor();

        int GetBaselineID() const {return fBaselineID;};
        void SetDiFXInputData(const json* input){fInput = input;}

        void AddRecord(MHO_DiFXVisibilityRecord* record);

        void ConstructVisibilityFileObjects();
        void WriteVisibilityObjects(std::string output_dir);

    private:

        void Organize();
        void Clear();

        int fBaselineID;
        std::string fRefStation;
        std::string fRemStation;
        std::string fBaselineName;
        std::string fBaselineShortName;
        double fAPLength;

        const json* fInput;
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

        //list of channel frequencies for this baseline, sorted in ascending order (freq)
        std::vector< std::pair<int, json> > fBaselineFreqs;

        //the baseline data in hops data containers
        ch_baseline_weight_type* fW;
        ch_baseline_data_type* fV;

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
            bool operator()(const std::pair<int, json>& a, const std::pair<int, json>& b) const 
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