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

template< typename XValueType, size_t RANK, typename XAxisPackType >
class HkTensorContainer: public HkArrayWrapper< XValueType, RANK>, public XAxisPackType, public HkNamed
{
    public:

        HkTensorContainer():
            HkArrayWrapper<XValueType,RANK>(),
            XAxisPackType(),
            HkNamed()
        {};

        HkTensorContainer(const size_t* dim):
            HkArrayWrapper<XValueType,RANK>(dim),
            XAxisPackType(dim),
            HkNamed()
        {};

        virtual ~HkTensorContainer(){};

        using HkNamed::IsNamed;
        using HkNamed::GetName;
        using HkNamed::SetName;

        //modify the Resize function to also resize the axes
        using XAxisPackType::resize_axis_pack;
        void Resize(const size_t* dim)
        {
            for(size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = dim[i];
            }
            fTotalArraySize = HkArrayMath::TotalArraySize<RANK>(fDimensions);
            fData.resize(fTotalArraySize);

            resize_axis_pack(dim);
        }

        //have to make base class functions visible
        using HkArrayWrapper<XValueType,RANK>::GetData;
        using HkArrayWrapper<XValueType,RANK>::GetRawData;
        using HkArrayWrapper<XValueType,RANK>::GetSize;
        using HkArrayWrapper<XValueType,RANK>::GetDimensions;
        using HkArrayWrapper<XValueType,RANK>::GetDimension;
        using HkArrayWrapper<XValueType,RANK>::GetOffsetForIndices;

        using HkArrayWrapper<XValueType,RANK>::operator();
        using HkArrayWrapper<XValueType,RANK>::operator[];

    protected:

        using HkArrayWrapper<XValueType,RANK>::fData;
        using HkArrayWrapper<XValueType,RANK>::fDimensions;
        using HkArrayWrapper<XValueType,RANK>::fTotalArraySize;

    public:


        // //experimental map for axis types
        // HkMultiTypeMap< size_t, XAxesTypeS... > fAxisMap;


};

}//end of namespace

#endif /* end of include guard: HkTensorContainer_HH__ */
