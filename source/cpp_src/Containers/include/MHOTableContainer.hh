#ifndef MHOTableContainer_HH__
#define MHOTableContainer_HH__

/*
*File: MHOTableContainer.hh
*Class: MHOTableContainer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:00.867Z
*Description:
*/


#include <tuple>
#include <utility>
#include <string>

#include "MHOUnits.hh"
#include "MHOMultiTypeMap.hh"
#include "MHONDArrayWrapper.hh"
#include "MHOVectorContainer.hh"
#include "MHOAxisPack.hh"


namespace hops
{

template< typename XValueType, typename XAxisPackType >
class MHOTableContainer: public MHONDArrayWrapper< XValueType, XAxisPackType::NAXES::value>, public XAxisPackType, public MHONamed
{
    public:

        MHOTableContainer():
            MHONDArrayWrapper<XValueType, XAxisPackType::NAXES::value>(),
            XAxisPackType(),
            MHONamed()
        {};

        MHOTableContainer(const std::size_t* dim):
            MHONDArrayWrapper<XValueType, XAxisPackType::NAXES::value>(dim),
            XAxisPackType(dim),
            MHONamed()
        {};

        //copy constructor
        MHOTableContainer(const MHOTableContainer& obj):
            MHONDArrayWrapper<XValueType, XAxisPackType::NAXES::value>(obj),
            XAxisPackType(obj),
            MHONamed(obj)
        {};

        //clone functionality
        MHOTableContainer* Clone(){ return new MHOTableContainer(*this); }

        virtual ~MHOTableContainer(){};

        using MHONamed::IsNamed;
        using MHONamed::GetName;
        using MHONamed::SetName;



        //modify the Resize function to also resize the axes
        using XAxisPackType::resize_axis_pack;

        virtual void Resize(const std::size_t* dim) override
        {
            MHONDArrayWrapper< XValueType, XAxisPackType::NAXES::value>::Resize(dim);
            resize_axis_pack(dim);
        }

        //have to make base class functions visible
        using MHONDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::Resize;
        using MHONDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetData;
        using MHONDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetSize;
        using MHONDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetDimensions;
        using MHONDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetDimension;
        using MHONDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetOffsetForIndices;

        using MHONDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::operator();
        using MHONDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::operator[];

    protected:

        using MHONDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::fData;
        using MHONDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::fDimensions;
        using MHONDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::fTotalArraySize;

    public:

        //temporary -- for testing
        MHOMultiTypeMap< std::string, std::string, int, double > fTags;

};

}//end of namespace

#endif /* end of include guard: MHOTableContainer_HH__ */
