#ifndef MHO_AbsoluteValue_HH__
#define MHO_AbsoluteValue_HH__

/*!
*@file MHO_AbsoluteValue.hh
*@class MHO_AbsoluteValue
*@author J. Barrett - barrettj@mit.edu
*@date
*@brief
*/

#include <cmath>
#include <complex>
#include "MHO_Message.hh"
#include "MHO_UnaryFunctor.hh"

namespace hops
{

template< class XArrayType >
class MHO_AbsoluteValue: public MHO_UnaryFunctor< XArrayType >
{
    public:

        MHO_AbsoluteValue(){};
        virtual ~MHO_AbsoluteValue(){};

        using iterator_type = typename MHO_UnaryFunctor< XArrayType >::iterator_type;
        using citerator_type = typename MHO_UnaryFunctor< XArrayType >::citerator_type;


        virtual void operator() ( iterator_type& input ) override
        {
            *input = std::abs((*input));
        }

        virtual void operator() ( citerator_type& input, iterator_type& output) override
        {
            *output = std::abs((*input));
        }

};


}//end of hops namespace

#endif /*! end of include guard: MHO_AbsoluteValue */
