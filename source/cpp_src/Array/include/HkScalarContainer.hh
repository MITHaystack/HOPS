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

#include "HkNamed.hh"
#include "HkArrayWrapper.hh"

namespace hops
{

template< typename XValueType >
class HkScalarContainer: public HkArrayWrapper< XValueType, 0>, public HkNamed
{
    public:
        HkScalarContainer():
            HkArrayWrapper<XValueType, 0>(),
            HkNamed()
        {};

        virtual ~HkScalarContainer(){};

        using HkNamed::IsNamed;
        using HkNamed::GetName;
        using HkNamed::SetName;

        void SetValue(const XValueType& value){fData = value;};
        XValueType GetValue(){ return fData;};

        //have to make base class functions visible
        using HkArrayWrapper<XValueType,0>::SetData;
        using HkArrayWrapper<XValueType,0>::GetData;

        std::size_t GetSize() const {return 1;};

    protected:

        using HkArrayWrapper<XValueType,0>::fData;

};

}//end of hops namespace

#endif /* end of include guard: HkScalarContainer */
