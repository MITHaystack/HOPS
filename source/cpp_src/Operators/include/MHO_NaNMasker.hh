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
#include "MHO_UnaryFunctor.hh"

namespace hops
{


template< class XArrayType>
class MHO_NaNMasker: public MHO_UnaryFunctor< XArrayType >
{
    public:

        MHO_NaNMasker(){};
        virtual ~MHO_NaNMasker(){};

        using iterator_type = typename MHO_UnaryFunctor< XArrayType >::iterator_type;
        using citerator_type = typename MHO_UnaryFunctor< XArrayType >::citerator_type;

        virtual void operator() ( iterator_type& input ) override
        {
            //check the value at the input iterator
            if( MHO_CheckForNaN< typename XArrayType::value_type >::isnan(*input) )
            {
                *input = 0.0; //zero out
                //msg_debug("calibration", "Replacing NaN with 0.0." << eom); //TODO more debug?
            }
        }

        virtual void operator() ( citerator_type& input, iterator_type& output) override
        {
            //check the value at the input iterator
            if( MHO_CheckForNaN< typename XArrayType::value_type >::isnan(*input) )
            {
                *output = 0.0; //zero out
                //msg_debug("calibration", "Replacing NaN with 0.0." << eom); //TODO more debug?
            }
            else{ *output = *input; } //pass through
        }



};


}//end of hops namespace

#endif /* end of include guard: MHO_NaNMasker */
