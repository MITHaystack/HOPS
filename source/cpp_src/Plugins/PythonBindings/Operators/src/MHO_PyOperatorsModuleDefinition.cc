#include <pybind11/stl.h>

#include "MHO_OperatorToolbox.hh"
#include "MHO_PyFringeDataInterface.hh"
#include "MHO_PyOperator.hh"

using namespace hops;

#define PYBIND11_DETAILED_ERROR_MESSAGES

PYBIND11_MODULE(pyMHO_Operators, m)
{
    m.doc() = "module to provide interface to extend MHO_Operators with python classes"; // optional module docstring

    // Ensure pyMHO_Containers is loaded so MHO_PyFringeDataInterface is already registered
    auto containers = py::module_::import("pyMHO_Containers");

    py::class_< MHO_Operator, MHO_PyOperator >(m, "MHO_Operator")
        .def(py::init<>())
        .def("initialize", &MHO_Operator::Initialize)
        .def("execute", &MHO_Operator::Execute)
        .def("set_priority", &MHO_Operator::SetPriority)
        .def("get_priority", &MHO_Operator::Priority)
        .def("set_name", &MHO_Operator::SetName)
        .def("get_name", &MHO_Operator::GetName);

    py::class_< MHO_OperatorToolbox >(m, "MHO_OperatorToolbox")
        .def("get_all_operators_by_name", &MHO_OperatorToolbox::GetAllOperatorsByName, py::return_value_policy::reference,
             "Retrieve all operators with the given name as a list, sorted by priority. "
             "Import pyMHO_Calibration first so that pybind11 can downcast each element "
             "to the correct derived type.")
        .def("get_n_operators", &MHO_OperatorToolbox::GetNOperators)
        .def("get_operators_by_category", &MHO_OperatorToolbox::GetOperatorsByCategory, py::return_value_policy::reference)
        .def("get_operator_names", [](MHO_OperatorToolbox& self) {
            // Return all operator names as a Python list
            std::vector< std::string > names;
            auto ops = self.GetAllOperators();
            // GetAllOperators returns pointers sorted by priority; retrieve names via map
            // Use GetNOperators as a rough bound and collect via the returned pointers
            for(auto* op : ops)
            {
                names.push_back(op->GetName());
            }
            return names;
        });

    // Extend MHO_PyFringeDataInterface (registered in pyMHO_Containers) with toolbox access.
    // We add the method dynamically so that pyMHO_Containers does not need to know about
    // MHO_OperatorToolbox or MHO_Operators.
    auto fringe_data_cls = containers.attr("MHO_PyFringeDataInterface");
    fringe_data_cls.attr("get_operator_toolbox") =
        py::cpp_function([](MHO_PyFringeDataInterface& self) -> MHO_OperatorToolbox* { return self.GetOperatorToolbox(); },
                         py::return_value_policy::reference, py::name("get_operator_toolbox"), py::is_method(fringe_data_cls),
                         "get the operator toolbox so C++ calibration operators can be retrieved and reconfigured");
}
