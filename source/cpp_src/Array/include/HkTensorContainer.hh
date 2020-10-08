#ifndef HkTensorContainer_HH__
#define HkTensorContainer_HH__

/*
*File: HkTensorContainer.hh
*Class: HkTensorContainer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:00.867Z
*Description:
*/


#include <tuple>
#include <utility>
#include <string>

#include "HkUnits.hh"
#include "HkMultiTypeMap.hh"
#include "HkArrayWrapper.hh"
#include "HkVectorContainer.hh"
#include "HkAxisPack.hh"


namespace hops
{

template< typename XValueType, typename XAxisPackType >
class HkTensorContainer: public HkArrayWrapper< XValueType, XAxisPackType::RANK::value>, public XAxisPackType, public HkNamed
{
    public:

        HkTensorContainer():
            HkArrayWrapper<XValueType,XAxisPackType::RANK::value>(),
            XAxisPackType(),
            HkNamed()
        {};

        HkTensorContainer(const std::size_t* dim):
            HkArrayWrapper<XValueType,XAxisPackType::RANK::value>(dim),
            XAxisPackType(dim),
            HkNamed()
        {};

        virtual ~HkTensorContainer(){};

        using HkNamed::IsNamed;
        using HkNamed::GetName;
        using HkNamed::SetName;

        //modify the Resize function to also resize the axes
        using XAxisPackType::resize_axis_pack;
        void Resize(const std::size_t* dim)
        {
            for(std::size_t i=0; i<XAxisPackType::RANK::value; i++)
            {
                fDimensions[i] = dim[i];
            }
            fTotalArraySize = HkArrayMath::TotalArraySize<XAxisPackType::RANK::value>(fDimensions);
            fData.resize(fTotalArraySize);

            resize_axis_pack(dim);
        }

        //have to make base class functions visible
        using HkArrayWrapper<XValueType,XAxisPackType::RANK::value>::GetData;
        using HkArrayWrapper<XValueType,XAxisPackType::RANK::value>::GetRawData;
        using HkArrayWrapper<XValueType,XAxisPackType::RANK::value>::GetSize;
        using HkArrayWrapper<XValueType,XAxisPackType::RANK::value>::GetDimensions;
        using HkArrayWrapper<XValueType,XAxisPackType::RANK::value>::GetDimension;
        using HkArrayWrapper<XValueType,XAxisPackType::RANK::value>::GetOffsetForIndices;

        using HkArrayWrapper<XValueType,XAxisPackType::RANK::value>::operator();
        using HkArrayWrapper<XValueType,XAxisPackType::RANK::value>::operator[];

    protected:

        using HkArrayWrapper<XValueType,XAxisPackType::RANK::value>::fData;
        using HkArrayWrapper<XValueType,XAxisPackType::RANK::value>::fDimensions;
        using HkArrayWrapper<XValueType,XAxisPackType::RANK::value>::fTotalArraySize;

    public:

        //temporary -- for testing
        HkMultiTypeMap< std::string, std::string, int, double > fTags;

};

}//end of namespace

#endif /* end of include guard: HkTensorContainer_HH__ */
