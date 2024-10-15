#include "MHO_PyOperator.hh"

using namespace hops;

#define PYBIND11_DETAILED_ERROR_MESSAGES

PYBIND11_MODULE(pyMHO_Operators, m)
{
    m.doc() = "module to provide interface to extend MHO_Operators with python classes"; // optional module docstring

    py::class_< MHO_Operator, MHO_PyOperator >(m, "MHO_Operator")
        .def(py::init<>())
        .def("Initialize", &MHO_Operator::Initialize)
        .def("Execute", &MHO_Operator::Execute);
}
