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

        template< typename XClassType >
        MHO_PyTableContainer< XClassType >& GetObject(const std::string& uuid_string)
        {
            MHO_UUID uuid;
            bool ok = uuid.from_string(uuid_string);
            if(!ok){msg_error("python_bindings", "error could not convert: "<<uuid_string<<" to valid UUID" <<eom);}
            XClassType* obj = fContainerStore->GetObject<XClassType>(uuid);
            auto ext_ptr = GetExtension(obj);
            if(ext_ptr != nullptr){return *ext_ptr;}
            else
            {
                #pragma message("TODO FIXME...this doesn't necessarily have be a fatal error")
                msg_fatal("python_bindings", "fatal error, object with uuid: "<<uuid_string<<" is not present."<<eom);
                //instead of failing in this case (object is missing), alternatively we could return a dummy object
                //however, maybe an obvious failure is good in this case?
                std::exit(1);
            }
        }

    private:

        //put the extension building/retrieval in this template
        template< typename XClassType >
        MHO_PyTableContainer< XClassType >*
        GetExtension(XClassType* obj)
        {
            if(obj != nullptr)
            {
                if( obj->template HasExtension< MHO_PyTableContainer< XClassType > >() )
                {
                    return obj->template AsExtension< MHO_PyTableContainer< XClassType > >();
                }
                else
                {
                    return obj->template MakeExtension< MHO_PyTableContainer< XClassType > >();
                }
            }
            return nullptr;
        }

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
        .def("GetVisibilityObject", &hops::MHO_PyContainerStoreInterface::GetObject<visibility_type>)
        .def("GetWeightObject", &hops::MHO_PyContainerStoreInterface::GetObject<weight_type>)
        .def("GetStationObject", &hops::MHO_PyContainerStoreInterface::GetObject<station_coord_type>)
        .def("GetVisibilityStoreObject", &hops::MHO_PyContainerStoreInterface::GetObject<visibility_store_type>)
        .def("GetWeightStoreObject", &hops::MHO_PyContainerStoreInterface::GetObject<visibility_store_type>);
}





}//end of namespace

#endif /* end of include guard: MHO_PyContainerStoreInterface */
