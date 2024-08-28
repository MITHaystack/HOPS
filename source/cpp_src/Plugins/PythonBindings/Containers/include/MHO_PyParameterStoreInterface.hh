#ifndef MHO_PyParameterStoreInterface_HH__
#define MHO_PyParameterStoreInterface_HH__



#include "MHO_ParameterStore.hh"

//need these extras to be able to translate between nl:json and py:dict or py::object
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;
namespace nl = nlohmann;
using namespace pybind11::literals;

namespace hops
{

/*!
*@file  MHO_PyParameterStoreInterface.hh
*@class  MHO_PyParameterStoreInterface
*@author  J. Barrett - barrettj@mit.edu
*@date Thu Sep 14 13:04:32 2023 -0400
*@brief python bindings for the MHO_ParameterStore
*/

class MHO_PyParameterStoreInterface
{
    public:

        MHO_PyParameterStoreInterface(MHO_ParameterStore* paramStore):fParameterStore(paramStore){};
        virtual ~MHO_PyParameterStoreInterface(){};

        bool IsPresent(const std::string& value_path) const
        {
            return fParameterStore->IsPresent(value_path);
        }

        //get a specific value
        py::object Get(const std::string& value_path) const
        {
            mho_json obj;
            bool ok = fParameterStore->Get(value_path, obj);
            if(!ok)
            {
                msg_error("python_bindings", "error getting value associated with key: "<< value_path << eom );
                py::print("error getting value associated with key: ", value_path);
            }

            return obj;
        }

        //set a specific value
        void Set(const std::string& value_path, py::object value) const
        {
            mho_json obj = value;
            bool ok = fParameterStore->Set(value_path, obj);
            if(!ok)
            {
                msg_error("python_bindings", "error setting value associated with key: "<< value_path << eom );
                py::print("error setting value associated with key: ", value_path);
            }
        }

        //returns a python dictionary containing all of the parameter stores contents
        py::dict GetContents()
        {
            mho_json obj;
            fParameterStore->DumpData(obj);
            return obj;
        }

    private:

        //pointer to the parameter store
        MHO_ParameterStore* fParameterStore;

};

void
DeclarePyParameterStoreInterface(py::module &m, std::string pyclass_name)
{
    py::class_< MHO_PyParameterStoreInterface >(m, pyclass_name.c_str() )
        //no __init__ def here, as this class is not meant to be constructable on the python side
        .def("is_present", &hops::MHO_PyParameterStoreInterface::IsPresent,
            "check if parameter with specified path is present in the store",
            py::arg("value_path")
        )
        .def("get_by_path", &hops::MHO_PyParameterStoreInterface::Get,
            "get parameter at path",
            py::arg("value_path")
        )
        .def("set_by_path", &hops::MHO_PyParameterStoreInterface::Set,
            "set the value of the parameter at the specified path",
            py::arg("value_path"),
            py::arg("value")
        )
        .def("get_contents", &hops::MHO_PyParameterStoreInterface::GetContents,
            py::return_value_policy::copy,
            "get a copy of the contents of the parameter store as a dictionary"
        );
}


}//end of namespace

#endif /*! end of include guard: MHO_PyParameterStoreInterface */
