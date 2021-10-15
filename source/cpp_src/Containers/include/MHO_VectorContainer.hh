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
#include "MHO_NDArrayWrapper.hh"

namespace hops
{

class MHO_VectorContainerBase{}; //only needed for SFINAE

template< typename XValueType >
class MHO_VectorContainer:
    public MHO_VectorContainerBase,
    public MHO_NDArrayWrapper< XValueType, 1>,
    virtual public MHO_Serializable
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
            MHO_NDArrayWrapper<XValueType,1>(obj)
        {};

        //clone functionality
        MHO_VectorContainer* Clone(){ return new MHO_VectorContainer(*this); }

        virtual ~MHO_VectorContainer(){};

        virtual uint64_t GetSerializedSize() const override
        {
            uint64_t size = 0;
            size += sizeof(MHO_ClassVersion);
            size += sizeof(std::size_t);
            size += fSize*sizeof(XValueType);
            return size;
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

    protected:

        using MHO_NDArrayWrapper<XValueType,1>::fData;
        using MHO_NDArrayWrapper<XValueType,1>::fDims;
        using MHO_NDArrayWrapper<XValueType,1>::fSize;


    template<typename XStream> friend XStream& operator>>(XStream& s, MHO_VectorContainer& aData)
    {
        MHO_ClassVersion vers;
        s >> vers;
        if( vers != aData.GetVersion() )
        {
            MHO_ClassIdentity::ClassVersionErrorMsg(aData, vers);
            //Flag this as an unknown object version so we can skip over this data
            MHO_ObjectStreamState<XStream>::SetUnknown(s);
        }
        else
        {
            size_t total_size[1];
            s >> total_size[0];
            aData.Resize(total_size);
            for(size_t i=0; i<aData.fSize; i++)
            {
                s >> aData.fData[i];
            }
        }
        return s;
    }

    template<typename XStream> friend XStream& operator<<(XStream& s, const MHO_VectorContainer& aData)
    {
        s << aData.GetVersion();
        s << aData.fSize;
        for(size_t i=0; i<aData.fSize; i++)
        {
            s << aData.fData[i];
        }
        return s;
    }

};

// ////////////////////////////////////////////////////////////////////////////////
//using declarations for all basic 'plain-old-data' types
using MHO_VectorBool = MHO_VectorContainer<bool>;
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
