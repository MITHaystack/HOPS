#ifndef MHOScalarContainer_HH__
#define MHOScalarContainer_HH__

/*
*File: MHOScalarContainer.hh
*Class: MHOScalarContainer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:06.227Z
*Description:
*/


#include <string>

#include "MHONamed.hh"
#include "MHONDArrayWrapper.hh"

namespace hops
{

template< typename XValueType >
class MHOScalarContainer: public MHONDArrayWrapper< XValueType, 0>, public MHONamed
{
    public:
        MHOScalarContainer():
            MHONDArrayWrapper<XValueType, 0>(),
            MHONamed()
        {};

        virtual ~MHOScalarContainer(){};

        using MHONamed::IsNamed;
        using MHONamed::GetName;
        using MHONamed::SetName;

        void SetValue(const XValueType& value){fData = value;};
        XValueType GetValue(){ return fData;};

        //have to make base class functions visible
        using MHONDArrayWrapper<XValueType,0>::SetData;
        using MHONDArrayWrapper<XValueType,0>::GetData;

        std::size_t GetSize() const {return 1;};

    protected:

        using MHONDArrayWrapper<XValueType,0>::fData;

};

}//end of hops namespace

#endif /* end of include guard: MHOScalarContainer */
