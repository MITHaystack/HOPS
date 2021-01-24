#ifndef MHOVectorContainer_HH__
#define MHOVectorContainer_HH__

/*
*File: MHOVectorContainer.hh
*Class: MHOVectorContainer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:00.867Z
*Description:
*/


#include <string>

#include "MHONamed.hh"
#include "MHONDArrayWrapper.hh"

namespace hops
{


template< typename XValueType >
class MHOVectorContainer: public MHONDArrayWrapper< XValueType, 1>, public MHONamed
{
    public:

        MHOVectorContainer():
            MHONDArrayWrapper<XValueType,1>(),
            MHONamed()
        {};

        MHOVectorContainer(std::size_t dim):
            MHONDArrayWrapper<XValueType,1>(dim),
            MHONamed()
        {};

        //copy constructor
        MHOVectorContainer(const MHOVectorContainer& obj):
            MHONDArrayWrapper<XValueType,1>(obj),
            MHONamed(obj)
        {};

        //clone functionality
        MHOVectorContainer* Clone(){ return new MHOVectorContainer(*this); }


        virtual ~MHOVectorContainer(){};

        using MHONamed::IsNamed;
        using MHONamed::GetName;
        using MHONamed::SetName;

        //have to make base class functions visible
        using MHONDArrayWrapper<XValueType,1>::Resize;
        using MHONDArrayWrapper<XValueType,1>::GetData;
        using MHONDArrayWrapper<XValueType,1>::GetSize;
        using MHONDArrayWrapper<XValueType,1>::GetDimensions;
        using MHONDArrayWrapper<XValueType,1>::GetDimension;
        using MHONDArrayWrapper<XValueType,1>::GetOffsetForIndices;
        using MHONDArrayWrapper<XValueType,1>::operator();
        using MHONDArrayWrapper<XValueType,1>::operator[];

    protected:

        using MHONDArrayWrapper<XValueType,1>::fData;
        using MHONDArrayWrapper<XValueType,1>::fDimensions;
        using MHONDArrayWrapper<XValueType,1>::fTotalArraySize;

};

}//end of hops namespace

#endif /* end of include guard: MHOVectorContainer_HH__ */
