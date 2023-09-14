#include <iostream>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
namespace py = pybind11;

#include "MHO_Message.hh"

#include "MHO_ParameterStore.hh"
#include "MHO_PyParameterStoreInterface.hh"

using namespace hops;


int main()
{
    py::scoped_interpreter guard{}; // start the interpreter and keep it alive

    std::cout<<"the python path directories: "<<std::endl;
    py::exec(R"(
        import sys
        import numpy
        print(sys.path)
        import pyMHO_Containers
    )");

    MHO_ParameterStore paramStore;
    paramStore.Set("/my/test/value", 1);

    MHO_PyParameterStoreInterface paramInterface(&paramStore);

    auto mho_param_test = py::module::import("mho_param_test");

    mho_param_test.attr("test_inter")(paramInterface);

    return 0;
}
