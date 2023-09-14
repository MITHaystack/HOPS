#ifndef MHO_PyContainerStoreInterface_HH__
#define MHO_PyContainerStoreInterface_HH__

#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_PyTableContainer.hh"

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

        MHO_PyTableContainer< visibility_type >& GetVisibilityObject(const std::string& uuid_string)
        {
            MHO_UUID uuid;
            bool ok = uuid.from_string(uuid_string);
            if(!ok){msg_error("python_bindings", "error could not convert: "<<uuid_string<<" to valid UUID" <<eom);}
            visibility_type* vis = fContainerStore->GetObject<visibility_type>(uuid);
            if(vis != nullptr)
            {
                if( vis->HasExtension< MHO_PyTableContainer< visibility_type > >() )
                {
                    return *( vis->AsExtension< MHO_PyTableContainer< visibility_type > >() );
                }
                else
                {
                    return *(vis->MakeExtension< MHO_PyTableContainer< visibility_type > >() );
                }
            }
            else
            {
                msg_fatal("python_bindings", "fatal error, object with uuid: "<<uuid_string<<" is not present."<<eom);
                std::exit(1);
            }
        }

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
        .def("IsObjectPresent", &hops::MHO_PyContainerStoreInterface::IsObjectPresent)
        .def("GetVisibilityObject", &hops::MHO_PyContainerStoreInterface::GetVisibilityObject);

        // .def("Get", &hops::MHO_PyContainerStoreInterface::Get)
        // .def("Set", &hops::MHO_PyContainerStoreInterface::Set)
        // .def("GetContents", &hops::MHO_PyContainerStoreInterface::GetContents);
}


}//end of namespace

#endif /* end of include guard: MHO_PyContainerStoreInterface */
