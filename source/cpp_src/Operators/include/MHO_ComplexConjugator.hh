#ifndef MHO_ComplexConjugator_HH__
#define MHO_ComplexConjugator_HH__

/*
*File: MHO_ComplexConjugator.hh
*Class: MHO_ComplexConjugator
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <cmath>
#include <complex>
#include "MHO_Message.hh"
#include "MHO_NDArrayFunctor.hh"

namespace hops
{

template< class XInputArrayType, class XOutputArrayType >
class MHO_ComplexConjugator: public MHO_NDArrayFunctor< XInputArrayType, XOutputArrayType >
{
    public:

        MHO_ComplexConjugator(){};
        virtual ~MHO_ComplexConjugator(){};

        using input_iterator = typename MHO_NDArrayFunctor< XInputArrayType, XOutputArrayType >::input_iterator;
        using output_iterator = typename MHO_NDArrayFunctor< XInputArrayType, XOutputArrayType >::output_iterator;

        //multiply by the scalar factor 
        virtual void operator() ( input_iterator& input, output_iterator& output) override
        {
            *output = std::conj((*input)); 
        }

};


}//end of hops namespace

#endif /* end of include guard: MHO_ComplexConjugator */
