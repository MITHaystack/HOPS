#ifndef MHO_ScalarContainer_HH__
#define MHO_ScalarContainer_HH__

/*
*File: MHO_ScalarContainer.hh
*Class: MHO_ScalarContainer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:06.227Z
*Description:
*/


#include <string>

#include "MHO_Named.hh"
#include "MHO_NDArrayWrapper.hh"

namespace hops
{

template< typename XValueType >
class MHO_ScalarContainer: public MHO_NDArrayWrapper< XValueType, 0>, public MHO_Named
{
    public:
        MHO_ScalarContainer():
            MHO_NDArrayWrapper<XValueType, 0>(),
            MHO_Named()
        {};

        virtual ~MHO_ScalarContainer(){};

        using MHO_Named::IsNamed;
        using MHO_Named::GetName;
        using MHO_Named::SetName;

        void SetValue(const XValueType& value){fData = value;};
        XValueType GetValue(){ return fData;};

        //have to make base class functions visible
        using MHO_NDArrayWrapper<XValueType,0>::SetData;
        using MHO_NDArrayWrapper<XValueType,0>::GetData;

        std::size_t GetSize() const {return 1;};

    protected:

        using MHO_NDArrayWrapper<XValueType,0>::fData;

};

}//end of hops namespace

#endif /* end of include guard: MHO_ScalarContainer */
