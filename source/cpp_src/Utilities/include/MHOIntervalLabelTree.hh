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

#include <vector>

#include "MHOIntervalLabel.hh"

namespace hops
{

class MHOIntervalLabelTree
{
    public:
        MHOIntervalLabelTree();
        virtual ~MHOIntervalLabelTree();

        void Insert(MHOIntervalLabel* label);

        std::vector< MHOIntervalLabel* > GetIntervalsWhichIntersect(const std::size_t& idx);
        std::vector< MHOIntervalLabel* > GetIntervalsWhichIntersect(const MHOInterval<std::size_t>* interval);

        //TODO: consider replacing the following boiler-plate functions with a template function
        std::vector< MHOIntervalLabel* > GetIntervalsWithKeyValue(const std::string& key, const std::string& value);
        std::vector< MHOIntervalLabel* > GetIntervalsWithKeyValue(const std::string& key, const char& value);
        std::vector< MHOIntervalLabel* > GetIntervalsWithKeyValue(const std::string& key, const bool& value);
        std::vector< MHOIntervalLabel* > GetIntervalsWithKeyValue(const std::string& key, const int& value);
        std::vector< MHOIntervalLabel* > GetIntervalsWithKeyValue(const std::string& key, const double& value);

    protected:

        //we are storing them all in a vector currently, this needs to to be
        //replaced with a tree data structure
        std::vector< MHOIntervalLabel* > fIntervals;



};

}//end namespace

#endif /* end of include guard: MHOIntervalLabelTree */
