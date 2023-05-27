#ifndef MHO_VectorContainer_HH__
#define MHO_VectorContainer_HH__

/*
*File: MHO_VectorContainer.hh
*Class: MHO_VectorContainer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:00.867Z
*Description:
*/


#include <string>
#include <complex>

#include "MHO_Serializable.hh"
#include "MHO_Taggable.hh"
#include "MHO_NDArrayWrapper.hh"

namespace hops
{

class MHO_VectorContainerBase{};  //only needed for dependent template specializations

template< typename XValueType >
class MHO_VectorContainer:
    public MHO_VectorContainerBase,
    public MHO_NDArrayWrapper< XValueType, 1>,
    public MHO_Taggable
{
    public:

        MHO_VectorContainer():
            MHO_NDArrayWrapper<XValueType,1>()
        {};

        MHO_VectorContainer(std::size_t dim):
            MHO_NDArrayWrapper<XValueType,1>(dim)
        {};

        MHO_VectorContainer(std::size_t* dim):
            MHO_NDArrayWrapper<XValueType,1>(dim)
        {};

        //copy constructor
        MHO_VectorContainer(const MHO_VectorContainer& obj):
            MHO_NDArrayWrapper<XValueType,1>(obj),
            MHO_Taggable(obj)
        {};

        //clone functionality
        MHO_VectorContainer* Clone(){ return new MHO_VectorContainer(*this); }

        virtual ~MHO_VectorContainer(){};

        virtual MHO_ClassVersion GetVersion() const override {return 0;};

        virtual uint64_t GetSerializedSize() const override
        {
            return ComputeSerializedSize();
        }

        //have to make base class functions visible
        using MHO_NDArrayWrapper<XValueType,1>::Resize;
        using MHO_NDArrayWrapper<XValueType,1>::GetData;
        using MHO_NDArrayWrapper<XValueType,1>::GetSize;
        using MHO_NDArrayWrapper<XValueType,1>::GetDimensions;
        using MHO_NDArrayWrapper<XValueType,1>::GetDimension;
        using MHO_NDArrayWrapper<XValueType,1>::GetOffsetForIndices;
        using MHO_NDArrayWrapper<XValueType,1>::operator();
        using MHO_NDArrayWrapper<XValueType,1>::operator[];

        //expensive copy
        //pointers to exernally managed memory are not transferred)
        virtual void Copy(const MHO_VectorContainer& rhs)
        {
            if(&rhs != this)
            {
                //copy the array
                MHO_NDArrayWrapper<XValueType,1>::Copy(rhs);
                //then copy the tags 
                this->CopyTags(rhs);
            }
        }

    public:

        uint64_t ComputeSerializedSize() const
        {
            uint64_t total_size = 0;
            total_size += sizeof(MHO_ClassVersion);
            total_size += MHO_Taggable::GetSerializedSize();
            total_size += sizeof(uint64_t);
            total_size += this->GetSize()*sizeof(XValueType); //all elements have the same size
            return total_size;
        }

        template<typename XStream> friend XStream& operator>>(XStream& s, MHO_VectorContainer& aData)
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


        template<typename XStream> friend XStream& operator<<(XStream& s, const MHO_VectorContainer& aData)
        {
            switch( aData.GetVersion() ) 
            {
                case 0:
                    s << aData.GetVersion();
                    aData.StreamOutData_V0(s);
                break;
                default:
                    msg_error("containers", 
                        "error, cannot stream out MHO_VectorContainer object with unknown version: " 
                        << aData.GetVersion() << eom );
            }
            return s;
        }

    private:

        template<typename XStream> void StreamOutData_V0(XStream& s) const
        {
            s << static_cast<const MHO_Taggable& >(*this);
            uint64_t dsize = this->GetSize();
            s << (uint64_t) dsize;
            auto data_ptr = this->GetData();
            for(size_t i=0; i<dsize; i++)
            {
                s << data_ptr[i];
            }
        }

        template<typename XStream> void StreamInData_V0(XStream& s)
        {
            s >> static_cast< MHO_Taggable& >(*this);
            size_t total_size[1];
            s >> total_size[0];
            this->Resize(total_size);
            auto data_ptr = this->GetData();
            for(size_t i=0; i<total_size[0]; i++)
            {
                s >> data_ptr[i];
            }
        }

};


//specialization for string elements 
//(NOTE: we need to use 'inline' to satisfy one-definiton rule, otherwise we have to stash this in a .cc file)
template<>
inline uint64_t 
MHO_VectorContainer<std::string>::ComputeSerializedSize() const
{
    uint64_t total_size = 0;
    total_size += sizeof(MHO_ClassVersion);
    total_size += MHO_Taggable::GetSerializedSize();
    total_size += sizeof(uint64_t);
    std::size_t dsize = this->GetSize();
    auto data_ptr = this->GetData();
    for(size_t i=0; i<dsize; i++)
    {
        total_size += sizeof(uint64_t); //every string get streamed with a size
        total_size += data_ptr[i].size(); //n char in each string
    }
    return total_size;
}





// ////////////////////////////////////////////////////////////////////////////////
//using declarations for all basic 'plain-old-data' types (except bool!)
using MHO_VectorChar = MHO_VectorContainer<char>;
using MHO_VectorUChar = MHO_VectorContainer<unsigned char>;
using MHO_VectorShort = MHO_VectorContainer<short>;
using MHO_VectorUShort = MHO_VectorContainer<unsigned short>;
using MHO_VectorInt = MHO_VectorContainer<int>;
using MHO_VectorUInt = MHO_VectorContainer<unsigned int>;
using MHO_VectorLong = MHO_VectorContainer<long>;
using MHO_VectorULong = MHO_VectorContainer<unsigned long>;
using MHO_VectorLongLong = MHO_VectorContainer<long long>;
using MHO_VectorULongLong = MHO_VectorContainer<unsigned long long>;
using MHO_VectorFloat = MHO_VectorContainer<float>;
using MHO_VectorDouble = MHO_VectorContainer<double>;
using MHO_VectorLongDouble = MHO_VectorContainer<long double>;
using MHO_VectorComplexFloat = MHO_VectorContainer< std::complex<float> >;
using MHO_VectorComplexDouble = MHO_VectorContainer< std::complex<double> >;
using MHO_VectorComplexLongDouble = MHO_VectorContainer< std::complex<long double> >;
using MHO_VectorString = MHO_VectorContainer< std::string >;

////////////////////////////////////////////////////////////////////////////////

}//end of hops namespace

#endif /* end of include guard: MHO_VectorContainer_HH__ */
