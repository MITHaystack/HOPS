#ifndef HkVectorContainer_HH__
#define HkVectorContainer_HH__

/*
*File: HkVectorContainer.hh
*Class: HkVectorContainer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:00.867Z
*Description:
*/


#include <string>

#include "HkUnits.hh"
#include "HkNamed.hh"
#include "HkArrayWrapper.hh"

namespace hops
{


template< typename XValueType, typename XUnitType = HkEmptyUnit >
class HkVectorContainer: public HkArrayWrapper< XValueType, 1>
{
    public:

        HkVectorContainer():
            HkArrayWrapper<XValueType,1>()
        {};

        HkVectorContainer(const size_t* dim):
            HkArrayWrapper<XValueType,1>(dim)
        {};

        virtual ~HkVectorContainer(){};

        //declare the unit type (not implemented for now)
        using unit = XUnitType;

        //have to make base class functions visible
        using HkArrayWrapper<XValueType,1>::GetData;
        using HkArrayWrapper<XValueType,1>::GetRawData;
        using HkArrayWrapper<XValueType,1>::GetArraySize;
        using HkArrayWrapper<XValueType,1>::GetArrayDimensions;
        using HkArrayWrapper<XValueType,1>::GetArrayDimension;
        using HkArrayWrapper<XValueType,1>::GetOffsetForIndices;

        using HkArrayWrapper<XValueType,1>::operator();
        using HkArrayWrapper<XValueType,1>::operator[];

    protected:

        using HkArrayWrapper<XValueType,1>::fData;
        using HkArrayWrapper<XValueType,1>::fDimensions;
        using HkArrayWrapper<XValueType,1>::fTotalArraySize;


};

}//end of hops namespace

#endif /* end of include guard: HkVectorContainer_HH__ */
