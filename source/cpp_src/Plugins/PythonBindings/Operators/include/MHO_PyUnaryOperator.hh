#ifndef MHO_PyOperator_HH__
#define MHO_PyOperator_HH__

#include "MHO_PyOperator.hh"
#include "MHO_PyTableContainer.hh"

#include <pybind11/numpy.h> //this is important to have for std::complex<T> support!
#include <pybind11/pybind11.h>
namespace py = pybind11;

namespace hops
{

/*!
 *@file  MHO_PyOperator.hh
 *@class  MHO_PyOperator
 *@author  J. Barrett - barrettj@mit.edu
 *@date Thu Sep 23 16:03:48 2021 -0400
 *@brief  trampoline for unary data operator
 */

class MHO_UnaryOperator:
{
    public:
        using MHO_Operator::MHO_Operator;

        virtual bool Initialize() override = 0;
        virtual bool Execute() override = 0;

    private:
};

class MHO_PyUnaryOperator:
{
    public:
        using MHO_Operator::MHO_Operator;

        virtual bool Initialize() override { PYBIND11_OVERLOAD_PURE(bool, MHO_Operator, Initialize); }

        virtual bool Execute() override { PYBIND11_OVERLOAD_PURE(bool, MHO_Operator, Execute); }

    private:
};

} // namespace hops

#endif /*! end of include guard: MHO_PyOperator */
