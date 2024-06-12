#ifndef MHO_Axis_HH__
#define MHO_Axis_HH__



#include <set>

#include "MHO_Meta.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_VectorContainer.hh"
#include "MHO_IndexLabelInterface.hh"
#include "MHO_IntervalLabelInterface.hh"

namespace hops
{

/*!
*@file MHO_Axis.hh
*@class MHO_Axis
*@author J. Barrett - barrettj@mit.edu
*@date Mon Oct 19 11:34:27 2020 -0400
*@brief
*/

template< typename XValueType >
class MHO_Axis:
    public MHO_AxisBase,
    public MHO_VectorContainer< XValueType >,
    public MHO_IndexLabelInterface,
    public MHO_IntervalLabelInterface
{

    public:
        MHO_Axis():
            MHO_VectorContainer<XValueType>(),
            MHO_IndexLabelInterface(),
            MHO_IntervalLabelInterface()
        {
            //create and set the pointer to the index label object
            //std::vector<mho_json> tmp;
            this->fObject["index_labels"] = mho_json();// mho_json::array(); //tmp;
            this->SetIndexLabelObject( &(this->fObject["index_labels"]) );

            //create and set the pointer to the interval label object
            this->fObject["interval_labels"] = mho_json();
            this->SetIntervalLabelObject( &(this->fObject["interval_labels"]) );
        };


        MHO_Axis(std::size_t dim):
            MHO_VectorContainer< XValueType >(dim),
            MHO_IndexLabelInterface(),
            MHO_IntervalLabelInterface()
        {
            //create and set the pointer to the index label object
            //std::vector<mho_json> tmp;
            this->fObject["index_labels"] = mho_json();// mho_json::array(); //tmp;
            this->SetIndexLabelObject( &(this->fObject["index_labels"]) );

            //create and set the pointer to the interval label object
            this->fObject["interval_labels"] = mho_json();
            this->SetIntervalLabelObject( &(this->fObject["interval_labels"]) );
        };

        //copy constructor
        MHO_Axis(const MHO_Axis& obj):
            MHO_VectorContainer< XValueType >(obj),
            MHO_IndexLabelInterface(),
            MHO_IntervalLabelInterface()
        {
            if( obj.fObject.contains("index_labels") ){ this->fObject["index_labels"] = obj.fObject["index_labels"]; }
            if( obj.fObject.contains("interval_labels") ){ this->fObject["interval_labels"] = obj.fObject["interval_labels"]; }
            
            this->SetIndexLabelObject( &(this->fObject["index_labels"]) );
            this->SetIntervalLabelObject( &(this->fObject["interval_labels"]) );
        };


        virtual ~MHO_Axis(){};

        //overload the CopyTags function to get special treatment of the index/interval labels
        virtual void CopyTags(const MHO_Axis& rhs)
        {
            if(this != &rhs && !(rhs.fObject.empty()) )
            {
                this->fObject = rhs.fObject;
                if( this->fObject.contains("index_labels") ){this->SetIndexLabelObject( &(this->fObject["index_labels"]) ); }
                if( this->fObject.contains("interval_labels") ){this->SetIntervalLabelObject( &(this->fObject["interval_labels"]) ); }
            }
        }


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

        virtual uint64_t GetSerializedSize() const override
        {
            uint64_t total_size = 0;
            total_size += sizeof(MHO_ClassVersion);
            total_size += MHO_VectorContainer< XValueType >::GetSerializedSize();
            return total_size;
        }

        //expensive copy (as opposed to the assignment operator,
        //pointers to exernally managed memory are not transferred)
        virtual void Copy(const MHO_Axis& rhs)
        {
            if(&rhs != this)
            {
                MHO_VectorContainer<XValueType>::Copy(rhs); //copy the 1-d array
                //make sure we point to the correct index_labels object
                if(this->fObject.contains("index_labels"))
                {
                    this->SetIndexLabelObject( &(this->fObject["index_labels"] ) );
                }

                if(this->fObject.contains("interval_labels"))
                {
                    this->SetIntervalLabelObject( &(this->fObject["interval_labels"] ) );
                }
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
            //make sure we point to the correct index_labels object
            if(this->fObject.contains("index_labels"))
            {
                this->SetIndexLabelObject( &(this->fObject["index_labels"] ) );
            }
            if(this->fObject.contains("interval_labels"))
            {
                this->SetIntervalLabelObject( &(this->fObject["interval_labels"] ) );
            }
        }

        template<typename XStream> void StreamOutData_V0(XStream& s) const
        {
            s << static_cast< const MHO_VectorContainer< XValueType >& >(*this);
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

#endif /*! end of include guard: MHO_Axis */
