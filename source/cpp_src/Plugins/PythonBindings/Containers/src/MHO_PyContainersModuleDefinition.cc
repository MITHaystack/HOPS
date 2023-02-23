#include "MHO_PyNDArrayWrapper.hh"
#include "MHO_PyTableContainer.hh"
#include "MHO_PyContainerInterface.hh"


using namespace hops;


PYBIND11_MODULE(pyMHO_Containers, m)
{
        m.doc() = "module to interact with MHO_Containers"; // optional module docstring

        DeclarePyTableContainer< visibility_type >(m, std::string("visibility_type") );

        // //just for testing
        // DeclarePyNDArrayWrapper< MHO_NDArrayWrapper<double, 2> >(m, std::string("mx") );

        py::class_<MHO_PyContainerInterface, std::unique_ptr<MHO_PyContainerInterface, py::nodelete> >(m, "MHO_PyContainerInterface")
            .def(py::init<>())
            .def("GetVisibilityTable", &hops::MHO_PyContainerInterface::GetVisibilityTable);
}
