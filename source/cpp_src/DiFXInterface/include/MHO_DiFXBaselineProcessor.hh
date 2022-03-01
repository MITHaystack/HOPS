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

#include "MHO_DiFXVisibilityRecord.hh"
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
    
    private:

        int fBaselineID;
        json* fInput;
        std::vector< MHO_DiFXVisibilityRecord* > fRecords;
        std::set< std::string > fUniquePolPairs;

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



};

}//end of namespace

#endif /* end of include guard: MHO_DiFXBaselineProcessor */