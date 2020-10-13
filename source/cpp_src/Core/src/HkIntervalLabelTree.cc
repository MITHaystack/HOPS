#include "HkIntervalLabelTree.hh"

namespace hops
{

HkIntervalLabelTree::HkIntervalLabelTree(){}

HkIntervalLabelTree::~HkIntervalLabelTree(){}

void 
HkIntervalLabelTree::Insert(HkIntervalLabel* label)
{
    //TODO make sure we are not inserting duplicates
    fIntervals.push_back(label);
}


std::vector< HkIntervalLabel* > HkIntervalLabelTree::GetIntervalsWhichIntersect(const std::size_t& idx)
{
    std::vector< HkIntervalLabel* > labels;
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

std::vector< HkIntervalLabel* > HkIntervalLabelTree::GetIntervalsWhichIntersect(const HkInterval<std::size_t>* interval)
{
    std::vector< HkIntervalLabel* > labels;
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

std::vector< HkIntervalLabel* > 
HkIntervalLabelTree::GetIntervalsWithKeyValue(const std::string& key, const std::string& value)
{
    std::vector< HkIntervalLabel* > labels;
    std::string tmp_value;
    //dumb brute for search over all intervals O(n)
    for(std::size_t i=0; i<fIntervals.size(); i++)
    {
        if(fIntervals[i]->Retrieve(key,tmp_value) )
        {
            if(tmp_value == value)
            {
                labels.push_back(fIntervals[i]);
            }
        }
    }
    return labels;
}

std::vector< HkIntervalLabel* > 
HkIntervalLabelTree::GetIntervalsWithKeyValue(const std::string& key, const char& value)
{
    std::vector< HkIntervalLabel* > labels;
    char tmp_value;
    //dumb brute for search over all intervals O(n)
    for(std::size_t i=0; i<fIntervals.size(); i++)
    {
        if(fIntervals[i]->Retrieve(key,tmp_value) )
        {
            if(tmp_value == value)
            {
                labels.push_back(fIntervals[i]);
            }
        }
    }
    return labels;
}

std::vector< HkIntervalLabel* > 
HkIntervalLabelTree::GetIntervalsWithKeyValue(const std::string& key, const bool& value)
{
    std::vector< HkIntervalLabel* > labels;
    bool tmp_value;
    //dumb brute for search over all intervals O(n)
    for(std::size_t i=0; i<fIntervals.size(); i++)
    {
        if(fIntervals[i]->Retrieve(key,tmp_value) )
        {
            if(tmp_value == value)
            {
                labels.push_back(fIntervals[i]);
            }
        }
    }
    return labels;
}

std::vector< HkIntervalLabel* > 
HkIntervalLabelTree::GetIntervalsWithKeyValue(const std::string& key, const int& value)
{
    std::vector< HkIntervalLabel* > labels;
    int tmp_value;
    //dumb brute for search over all intervals O(n)
    for(std::size_t i=0; i<fIntervals.size(); i++)
    {
        if(fIntervals[i]->Retrieve(key,tmp_value) )
        {
            if(tmp_value == value)
            {
                labels.push_back(fIntervals[i]);
            }
        }
    }
    return labels;
}

std::vector< HkIntervalLabel* > 
HkIntervalLabelTree::GetIntervalsWithKeyValue(const std::string& key, const double& value)
{
    std::vector< HkIntervalLabel* > labels;
    double tmp_value;
    //dumb brute for search over all intervals O(n)
    for(std::size_t i=0; i<fIntervals.size(); i++)
    {
        if(fIntervals[i]->Retrieve(key,tmp_value) )
        {
            if(tmp_value == value)
            {
                labels.push_back(fIntervals[i]);
            }
        }
    }
    return labels;
}



}