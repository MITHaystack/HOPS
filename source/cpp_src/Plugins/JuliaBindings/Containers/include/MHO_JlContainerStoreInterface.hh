#ifndef MHO_JlContainerStoreInterface_HH__
#define MHO_JlContainerStoreInterface_HH__

#include <string>
#include <vector>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_JlTableContainer.hh"

#include "jlcxx/jlcxx.hpp"

namespace hops
{

/*!
 *@file  MHO_JlContainerStoreInterface.hh
 *@class  MHO_JlContainerStoreInterface
 *@author  J. Barrett - barrettj@mit.edu
 *@brief Julia (CxxWrap) bindings for MHO_ContainerStore.
 *
 * Because CxxWrap cannot return a polymorphic "any" value from C++ the way
 * pybind11's py::object can, polymorphic dispatch is exposed via:
 *   1. get_object_type_uuid(uuid) -> String   (determine the concrete type)
 *   2. Separate typed getters:
 *        get_visibility(uuid), get_weight(uuid), get_phasor(uuid), ...
 *
 * The Julia wrapper module (HOPS.jl) provides a single get_object() that
 * dispatches based on the type UUID.
 */

class MHO_JlContainerStoreInterface
{
    public:
        MHO_JlContainerStoreInterface(): fContainerStore(nullptr){};
        MHO_JlContainerStoreInterface(MHO_ContainerStore* conStore): fContainerStore(conStore){};
        virtual ~MHO_JlContainerStoreInterface(){};

        bool IsValid() const { return fContainerStore != nullptr; }

        std::size_t GetNObjects() const
        {
            return fContainerStore ? fContainerStore->GetNObjects() : 0;
        }

        bool IsObjectPresent(const std::string& uuid_string) const
        {
            if(!fContainerStore) { return false; }
            MHO_UUID uuid;
            uuid.from_string(uuid_string);
            return fContainerStore->IsObjectPresent(uuid);
        }

        std::string GetObjectTypeUUID(const std::string& uuid_string) const
        {
            if(!fContainerStore) { return ""; }
            MHO_UUID uuid;
            uuid.from_string(uuid_string);
            return fContainerStore->GetObjectTypeUUID(uuid).as_string();
        }

        //! Return a JSON string listing all objects:
        //! [ {"type_uuid":..., "object_uuid":..., "shortname":...}, ... ]
        std::string GetObjectListJSON() const
        {
            mho_json info_array = mho_json::array();
            if(fContainerStore)
            {
                auto info = fContainerStore->GetAllObjectInfo();
                for(auto& item : info)
                {
                    mho_json entry;
                    entry["type_uuid"]   = std::get< 0 >(item);
                    entry["object_uuid"] = std::get< 1 >(item);
                    entry["shortname"]   = std::get< 2 >(item);
                    info_array.push_back(entry);
                }
            }
            return info_array.dump();
        }

        // Typed accessors - return the wrapper for the concrete container type.
        // Returns nullptr when the UUID is not found or has the wrong type.

        MHO_JlTableContainer< visibility_type >*   GetVisibility(const std::string& uuid)
        { return GetExtension< visibility_type >(uuid); }

        MHO_JlTableContainer< weight_type >*       GetWeight(const std::string& uuid)
        { return GetExtension< weight_type >(uuid); }

        MHO_JlTableContainer< visibility_store_type >* GetVisibilityStore(const std::string& uuid)
        { return GetExtension< visibility_store_type >(uuid); }

        MHO_JlTableContainer< weight_store_type >* GetWeightStore(const std::string& uuid)
        { return GetExtension< weight_store_type >(uuid); }

        MHO_JlTableContainer< station_coord_type >* GetStationCoord(const std::string& uuid)
        { return GetExtension< station_coord_type >(uuid); }

        MHO_JlTableContainer< phasor_type >*       GetPhasor(const std::string& uuid)
        { return GetExtension< phasor_type >(uuid); }

        MHO_JlTableContainer< multitone_pcal_type >* GetMultitonePcal(const std::string& uuid)
        { return GetExtension< multitone_pcal_type >(uuid); }

        //! Return the MHO_ObjectTags metadata for a tag object as a JSON string.
        std::string GetObjectTagsJSON(const std::string& uuid_string) const
        {
            if(!fContainerStore) { return "{}"; }
            MHO_UUID uuid;
            uuid.from_string(uuid_string);
            MHO_ObjectTags* tags = fContainerStore->GetObject< MHO_ObjectTags >(uuid);
            if(!tags) { return "{}"; }
            mho_json meta_data = tags->GetMetaDataAsJSON();
            std::set< MHO_UUID > tagged_ids = tags->GetTaggedObjectUUIDSet();
            std::vector< std::string > id_list;
            for(auto& id : tagged_ids) { id_list.push_back(id.as_string()); }
            meta_data["tagged_object_uuid_list"] = id_list;
            return meta_data.dump();
        }

        //! Return the registered runtime type UUID for each container type (useful for dispatch in Julia).
        std::string GetVisibilityTypeUUID()      const { return TypeUUIDStr< visibility_type >(); }
        std::string GetWeightTypeUUID()          const { return TypeUUIDStr< weight_type >(); }
        std::string GetVisibilityStoreTypeUUID() const { return TypeUUIDStr< visibility_store_type >(); }
        std::string GetWeightStoreTypeUUID()     const { return TypeUUIDStr< weight_store_type >(); }
        std::string GetStationCoordTypeUUID()    const { return TypeUUIDStr< station_coord_type >(); }
        std::string GetPhasorTypeUUID()          const { return TypeUUIDStr< phasor_type >(); }
        std::string GetMultitonePcalTypeUUID()   const { return TypeUUIDStr< multitone_pcal_type >(); }

        // Exposed for C++ internal use (e.g. MHO_JlScanStoreInterface)
        MHO_ContainerStore* GetContainerStore() { return fContainerStore; }

    private:
        template< typename XClassType >
        std::string TypeUUIDStr() const
        {
            if(!fContainerStore) { return ""; }
            return fContainerStore->GetTypeUUID< XClassType >().as_string();
        }

        template< typename XClassType >
        MHO_JlTableContainer< XClassType >* GetExtension(const std::string& uuid_string)
        {
            if(!fContainerStore) { return nullptr; }
            MHO_UUID uuid;
            bool ok = uuid.from_string(uuid_string);
            if(!ok)
            {
                msg_error("julia_bindings",
                          "cannot convert " << uuid_string << " to UUID" << eom);
                return nullptr;
            }
            XClassType* obj = fContainerStore->GetObject< XClassType >(uuid);
            if(!obj) { return nullptr; }
            if(obj->template HasExtension< MHO_JlTableContainer< XClassType > >())
            {
                return obj->template AsExtension< MHO_JlTableContainer< XClassType > >();
            }
            return obj->template MakeExtension< MHO_JlTableContainer< XClassType > >();
        }

        MHO_ContainerStore* fContainerStore;
};


inline void DeclareJlContainerStoreInterface(jlcxx::Module& mod, const std::string& jl_type_name)
{
    mod.add_type< MHO_JlContainerStoreInterface >(jl_type_name)
        .method("is_valid",              &hops::MHO_JlContainerStoreInterface::IsValid)
        .method("get_nobjects",          &hops::MHO_JlContainerStoreInterface::GetNObjects)
        .method("is_object_present",     &hops::MHO_JlContainerStoreInterface::IsObjectPresent)
        .method("get_object_type_uuid",  &hops::MHO_JlContainerStoreInterface::GetObjectTypeUUID)
        .method("get_object_id_list",    &hops::MHO_JlContainerStoreInterface::GetObjectListJSON)
        .method("get_object_tags",       &hops::MHO_JlContainerStoreInterface::GetObjectTagsJSON)
        // Typed getters for each container type:
        .method("get_visibility",        &hops::MHO_JlContainerStoreInterface::GetVisibility)
        .method("get_weight",            &hops::MHO_JlContainerStoreInterface::GetWeight)
        .method("get_visibility_store",  &hops::MHO_JlContainerStoreInterface::GetVisibilityStore)
        .method("get_weight_store",      &hops::MHO_JlContainerStoreInterface::GetWeightStore)
        .method("get_station_coord",     &hops::MHO_JlContainerStoreInterface::GetStationCoord)
        .method("get_phasor",            &hops::MHO_JlContainerStoreInterface::GetPhasor)
        .method("get_multitone_pcal",    &hops::MHO_JlContainerStoreInterface::GetMultitonePcal)
        // Type UUID helpers for dispatch in Julia:
        .method("get_visibility_type_uuid",       &hops::MHO_JlContainerStoreInterface::GetVisibilityTypeUUID)
        .method("get_weight_type_uuid",           &hops::MHO_JlContainerStoreInterface::GetWeightTypeUUID)
        .method("get_visibility_store_type_uuid", &hops::MHO_JlContainerStoreInterface::GetVisibilityStoreTypeUUID)
        .method("get_weight_store_type_uuid",     &hops::MHO_JlContainerStoreInterface::GetWeightStoreTypeUUID)
        .method("get_station_coord_type_uuid",    &hops::MHO_JlContainerStoreInterface::GetStationCoordTypeUUID)
        .method("get_phasor_type_uuid",           &hops::MHO_JlContainerStoreInterface::GetPhasorTypeUUID)
        .method("get_multitone_pcal_type_uuid",   &hops::MHO_JlContainerStoreInterface::GetMultitonePcalTypeUUID);
}

} // namespace hops

#endif /*! end of include guard: MHO_JlContainerStoreInterface */
