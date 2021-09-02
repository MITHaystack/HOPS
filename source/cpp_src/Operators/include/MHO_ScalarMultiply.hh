#ifndef MHO_ScalarMultiply_HH__
#define MHO_ScalarMultiply_HH__

/*
*File: MHO_ScalarMultiply.hh
*Class: MHO_ScalarMultiply
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


template< typename XFactorType, class XInputArrayType, class XOutputArrayType >
class MHO_ScalarMultiply: public MHO_NDArrayFunctor< XInputArrayType, XOutputArrayType >
{
    public:

        MHO_ScalarMultiply()
        {
            fFactor = 0.0;
        };
        virtual ~MHO_ScalarMultiply(){};

        void SetFactor(XFactorType factor){fFactor = factor;};
        XFactorType GetFactor() const {return fFactor;};

        using input_iterator = typename MHO_NDArrayFunctor< XInputArrayType, XOutputArrayType >::input_iterator;
        using output_iterator = typename MHO_NDArrayFunctor< XInputArrayType, XOutputArrayType >::output_iterator;

        //multiply by the scalar factor 
        virtual void operator() ( input_iterator& input, output_iterator& output) override
        {
            *output = (*input)*fFactor; 
        }

    private:

        XFactorType fFactor;
};


}//end of hops namespace

#endif /* end of include guard: MHO_ScalarMultiply */
