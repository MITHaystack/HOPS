#ifndef MHO_NDArrayFunctor_HH__
#define MHO_NDArrayFunctor_HH__

#include <algorithm>
#include "MHO_NDArrayWrapper.hh"

/*
*File: MHO_NDArrayFunctor.hh
*Class: MHO_NDArrayFunctor
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: abstract baseclass for an functor which takes array iterators
*/


namespace hops
{

template< class XInputArrayType, class XOutputArrayType >
class MHO_NDArrayFunctor
{
    public:

        MHO_NDArrayFunctor(){};
        virtual ~MHO_NDArrayFunctor(){};

        using input_iterator = typename XInputArrayType::iterator;
        using output_iterator = typename XOutputArrayType::iterator;

        virtual void operator() ( input_iterator& input, output_iterator& output) = 0;

};

}


#endif /* MHO_NDArrayFunctor_H__ */
