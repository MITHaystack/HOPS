#ifndef MHO_TableContainer_HH__
#define MHO_TableContainer_HH__

/*
*File: MHO_TableContainer.hh
*Class: MHO_TableContainer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:00.867Z
*Description:
*/


#include <tuple>
#include <utility>
#include <string>

#include "MHO_Units.hh"
#include "MHO_MultiTypeMap.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_VectorContainer.hh"
#include "MHO_Axis.hh"
#include "MHO_AxisPack.hh"


namespace hops
{

class MHO_TableContainerBase{}; //only needed for SFINAE

template< typename XValueType, typename XAxisPackType >
class MHO_TableContainer:
    public MHO_TableContainerBase,
    public MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value>,
    public XAxisPackType
{
    public:

        MHO_TableContainer():
            MHO_NDArrayWrapper<XValueType, XAxisPackType::NAXES::value>(),
            XAxisPackType()
        {};

        MHO_TableContainer(const std::size_t* dim):
            MHO_NDArrayWrapper<XValueType, XAxisPackType::NAXES::value>(dim),
            XAxisPackType(dim)
        {};

        //copy constructor
        MHO_TableContainer(const MHO_TableContainer& obj):
            MHO_NDArrayWrapper<XValueType, XAxisPackType::NAXES::value>(obj),
            XAxisPackType(obj)
        {};


        //clone entire table, contents, axes and all
        MHO_TableContainer* Clone(){ return new MHO_TableContainer(*this); }

        //clone table shape, but leave contents/axes empty
        MHO_TableContainer* CloneEmpty(){ return new MHO_TableContainer( this->fDims ); }

        virtual ~MHO_TableContainer(){};

        virtual uint64_t GetSerializedSize() const override
        {
            //TODO FIXME
            uint64_t total_size = 0;
            total_size += sizeof(MHO_ClassVersion);
            total_size += XAxisPackType::NAXES::value*sizeof(std::size_t);
            total_size += XAxisPackType::GetSerializedSize();
            total_size += (this->fSize)*sizeof(XValueType);
            return total_size;
        }

        //modify the Resize function to also resize the axes
        using XAxisPackType::resize_axis_pack;

        virtual void Resize(const std::size_t* dim) override
        {
            MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value>::Resize(dim);
            resize_axis_pack(dim);
        }

        //access to axis pack type alone
        XAxisPackType* GetAxisPack() {return this;}
        const XAxisPackType* GetAxisPack() const {return this;}

        //have to make base class functions visible
        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::Resize;
        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetData;
        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetSize;
        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetDimensions;
        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetDimension;
        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetOffsetForIndices;

        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::operator();
        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::operator[];


        //expensive copy (as opposed to the assignment operator,
        //pointers to exernally managed memory are not transfer)
        virtual void Copy(const MHO_TableContainer& rhs)
        {
            //copy the array, then copy the axis pack
            MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::Copy(rhs);
            *( this->GetAxisPack() ) = *(rhs.GetAxisPack());
        }

    protected:

        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::fData;
        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::fDims;
        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::fSize;

    public:

        //temporary -- for testing
        //MHO_MultiTypeMap< std::string, std::string, int, double > fTags;

        template<typename XStream> friend XStream& operator<<(XStream& s, const MHO_TableContainer& aData)
        {
            //first stream version and dimensions
            s << aData.GetVersion();
            for(size_t i=0; i < XAxisPackType::NAXES::value; i++)
            {
                s << aData.fDims[i];
            }
            //then dump axes
            s << static_cast< const XAxisPackType& >(aData);
            //finally dump the array data
            for(size_t i=0; i<aData.fSize; i++)
            {
                s << aData.fData[i];
            }
            return s;
        }

        template<typename XStream> friend XStream& operator>>(XStream& s, MHO_TableContainer& aData)
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
                //first stream the axis-pack
                std::size_t dims[XAxisPackType::NAXES::value];
                for(size_t i=0; i < XAxisPackType::NAXES::value; i++)
                {
                    s >> dims[i];
                }
                aData.Resize(dims);
                //now stream in the axes
                s >> static_cast< XAxisPackType& >(aData);
                //now stream the mult-dim array data
                for(size_t i=0; i<aData.fSize; i++)
                {
                    s >> aData.fData[i];
                }
            }
            return s;
        }

};


////////////////////////////////////////////////////////////////////////////////
//enumerate all of the table container types we might possibly use, this is not
//an exhaustive list and many other basic (POD)-based types are possible, if they
//are POD-based then additions should be listed here. If there are there are additional
//table container types which involve complicated classes or structs (which would
//would require an '#include' in this file, then they should be defined elsewhere.
//Since we are manily concerned with data (visibility and or cal) we only define
//tables for the following types:

using Int = int;
using Double = double;
using ComplexD = std::complex<double>;
using ComplexF = std::complex<float>;

#define DefTableContainers(TYPE)                                               \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int =                           \
MHO_TableContainer< TYPE, MHO_AxisPack_Int >;                                  \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double =                        \
MHO_TableContainer< TYPE, MHO_AxisPack_Double >;                               \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String =                        \
MHO_TableContainer< TYPE, MHO_AxisPack_String >;                               \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int =                       \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int >;                              \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_String >;                           \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_String_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_Int =                       \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_Int >;                              \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_Double =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_Double >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_String =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_String >;                           \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_Int_AxisPack_String_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_Int =                       \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_Int >;                              \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_Double =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_Double >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_String =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_String >;                           \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_Int =                       \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_Int >;                              \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_Double =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_Double >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_String =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_String >;                           \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_String_String_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_String_String_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_String_String_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_Int_Int =                       \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_Int_Int >;                              \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_Int_Double =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_Int_Double >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_Int_String =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_Int_String >;                           \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_Double_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_Double_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_Double_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_Double_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_Double_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_Double_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_String_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_String_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_String_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_String_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_String_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_Int_Int =                       \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_Int_Int >;                              \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_Int_Double =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_Int_Double >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_Int_String =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_Int_String >;                           \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_Double_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_Double_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_Double_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_Double_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_Double_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_Double_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_String_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_String_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_String_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_String_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_String_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_String_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_Int_Int =                       \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_Int_Int >;                              \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_Int_Double =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_Int_Double >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_Int_String =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_Int_String >;                           \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_Double_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_Double_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_Double_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_Double_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_Double_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_Double_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_String_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_String_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_String_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_String_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_String_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_String_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_Int_Int =                       \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_Int_Int >;                              \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_Int_Double =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_Int_Double >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_Int_String =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_Int_String >;                           \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_Double_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_Double_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_Double_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_Double_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_Double_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_Double_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_String_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_String_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_String_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_String_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_String_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_String_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_Int_Int =                       \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_Int_Int >;                              \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_Int_Double =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_Int_Double >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_Int_String =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_Int_String >;                           \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_Double_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_Double_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_Double_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_Double_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_Double_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_Double_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_String_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_String_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_String_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_String_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_String_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_String_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_Int_Int =                       \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_Int_Int >;                              \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_Int_Double =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_Int_Double >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_Int_String =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_Int_String >;                           \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_Double_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_Double_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_Double_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_Double_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_Double_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_Double_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_String_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_String_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_String_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_String_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_String_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_String_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_Int_Int =                       \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_Int_Int >;                              \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_Int_Double =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_Int_Double >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_Int_String =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_Int_String >;                           \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_Double_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_Double_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_Double_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_Double_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_Double_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_Double_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_String_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_String_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_String_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_String_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_String_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_String_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_Int_Int =                       \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_Int_Int >;                              \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_Int_Double =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_Int_Double >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_Int_String =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_Int_String >;                           \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_Double_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_Double_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_Double_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_Double_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_Double_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_Double_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_String_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_String_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_String_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_String_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_String_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_String_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_Int_Int =                       \
MHO_TableContainer< TYPE, MHO_AxisPack_String_String_Int_Int >;                              \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_Int_Double =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_String_String_Int_Double >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_Int_String =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_String_String_Int_String >;                           \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_Double_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_String_String_Double_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_Double_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_String_String_Double_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_Double_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_String_String_Double_String >;                        \
\
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_String_Int =                    \
MHO_TableContainer< TYPE, MHO_AxisPack_String_String_String_Int >;                           \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_String_Double =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_String_String_String_Double >;                        \
using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_String_String =                 \
MHO_TableContainer< TYPE, MHO_AxisPack_String_String_String_String >;

DefTableContainers(Int);
DefTableContainers(Double);
DefTableContainers(ComplexD);
DefTableContainers(ComplexF);


}//end of namespace

#endif /* end of include guard: MHO_TableContainer_HH__ */
