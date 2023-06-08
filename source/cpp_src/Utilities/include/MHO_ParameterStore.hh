#ifndef MHO_ParameterStore_HH__
#define MHO_ParameterStore_HH__

#include <string>

#include "MHO_JSONHeaderWrapper.hh"

namespace hops
{

//class to store parameters (typically from control file) for later retrieval

class MHO_ParameterStore
{
    public:
        MHO_ParameterStore(){};
        ~MHO_ParameterStore(){};

        template< typename XValueType>
        void Set(const std::string& value_path, const XValueType& value);

        template< typename XValueType>
        bool Get(const std::string& value_path, XValueType& value);

        // template< typename XValueType>
        // XValueType GetAs(const std::string& value_path);

    private:

        //stash data in a json object
        mho_json fStore;
};

template< typename XValueType>
XValueType
MHO_ParameterStore::Set(const std::string& value_path, const XValueType& value)
{
    // auto* p = &fStore;
    // for(const auto& k : value_path)
    // {
    //     if( !p->is_object() ){ *p = json::object(); }
    //     p = &(*p)[k];
    // }

}

// template< typename XValueType>
// XValueType
// MHO_ParameterStore::GetAs(const std::string& value_path)
// {
//     mho_json::json_pointer jptr(value_path);
//     XValueType value = fStore.at(jptr).get<XValueType>();
//     return value;
// }



}//end of namespace

#endif /* end of include guard: MHO_ParameterStore_HH__ */
