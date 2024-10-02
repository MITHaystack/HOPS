#include <iomanip>
#include <iostream>

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;

#include "MHO_Message.hh"

#include "MHO_ParameterStore.hh"
#include "MHO_PyConfigurePath.hh"
#include "MHO_PyParameterStoreInterface.hh"

using namespace hops;

int main()
{
    py::scoped_interpreter guard{}; // start the interpreter and keep it alive
    configure_pypath();

    std::cout << "the python path directories: " << std::endl;
    py::exec(R"(
        import sys
        import numpy
        print(sys.path)
        import pyMHO_Containers
    )");

    MHO_ParameterStore paramStore;
    paramStore.Set("/my/test/value", 1);
    paramStore.Set("/a/float/value", 3.14159);
    paramStore.Set("/a/string/value", "my string");
    paramStore.Set("/a/int/value", 10);
    paramStore.Set("/another/test/value", 9874.23);

    MHO_PyParameterStoreInterface paramInterface(&paramStore);

    auto mho_param_test = py::module::import("mho_param_test");

    mho_param_test.attr("test_inter")(paramInterface);

    //now see what has turned up on the C++ side
    std::string my_str = paramStore.GetAs< std::string >("/a/string/value");
    int my_int = paramStore.GetAs< int >("/a/int/value");
    double my_float = paramStore.GetAs< double >("/a/float/value");

    std::cout << std::setprecision(9);
    std::cout << "Contents on c++ side:" << std::endl;

    paramStore.Dump();

    std::vector< double > alist = paramStore.GetAs< std::vector< double > >("a_list");

    std::cout << "the list object as a std::vector<double> = " << std::endl;
    for(std::size_t i = 0; i < alist.size(); i++)
    {
        std::cout << alist[i] << ",";
    }
    std::cout << std::endl;

    return 0;
}
