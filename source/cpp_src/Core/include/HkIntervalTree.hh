#ifndef HkIntervalTree_HH__
#define HkIntervalTree_HH__

/*
*File: HkIntervalTree.hh
*Class: HkIntervalTree
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: Inteval-tree stucture, to allow for fast location of an interval
* currently un-implemented...just does a dumb brute force O(n) search.
*/

#include "HkInterval.hh"

class HkIntervalTree
{
    public:
        HkIntervalTree();
        virtual ~HkIntervalTree();

        void Insert(HkInterval* )

    protected:

        //we are storing
        std::vector< HkInterval* > fIntervals;



};

#endif /* end of include guard: HkIntervalTree */
