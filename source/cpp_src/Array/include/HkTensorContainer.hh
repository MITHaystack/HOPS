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

#include "HkUnits.hh"
#include "HkArrayWrapper.hh"
#include "HkVectorContainer.hh"

namespace hops
{

template< typename XValueType, size_t RANK >
class HkTensorContainer: public HkArrayWrapper< XValueType, RANK>, public HkNamed
{
    public:

        HkTensorContainer():
            HkArrayWrapper<XValueType,RANK>(),
            HkNamed()
        {};

        HkTensorContainer(const size_t* dim):
            HkArrayWrapper<XValueType,RANK>(dim)
        {};

        virtual ~HkTensorContainer(){};

        using HkNamed::IsNamed;
        using HkNamed::GetName;
        using HkNamed::SetName;

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


};

}//end of namespace

#endif /* end of include guard: HkTensorContainer_HH__ */
