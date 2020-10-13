#ifndef HkIntervalLabelTree_HH__
#define HkIntervalLabelTree_HH__

/*
*File: HkIntervalLabelTree.hh
*Class: HkIntervalLabelTree
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: Inteval-tree stucture, to allow for fast location of an interval
* currently un-implemented...just does a dumb brute force O(n) search.
* TODO: consider use of smart pointers for interval label objects
*/

#include "HkIntervalLabel.hh"

namespace hops
{

class HkIntervalLabelTree
{
    public:
        HkIntervalLabelTree();
        virtual ~HkIntervalLabelTree();

        void Insert(HkIntervalLabel* label);

        std::vector< HkIntervalLabel* > GetIntervalsWhichIntersect(const std::size_t& idx);
        std::vector< HkIntervalLabel* > GetIntervalsWhichIntersect(const HkInterval<std::size_t>* interval);

        //TODO: consider replacing the following boiler-plate functions with a template function
        std::vector< HkIntervalLabel* > GetIntervalsWithKeyValue(const std::string& key, const std::string& value);
        std::vector< HkIntervalLabel* > GetIntervalsWithKeyValue(const std::string& key, const char& value);
        std::vector< HkIntervalLabel* > GetIntervalsWithKeyValue(const std::string& key, const bool& value);
        std::vector< HkIntervalLabel* > GetIntervalsWithKeyValue(const std::string& key, const int& value);
        std::vector< HkIntervalLabel* > GetIntervalsWithKeyValue(const std::string& key, const double& value);

    protected:

        //we are storing them all in a vector currently, this needs to to be 
        //replaced with a tree data structure
        std::vector< HkIntervalLabel* > fIntervals;



};

}//end namespace

#endif /* end of include guard: HkIntervalLabelTree */
