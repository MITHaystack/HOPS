#ifndef JSON_WRAPPER_HH__
#define JSON_WRAPPER_HH__

/*!
*@file MHO_JSONHeaderWrapper.hh
*@class MHO_JSONHeaderWrapper
*@date
*@brief
*@author J. Barrett - barrettj@mit.edu
*/

#include "nlohmann/json.hpp"
using mho_json = nlohmann::json;
using mho_ordered_json = nlohmann::ordered_json;
namespace nl=nlohmann;

#include <sstream>
#include <string>
#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{


//constructor is protected
//this class is only intended to provide an interface that derived classes may inherit
class MHO_JSONWrapper
{
    protected:

        MHO_JSONWrapper(){};
        MHO_JSONWrapper(const MHO_JSONWrapper& copy)
        {
            fObject = copy.fObject;
        };

        void SetObject(mho_json obj){fObject = obj;}

    protected:

        mho_json fObject;

    public:

        virtual ~MHO_JSONWrapper(){};

        bool HasKey(const std::string& key) const
        {
            auto it = fObject.find(key);
            if(it != fObject.end()){return true;}
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

        void Clear()
        {
            fObject.clear();
        }

        template< typename XValueType>
        void Insert(const std::string& key, const XValueType& value)
        {
            //allow replacement of values
            fObject[key] = value;
        }

        template< typename XValueType>
        bool Retrieve(const std::string& key, XValueType& value) const
        {
            auto iter = fObject.find(key);
            if(iter == fObject.end()){return false;}
            mho_json test;
            test["test"] = value;
            if( iter.value().type() ==  test.begin().value().type() )
            {
                value = iter.value().get<XValueType>();
                return true;
            }
            else{return false;}
        }

        std::vector<std::string> DumpKeys() const
        {
            std::vector< std::string > keys;
            for(auto iter = fObject.begin(); iter != fObject.end(); iter++)
            {
                keys.push_back(iter.key());
            }
            return keys;
        }

        //TODO eliminate me
        void DumpMap() const
        {
            for(auto iter = fObject.begin(); iter != fObject.end(); iter++)
            {
                std::cout<<iter.key()<<" : "<<iter.value()<<std::endl;
            }
        }

        bool ContainsKey(const std::string& key) const
        {
            auto iter = fObject.find(key);
            if(iter == fObject.end()){return false;}
            else{return true;}
        }
};

}//end namespace

#endif /*! end of include guard: JSON_WRAPPER */
