#ifndef MHO_PyOperator_HH__
#define MHO_PyOperator_HH__

/*!
*@file  MHO_PyOperator.hh
*@class  MHO_PyOperator
*@author  J. Barrett - barrettj@mit.edu 
*
*@date 
*@brief  trampoline for generic data operator
*/

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h> //this is important to have for std::complex<T> support!
namespace py = pybind11;

#include "MHO_Operator.hh"

namespace hops
{

class MHO_PyOperator: public MHO_Operator
{
    public:
        using MHO_Operator::MHO_Operator;

        virtual bool Initialize() override
        {
            PYBIND11_OVERLOAD_PURE(bool, MHO_Operator, Initialize);
        }

        virtual bool Execute() override
        {
            PYBIND11_OVERLOAD_PURE(bool, MHO_Operator, Execute);
        }

    private:
};


}

#endif /*! end of include guard: MHO_PyOperator */
