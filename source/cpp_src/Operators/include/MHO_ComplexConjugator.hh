#ifndef MHO_ComplexConjugator_HH__
#define MHO_ComplexConjugator_HH__

/*!
*@file MHO_ComplexConjugator.hh
*@class MHO_ComplexConjugator
*@author J. Barrett - barrettj@mit.edu 
*
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
class MHO_ComplexConjugator: public MHO_UnaryFunctor< XArrayType >
{
    public:

        MHO_ComplexConjugator(){};
        virtual ~MHO_ComplexConjugator(){};

        using iterator_type = typename MHO_UnaryFunctor< XArrayType >::iterator_type;
        using citerator_type = typename MHO_UnaryFunctor< XArrayType >::citerator_type;


        virtual void operator() ( iterator_type& input ) override
        {
            *input = std::conj((*input));
        }

        virtual void operator() ( citerator_type& input, iterator_type& output) override
        {
            *output = std::conj((*input));
        }

};


}//end of hops namespace

#endif /*! end of include guard: MHO_ComplexConjugator */
