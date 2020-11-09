#include "MHOIntervalLabelTree.hh"

namespace hops
{

MHOIntervalLabelTree::MHOIntervalLabelTree(){}

MHOIntervalLabelTree::~MHOIntervalLabelTree(){}

// void
// MHOIntervalLabelTree::InsertLabel(MHOIntervalLabel* label)
// {
//     //TODO make sure we are not inserting duplicates
//     fIntervals.push_back(label);
// }

void
MHOIntervalLabelTree::InsertLabel(const MHOIntervalLabel& label)
{
    //insert copy of this label as a new object
    fIntervals.push_back( new MHOIntervalLabel(label) );
}

std::vector< MHOIntervalLabel* >
MHOIntervalLabelTree::GetIntervalsWithKey(const std::string& key)
{
    std::vector< MHOIntervalLabel* > labels;
    //dumb brute for search over all intervals O(n)
    for(std::size_t i=0; i<fIntervals.size(); i++)
    {
            if(fIntervals[i]->HasKey(key) )
            {
                labels.push_back(fIntervals[i]);
            }
    }
    return labels;
}

std::vector< MHOIntervalLabel* >
MHOIntervalLabelTree::GetIntervalsWhichIntersect(const std::size_t& idx)
{
    std::vector< MHOIntervalLabel* > labels;
    //dumb brute for search over all intervals O(n)
    for(std::size_t i=0; i<fIntervals.size(); i++)
    {
        if(fIntervals[i]->Intersects(idx))
        {
            labels.push_back(fIntervals[i]);
        }
    }
    return labels;
}

std::vector< MHOIntervalLabel* >
MHOIntervalLabelTree::GetIntervalsWhichIntersect(const MHOInterval<std::size_t>* interval)
{
    std::vector< MHOIntervalLabel* > labels;
    //dumb brute for search over all intervals O(n)
    for(std::size_t i=0; i<fIntervals.size(); i++)
    {
        if(fIntervals[i]->Intersects(*interval))
        {
            labels.push_back(fIntervals[i]);
        }
    }
    return labels;
}


}
