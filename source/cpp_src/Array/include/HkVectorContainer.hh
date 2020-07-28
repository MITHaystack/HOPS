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

namespace hops
{


template< typename XValueType, typename XUnitType = HEmptyUnit >
class HkVectorContainer: public HNamed, public HkArrayWrapper< XValueType, 1>
{
    public:

        HkVectorContainer(): HkArrayWrapper<XValueType, 1>(){};

        HkVectorContainer( XValueType* data, const std::size_t* dim):
            HkArrayWrapper<XValueType,1>(data, dim)
        {

        };


        HkVectorContainer(): HkArrayWrapper<XValueType, 1>(size_t ) {};
        virtual ~HkVectorContainer(){};


        //declare the unit type (not implemented for now)
        using unit = XUnitType;

        //have to make base class functions visible
        using HkArrayWrapper<XValueType,0>::SetData;
        using HkArrayWrapper<XValueType,0>::GetData;

        std::size_t GetArraySize() const {return 1;};

    protected:

        using HkArrayWrapper<XValueType,0>::fData;

};

}//end of hops namespace

#endif /* end of include guard: HkVectorContainer_HH__ */
