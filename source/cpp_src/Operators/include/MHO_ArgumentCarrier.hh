#ifndef MHO_ArgumentCarrier_HH__
#define MHO_ArgumentCarrier_HH__

#include "MHO_Meta.hh"
#include "MHO_Operator.hh"

#include <tuple>

namespace hops
{

/*!
 *@file MHO_ArgumentCarrier.hh
 *@class MHO_ArgumentCarrier
 *@author J. Barrett - barrettj@mit.edu
 *@brief Variadic base that stores an operator's typed arguments (pointers expected) in a tuple and
 *       exposes a protected Apply() helper for dispatching to Impl methods.
 *       All pointer slots are value-initialized to nullptr.
 */

template< typename... Args > class MHO_ArgumentCarrier: public MHO_Operator
{
    protected:
        std::tuple< Args... > fArgs;

        template< typename Func > auto Apply(Func&& func) -> decltype(mho_tuple_apply(std::forward< Func >(func), fArgs))
        {
            return mho_tuple_apply(std::forward< Func >(func), fArgs);
        }
};

} // namespace hops

#endif /*! end of include guard: MHO_ArgumentCarrier_HH__ */
