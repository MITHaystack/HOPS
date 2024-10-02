#ifndef MHO_UnaryInPlaceOperator_HH__
#define MHO_UnaryInPlaceOperator_HH__

#include "MHO_Operator.hh"

#include <tuple>

namespace hops
{

/*!
 *@file MHO_UnaryInPlaceOperator.hh
 *@class MHO_UnaryInPlaceOperator
 *@author J. Barrett - barrettj@mit.edu
 *@date Mon Jul 31 14:54:52 2023 -0400
 *@brief
 */

//only operates on a single array, input = ouput
template< class XArgType > class MHO_UnaryInPlaceOperator: public MHO_Operator
{
    public:
        MHO_UnaryInPlaceOperator() { std::get< 0 >(fInPlaceArgs) = nullptr; };

        virtual ~MHO_UnaryInPlaceOperator(){};

        virtual void SetArgs(XArgType* in) { fInPlaceArgs = std::make_tuple(in); }

        virtual bool Initialize() override { return InitializeInPlace(std::get< 0 >(fInPlaceArgs)); }

        virtual bool Execute() override { return ExecuteInPlace(std::get< 0 >(fInPlaceArgs)); }

    protected:
        virtual bool InitializeInPlace(XArgType* in) = 0;
        virtual bool ExecuteInPlace(XArgType* in) = 0;

        std::tuple< XArgType* > fInPlaceArgs;
};

} // namespace hops

#endif /*! __MHO_UnaryInPlaceOperator_HH__ */
