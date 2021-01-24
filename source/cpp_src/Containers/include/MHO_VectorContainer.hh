#ifndef MHO_VectorContainer_HH__
#define MHO_VectorContainer_HH__

/*
*File: MHO_VectorContainer.hh
*Class: MHO_VectorContainer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:00.867Z
*Description:
*/


#include <string>

#include "MHO_Named.hh"
#include "MHO_NDArrayWrapper.hh"

namespace hops
{


template< typename XValueType >
class MHO_VectorContainer: public MHO_NDArrayWrapper< XValueType, 1>, public MHO_Named
{
    public:

        MHO_VectorContainer():
            MHO_NDArrayWrapper<XValueType,1>(),
            MHO_Named()
        {};

        MHO_VectorContainer(std::size_t dim):
            MHO_NDArrayWrapper<XValueType,1>(dim),
            MHO_Named()
        {};

        //copy constructor
        MHO_VectorContainer(const MHO_VectorContainer& obj):
            MHO_NDArrayWrapper<XValueType,1>(obj),
            MHO_Named(obj)
        {};

        //clone functionality
        MHO_VectorContainer* Clone(){ return new MHO_VectorContainer(*this); }


        virtual ~MHO_VectorContainer(){};

        using MHO_Named::IsNamed;
        using MHO_Named::GetName;
        using MHO_Named::SetName;

        //have to make base class functions visible
        using MHO_NDArrayWrapper<XValueType,1>::Resize;
        using MHO_NDArrayWrapper<XValueType,1>::GetData;
        using MHO_NDArrayWrapper<XValueType,1>::GetSize;
        using MHO_NDArrayWrapper<XValueType,1>::GetDimensions;
        using MHO_NDArrayWrapper<XValueType,1>::GetDimension;
        using MHO_NDArrayWrapper<XValueType,1>::GetOffsetForIndices;
        using MHO_NDArrayWrapper<XValueType,1>::operator();
        using MHO_NDArrayWrapper<XValueType,1>::operator[];

    protected:

        using MHO_NDArrayWrapper<XValueType,1>::fData;
        using MHO_NDArrayWrapper<XValueType,1>::fDimensions;
        using MHO_NDArrayWrapper<XValueType,1>::fTotalArraySize;

};

}//end of hops namespace

#endif /* end of include guard: MHO_VectorContainer_HH__ */
