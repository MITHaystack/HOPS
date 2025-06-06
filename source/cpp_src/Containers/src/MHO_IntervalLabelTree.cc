#include "MHO_IntervalLabelTree.hh"

namespace hops
{

MHO_IntervalLabelTree::MHO_IntervalLabelTree()
{}

MHO_IntervalLabelTree::~MHO_IntervalLabelTree()
{}

MHO_IntervalLabelTree::MHO_IntervalLabelTree(const MHO_IntervalLabelTree& obj)
{
    for(auto iter = obj.fIntervals.begin(); iter != obj.fIntervals.end(); iter++)
    {
        fIntervals.push_back(MHO_IntervalLabel(*iter));
    }
}

void MHO_IntervalLabelTree::InsertLabel(MHO_IntervalLabel label)
{
    //insert copy of this label as a new object
    fIntervals.push_back(label);
}

void MHO_IntervalLabelTree::ClearLabels()
{
    fIntervals.clear();
}

std::vector< MHO_IntervalLabel > MHO_IntervalLabelTree::GetIntervalsWithKey(const std::string& key) const
{
    std::vector< MHO_IntervalLabel > labels;
    //dumb brute for search over all intervals O(n)
    for(std::size_t i = 0; i < fIntervals.size(); i++)
    {
        if(fIntervals[i].HasKey(key))
        {
            labels.push_back(fIntervals[i]);
        }
    }
    return labels;
}

std::vector< MHO_IntervalLabel > MHO_IntervalLabelTree::GetIntervalsWhichIntersect(const std::size_t& idx) const
{
    std::vector< MHO_IntervalLabel > labels;
    //dumb brute for search over all intervals O(n)
    for(std::size_t i = 0; i < fIntervals.size(); i++)
    {
        if(fIntervals[i].Intersects(idx))
        {
            labels.push_back(fIntervals[i]);
        }
    }
    return labels;
}

std::vector< MHO_IntervalLabel >
MHO_IntervalLabelTree::GetIntervalsWhichIntersect(const MHO_Interval< std::size_t >& interval) const
{
    std::vector< MHO_IntervalLabel > labels;
    //dumb brute for search over all intervals O(n)
    for(std::size_t i = 0; i < fIntervals.size(); i++)
    {
        if(fIntervals[i].Intersects(interval))
        {
            labels.push_back(fIntervals[i]);
        }
    }
    return labels;
}

} // namespace hops
