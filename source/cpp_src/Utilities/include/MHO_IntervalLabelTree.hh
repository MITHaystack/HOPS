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
*/

#include <iostream>
#include <vector>

#include "MHO_Message.hh"
#include "MHO_IntervalLabel.hh"
#include "MHO_Serializable.hh"


namespace hops
{

class MHO_IntervalLabelTree: virtual public MHO_Serializable
{
    public:
        MHO_IntervalLabelTree();
        virtual ~MHO_IntervalLabelTree();

        //copy constructor
        MHO_IntervalLabelTree(const MHO_IntervalLabelTree& obj);

        //void InsertLabel(MHO_IntervalLabel* label);
        void InsertLabel(const MHO_IntervalLabel& label);

        std::vector< MHO_IntervalLabel* > GetIntervalsWhichIntersect(const std::size_t& idx);
        std::vector< const MHO_IntervalLabel* > GetIntervalsWhichIntersect(const std::size_t& idx) const;
        std::vector< MHO_IntervalLabel* > GetIntervalsWhichIntersect(const MHO_Interval<std::size_t>* interval);
        std::vector< const MHO_IntervalLabel* > GetIntervalsWhichIntersect(const MHO_Interval<std::size_t>* interval) const;

        std::vector< MHO_IntervalLabel* >
        GetIntervalsWithKey(const std::string& key);

        std::vector< const MHO_IntervalLabel* >
        GetIntervalsWithKey(const std::string& key) const;

        template<typename XLabelValueType>
        std::size_t
        GetNIntervalsWithKeyValue(const std::string& key, const XLabelValueType& value) const;

        template<typename XLabelValueType>
        std::vector< MHO_IntervalLabel* >
        GetIntervalsWithKeyValue(const std::string& key, const XLabelValueType& value);

        template<typename XLabelValueType>
        MHO_IntervalLabel*
        GetFirstIntervalWithKeyValue(const std::string& key, const XLabelValueType& value);

        template<typename XLabelValueType>
        const MHO_IntervalLabel*
        GetFirstIntervalWithKeyValue(const std::string& key, const XLabelValueType& value) const;

        virtual void CopyIntervalLabels(const MHO_IntervalLabelTree& obj)
        {
            fIntervals.clear();
            for(auto iter = obj.fIntervals.begin(); iter != obj.fIntervals.end(); iter++)
            {
                fIntervals.push_back( new MHO_IntervalLabel( *(*iter) ) );
            }
        }

    protected:

        //we are storing them all in a vector currently, this probably
        //fine for storage, but the access interface needs to to be
        //replaced with a tree-like data structure for faster lookup
        std::vector< MHO_IntervalLabel* > fIntervals;

    public:

        virtual uint64_t GetSerializedSize() const override
        {
            uint64_t total_size = 0;
            total_size += sizeof(MHO_ClassVersion); //version number
            total_size += sizeof(std::size_t); //number of intervals
            for(std::size_t i=0; i<fIntervals.size(); i++)
            {
                total_size += fIntervals[i]->GetSerializedSize();
            }
            return total_size;
        }

        template<typename XStream> friend XStream& operator>>(XStream& s, MHO_IntervalLabelTree& aData)
        {
            MHO_ClassVersion vers;
            s >> vers;
            switch(vers) 
            {
                case 0:
                    aData.StreamInData_V0(s);
                break;
                default:
                    MHO_ClassIdentity::ClassVersionErrorMsg(aData, vers);
                    //Flag this as an unknown object version so we can skip over this data
                    MHO_ObjectStreamState<XStream>::SetUnknown(s);
            }
            return s;
        }

        template<typename XStream> friend XStream& operator<<(XStream& s, const MHO_IntervalLabelTree& aData)
        {
            switch( aData.GetVersion() ) 
            {
                case 0:
                    s << aData.GetVersion();
                    aData.StreamOutData_V0(s);
                break;
                default:
                    msg_error("containers", 
                        "error, cannot stream out MHO_IntervalLabel object with unknown version: " 
                        << aData.GetVersion() << eom );
            }
            return s;
        }

    private:

        template<typename XStream> void StreamInData_V0(XStream& s)
        {
            std::size_t n_labels;
            s >> n_labels;
            for(std::size_t i=0; i<n_labels; i++)
            {
                MHO_IntervalLabel tmp;
                s >> tmp;
                this->InsertLabel(tmp);
            }
        }

        template<typename XStream> void StreamOutData_V0(XStream& s) const
        {
            s << this->fIntervals.size();
            for(std::size_t i=0; i<this->fIntervals.size(); i++)
            {
                s << *( this->fIntervals[i] );
            }
        }
};









template<typename XLabelValueType>
std::size_t
MHO_IntervalLabelTree::GetNIntervalsWithKeyValue(const std::string& key, const XLabelValueType& value) const
{
    std::size_t count = 0;
    XLabelValueType tmp_value;
    for(std::size_t i=0; i<fIntervals.size(); i++)
    {
        if( fIntervals[i]->Retrieve(key,tmp_value) )
        {
            if(tmp_value == value){++count;}
        }
    }
    return count;
}

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
    //TODO FIXME -- use an actual interval tree
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

template<typename XLabelValueType>
const MHO_IntervalLabel*
MHO_IntervalLabelTree::GetFirstIntervalWithKeyValue(const std::string& key, const XLabelValueType& value) const
{
    const MHO_IntervalLabel* label = nullptr;
    XLabelValueType tmp_value;
    //TODO FIXME -- use an actual interval tree
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
