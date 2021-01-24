#ifndef MHO_IntervalLabelTree_HH__
#define MHO_IntervalLabelTree_HH__

/*
*File: MHO_IntervalLabelTree.hh
*Class: MHO_IntervalLabelTree
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: Inteval-tree stucture, to allow for fast location of an interval
* currently un-implemented...just does a dumb brute force O(n) search.
* TODO: consider use of smart pointers for interval label objects
*/

#include <iostream>
#include <vector>

#include "MHO_Message.hh"
#include "MHO_IntervalLabel.hh"

namespace hops
{

class MHO_IntervalLabelTree
{
    public:
        MHO_IntervalLabelTree();
        virtual ~MHO_IntervalLabelTree();

        //void InsertLabel(MHO_IntervalLabel* label);
        void InsertLabel(const MHO_IntervalLabel& label);

        std::vector< MHO_IntervalLabel* > GetIntervalsWhichIntersect(const std::size_t& idx);
        std::vector< MHO_IntervalLabel* > GetIntervalsWhichIntersect(const MHO_Interval<std::size_t>* interval);

        std::vector< MHO_IntervalLabel* >
        GetIntervalsWithKey(const std::string& key);

        template<typename XLabelValueType>
        std::vector< MHO_IntervalLabel* >
        GetIntervalsWithKeyValue(const std::string& key, const XLabelValueType& value);

        template<typename XLabelValueType>
        MHO_IntervalLabel*
        GetFirstIntervalWithKeyValue(const std::string& key, const XLabelValueType& value);


    protected:

        //we are storing them all in a vector currently, this needs to to be
        //replaced with a tree data structure
        std::vector< MHO_IntervalLabel* > fIntervals;
};



template<typename XLabelValueType>
std::vector< MHO_IntervalLabel* >
MHO_IntervalLabelTree::GetIntervalsWithKeyValue(const std::string& key, const XLabelValueType& value)
{
    std::vector< MHO_IntervalLabel* > labels;
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


template<typename XLabelValueType>
MHO_IntervalLabel*
MHO_IntervalLabelTree::GetFirstIntervalWithKeyValue(const std::string& key, const XLabelValueType& value)
{
    MHO_IntervalLabel* label = nullptr;
    XLabelValueType tmp_value;
    //dumb brute force search over all intervals O(n)
    //we may want to make this smarter

    for(std::size_t i=0; i<fIntervals.size(); i++)
    {
        if( fIntervals[i]->Retrieve(key,tmp_value) )
        {
            if(tmp_value == value)
            {
                label = fIntervals[i];
                break;
            }
        }
    }
    return label;
}


}//end namespace

#endif /* end of include guard: MHO_IntervalLabelTree */
