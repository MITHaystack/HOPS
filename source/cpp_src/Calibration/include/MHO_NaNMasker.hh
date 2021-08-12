#ifndef MHO_NaNMasker_HH__
#define MHO_NaNMasker_HH__

/*
*File: MHO_NaNMasker.hh
*Class: MHO_NaNMasker
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <cmath>
#include <complex>
#include "MHO_CheckForNaN.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayFunctor.hh"

namespace hops
{


template< class XInputArrayType, class XOutputArrayType >
class MHO_NaNMasker: public MHO_NDArrayFunctor< XInputArrayType, XOutputArrayType >
{
    public:

        MHO_NaNMasker(){};
        virtual ~MHO_NaNMasker(){};

        using input_iterator = typename MHO_NDArrayFunctor< XInputArrayType, XOutputArrayType >::input_iterator;
        using output_iterator = typename MHO_NDArrayFunctor< XInputArrayType, XOutputArrayType >::output_iterator;

        //replaces all NaNs with zero
        virtual void operator() ( input_iterator& input, output_iterator& output) override
        {
            //check the value at the input iterator
            if( MHO_CheckForNaN< typename XInputArrayType::value_type >::isnan(*input) )
            {
                *output = 0.0; //zero out
                msg_debug("calibration", "Replacing NaN with 0.0." << eom); //TODO more debug?
            }
            else{ *output = *input; } //pass through 
        }

};


}//end of hops namespace

#endif /* end of include guard: MHO_NaNMasker */
