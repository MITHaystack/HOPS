#ifndef MHO_AbsoluteValue_HH__
#define MHO_AbsoluteValue_HH__

#include "MHO_Message.hh"
#include "MHO_UnaryFunctor.hh"
#include <cmath>
#include <complex>

namespace hops
{

/*!
 *@file MHO_AbsoluteValue.hh
 *@class MHO_AbsoluteValue
 *@author J. Barrett - barrettj@mit.edu
 *@date Wed Aug 18 12:02:32 2021 -0400
 *@brief takes the absolute value of every element in the XArrayType
 */

/**
 * @brief Class MHO_AbsoluteValue
 */
template< class XArrayType > class MHO_AbsoluteValue: public MHO_UnaryFunctor< XArrayType >
{
    public:
        MHO_AbsoluteValue(){};
        virtual ~MHO_AbsoluteValue(){};

        using iterator_type = typename MHO_UnaryFunctor< XArrayType >::iterator_type;
        using citerator_type = typename MHO_UnaryFunctor< XArrayType >::citerator_type;

        virtual void operator()(iterator_type& input) override { *input = std::abs((*input)); }

        virtual void operator()(citerator_type& input, iterator_type& output) override { *output = std::abs((*input)); }
};

} // namespace hops

#endif /*! end of include guard: MHO_AbsoluteValue */
