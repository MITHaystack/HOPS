#include <iostream>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
namespace py = pybind11;

#include "MHO_Message.hh"
#include "MHO_PyConfigurePath.hh"

using namespace hops;


int main()
{
    py::scoped_interpreter guard{}; // start the interpreter and keep it alive
    configure_pypath();

    //MHO_PyContainerInterface myInterface;
    //myInterface.SetVisibilities(visibilities);

    auto mho_test = py::module::import("mho_test");
    //mho_test.attr("test_inter")();

    auto mho_test2 = py::module::import("mho_test2");

    auto test = mho_test2.attr("TestOperator")();
    test.attr("Initialize")();
    test.attr("Execute")();




    return 0;
}
