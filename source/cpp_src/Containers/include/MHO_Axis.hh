#ifndef MHO_Axis_HH__
#define MHO_Axis_HH__

/*
*File: MHO_Axis.hh
*Class: MHO_Axis
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <set>

#include "MHO_Meta.hh"
#include "MHO_VectorContainer.hh"
#include "MHO_Interval.hh"
#include "MHO_IntervalLabel.hh"
#include "MHO_IntervalLabelTree.hh"

namespace hops
{


template< typename XValueType >
class MHO_Axis:
    public MHO_AxisBase,
    public MHO_VectorContainer< XValueType >,
    public MHO_IntervalLabelTree
{

    public:
        MHO_Axis():
            MHO_VectorContainer<XValueType>(),
            MHO_IntervalLabelTree()
        {};


        MHO_Axis(std::size_t dim):
            MHO_VectorContainer< XValueType >(dim),
            MHO_IntervalLabelTree()
        {};

        //copy constructor
        MHO_Axis(const MHO_Axis& obj):
            MHO_VectorContainer< XValueType >(obj),
            MHO_IntervalLabelTree(obj)
        {};


        virtual ~MHO_Axis(){};

        //have to make base class functions visible
        using MHO_VectorContainer<XValueType>::Resize;
        using MHO_VectorContainer<XValueType>::GetData;
        using MHO_VectorContainer<XValueType>::GetSize;
        using MHO_VectorContainer<XValueType>::GetDimensions;
        using MHO_VectorContainer<XValueType>::GetDimension;
        using MHO_VectorContainer<XValueType>::GetOffsetForIndices;
        using MHO_VectorContainer<XValueType>::operator();
        using MHO_VectorContainer<XValueType>::operator[];


        //index selection from matching axis values
        std::vector< std::size_t >
        SelectMatchingIndexes(const std::set<XValueType> label_values)
        {
            std::vector< std::size_t > selected_idx;
            //dumb brute force search, for each label value
            //check all the axis elements for a match
            for(auto label_it = label_values.begin(); label_it != label_values.end(); label_it++)
            {
                for(std::size_t i = 0; i < this->GetSize(); i++)
                {
                    if( (*this)[i] == *label_it )
                    {
                        selected_idx.push_back(i);
                    }
                }
            }
            return selected_idx;
        }

        //index selection for matching axis values (given a single value)
        std::vector< std::size_t >
        SelectMatchingIndexes(const XValueType& label_value)
        {
            std::vector< std::size_t > selected_idx;
            //dumb brute force search, for a single label value
            //check all the axis elements for a match
            for(std::size_t i = 0; i < this->GetSize(); i++)
            {
                if( (*this)[i] == label_value )
                {
                    selected_idx.push_back(i);
                }
            }
            return selected_idx;
        }


        //index selection for first matching axis values (given a single value)
        bool
        SelectFirstMatchingIndex(const XValueType& label_value, std::size_t& result)
        {
            result = 0;
            for(std::size_t i = 0; i < this->GetSize(); i++)
            {
                if( (*this)[i] == label_value )
                {
                    result = i;
                    return true;
                }
            }
            return false;
        }

        //index selection from matching label values (e.g. gets the indices for
        //which column is tagged with "channel_label":"a" etc.)
        //extra dumb brute force, TODO make me smarter
        template< typename XLabelValueType >
        std::vector< std::size_t >
        SelectMatchingIndexesByLabelValue(const std::string& label_key, const XLabelValueType& label_value)
        {
            std::vector< std::size_t > matching_idx;
            for(std::size_t i=0; i < this->GetSize(); i++)
            {
                std::vector< MHO_IntervalLabel* > labels;
                labels = this->GetIntervalsWhichIntersect(i);
                for(std::size_t j=0; j < labels.size(); j++)
                {
                    XLabelValueType value;
                    if(labels[j]->HasKey(label_key))
                    {
                        labels[j]->Retrieve(label_key, value);
                        if(value == label_value){matching_idx.push_back(i);}
                    }
                }
            }
            return matching_idx;
        }

        //collect all indices which match any value in the set
        template< typename XLabelValueType >
        std::vector< std::size_t >
        SelectMatchingIndexesByLabelValue(const std::string& label_key, const std::set<XLabelValueType>& label_values)
        {
            std::vector< std::size_t > matching_idx;
            std::set< std::size_t > idx;
            for(std::size_t i=0; i < this->GetSize(); i++)
            {
                std::vector< MHO_IntervalLabel* > labels;
                labels = this->GetIntervalsWhichIntersect(i);
                for(std::size_t j=0; j<labels.size(); j++)
                {
                    XLabelValueType value;
                    if(labels[j]->HasKey(label_key))
                    {
                        labels[j]->Retrieve(label_key, value);
                        if( label_values.find(value) != label_values.end() ){idx.insert(i);}
                    }
                }
            }
            std::copy(idx.begin(), idx.end(), std::inserter(matching_idx, matching_idx.end()));
            return matching_idx;
        }


        template< typename XLabelValueType >
        void CollectAxisElementLabelValues(const std::string& label_key, std::vector< XLabelValueType >& label_values )
        {
            label_values.clear();
            for(std::size_t i=0; i < this->GetSize(); i++)
            {
                std::vector< MHO_IntervalLabel* > labels;
                labels = this->GetIntervalsWhichIntersect(i);
                for(std::size_t j=0; j < labels.size(); j++)
                {
                    XLabelValueType value;
                    if(labels[j]->HasKey(label_key))
                    {
                        labels[j]->Retrieve(label_key, value);
                        label_values.push_back(value);
                        break;
                    }
                }
            }
        }


        virtual uint64_t GetSerializedSize() const override
        {
            uint64_t total_size = 0;
            total_size += sizeof(MHO_ClassVersion);
            total_size += MHO_VectorContainer< XValueType >::GetSerializedSize();
            total_size += MHO_IntervalLabelTree::GetSerializedSize();
            return total_size;
        }

        //expensive copy (as opposed to the assignment operator,
        //pointers to exernally managed memory are not transferred)
        virtual void Copy(const MHO_Axis& rhs)
        {
            if(&rhs != this)
            {
                MHO_VectorContainer<XValueType>::Copy(rhs); //copy the 1-d array
                MHO_IntervalLabelTree::CopyIntervalLabels(rhs); //copy interval tree
            }
        }

        //access the underlying array data as a std::vector<XValueType>
        //we have only exposed this for use in the python bindings
        //perhaps we should make them a friend class and hide this?
        // std::vector< XValueType >* GetRawVector(){return &(this->fData);}

        template<typename XStream> friend XStream& operator>>(XStream& s, MHO_Axis& aData)
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


        template<typename XStream> friend XStream& operator<<(XStream& s, const MHO_Axis& aData)
        {
            switch( aData.GetVersion() )
            {
                case 0:
                    s << aData.GetVersion();
                    aData.StreamOutData_V0(s);
                break;
                default:
                    msg_error("containers",
                        "error, cannot stream out MHO_Axis object with unknown version: "
                        << aData.GetVersion() << eom );
            }
            return s;
        }

    private:

        template<typename XStream> void StreamInData_V0(XStream& s)
        {
            s >> static_cast< MHO_VectorContainer< XValueType >& >(*this);
            s >> static_cast< MHO_IntervalLabelTree& >(*this);
        }

        template<typename XStream> void StreamOutData_V0(XStream& s) const
        {
            s << static_cast< const MHO_VectorContainer< XValueType >& >(*this);
            s << static_cast< const MHO_IntervalLabelTree& >(*this);
        }

        virtual MHO_UUID DetermineTypeUUID() const override
        {
            MHO_MD5HashGenerator gen;
            gen.Initialize();
            std::string name = MHO_ClassIdentity::ClassName(*this);
            gen << name;
            gen.Finalize();
            return gen.GetDigestAsUUID();
        }


};

// ////////////////////////////////////////////////////////////////////////////////
// //using declarations for all basic 'plain-old-data' types (except bool!)
using MHO_AxisChar = MHO_Axis<char>;
using MHO_AxisUChar = MHO_Axis<unsigned char>;
using MHO_AxisShort = MHO_Axis<short>;
using MHO_AxisUShort = MHO_Axis<unsigned short>;
using MHO_AxisInt = MHO_Axis<int>;
using MHO_AxisUInt = MHO_Axis<unsigned int>;
using MHO_AxisLong = MHO_Axis<long>;
using MHO_AxisULong = MHO_Axis<unsigned long>;
using MHO_AxisLongLong = MHO_Axis<long long>;
using MHO_AxisULongLong = MHO_Axis<unsigned long long>;
using MHO_AxisFloat = MHO_Axis<float>;
using MHO_AxisDouble = MHO_Axis<double>;
using MHO_AxisLongDouble = MHO_Axis<long double>;
using MHO_AxisComplexFloat = MHO_Axis< std::complex<float> >;
using MHO_AxisComplexDouble = MHO_Axis< std::complex<double> >;
using MHO_AxisComplexLongDouble = MHO_Axis< std::complex<long double> >;
using MHO_AxisString = MHO_Axis< std::string >;
// ////////////////////////////////////////////////////////////////////////////////

}

#endif /* end of include guard: MHO_Axis */
