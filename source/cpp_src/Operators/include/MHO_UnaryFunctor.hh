#ifndef MHO_UnaryFunctor_HH__
#define MHO_UnaryFunctor_HH__

#include <algorithm>
#include "MHO_NDArrayWrapper.hh"

/*!
*@file MHO_UnaryFunctor.hh
*@class MHO_UnaryFunctor
*@author J. Barrett - barrettj@mit.edu 
*
*@date
*@brief abstract baseclass for an functor which takes array iterators
*/


namespace hops
{

template< class XArrayType >
class MHO_UnaryFunctor
{
    public:

        MHO_UnaryFunctor(){};
        virtual ~MHO_UnaryFunctor(){};

        using iterator_type = typename XArrayType::iterator;
        using citerator_type = typename XArrayType::const_iterator;

        virtual void operator() ( iterator_type& input ) = 0;
        virtual void operator() ( citerator_type& input, iterator_type& output) = 0;

};



}


#endif /*! MHO_UnaryFunctor_H__ */
