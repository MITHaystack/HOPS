#ifndef HkScalarContainer_HH__
#define HkScalarContainer_HH__

/*
*File: HkScalarContainer.hh
*Class: HkScalarContainer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:06.227Z
*Description:
*/


#include <string>

#include "HkUnits.hh"
#include "HkNamed.hh"

#include "HkArrayWrapper.hh"

namespace hops
{



template< typename XValueType, typename XUnitType = HEmptyUnit >
class HkScalarContainer: public HNamed, public HkArrayWrapper< XValueType, 0>
{
    public:
        HkScalarContainer(): HkArrayWrapper<XValueType, 0>() {};
        virtual ~HkScalarContainer(){};

        void SetValue(const XValueType& value){fData = value;};
        XValueType GetValue(){ return fData;};

        //declare the unit type (not implemented for now)
        using unit = XUnitType;

        //have to make base class functions visible
        using HkArrayWrapper<XValueType,0>::SetData;
        using HkArrayWrapper<XValueType,0>::GetData;

        std::size_t GetArraySize() const {return 1;};

    protected:

        using HkArrayWrapper<XValueType,0>::fData;

        std::string fName;

};

}//end of hops namespace

#endif /* end of include guard: HkScalarContainer */
