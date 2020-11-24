#ifndef MHOArrayFunctor_HH__
#define MHOArrayFunctor_HH__

#include <algorithm>
#include "MHOArrayWrapper.hh"

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
class MHOArrayFunctor
{
    public:

        MHOArrayFunctor(){};
        virtual ~MHOArrayFunctor(){};

        using input_iterator = typename XInputArrayType::iterator;
        using output_iterator = typename XOutputArrayType::iterator;

        virtual void operator() ( input_iterator& input, output_iterator& output) = 0;

};

}


#endif /* MHOArrayFunctor_H__ */
