#ifndef JSON_WRAPPER_HH__
#define JSON_WRAPPER_HH__

#include "nlohmann/json.hpp"
using mho_json = nlohmann::json;
using mho_ordered_json = nlohmann::ordered_json;
namespace nl=nlohmann;

#include <iostream>

namespace hops
{

class MHO_JSONWrapper
{
    //this class is only intended to provide an interface for derived classes to inherit
    protected:

        MHO_JSONWrapper():fObject(nullptr){};
        MHO_JSONWrapper(mho_json* obj):fObject(obj){};
        MHO_JSONWrapper(const MHO_JSONWrapper& copy)
        {
            fObject = copy.fObject;
        };
        
        void SetObject(mho_json* obj){fObject = obj;}
        
    private:
        
        mho_json* fObject;

    public:
        
        virtual ~MHO_JSONWrapper(){};

        bool HasKey(const std::string& key) const
        {
            if(fObject == nullptr){return false;}
            auto it = fObject->find(key);
            if(it != fObject->end()){return true;}
            return false;
        }

        bool HasKey(const char* char_key) const
        {
            std::string key(char_key);
            return HasKey(key);
        }

        MHO_JSONWrapper& operator=(const MHO_JSONWrapper& rhs)
        {
            if(this != &rhs){fObject = rhs.fObject;}
            return *this;
        }

        //start of multi-type map interface 
        std::size_t MapSize() const 
        {
            if(fObject != nullptr){return fObject->size();}
            return 0;
        }
        
        void Clear()
        {
            if(fObject != nullptr){fObject->clear();};
        }

        template< typename XValueType> 
        void Insert(const std::string& key, const XValueType& value)
        {
            //allow replacement of values
            if(fObject != nullptr){(*fObject)[key] = value;}
        }

        template< typename XValueType> 
        bool Retrieve(const std::string& key, XValueType& value) const
        {
            if(fObject == nullptr){return false;}
            auto iter = fObject->find(key);
            if(iter == fObject->end()){return false;}
            else
            {
                mho_json test;
                test["test"] = value;
                //TODO FIXME - this is a major KLUDGE 
                //but needed to avoid exceptions when key is present, but value type is different
                if(test["test"].type() == (*fObject)[key].type()) 
                {
                    value = (*fObject)[key].get<XValueType>();
                    return true;
                }
                else 
                {
                    return false;
                }
            }
        }

        std::vector<std::string> DumpKeys() const
        {
            std::vector< std::string > keys;
            if(fObject != nullptr)
            {
                for(auto iter = fObject->begin(); iter != fObject->end(); iter++)
                {
                    keys.push_back(iter.key());
                }
            }
            return keys;
        }

        //TODO eliminate me
        void DumpMap() const
        {
            if(fObject != nullptr)
            {
                for(auto iter = fObject->begin(); iter != fObject->end(); iter++)
                {
                    std::cout<<iter.key()<<" : "<<iter.value()<<std::endl;
                }
            }
        }

        bool ContainsKey(const std::string& key) const
        {
            if(fObject == nullptr){return false;}
            auto iter = fObject->find(key);
            if(iter == fObject->end()){return false;}
            else{return true;}
        }
};


}//end namespace

#endif /* end of include guard: JSON_WRAPPER */
