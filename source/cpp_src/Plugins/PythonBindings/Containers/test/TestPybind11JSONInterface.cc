#include <iostream>

#include <string>
#include <vector>
#include <map>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include "pybind11_json/pybind11_json.hpp"

namespace py = pybind11;
namespace nl = nlohmann;
using namespace pybind11::literals;

#include "MHO_Message.hh"

#include "MHO_PyNDArrayWrapper.hh"
#include "MHO_PyContainerInterface.hh"

using namespace hops;


int main()
{
    py::scoped_interpreter guard{}; // start the interpreter and keep it alive, need this or we segfault


    py::dict obj = py::dict("number"_a=1234, "hello"_a="world");

    // Automatic py::dict->nl::json conversion
    nl::json j = obj;

    // Automatic nl::json->py::object conversion
    py::object result1 = j;
    // Automatic nl::json->py::dict conversion
    py::dict result2 = j;

    std::map< std::string, double > channel_phases;
    channel_phases["a"] = 2.0;
    channel_phases["abc"] = 44.0;

    nl::json map_obj = channel_phases;
    py::dict map_dict = map_obj;

    std::cout<<"dumping map obj = "<<map_obj.dump(2)<<std::endl;


    return 0;
}
