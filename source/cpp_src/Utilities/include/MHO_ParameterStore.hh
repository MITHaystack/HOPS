#ifndef MHO_ParameterStore_HH__
#define MHO_ParameterStore_HH__

#include <string>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Tokenizer.hh"

#define SAFETY_DANCE 

namespace hops
{

//class to store parameters (typically from control file) for later retrieval
//TODO -- evaluate if using json is appropriate (fast?), could also use MHO_MultiTypeMap 
//but we'd need to add an option to pass std::vector<T> values as well as single values

//There are some deficiencies with the json approach, for example everything except terminal values
//must be named objects (no lists allowd), though maybe we don't really need that functionality

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

        template< typename XValueType>
        XValueType GetAs(const std::string& value_path);

        bool IsPresent(const std::string& value_path)
        {
            std::string path = SanitizePath(value_path);
            fPath.clear();
            fTokenizer.SetString(&path);
            fTokenizer.GetTokens(&fPath);

            bool present = false;
            auto* p = &fStore;
            for(auto it = fPath.begin(); it != fPath.end(); it++)
            {
                auto jit = p->find(*it);
                if(jit == p->end())
                {
                    return false;
                }
                else if( *it == *(fPath.rbegin() ) )
                {
                    return true;
                }
                else
                {
                    p = &(jit.value());
                }
            }
            return false;
        }

    private:

        //TODO sanitise the value_path string -- for example a trailing '/' will cause a crash
        std::string SanitizePath(const std::string& value_path){return value_path;}

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
    std::string path = SanitizePath(value_path);
    fPath.clear();
    fTokenizer.SetString(&path);
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
    #ifdef SAFETY_DANCE
    //kind of silly to do this - should just re-implement the access method below
    //to use the same check-logic to retrieve the value, instead of using json_pointer
    if(!IsPresent(value_path)){return false;}
    #endif
    
    std::string path = SanitizePath(value_path);
    mho_json::json_pointer jptr(path);
    auto item = fStore.at(jptr);
    if( !item.empty() )
    {
        value = item.get<XValueType>();
        return true;
    }
    return false;
}

template< typename XValueType>
XValueType
MHO_ParameterStore::GetAs(const std::string& value_path)
{
    XValueType v = XValueType(); //default constructor (zero for int, double, etc)
    bool ok = Get(value_path,v);
    if(!ok){msg_debug("utility", "failed to retrieve value: "<< value_path<<" returning default: " << v << eom );}
    return v;
}

}//end of namespace

#endif /* end of include guard: MHO_ParameterStore_HH__ */
