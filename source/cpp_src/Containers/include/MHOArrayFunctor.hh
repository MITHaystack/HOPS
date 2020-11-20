#ifndef MHOArrayFunctor_HH__
#define MHOArrayFunctor_HH__

#include <algorithm>

#include "MHOMessage.hh"
#include "MHOArrayWrapper.hh"
#include "MHOArrayOperator.hh"



/*
*File: MHOArrayFunctor.hh
*Class: MHOArrayFunctor
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: abstract baseclass for an functor which takes array iterators
*/


namespace hops
{

template< class XInputArrayType, class XOutputArrayType >
class MHOArrayFunctor:
{
    public:

        MHOArrayFunctor(){};
        virtual ~MHOArrayFunctor(){};

        virtual void operator( XInputArrayType::iterator& input, XOutputArrayType::iterator& output) = 0;

};

}


#endif /* MHOArrayFunctor_H__ */
