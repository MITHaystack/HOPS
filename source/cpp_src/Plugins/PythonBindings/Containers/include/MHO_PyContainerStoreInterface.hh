#ifndef MHO_PyContainerStoreInterface_HH__
#define MHO_PyContainerStoreInterface_HH__

#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerDictionary.hh"

#include "MHO_ContainerStore.hh"
#include "MHO_PyTableContainer.hh"

//need these extras to be able to translate between nl:json and py:dict or py::object
#include "pybind11_json/pybind11_json.hpp"
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;
namespace nl = nlohmann;
using namespace pybind11::literals;

namespace hops
{

/*!
 *@file  MHO_PyContainerStoreInterface.hh
 *@class  MHO_PyContainerStoreInterface
 *@author  J. Barrett - barrettj@mit.edu
 *@date Thu Sep 23 16:03:48 2021 -0400
 *@brief python binding to the MHO_ContainerStore
 */

class MHO_PyContainerStoreInterface
{
    public:
        MHO_PyContainerStoreInterface(): fContainerStore(nullptr){}; //dummy constructor
        MHO_PyContainerStoreInterface(MHO_ContainerStore* conStore): fContainerStore(conStore){};
        virtual ~MHO_PyContainerStoreInterface(){};

        bool IsValid()
        {
            if(fContainerStore != nullptr)
            {
                return true;
            }
            return false;
        }

        std::size_t GetNObjects()
        {
            if(fContainerStore)
            {
                return fContainerStore->GetNObjects();
            }
            return 0;
        }

        bool IsObjectPresent(const std::string& uuid_string) const
        {
            if(fContainerStore != nullptr)
            {
                MHO_UUID uuid;
                bool ok = uuid.from_string(uuid_string);
                if(!ok)
                {
                    msg_error("python_bindings", "error could not convert: " << uuid_string << " to valid UUID" << eom);
                }
                return fContainerStore->IsObjectPresent(uuid);
            }
            return false;
        }

        std::string GetObjectTypeUUID(const std::string& uuid_string) const
        {
            MHO_UUID type_uuid;
            if(fContainerStore != nullptr)
            {
                MHO_UUID uuid;
                bool ok = uuid.from_string(uuid_string);
                type_uuid = fContainerStore->GetObjectTypeUUID(uuid);
            }
            return type_uuid.as_string(); //empyt if uuid was invalid
        }

        template< typename XClassType > MHO_PyTableContainer< XClassType >& GetObject(const std::string& uuid_string)
        {
            MHO_UUID uuid;
            bool ok = uuid.from_string(uuid_string);
            if(!ok)
            {
                msg_error("python_bindings", "error could not convert: " << uuid_string << " to valid UUID" << eom);
            }
            XClassType* obj = fContainerStore->GetObject< XClassType >(uuid);
            auto ext_ptr = GetExtension(obj);
            if(ext_ptr != nullptr)
            {
                return *ext_ptr;
            }
            else
            {
                msg_fatal("python_bindings", "fatal error, object with uuid: " << uuid_string << " is not present." << eom);
                //instead of failing in this case (object is missing), alternatively we could return a dummy object?
                //however, maybe an obvious failure is good in this case?
                std::exit(1);
            }
        }

        //return a list of (UUID <-> typename <-> shortname) tuples
        py::list GetObjectList()
        {
            mho_json info_obj;
            std::vector< std::tuple< std::string, std::string, std::string > > info = fContainerStore->GetAllObjectInfo();
            for(std::size_t i = 0; i < info.size(); i++)
            {
                mho_json item_info;
                item_info["type_uuid"] = std::get< 0 >(info[i]);
                item_info["object_uuid"] = std::get< 1 >(info[i]);
                item_info["shortname"] = std::get< 2 >(info[i]);
                info_obj.push_back(item_info);
            }
            py::list ret_obj = info_obj;
            return ret_obj;
        };

        //publically EXPOSED for C++ lambda's
        //DO NOT EXPOSE THIS CLASS TO PYTHON
        MHO_ContainerStore* GetContainerStore() { return fContainerStore; }

    private:
        //put the extension building/retrieval in this template
        template< typename XClassType > MHO_PyTableContainer< XClassType >* GetExtension(XClassType* obj)
        {
            if(obj != nullptr)
            {
                if(obj->template HasExtension< MHO_PyTableContainer< XClassType > >())
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

inline void DeclarePyContainerStoreInterface(py::module& m, std::string pyclass_name)
{
    py::class_< MHO_PyContainerStoreInterface >(m, pyclass_name.c_str())
        //no __init__ def here, as this class is not meant to be constructable on the python side
        .def("is_valid", &hops::MHO_PyContainerStoreInterface::IsValid,
             "check if the underlying container store object exists and is valid")
        .def("get_nobjects", &hops::MHO_PyContainerStoreInterface::GetNObjects,
             "get the number of objects present in the container store")
        .def("is_object_present", &hops::MHO_PyContainerStoreInterface::IsObjectPresent,
             "check if an object with the passed UUID is present", py::arg("uuid"))
        .def("get_object_id_list", &hops::MHO_PyContainerStoreInterface::GetObjectList,
             "get a list of the object information (list of dict's containing each object's type_uuid, object_uuid, and "
             "shortname")
        .def(
            "get_object", //lambda for returing either object data or none type
            [=](MHO_PyContainerStoreInterface& m, std::string object_uuid) -> py::object {
                MHO_UUID uuid;
                bool is_valid = uuid.from_string(object_uuid);
                MHO_ContainerStore* cStore = m.GetContainerStore();
                if(is_valid && cStore != nullptr && cStore->IsObjectPresent(uuid)) //use MHO_UUID, not string
                {
                    MHO_UUID type_id = cStore->GetObjectTypeUUID(uuid);

                    if(type_id == cStore->GetTypeUUID< visibility_type >())
                    {
                        return py::cast(m.GetObject< visibility_type >(object_uuid));
                    }
                    if(type_id == cStore->GetTypeUUID< weight_type >())
                    {
                        return py::cast(m.GetObject< weight_type >(object_uuid));
                    }
                    if(type_id == cStore->GetTypeUUID< station_coord_type >())
                    {
                        return py::cast(m.GetObject< station_coord_type >(object_uuid));
                    }
                    if(type_id == cStore->GetTypeUUID< visibility_store_type >())
                    {
                        return py::cast(m.GetObject< visibility_store_type >(object_uuid));
                    }

                    if(type_id == cStore->GetTypeUUID< weight_store_type >())
                    {
                        return py::cast(m.GetObject< weight_store_type >(object_uuid));
                    }
                    
                    if(type_id == cStore->GetTypeUUID< phasor_type >())
                    {
                        return py::cast(m.GetObject< phasor_type >(object_uuid));
                    }
                    
                    if(type_id == cStore->GetTypeUUID< multitone_pcal_type >())
                    {
                        return py::cast(m.GetObject< multitone_pcal_type >(object_uuid));
                    }

                    if(type_id == cStore->GetTypeUUID< MHO_ObjectTags >())
                    {
                        //handle tag data type
                        MHO_ObjectTags* tags = cStore->GetObject< MHO_ObjectTags >(uuid); //use MHO_UUID, not string
                        if(tags != nullptr)
                        {
                            mho_json meta_data = tags->GetMetaDataAsJSON();
                            std::set< MHO_UUID > tagged_ids = tags->GetTaggedObjectUUIDSet();
                            std::vector< std::string > id_list;
                            for(auto it = tagged_ids.begin(); it != tagged_ids.end(); it++)
                            {
                                id_list.push_back(it->as_string());
                            }
                            //append the tagged object uuid list
                            meta_data["tagged_object_uuid_list"] = id_list;
                            //convert to py::dict
                            py::dict dict_obj = meta_data;
                            return dict_obj;
                        }
                        else
                        {
                            py::print("MHO_ObjectTags object with uuid: ", object_uuid, " cannot be loaded.");
                        }
                    }
                }
                py::print("object uuid ", object_uuid, " is not recognized, returning None.");
                return py::object(py::cast(nullptr));
            },
            py::return_value_policy::reference, "return the object matching the specified uuid", py::arg("uuid"));
}

} // namespace hops

#endif /*! end of include guard: MHO_PyContainerStoreInterface */
