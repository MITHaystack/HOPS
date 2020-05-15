#ifndef HScalarContainer_HH__
#define HScalarContainer_HH__

/*
*File: HScalarContainer.hh
*Class: HScalarContainer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:06.227Z
*Description:
*/


#include <string>

#include "HUnits.hh"
#include "HNamed.hh"

namespace hops
{



template< typename XValueType, typename XUnitType = HEmptyUnit >
class HScalarContainer: public HNamed, public HArrayWrapper< XValueType, 0>
{
    public:
        HScalarContainer(): HArrayWrapper<XValueType, 0>() {};
        virtual ~HScalarContainer(){};

        void SetValue(const XValueType& value){fData = fValue};
        XValueType GetValue(){ return fData;};

        //declare the unit type (not implemented for now)
        using unit = XUnitType;

        //have to make base class functions visible
        using HArrayWrapper<XValueType,0>::SetData;
        using HArrayWrapper<XValueType,0>::GetData;

        std::size_t GetArraySize() const {return 1;};

    protected:

        using HArrayWrapper<XValueType,0>::fData;

        std::string fName;

};

}//end of hops namespace

#endif /* end of include guard: HScalarContainer */
