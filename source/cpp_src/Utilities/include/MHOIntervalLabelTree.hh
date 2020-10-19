#ifndef MHOIntervalLabelTree_HH__
#define MHOIntervalLabelTree_HH__

/*
*File: MHOIntervalLabelTree.hh
*Class: MHOIntervalLabelTree
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: Inteval-tree stucture, to allow for fast location of an interval
* currently un-implemented...just does a dumb brute force O(n) search.
* TODO: consider use of smart pointers for interval label objects
*/

#include <iostream>
#include <vector>

#include "MHOIntervalLabel.hh"

namespace hops
{

class MHOIntervalLabelTree
{
    public:
        MHOIntervalLabelTree();
        virtual ~MHOIntervalLabelTree();

        //void InsertLabel(MHOIntervalLabel* label);
        void InsertLabel(const MHOIntervalLabel& label);

        std::vector< MHOIntervalLabel* > GetIntervalsWhichIntersect(const std::size_t& idx);
        std::vector< MHOIntervalLabel* > GetIntervalsWhichIntersect(const MHOInterval<std::size_t>* interval);

        template<typename XLabelValueType>
        std::vector< MHOIntervalLabel* >
        GetIntervalsWithKeyValue(const std::string& key, const XLabelValueType& value);

    protected:

        //we are storing them all in a vector currently, this needs to to be
        //replaced with a tree data structure
        std::vector< MHOIntervalLabel* > fIntervals;
};



template<typename XLabelValueType>
std::vector< MHOIntervalLabel* >
MHOIntervalLabelTree::GetIntervalsWithKeyValue(const std::string& key, const XLabelValueType& value)
{
    std::vector< MHOIntervalLabel* > labels;
    XLabelValueType tmp_value;
    //dumb brute force search over all intervals O(n)
    //we may want to make this smarter
    for(std::size_t i=0; i<fIntervals.size(); i++)
    {
        if( fIntervals[i]->Retrieve(key,tmp_value) )
        {
            if(tmp_value == value)
            {
                labels.push_back(fIntervals[i]);
            }
        }
    }
    return labels;
}



}//end namespace

#endif /* end of include guard: MHOIntervalLabelTree */
