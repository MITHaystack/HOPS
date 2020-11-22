#ifndef MHOVRotationFunctor_HH__
#define MHOVRotationFunctor_HH__

/*
*File: MHOVRotationFunctor.hh
*Class: MHOVRotationFunctor
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include "MHOArrayFunctor.hh"
#include "MHOTensorContainer.hh"
#include "MHOChannelizedVisibilities.hh"

class MHOVRotationFunctor:
{
    public:
        MHOVRotationFunctor();
        virtual ~MHOVRotationFunctor();
    private:
};

#endif /* end of include guard: MHOVRotationFunctor */



template< class XInputArrayType, class XOutputArrayType >
class MHOArrayFunctor:
{
    public:

        MHOArrayFunctor(){};
        virtual ~MHOArrayFunctor(){};

        virtual void operator( XInputArrayType::iterator& input, XOutputArrayType::iterator& output) = 0;

};
