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


#include <string>
#include <tuple>

#include "HkUnits.hh"
#include "HkArrayWrapper.hh"
#include "HkVectorContainer.hh"

namespace hops
{


template< typename XValueType, typename XUnitType, size_t RANK >
class HkTensorContainer: public HkArrayWrapper< XValueType, RANK>
{
    public:

        HkTensorContainer():
            HkArrayWrapper<XValueType,RANK>()
        {};

        HkTensorContainer(const size_t* dim):
            HkArrayWrapper<XValueType,RANK>(dim)
        {};

        virtual ~HkTensorContainer(){};

        //declare the unit type (not implemented for now)
        using unit = XUnitType;

        //have to make base class functions visible
        using HkArrayWrapper<XValueType,RANK>::GetData;
        using HkArrayWrapper<XValueType,RANK>::GetRawData;
        using HkArrayWrapper<XValueType,RANK>::GetArraySize;
        using HkArrayWrapper<XValueType,RANK>::GetArrayDimensions;
        using HkArrayWrapper<XValueType,RANK>::GetArrayDimension;
        using HkArrayWrapper<XValueType,RANK>::GetOffsetForIndices;

        using HkArrayWrapper<XValueType,RANK>::operator();
        using HkArrayWrapper<XValueType,RANK>::operator[];

    protected:

        using HkArrayWrapper<XValueType,RANK>::fData;
        using HkArrayWrapper<XValueType,RANK>::fDimensions;
        using HkArrayWrapper<XValueType,RANK>::fTotalArraySize;


};

}//end of namespace

#endif /* end of include guard: HkTensorContainer_HH__ */
