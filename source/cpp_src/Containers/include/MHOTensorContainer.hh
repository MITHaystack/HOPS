#ifndef MHOTensorContainer_HH__
#define MHOTensorContainer_HH__

/*
*File: MHOTensorContainer.hh
*Class: MHOTensorContainer
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
#include "MHOArrayWrapper.hh"
#include "MHOVectorContainer.hh"
#include "MHOAxisPack.hh"


namespace hops
{

template< typename XValueType, typename XAxisPackType >
class MHOTensorContainer: public MHOArrayWrapper< XValueType, XAxisPackType::NAXES::value>, public XAxisPackType, public MHONamed
{
    public:

        MHOTensorContainer():
            MHOArrayWrapper<XValueType, XAxisPackType::NAXES::value>(),
            XAxisPackType(),
            MHONamed()
        {};

        MHOTensorContainer(const std::size_t* dim):
            MHOArrayWrapper<XValueType, XAxisPackType::NAXES::value>(dim),
            XAxisPackType(dim),
            MHONamed()
        {};

        virtual ~MHOTensorContainer(){};

        using MHONamed::IsNamed;
        using MHONamed::GetName;
        using MHONamed::SetName;

        //modify the Resize function to also resize the axes
        using XAxisPackType::resize_axis_pack;

        virtual void Resize(const std::size_t* dim) override
        {
            std::cout<<"in derived call"<<std::endl;
            MHOArrayWrapper< XValueType, XAxisPackType::NAXES::value>::Resize(dim);
            resize_axis_pack(dim);
        }

        //have to make base class functions visible
        using MHOArrayWrapper<XValueType,XAxisPackType::NAXES::value>::Resize;
        using MHOArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetData;
        using MHOArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetSize;
        using MHOArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetDimensions;
        using MHOArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetDimension;
        using MHOArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetOffsetForIndices;

        using MHOArrayWrapper<XValueType,XAxisPackType::NAXES::value>::operator();
        using MHOArrayWrapper<XValueType,XAxisPackType::NAXES::value>::operator[];

    protected:

        using MHOArrayWrapper<XValueType,XAxisPackType::NAXES::value>::fData;
        using MHOArrayWrapper<XValueType,XAxisPackType::NAXES::value>::fDimensions;
        using MHOArrayWrapper<XValueType,XAxisPackType::NAXES::value>::fTotalArraySize;

    public:

        //temporary -- for testing
        MHOMultiTypeMap< std::string, std::string, int, double > fTags;

};

}//end of namespace

#endif /* end of include guard: MHOTensorContainer_HH__ */
