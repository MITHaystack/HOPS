#include "MHOIntervalLabelTree.hh"

namespace hops
{

MHOIntervalLabelTree::MHOIntervalLabelTree(){}

MHOIntervalLabelTree::~MHOIntervalLabelTree(){}

void 
MHOIntervalLabelTree::Insert(MHOIntervalLabel* label)
{
    //TODO make sure we are not inserting duplicates
    fIntervals.push_back(label);
}


std::vector< MHOIntervalLabel* > MHOIntervalLabelTree::GetIntervalsWhichIntersect(const std::size_t& idx)
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

std::vector< MHOIntervalLabel* > MHOIntervalLabelTree::GetIntervalsWhichIntersect(const MHOInterval<std::size_t>* interval)
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

std::vector< MHOIntervalLabel* > 
MHOIntervalLabelTree::GetIntervalsWithKeyValue(const std::string& key, const std::string& value)
{
    std::vector< MHOIntervalLabel* > labels;
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

std::vector< MHOIntervalLabel* > 
MHOIntervalLabelTree::GetIntervalsWithKeyValue(const std::string& key, const char& value)
{
    std::vector< MHOIntervalLabel* > labels;
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

std::vector< MHOIntervalLabel* > 
MHOIntervalLabelTree::GetIntervalsWithKeyValue(const std::string& key, const bool& value)
{
    std::vector< MHOIntervalLabel* > labels;
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

std::vector< MHOIntervalLabel* > 
MHOIntervalLabelTree::GetIntervalsWithKeyValue(const std::string& key, const int& value)
{
    std::vector< MHOIntervalLabel* > labels;
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

std::vector< MHOIntervalLabel* > 
MHOIntervalLabelTree::GetIntervalsWithKeyValue(const std::string& key, const double& value)
{
    std::vector< MHOIntervalLabel* > labels;
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