#ifndef MHO_NaNMasker_HH__
#define MHO_NaNMasker_HH__

#include "MHO_CheckForNaN.hh"
#include "MHO_Message.hh"
#include "MHO_UnaryFunctor.hh"
#include <cmath>
#include <complex>

namespace hops
{

/*!
 *@file MHO_NaNMasker.hh
 *@class MHO_NaNMasker
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Aug 12 11:16:36 2021 -0400
 *@brief
 */

template< class XArrayType > class MHO_NaNMasker: public MHO_UnaryFunctor< XArrayType >
{
    public:
        MHO_NaNMasker(){};
        virtual ~MHO_NaNMasker(){};

        using iterator_type = typename MHO_UnaryFunctor< XArrayType >::iterator_type;
        using citerator_type = typename MHO_UnaryFunctor< XArrayType >::citerator_type;

        virtual void operator()(iterator_type& input) override
        {
            //check the value at the input iterator
            if(MHO_CheckForNaN< typename XArrayType::value_type >::isnan(*input))
            {
                *input = 0.0; //zero out
                //msg_debug("calibration", "Replacing NaN with 0.0." << eom); //TODO more debug?
            }
        }

        virtual void operator()(citerator_type& input, iterator_type& output) override
        {
            //check the value at the input iterator
            if(MHO_CheckForNaN< typename XArrayType::value_type >::isnan(*input))
            {
                *output = 0.0; //zero out
                //msg_debug("calibration", "Replacing NaN with 0.0." << eom); //TODO more debug?
            }
            else
            {
                *output = *input;
            } //pass through
        }
};

} // namespace hops

#endif /*! end of include guard: MHO_NaNMasker */
