#ifndef MHO_JlParameterStoreInterface_HH__
#define MHO_JlParameterStoreInterface_HH__

#include <string>

#include "MHO_ParameterStore.hh"

#include "jlcxx/jlcxx.hpp"

namespace hops
{

/*!
 *@file  MHO_JlParameterStoreInterface.hh
 *@class  MHO_JlParameterStoreInterface
 *@author  J. Barrett - barrettj@mit.edu
 *@brief Julia (CxxWrap) bindings for MHO_ParameterStore.
 *
 * All parameter values are exchanged as JSON strings; on the Julia side
 * parse/encode with JSON3.jl:
 *   val = JSON3.read(get_by_path(ps, "/key"))
 *   set_by_path(ps, "/key", JSON3.write(val))
 */

class MHO_JlParameterStoreInterface
{
    public:
        MHO_JlParameterStoreInterface(MHO_ParameterStore* paramStore): fParameterStore(paramStore){};
        virtual ~MHO_JlParameterStoreInterface(){};

        bool IsPresent(const std::string& value_path) const
        {
            return fParameterStore->IsPresent(value_path);
        }

        //! Return the value at path as a JSON string.
        std::string GetJSON(const std::string& value_path) const
        {
            mho_json obj;
            bool ok = fParameterStore->Get(value_path, obj);
            if(!ok)
            {
                msg_error("julia_bindings",
                          "error getting value associated with key: " << value_path << eom);
                return "null";
            }
            return obj.dump();
        }

        //! Set the value at path from a JSON string.
        void SetFromJSON(const std::string& value_path, const std::string& json_str) const
        {
            mho_json obj = mho_json::parse(json_str);
            bool ok = fParameterStore->Set(value_path, obj);
            if(!ok)
            {
                msg_error("julia_bindings",
                          "error setting value associated with key: " << value_path << eom);
            }
        }

        //! Return the entire parameter store contents as a JSON string.
        std::string GetContentsJSON() const
        {
            mho_json obj;
            fParameterStore->DumpData(obj);
            return obj.dump();
        }

    private:
        MHO_ParameterStore* fParameterStore;
};


inline void DeclareJlParameterStoreInterface(jlcxx::Module& mod, const std::string& jl_type_name)
{
    mod.add_type< MHO_JlParameterStoreInterface >(jl_type_name)
        // Not constructable from Julia (wraps a C++-owned object).
        .method("is_present",    &hops::MHO_JlParameterStoreInterface::IsPresent)
        .method("get_by_path",   &hops::MHO_JlParameterStoreInterface::GetJSON)
        .method("set_by_path",   &hops::MHO_JlParameterStoreInterface::SetFromJSON)
        .method("get_contents",  &hops::MHO_JlParameterStoreInterface::GetContentsJSON);
}

} // namespace hops

#endif /*! end of include guard: MHO_JlParameterStoreInterface */
