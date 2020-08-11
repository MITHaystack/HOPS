
//
// template< typename XValueType, typename XUnitType, size_t RANK, typename... XAxisTypes >
// class TensorContainer {
//     public:
//         TensorContainer();
//         virtual ~TensorContainer();
//         //...TBD impl...
//     private:
//         std::string fName;
//         XUnitType fUnit; //units of the data
//         std::vector< XValueType > fData; //row-indexed block of data
//         std::tuple< XAxisTypes > fAxes; //tuple of length RANK of VectorContainers
// };
//



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


template< typename XValueType, typename XUnitType, size_t RANK, class... XAxisTypeS >
class HkTensorContainer: public HkArrayWrapper< XValueType, RANK>
{
    public:

        HkTensorContainer():
            HkArrayWrapper<XValueType,RANK>()
        {};

        HkTensorContainer( XValueType* data, const std::size_t* dim):
            HkArrayWrapper<XValueType,RANK>(data, dim)
        {};

        HkTensorContainer(const size_t* dim):
            HkArrayWrapper<XValueType,RANK>(dim)
        {};

        virtual ~HkTensorContainer(){};

        //declare the unit type (not implemented for now)
        using unit = XUnitType;

        //have to make base class functions visible
        using HkArrayWrapper<XValueType,RANK>::SetData;
        using HkArrayWrapper<XValueType,RANK>::GetData;
        using HkArrayWrapper<XValueType,RANK>::GetArraySize;
        using HkArrayWrapper<XValueType,RANK>::SetArrayDimensions;
        using HkArrayWrapper<XValueType,RANK>::GetArrayDimensions;
        using HkArrayWrapper<XValueType,RANK>::GetArrayDimension;
        using HkArrayWrapper<XValueType,RANK>::GetOffsetForIndices;

    protected:

        using HkArrayWrapper<XValueType,RANK>::fData;
        using HkArrayWrapper<XValueType,RANK>::fDimensions;
        using HkArrayWrapper<XValueType,RANK>::fTotalArraySize;

        std::tuple< XAxisTypeS... > fAxes;


};

}//end of namespace

#endif /* end of include guard: HkTensorContainer_HH__ */
