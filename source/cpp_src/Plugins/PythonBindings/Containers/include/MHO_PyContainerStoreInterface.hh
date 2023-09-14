#ifndef MHO_PyContainerStoreInterface_HH__
#define MHO_PyContainerStoreInterface_HH__

#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerStore.hh"

//need these extras to be able to translate between nl:json and py:dict or py::object
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;
namespace nl = nlohmann;
using namespace pybind11::literals;


/*
*@file: MHO_PyContainerStoreInterface.hh
*@class: MHO_PyContainerStoreInterface
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

namespace hops
{


class MHO_PyContainerStoreInterface
{
    public:

        MHO_PyContainerStoreInterface(MHO_ContainerStore* conStore):fContainerStore(conStore){};
        virtual ~MHO_PyContainerStoreInterface(){};

        std::size_t GetNObjects()
        {
            return fContainerStore->GetNObjects();
        }

        bool IsObjectPresent(const std::string& uuid_string) const
        {
            MHO_UUID uuid;
            bool ok = uuid.from_string(uuid_string);
            if(!ok){msg_error("python_bindings", "error could not convert: "<<uuid_string<<" to valid UUID" <<eom);}
            return fContainerStore->IsObjectPresent(uuid);
        }
        //





        // //get a specific value
        // py::object Get(const std::string& value_path) const
        // {
        //     mho_json obj;
        //     bool ok = fContainerStore->Get(value_path, obj);
        //     if(!ok){msg_error("python_bindings", "error getting value associated with key: "<< value_path << eom );}
        //     return obj;
        // }
        //
        // //set a specific value
        // void Set(const std::string& value_path, py::object value) const
        // {
        //     mho_json obj = value;
        //     bool ok = fContainerStore->Set(value_path, obj);
        //     if(!ok){msg_error("python_bindings", "error setting value associated with key: "<< value_path << eom );}
        // }
        //
        // //returns a python dictionary containing all of the parameter stores contents
        // py::dict GetContents()
        // {
        //     mho_json obj;
        //     fContainerStore->DumpData(obj);
        //     return obj;
        // }

    private:

        //pointer to the parameter store
        MHO_ContainerStore* fContainerStore;

};

void
DeclarePyContainerStoreInterface(py::module &m, std::string pyclass_name)
{
    py::class_< MHO_PyContainerStoreInterface >(m, pyclass_name.c_str() )
        //no __init__ def here, as this class is not meant to be constructable on the python side
        .def("GetNObjects", &hops::MHO_PyContainerStoreInterface::GetNObjects)
        .def("IsObjectPresent", &hops::MHO_PyContainerStoreInterface::IsObjectPresent);
        // .def("Get", &hops::MHO_PyContainerStoreInterface::Get)
        // .def("Set", &hops::MHO_PyContainerStoreInterface::Set)
        // .def("GetContents", &hops::MHO_PyContainerStoreInterface::GetContents);
}


}//end of namespace

#endif /* end of include guard: MHO_PyContainerStoreInterface */
