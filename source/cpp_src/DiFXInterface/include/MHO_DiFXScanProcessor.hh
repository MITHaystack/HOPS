#ifndef MHO_DiFXScanProcessor_HH__
#define MHO_DiFXScanProcessor_HH__

/*
*@file: MHO_DiFXScanProcessor.hh
*@class: MHO_DiFXScanProcessor
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/


#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <set>

#include "difxio/difx_input.h"
#include "difxio/parsevis.h"

#include "MHO_Message.hh"

#include "MHO_Visibilities.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_VisibilityChannelizer.hh"
#include "MHO_WeightChannelizer.hh"
#include "MHO_StationCoordinates.hh"

#include "MHO_BinaryFileStreamer.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_ClassIdentityMap.hh"

#include "MHO_DiFXScanFileSet.hh"
#include "MHO_DiFXVisibilityRecord.hh"
#include "MHO_DiFXVisibilityProcessor.hh"

namespace hops 
{

class MHO_DiFXScanProcessor
{
    public:
        MHO_DiFXScanProcessor();
        virtual ~MHO_DiFXScanProcessor();

        void ProcessScan(MHO_DiFXScanFileSet& fileSet);

    private:

        ////////////////////////////////////////////////////////////////////////
        //members for dealing with a single (current) scan of data /////////////
        MHO_DiFXScanFileSet* fFileSet;

        void LoadInputFile(std::string filename);

        //organize all of the visibility records of this baseline by time and frequency
        void OrganizeBaseline(int baseline);

        // void ReadPCAL_File(std::string filename);
        // void ReadIM_File(std::string filename);
        void ConstructRootFileObject();
        void ConstructStationFileObjects();
        void ConstructVisibilityFileObjects();
        void WriteVisibilityObjects();

        //the DiFX input file structure 
        DifxInput* fDInput;
        int fCurrentBaselineIndex;
        //maps all freqindex's to the difx frequency description
        std::map<int, DifxFreq*> fAllFreqTable;


        //maps DiFX baseline index to vector of all associated visibility records 
        std::map<int, std::vector< MHO_DiFXVisibilityRecord* > > fAllBaselineVisibilities;
        //tracks the set of pol-pairs present on each baseline
        std::map< int, std::set< std::string > > fAllBaselineUniquePolPairs;

        //for a single baseline, maps pol-pair, then freqindex to visiblity records 
        //needed to recorganized the visibilities into tables 
        std::map< std::string, std::map<int, std::vector<MHO_DiFXVisibilityRecord* > > > fVisibilities;

        //maps freqindex to vector of associated visiblity records
        //std::map<int, std::vector<MHO_DiFXVisibilityRecord> > fChannels;

        //list of channel frequencies for this baseline, sorted in ascending order (freq)
        std::vector< std::pair<int, DifxFreq*> > fBaselineFreqs;

        //the baseline data (created and destroyed for each baseline processed)
        ch_baseline_weight_type* fW;
        ch_baseline_data_type* fV;

        //determines the size of the (channelized) visibilities
        std::set<std::string> fPolPairSet;
        std::set<int> fFreqIndexSet;
        std::set<int> fAPSet;
        std::set<int> fSpecPointSet;
        std::size_t fNPolPairs;
        std::size_t fNChannels;
        std::size_t fNAPs;
        std::size_t fNSpectralPoints;

        bool fPadAPs; //true if NAPS is not the same for all channels
        bool fCanChannelize; //false if n-spectral points is not the same for all channels
    

        ////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////

        //comparison predicate for time-sorting visibility record data
        typedef struct 
        {
            bool operator()(const MHO_DiFXVisibilityRecord* a, const MHO_DiFXVisibilityRecord* b) const 
            {
                if(a->mjd == b->mjd){return a->seconds < b->seconds;}
                else{return a->mjd < b->mjd;}
            }
        } VisRecordTimeLess;
        VisRecordTimeLess fTimePredicate;

        //comparison predicate for sorting index-frequency record pairs
        typedef struct 
        {
            bool operator()(const std::pair<int, DifxFreq*>& a, const std::pair<int, DifxFreq*>& b) const 
            {
                return a.second->freq < b.second->freq;
            }
        } FreqIndexPairLess;
        FreqIndexPairLess fFreqPredicate;


        ////////////////////////////////////////////////////////////////////////

        void ClearVisibilityRecords();

};

}//end of hops namespace

#endif /* end of include guard: MHO_DiFXScanProcessor */