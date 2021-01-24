#ifndef MHONDArrayFunctor_HH__
#define MHONDArrayFunctor_HH__

#include <algorithm>
#include "MHONDArrayWrapper.hh"

/*
*File: MHONDArrayFunctor.hh
*Class: MHONDArrayFunctor
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: abstract baseclass for an functor which takes array iterators
*/


namespace hops
{

template< class XInputArrayType, class XOutputArrayType >
class MHONDArrayFunctor
{
    public:

        MHONDArrayFunctor(){};
        virtual ~MHONDArrayFunctor(){};

        using input_iterator = typename XInputArrayType::iterator;
        using output_iterator = typename XOutputArrayType::iterator;

        virtual void operator() ( input_iterator& input, output_iterator& output) = 0;

};

}


#endif /* MHONDArrayFunctor_H__ */
