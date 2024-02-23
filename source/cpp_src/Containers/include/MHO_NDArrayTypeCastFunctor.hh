#ifndef MHO_NDArrayTypeCastFunctor_HH__
#define MHO_NDArrayTypeCastFunctor_HH__

/*!
*@file MHO_NDArrayTypeCastFunctor.hh
*@class MHO_NDArrayTypeCastFunctor
*@author J. Barrett - barrettj@mit.edu
*@date
*@brief functor to cast array items from one type to another
*/



#include <algorithm>
#include "MHO_NDArrayWrapper.hh"


namespace hops
{

template< class XInputArrayType, class XOutputArrayType >
class MHO_NDArrayTypeCastFunctor
{
    public:

        MHO_NDArrayTypeCastFunctor(){};
        virtual ~MHO_NDArrayTypeCastFunctor(){};

        using input_iterator = typename XInputArrayType::iterator;
        using output_iterator = typename XOutputArrayType::iterator;

        using input_value_type = typename XInputArrayType::value_type;
        using output_value_type = typename XOutputArrayType::value_type;

        virtual void operator() ( input_iterator& input, output_iterator& output)
        {
            //cast input type to output type
            *output = std::static_cast<output_value_type>(*input);
        }

};

}


#endif /*! MHO_NDArrayTypeCastFunctor_H__ */
