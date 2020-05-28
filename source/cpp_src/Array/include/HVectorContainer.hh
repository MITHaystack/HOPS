#ifndef HVectorContainer_HH__
#define HVectorContainer_HH__

/*
*File: HVectorContainer.hh
*Class: HVectorContainer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:00.867Z
*Description:
*/


#include <string>

#include "HUnits.hh"
#include "HNamed.hh"

namespace hops
{


template< typename XValueType, typename XUnitType = HEmptyUnit >
class HVectorContainer: public HNamed, public HArrayWrapper< XValueType, 1>
{
    public:

        HVectorContainer(): HArrayWrapper<XValueType, 1>(){};

        HVectorContainer( XValueType* data, const std::size_t* dim):
            HArrayWrapper<XValueType,1>(data, dim)
        {

        };


        HVectorContainer(): HArrayWrapper<XValueType, 1>(size_t ) {};
        virtual ~HVectorContainer(){};


        //declare the unit type (not implemented for now)
        using unit = XUnitType;

        //have to make base class functions visible
        using HArrayWrapper<XValueType,0>::SetData;
        using HArrayWrapper<XValueType,0>::GetData;

        std::size_t GetArraySize() const {return 1;};

    protected:

        using HArrayWrapper<XValueType,0>::fData;

};

}//end of hops namespace

#endif /* end of include guard: HVectorContainer_HH__ */
