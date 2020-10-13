#include "HkIntevalLabelTree.hh"

namespace hops
{

HkIntevalLabelTree::HkIntevalLabelTree(){}

HkIntevalLabelTree::~HkIntervalTree(){}

void 
HkIntevalLabelTree::Insert(HkIntervalLabel* label)
{
    //TODO make sure we are not inserting duplicates
    fIntervals.insert(label);
}


std::vector< HkIntervalLabel* > HkIntevalLabelTree::GetIntervalsWhichIntersect(const std::size_t& idx)
{
    std::vector< HkIntervalLabel* > labels;
    //dumb brute for search over all intervals O(n)
    for(std::size_t i=0; i<fIntervals.size(); i++)
    {
        if(fIntervals[i]->Intersects(idx))
        {
            labels.insert(fIntervals[i]);
        }
    }
    return labels;

}

std::vector< HkIntervalLabel* > HkIntevalLabelTree::GetIntervalsWhichIntersect(const HkInterval& interval)
{

}



std::vector< HkIntervalLabel* > 
HkIntevalLabelTree::GetIntervalsForIndex(const std::size_t& idx)
{

}

std::vector< HkIntervalLabel* > 
HkIntevalLabelTree::GetIntervalsWithKeyValue(const std::string& key, const std::string& value)
{

}

std::vector< HkIntervalLabel* > 
HkIntevalLabelTree::GetIntervalsWithKeyValue(const std::string& key, const char& value)
{

}

std::vector< HkIntervalLabel* > 
HkIntevalLabelTree::GetIntervalsWithKeyValue(const std::string& key, const bool& value)
{

}

std::vector< HkIntervalLabel* > 
HkIntevalLabelTree::GetIntervalsWithKeyValue(const std::string& key, const int& value)
{

}

std::vector< HkIntervalLabel* > 
HkIntevalLabelTree::GetIntervalsWithKeyValue(const std::string& key, const double& value)
{

}



}