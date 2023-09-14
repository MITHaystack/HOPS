#ifndef MHO_PyParameterStoreInterface_HH__
#define MHO_PyParameterStoreInterface_HH__


// #include "MHO_ContainerDefinitions.hh"
// #include "MHO_ContainerStore.hh"
// #include "MHO_PyTableContainer.hh"

#include "MHO_ParameterStore.hh"


//pybind11 stuff to interface with python
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;
namespace nl = nlohmann;
using namespace pybind11::literals;


/*
*@file: MHO_PyParameterStoreInterface.hh
*@class: MHO_PyParameterStoreInterface
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

namespace hops
{


class MHO_PyParameterStoreInterface
{
    public:

        MHO_PyParameterStoreInterface(MHO_ParameterStore* paramStore):fParameterStore(paramStore){};
        virtual ~MHO_PyParameterStoreInterface(){};

        py::bool_ IsPresent(const std::string& value_path) const
        {
            return fParameterStore->IsPresent(value_path);
        }

        py::str GetAsString(const std::string& value_path) const
        {
            return fParameterStore->GetAs<std::string>(value_path);
        }

        py::float_ GetAsFloat(const std::string& value_path) const
        {
            return fParameterStore->GetAs<double>(value_path);
        }

        py::int_ GetAsInt(const std::string& value_path) const
        {
            return fParameterStore->GetAs<int>(value_path);
        }

        py::object Get(const std::string& value_path) const
        {
            int ival;
            double fval;
            std::string sval;

            if( fParameterStore->Get(value_path, ival) ){return py::int_(ival);}
            if( fParameterStore->Get(value_path, fval) ){return py::float_(fval);}
            if( fParameterStore->Get(value_path, sval) ){return py::str(sval);}

            return py::none();
        }

        py::dict Get2(const std::string& value_path) const
        {
            mho_json obj = fParameterStore->GetJSONObject(value_path);
            py::dict ret_obj = obj;
            return ret_obj;
        }

    private:

        //pointer to the parameter store
        MHO_ParameterStore* fParameterStore;

};



//XTableType must inherit from MHO_NDArrayWrapper<XValueType, RANK>
void
DeclarePyParameterStoreInterface(py::module &m, std::string pyclass_name)
{
    py::class_< MHO_PyParameterStoreInterface >(m, pyclass_name.c_str() )
        //no __init__ def here, as this class is not meant to be constructable on the python side
        .def("IsPresent", &hops::MHO_PyParameterStoreInterface::IsPresent)
        .def("GetAsString", &hops::MHO_PyParameterStoreInterface::GetAsString)
        .def("GetAsFloat", &hops::MHO_PyParameterStoreInterface::GetAsFloat)
        .def("GetAsInt", &hops::MHO_PyParameterStoreInterface::GetAsInt)
        .def("Get", &hops::MHO_PyParameterStoreInterface::Get)
        .def("Get2", &hops::MHO_PyParameterStoreInterface::Get2);
}


}//end of namespace

#endif /* end of include guard: MHO_PyParameterStoreInterface */
