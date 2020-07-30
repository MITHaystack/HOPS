
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

#include "HkUnits.hh"
#include "HkNamed.hh"
#include "HkArrayWrapper.hh"

#include "HkVectorContainer.hh"

namespace hops
{


template< typename XValueType, typename XUnitType = HEmptyUnit, size_t RANK >
class HkTensorContainer: public HNamed, public HkArrayWrapper< XValueType, RANK >
{
    public:

        HkTensorContainer(): HkArrayWrapper<XValueType, RANK>(){};

        HkTensorContainer( XValueType* data, const std::size_t* dim):
            HkArrayWrapper<XValueType, RANK >(data, dim)
        {

        };


        HkTensorContainer(): HkArrayWrapper< XValueType, RANK >(size_t ) {};
        virtual ~HkTensorContainer(){};


        //declare the unit type (not implemented for now)
        using unit = XUnitType;

        //have to make base class functions visible
        using HkArrayWrapper<XValueType, RANK >::SetData;
        using HkArrayWrapper<XValueType, RANK >::GetData;

        std::size_t GetArraySize() const {return 1;};

    protected:

        using HkArrayWrapper<XValueType,0>::fData;

};

}//end of hops namespace

#endif /* end of include guard: HkTensorContainer_HH__ */
