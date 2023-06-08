#ifndef MHO_ParameterStore_HH__
#define MHO_ParameterStore_HH__

#include <string>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

//class to store parameters (typically from control file) for later retrieval
//TODO -- evaluate if using json is appropriate, could also use MHO_MultiTypeMap 

class MHO_ParameterStore
{
    public:
        
        MHO_ParameterStore()
        {
            fTokenizer.SetDelimiter("/");
            fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
            fTokenizer.SetIncludeEmptyTokensFalse();
        };

        ~MHO_ParameterStore(){};

        template< typename XValueType>
        bool Set(const std::string& value_path, const XValueType& value);

        template< typename XValueType>
        bool Get(const std::string& value_path, XValueType& value);

        // template< typename XValueType>
        // XValueType GetAs(const std::string& value_path);

    private:

        //helpers
        MHO_Tokenizer fTokenizer;
        std::vector< std::string > fPath;

        //stash data in a json object
        mho_json fStore;
};

template< typename XValueType>
bool
MHO_ParameterStore::Set(const std::string& value_path, const XValueType& value)
{
  //first we tokenize the value path into a sequence of names 
    fPath.clear();
    fTokenizer.SetString(&value_path);
    fTokenizer.GetTokens(&fPath);

    bool ok = false;
    if(fPath.size() > 0)
    {
        //insert and/or replace an object
        auto* p = &fStore;
        for(auto it = fPath.begin(); it != fPath.end(); it++)
        {
            if( *it != *(fPath.rbegin()) )
            {
                if( p->contains(*it) ) //item with path exists
                {
                    p = &(p->at(*it) ); //point to this item 
                }
                else 
                {
                    //item doesn't exist yet, so create an entry with this key (*it)
                    p = &(*p)[ *it ]; 
                }
            }
            else //we've arrived at the terminal point, so set this item to the value
            {
                p = &(*p)[ *it ];
                *p = value;
                ok = true;
            }
        }
    }
    return ok;
}

template< typename XValueType>
bool
MHO_ParameterStore::Get(const std::string& value_path, XValueType& value)
{
    //TODO sanitise the value_path string -- for example a trailing '/' will cause a crash
    mho_json::json_pointer jptr(value_path);
    auto item = fStore.at(jptr);
    if( !item.empty() )
    {
        value = item.get<XValueType>();
        return true;
    }
    return false;
}



}//end of namespace

#endif /* end of include guard: MHO_ParameterStore_HH__ */
