#include "MHO_PyOperator.hh"

using namespace hops;

PYBIND11_MODULE(pyMHO_Operators, m)
{
    m.doc() = "module to provide interface to extend MHO_Operators with python classes"; // optional module docstring

    py::class_<MHO_Operator, MHO_PyOperator>(m, "MHO_Operator")
        .def( py::init<>() )
        .def( "Initialize", &MHO_Operator::Initialize)
        .def( "ExecuteOperation", &MHO_Operator::ExecuteOperation);
}
